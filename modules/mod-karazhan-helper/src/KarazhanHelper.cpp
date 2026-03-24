#include "Config.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "StringFormat.h"

#include "thirdparty/httplib.h"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace
{
    struct ItemIndexEntry
    {
        uint32 itemId = 0;
        std::string name;
        std::string normalized;
    };

    struct ItemFacts
    {
        uint32 itemId = 0;
        std::string name;
        uint32 soldByCount = 0;
        uint32 droppedByCount = 0;
        std::vector<std::string> vendorNames;
        std::vector<std::string> dropperNames;
        std::string vendorSummary;
        std::string dropSummary;
    };

    struct CandidateFacts
    {
        ItemIndexEntry const* item = nullptr;
        uint32 score = 0;
        ItemFacts facts;
    };

    struct HelperAnswer
    {
        uint32 matchedItemId = 0;
        std::string resolvedName;
        std::string answer;
        std::string source = "fallback";
    };

    class HelperStore
    {
    public:
        static HelperStore& Instance()
        {
            static HelperStore instance;
            return instance;
        }

        void Load()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _enabled =
                sConfigMgr->GetOption<bool>("KarazhanHelper.Enable", true);
            _llmEnabled =
                sConfigMgr->GetOption<bool>("KarazhanHelper.LLM.Enable", true);
            _requireQuestionHint = sConfigMgr->GetOption<bool>(
                "KarazhanHelper.RequireQuestionHint", true);
            _minQueryLength =
                sConfigMgr->GetOption<uint32>("KarazhanHelper.MinQueryLength",
                                              4);
            _maxCandidates =
                sConfigMgr->GetOption<uint32>("KarazhanHelper.MaxCandidates",
                                              8);
            _llmUrl = sConfigMgr->GetOption<std::string>(
                "KarazhanHelper.LLM.Url", "http://127.0.0.1:8000");
            _llmTimeoutMs = sConfigMgr->GetOption<uint32>(
                "KarazhanHelper.LLM.TimeoutMs", 4000);
            _allowSay = sConfigMgr->GetOption<bool>(
                "KarazhanHelper.Channel.Say", true);
            _allowParty = sConfigMgr->GetOption<bool>(
                "KarazhanHelper.Channel.Party", true);
            _allowRaid = sConfigMgr->GetOption<bool>(
                "KarazhanHelper.Channel.Raid", true);
            _allowGuild = sConfigMgr->GetOption<bool>(
                "KarazhanHelper.Channel.Guild", true);
            _allowWhisper = sConfigMgr->GetOption<bool>(
                "KarazhanHelper.Channel.Whisper", true);

            LoadItemIndex();
        }

        bool IsEnabled() const { return _enabled; }

        bool IsChannelAllowed(uint32 type) const
        {
            switch (type)
            {
            case CHAT_MSG_SAY:
                return _allowSay;
            case CHAT_MSG_PARTY:
            case CHAT_MSG_PARTY_LEADER:
                return _allowParty;
            case CHAT_MSG_RAID:
            case CHAT_MSG_RAID_LEADER:
            case CHAT_MSG_RAID_WARNING:
                return _allowRaid;
            case CHAT_MSG_GUILD:
            case CHAT_MSG_OFFICER:
                return _allowGuild;
            case CHAT_MSG_WHISPER:
                return _allowWhisper;
            default:
                return false;
            }
        }

        bool ShouldHandle(std::string const& message) const
        {
            if (message.size() < _minQueryLength)
                return false;

            if (!_requireQuestionHint)
                return true;

            static std::vector<std::string> const hints = {
                "어디", "어디서", "드랍", "드롭", "구매", "판매", "파는",
                "파나요", "팝니까", "획득", "얻", "보스", "누구", "상인",
                "드랍처", "판매처", "어디에", "가능해", "가능합니까", "?"};

            for (std::string const& hint : hints)
            {
                if (message.find(hint) != std::string::npos)
                    return true;
            }

            return false;
        }

        HelperAnswer BuildAnswer(std::string const& query)
        {
            std::vector<CandidateFacts> candidates = GatherCandidates(query);
            if (candidates.empty())
            {
                return {0, "", "현재 서버 기준으로 관련 아이템을 찾지 못했습니다.",
                        "fallback"};
            }

            if (_llmEnabled)
            {
                HelperAnswer llmAnswer = TryResolveWithLlm(query, candidates);
                if (!llmAnswer.answer.empty())
                    return llmAnswer;
            }

            CandidateFacts const& best = candidates.front();
            return BuildFallbackAnswer(best);
        }

    private:
        static std::string Normalize(std::string text)
        {
            std::string normalized;
            normalized.reserve(text.size());

            for (unsigned char ch : text)
            {
                if (ch < 0x80)
                {
                    if (std::isalnum(ch))
                        normalized.push_back(
                            char(std::tolower(static_cast<unsigned char>(ch))));
                    continue;
                }

                normalized.push_back(char(ch));
            }

            return normalized;
        }

        static void ReplaceAll(std::string& text,
                               std::string const& from,
                               std::string const& to)
        {
            if (from.empty())
                return;

            std::size_t pos = 0;
            while ((pos = text.find(from, pos)) != std::string::npos)
            {
                text.replace(pos, from.length(), to);
                pos += to.length();
            }
        }

        static std::string CollapseSpaces(std::string text)
        {
            std::ostringstream out;
            bool lastWasSpace = false;
            for (char ch : text)
            {
                bool isSpace = std::isspace(static_cast<unsigned char>(ch)) != 0;
                if (isSpace)
                {
                    if (!lastWasSpace)
                        out << ' ';
                }
                else
                    out << ch;

                lastWasSpace = isSpace;
            }

            std::string collapsed = out.str();
            if (!collapsed.empty() && collapsed.front() == ' ')
                collapsed.erase(collapsed.begin());
            if (!collapsed.empty() && collapsed.back() == ' ')
                collapsed.pop_back();
            return collapsed;
        }

        static std::string ExtractLikelySubject(std::string query)
        {
            static std::vector<std::string> const removals = {
                "아이템",   "장비",    "어디서",    "어디",      "구매",
                "판매",     "판매처",  "드랍처",    "드랍",      "드롭",
                "획득",     "얻어",    "얻나요",    "얻지",      "누가",
                "누구",     "보스",    "상인",      "파나요",    "팝니까",
                "가능해",   "가능한가", "가능합니까", "가능",      "해요",
                "인가요",   "입니까",  "알려줘",    "알려주세요", "주세요"};

            ReplaceAll(query, "?", " ");
            ReplaceAll(query, "!", " ");
            ReplaceAll(query, ",", " ");
            ReplaceAll(query, ".", " ");

            for (std::string const& token : removals)
                ReplaceAll(query, token, " ");

            query = CollapseSpaces(query);
            return query.empty() ? CollapseSpaces(query) : query;
        }

        static uint32 LevenshteinDistance(std::string const& a,
                                          std::string const& b)
        {
            if (a.empty())
                return uint32(b.size());
            if (b.empty())
                return uint32(a.size());

            std::vector<uint32> costs(b.size() + 1);
            for (std::size_t j = 0; j <= b.size(); ++j)
                costs[j] = uint32(j);

            for (std::size_t i = 1; i <= a.size(); ++i)
            {
                costs[0] = uint32(i);
                uint32 corner = uint32(i - 1);
                for (std::size_t j = 1; j <= b.size(); ++j)
                {
                    uint32 upper = costs[j];
                    uint32 cost = a[i - 1] == b[j - 1] ? 0u : 1u;
                    costs[j] =
                        std::min({costs[j] + 1, costs[j - 1] + 1,
                                  corner + cost});
                    corner = upper;
                }
            }

            return costs.back();
        }

        static uint32 ScoreCandidate(std::string const& subject,
                                     ItemIndexEntry const& item)
        {
            if (subject.empty() || item.normalized.empty())
                return 0;

            if (subject == item.normalized)
                return 1000;

            uint32 score = 0;
            if (item.normalized.find(subject) != std::string::npos)
                score += 850;
            if (subject.find(item.normalized) != std::string::npos)
                score += 600;

            uint32 distance = LevenshteinDistance(subject, item.normalized);
            uint32 maxLen =
                uint32(std::max(subject.size(), item.normalized.size()));
            uint32 similarity =
                maxLen > distance ? (maxLen - distance) * 20 : 0;

            score += similarity;
            if (item.name.find(subject) != std::string::npos)
                score += 120;

            return score;
        }

        static bool ParseHttpUrl(std::string const& url,
                                 std::string& host,
                                 std::string& path)
        {
            static std::regex const urlRegex(
                "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");
            std::smatch matches;

            if (!std::regex_search(url, matches, urlRegex))
                return false;

            std::string scheme = matches[2].str();
            host = matches[4].str();
            path = matches[5].str();
            if (path.empty())
                path = "/";

            if (!matches[7].str().empty())
                path += "?" + matches[7].str();

            return scheme == "http" && !host.empty();
        }

        static bool TryExtractUInt(std::string const& body,
                                   char const* key,
                                   uint32& value)
        {
            std::regex pattern(
                Acore::StringFormat("\\\"{}\\\"\\s*:\\s*(\\d+)", key));
            std::smatch match;

            if (!std::regex_search(body, match, pattern))
                return false;

            value = uint32(std::stoul(match[1].str()));
            return true;
        }

        static bool TryExtractString(std::string const& body,
                                     char const* key,
                                     std::string& value)
        {
            std::regex pattern(Acore::StringFormat(
                "\\\"{}\\\"\\s*:\\s*\\\"((?:\\\\.|[^\\\"])*)\\\"", key));
            std::smatch match;

            if (!std::regex_search(body, match, pattern))
                return false;

            value = match[1].str();
            value = std::regex_replace(value, std::regex("\\\\n"), "\n");
            value = std::regex_replace(value, std::regex("\\\\r"), "\r");
            value = std::regex_replace(value, std::regex("\\\\t"), "\t");
            value = std::regex_replace(value, std::regex("\\\\\""), "\"");
            value = std::regex_replace(value, std::regex("\\\\\\\\"), "\\");
            return true;
        }

        static std::string EscapeJson(std::string value)
        {
            value = std::regex_replace(value, std::regex("\\\\"), "\\\\");
            value = std::regex_replace(value, std::regex("\""), "\\\"");
            value = std::regex_replace(value, std::regex("\r"), "\\r");
            value = std::regex_replace(value, std::regex("\n"), "\\n");
            value = std::regex_replace(value, std::regex("\t"), "\\t");
            return value;
        }

        static std::string JoinNames(std::vector<std::string> const& names)
        {
            if (names.empty())
                return "";

            std::ostringstream out;
            for (std::size_t i = 0; i < names.size(); ++i)
            {
                if (i)
                    out << ", ";
                out << names[i];
            }
            return out.str();
        }

        void LoadItemIndex()
        {
            _itemIndex.clear();
            QueryResult result = WorldDatabase.Query(
                "SELECT entry, name FROM item_template WHERE name <> ''");
            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();
                ItemIndexEntry entry;
                entry.itemId = fields[0].Get<uint32>();
                entry.name = fields[1].Get<std::string>();
                entry.normalized = Normalize(entry.name);
                if (!entry.normalized.empty())
                    _itemIndex.push_back(std::move(entry));
            } while (result->NextRow());

            LOG_INFO("module.karazhan_helper",
                     "Loaded {} item names into Karazhan helper index",
                     _itemIndex.size());
        }

        ItemFacts QueryFacts(ItemIndexEntry const& item) const
        {
            ItemFacts facts;
            facts.itemId = item.itemId;
            facts.name = item.name;

            {
                QueryResult countResult = WorldDatabase.Query(Acore::StringFormat(
                    "SELECT COUNT(DISTINCT entry) FROM npc_vendor WHERE item = {}",
                    item.itemId));
                if (countResult)
                    facts.soldByCount = countResult->Fetch()[0].Get<uint32>();

                QueryResult namesResult = WorldDatabase.Query(Acore::StringFormat(
                    "SELECT DISTINCT ct.name "
                    "FROM npc_vendor nv "
                    "JOIN creature_template ct ON ct.entry = nv.entry "
                    "WHERE nv.item = {} "
                    "ORDER BY ct.name LIMIT 5",
                    item.itemId));
                if (namesResult)
                {
                    do
                    {
                        facts.vendorNames.push_back(
                            namesResult->Fetch()[0].Get<std::string>());
                    } while (namesResult->NextRow());
                }
            }

            {
                QueryResult countResult = WorldDatabase.Query(Acore::StringFormat(
                    "SELECT COUNT(*) FROM ("
                    "SELECT DISTINCT clt.entry AS source_entry "
                    "FROM creature_loot_template clt "
                    "WHERE clt.item = {0} "
                    "UNION "
                    "SELECT DISTINCT clt.entry AS source_entry "
                    "FROM creature_loot_template clt "
                    "JOIN reference_loot_template rlt "
                    "  ON rlt.entry = ABS(clt.mincountOrRef) "
                    "WHERE clt.mincountOrRef < 0 AND rlt.item = {0}"
                    ") AS helper_sources",
                    item.itemId));
                if (countResult)
                    facts.droppedByCount =
                        countResult->Fetch()[0].Get<uint32>();

                QueryResult namesResult = WorldDatabase.Query(Acore::StringFormat(
                    "SELECT DISTINCT name FROM ("
                    "SELECT ct.name AS name "
                    "FROM creature_loot_template clt "
                    "JOIN creature_template ct ON ct.entry = clt.entry "
                    "WHERE clt.item = {0} "
                    "UNION "
                    "SELECT ct.name AS name "
                    "FROM creature_loot_template clt "
                    "JOIN reference_loot_template rlt "
                    "  ON rlt.entry = ABS(clt.mincountOrRef) "
                    "JOIN creature_template ct ON ct.entry = clt.entry "
                    "WHERE clt.mincountOrRef < 0 AND rlt.item = {0}"
                    ") AS helper_drop_names "
                    "ORDER BY name LIMIT 5",
                    item.itemId));
                if (namesResult)
                {
                    do
                    {
                        facts.dropperNames.push_back(
                            namesResult->Fetch()[0].Get<std::string>());
                    } while (namesResult->NextRow());
                }
            }

            facts.vendorSummary = JoinNames(facts.vendorNames);
            facts.dropSummary = JoinNames(facts.dropperNames);
            return facts;
        }

        std::vector<CandidateFacts> GatherCandidates(std::string const& query) const
        {
            std::string subject = ExtractLikelySubject(query);
            std::string normalizedSubject = Normalize(subject);
            if (normalizedSubject.empty())
                normalizedSubject = Normalize(query);

            std::vector<CandidateFacts> candidates;
            candidates.reserve(_maxCandidates);

            for (ItemIndexEntry const& item : _itemIndex)
            {
                uint32 score = ScoreCandidate(normalizedSubject, item);
                if (score == 0)
                    continue;

                CandidateFacts candidate;
                candidate.item = &item;
                candidate.score = score;
                candidates.push_back(candidate);
            }

            std::sort(candidates.begin(), candidates.end(),
                      [](CandidateFacts const& left,
                         CandidateFacts const& right) {
                          if (left.score != right.score)
                              return left.score > right.score;
                          return left.item->itemId < right.item->itemId;
                      });

            if (candidates.size() > _maxCandidates)
                candidates.resize(_maxCandidates);

            for (CandidateFacts& candidate : candidates)
                candidate.facts = QueryFacts(*candidate.item);

            return candidates;
        }

        HelperAnswer BuildFallbackAnswer(CandidateFacts const& candidate) const
        {
            HelperAnswer answer;
            answer.matchedItemId = candidate.item->itemId;
            answer.resolvedName = candidate.item->name;
            answer.source = "fallback";

            if (candidate.facts.soldByCount > 0)
            {
                answer.answer = Acore::StringFormat(
                    "{}은(는) 판매 NPC가 있습니다. 판매 NPC 예시는 {}입니다.",
                    candidate.item->name,
                    candidate.facts.vendorSummary.empty()
                        ? "서버 DB에서 이름을 찾지 못했습니다"
                        : candidate.facts.vendorSummary);
                return answer;
            }

            if (candidate.facts.droppedByCount > 0)
            {
                answer.answer = Acore::StringFormat(
                    "{}은(는) 판매하지 않으며, 드랍처 예시는 {}입니다.",
                    candidate.item->name,
                    candidate.facts.dropSummary.empty()
                        ? "서버 DB에서 드랍 보스 이름을 찾지 못했습니다"
                        : candidate.facts.dropSummary);
                return answer;
            }

            answer.answer = Acore::StringFormat(
                "{}은(는) 현재 서버 DB 기준으로 판매처와 드랍처를 찾지 "
                "못했습니다.",
                candidate.item->name);
            return answer;
        }

        HelperAnswer TryResolveWithLlm(
            std::string const& query,
            std::vector<CandidateFacts> const& candidates) const
        {
            HelperAnswer fallback = BuildFallbackAnswer(candidates.front());

            std::string host;
            std::string basePath;
            if (!ParseHttpUrl(_llmUrl, host, basePath))
                return fallback;

            std::string path = basePath;
            if (!path.empty() && path.back() == '/')
                path.pop_back();
            path += "/helper/item-query";

            std::ostringstream payload;
            payload << "{";
            payload << "\"query\":\"" << EscapeJson(query) << "\",";
            payload << "\"candidates\":[";

            for (std::size_t i = 0; i < candidates.size(); ++i)
            {
                CandidateFacts const& candidate = candidates[i];
                if (i)
                    payload << ",";

                payload << "{"
                        << "\"item_id\":" << candidate.item->itemId << ","
                        << "\"name\":\"" << EscapeJson(candidate.item->name)
                        << "\","
                        << "\"score\":" << candidate.score << ","
                        << "\"sold_by_count\":"
                        << candidate.facts.soldByCount << ","
                        << "\"dropped_by_count\":"
                        << candidate.facts.droppedByCount << ","
                        << "\"vendor_summary\":\""
                        << EscapeJson(candidate.facts.vendorSummary) << "\","
                        << "\"drop_summary\":\""
                        << EscapeJson(candidate.facts.dropSummary) << "\""
                        << "}";
            }

            payload << "]}";

            uint32 timeoutSec = _llmTimeoutMs / 1000;
            uint32 timeoutUsec = (_llmTimeoutMs % 1000) * 1000;

            httplib::Client cli(host);
            cli.set_connection_timeout(timeoutSec, timeoutUsec);
            cli.set_read_timeout(timeoutSec, timeoutUsec);

            httplib::Headers headers = {
                {"Content-Type", "application/json; charset=utf-8"}};
            httplib::Result result =
                cli.Post(path.c_str(), headers, payload.str(),
                         "application/json; charset=utf-8");

            if (!result || result->status != 200)
                return fallback;

            HelperAnswer answer = fallback;
            answer.source = "llm";

            uint32 matchedItemId = 0;
            std::string resolvedName;
            std::string text;

            TryExtractUInt(result->body, "matched_item_id", matchedItemId);
            TryExtractString(result->body, "resolved_name", resolvedName);
            TryExtractString(result->body, "answer", text);

            if (!text.empty())
                answer.answer = text;
            if (!resolvedName.empty())
                answer.resolvedName = resolvedName;
            if (matchedItemId != 0)
                answer.matchedItemId = matchedItemId;

            return answer;
        }

        bool _enabled = true;
        bool _llmEnabled = true;
        bool _requireQuestionHint = true;
        bool _allowSay = true;
        bool _allowParty = true;
        bool _allowRaid = true;
        bool _allowGuild = true;
        bool _allowWhisper = true;
        uint32 _minQueryLength = 4;
        uint32 _maxCandidates = 8;
        uint32 _llmTimeoutMs = 4000;
        std::string _llmUrl = "http://127.0.0.1:8000";
        std::vector<ItemIndexEntry> _itemIndex;
        mutable std::mutex _mutex;
    };

    class KarazhanHelperWorldScript : public WorldScript
    {
    public:
        KarazhanHelperWorldScript() : WorldScript("KarazhanHelperWorldScript")
        {
        }

        void OnStartup() override { HelperStore::Instance().Load(); }

        void OnAfterConfigLoad(bool /*reload*/) override
        {
            HelperStore::Instance().Load();
        }
    };

    class KarazhanHelperPlayerScript : public PlayerScript
    {
    public:
        KarazhanHelperPlayerScript()
            : PlayerScript("KarazhanHelperPlayerScript")
        {
        }

        void OnPlayerBeforeSendChatMessage(Player* player,
                                           uint32& type,
                                           uint32& lang,
                                           std::string& msg) override
        {
            if (!player || lang == LANG_ADDON)
                return;

            HelperStore& store = HelperStore::Instance();
            if (!store.IsEnabled())
                return;

            if (!store.IsChannelAllowed(type))
                return;

            if (!store.ShouldHandle(msg))
                return;

            HelperAnswer answer = store.BuildAnswer(msg);
            if (answer.answer.empty())
                return;

            player->Whisper(answer.answer, LANG_UNIVERSAL, player);
        }
    };
}

void AddSC_mod_karazhan_helper()
{
    new KarazhanHelperWorldScript();
    new KarazhanHelperPlayerScript();
}
