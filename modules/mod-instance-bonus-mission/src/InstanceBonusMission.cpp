#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Group.h"
#include "Item.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "StringFormat.h"
#include "WorldSession.h"

#include "../../mod-ale/src/LuaEngine/libs/httplib.h"

#include <regex>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace
{
    struct MissionDefinition
    {
        uint32 mapId = 0;
        uint32 missionId = 0;
        uint8 missionType = 0;
        uint32 targetEntry = 0;
        uint32 targetCount = 0;
        uint32 timeLimitSec = 0;
        std::string title;
        std::string targetLabel;
        std::string fallbackAnnouncement;
        uint32 rewardItem = 0;
        uint32 rewardCount = 0;
    };

    struct MissionSelection
    {
        MissionDefinition const* definition = nullptr;
        std::string announcement;
        std::string source = "fallback";
    };

    struct MissionState
    {
        uint32 missionId = 0;
        uint32 mapId = 0;
        uint32 targetEntry = 0;
        uint32 targetCount = 0;
        uint32 currentCount = 0;
        uint32 timeLimitSec = 0;
        uint32 rewardItem = 0;
        uint32 rewardCount = 0;
        std::string title;
        std::string targetLabel;
        std::string announcement;
        std::string source;
        time_t startTime = 0;
        time_t expireTime = 0;
        bool completed = false;
        bool failed = false;
        bool announcedHalf = false;
        bool announcedFive = false;
        bool announcedThree = false;
        bool announcedOne = false;
        bool announcedTenMinute = false;
        bool announcedFiveMinute = false;
        bool announcedThreeMinute = false;
        bool announcedOneMinute = false;
        bool announcedThirtySec = false;
    };

    class MissionStore
    {
    public:
        static MissionStore& Instance()
        {
            static MissionStore instance;
            return instance;
        }

        void Load()
        {
            _enabled = sConfigMgr->GetOption<bool>(
                "InstanceBonusMission.Enable", true);
            _announceEnabled = sConfigMgr->GetOption<bool>(
                "InstanceBonusMission.Announce.Enable", true);
            _llmEnabled = sConfigMgr->GetOption<bool>(
                "InstanceBonusMission.LLM.Enable", false);
            _llmUrl = sConfigMgr->GetOption<std::string>(
                "InstanceBonusMission.LLM.Url",
                "http://127.0.0.1:8000");
            _llmTimeoutMs = sConfigMgr->GetOption<uint32>(
                "InstanceBonusMission.LLM.TimeoutMs", 3000);
            _defaultRewardItem = sConfigMgr->GetOption<uint32>(
                "InstanceBonusMission.Reward.Item", 49426);
            _defaultRewardCount = sConfigMgr->GetOption<uint32>(
                "InstanceBonusMission.Reward.Count", 1);
            _definitions.clear();

            if (!_enabled)
                return;

            QueryResult result = WorldDatabase.Query(
                "SELECT map_id, mission_id, mission_type, target_entry, "
                "target_count, time_limit_sec, title, target_label, "
                "fallback_announcement, reward_item, reward_count "
                "FROM instance_bonus_mission_pool "
                "WHERE enabled = 1 "
                "ORDER BY map_id, mission_id");

            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();

                MissionDefinition definition;
                definition.mapId = fields[0].Get<uint32>();
                definition.missionId = fields[1].Get<uint32>();
                definition.missionType = fields[2].Get<uint8>();
                definition.targetEntry = fields[3].Get<uint32>();
                definition.targetCount = fields[4].Get<uint32>();
                definition.timeLimitSec = fields[5].Get<uint32>();
                definition.title = fields[6].Get<std::string>();
                definition.targetLabel = fields[7].Get<std::string>();
                definition.fallbackAnnouncement =
                    fields[8].Get<std::string>();
                definition.rewardItem = fields[9].Get<uint32>();
                definition.rewardCount = fields[10].Get<uint32>();

                if (definition.targetLabel.empty())
                    definition.targetLabel = definition.title;

                if (!definition.rewardItem)
                    definition.rewardItem = _defaultRewardItem;

                if (!definition.rewardCount)
                    definition.rewardCount = _defaultRewardCount;

                _definitions[definition.mapId].push_back(definition);
            } while (result->NextRow());
        }

        bool IsEnabled() const
        {
            return _enabled;
        }

        bool IsAnnounceEnabled() const
        {
            return _announceEnabled;
        }

        bool IsLlmEnabled() const
        {
            return _llmEnabled;
        }

        std::string const& GetLlmUrl() const
        {
            return _llmUrl;
        }

        uint32 GetLlmTimeoutMs() const
        {
            return _llmTimeoutMs;
        }

        std::vector<MissionDefinition> const* GetByMap(uint32 mapId) const
        {
            auto itr = _definitions.find(mapId);
            if (itr == _definitions.end())
                return nullptr;

            return &itr->second;
        }

    private:
        bool _enabled = true;
        bool _announceEnabled = true;
        bool _llmEnabled = false;
        std::string _llmUrl;
        uint32 _llmTimeoutMs = 3000;
        uint32 _defaultRewardItem = 49426;
        uint32 _defaultRewardCount = 1;
        std::unordered_map<uint32, std::vector<MissionDefinition>>
            _definitions;
    };

    class MissionStateStore
    {
    public:
        static MissionStateStore& Instance()
        {
            static MissionStateStore instance;
            return instance;
        }

        MissionState* Get(uint32 instanceId)
        {
            auto itr = _states.find(instanceId);
            if (itr == _states.end())
                return nullptr;

            return &itr->second;
        }

        MissionState& Create(
            uint32 instanceId, MissionSelection const& selection)
        {
            MissionDefinition const& definition = *selection.definition;
            MissionState& state = _states[instanceId];
            state.missionId = definition.missionId;
            state.mapId = definition.mapId;
            state.targetEntry = definition.targetEntry;
            state.targetCount = definition.targetCount;
            state.currentCount = 0;
            state.timeLimitSec = definition.timeLimitSec;
            state.rewardItem = definition.rewardItem;
            state.rewardCount = definition.rewardCount;
            state.title = definition.title;
            state.targetLabel = definition.targetLabel;
            state.announcement = selection.announcement.empty()
                ? definition.fallbackAnnouncement
                : selection.announcement;
            state.source = selection.source;
            state.startTime = GameTime::GetGameTime().count();
            state.expireTime = state.startTime + state.timeLimitSec;
            state.completed = false;
            state.failed = false;
            state.announcedHalf = false;
            state.announcedFive = false;
            state.announcedThree = false;
            state.announcedOne = false;
            state.announcedTenMinute = false;
            state.announcedFiveMinute = false;
            state.announcedThreeMinute = false;
            state.announcedOneMinute = false;
            state.announcedThirtySec = false;
            return state;
        }

    private:
        std::unordered_map<uint32, MissionState> _states;
    };

    void SendMissionMessageToPlayer(Player* player, std::string const& msg)
    {
        if (!player || !MissionStore::Instance().IsAnnounceEnabled())
            return;

        ChatHandler(player->GetSession()).SendSysMessage(msg.c_str());
    }

    void SendMissionMessageToGroup(Player* player, std::string const& msg)
    {
        if (!player)
            return;

        if (Group* group = player->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr;
                 itr = itr->next())
            {
                if (Player* member = itr->GetSource())
                    SendMissionMessageToPlayer(member, msg);
            }
            return;
        }

        SendMissionMessageToPlayer(player, msg);
    }

    void SendMissionMessageToMap(Map* map, std::string const& msg)
    {
        if (!map)
            return;

        Map::PlayerList const& players = map->GetPlayers();
        for (auto const& ref : players)
        {
            if (Player* player = ref.GetSource())
                SendMissionMessageToPlayer(player, msg);
        }
    }

    std::string EscapeJson(std::string const& text)
    {
        std::ostringstream out;
        for (char c : text)
        {
            switch (c)
            {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << c; break;
            }
        }

        return out.str();
    }

    bool ParseHttpUrl(
        std::string const& url, std::string& host, std::string& path)
    {
        static std::regex const urlRegex(
            "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");
        std::smatch matches;

        if (!std::regex_search(url, matches, urlRegex))
            return false;

        std::string scheme = matches[2].str();
        std::string authority = matches[4].str();
        std::string query = matches[7].str();

        if (scheme.empty() || authority.empty())
            return false;

        host = scheme + "://" + authority;
        path = matches[5].str();
        if (path.empty())
            path = "/";

        if (!query.empty())
            path += "?" + query;

        return true;
    }

    bool TryExtractUInt(
        std::string const& body, char const* key, uint32& value)
    {
        std::regex pattern(
            Acore::StringFormat(
                "\\\"{}\\\"\\s*:\\s*(\\d+)", key));
        std::smatch match;

        if (!std::regex_search(body, match, pattern))
            return false;

        value = uint32(std::stoul(match[1].str()));
        return true;
    }

    bool TryExtractString(
        std::string const& body, char const* key, std::string& value)
    {
        std::regex pattern(
            Acore::StringFormat(
                "\\\"{}\\\"\\s*:\\s*\\\"((?:\\\\.|[^\\\"])*)\\\"",
                key));
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

    MissionSelection SelectMissionFallback(uint32 mapId)
    {
        MissionSelection selection;
        auto definitions = MissionStore::Instance().GetByMap(mapId);
        if (!definitions || definitions->empty())
            return selection;

        if (definitions->size() == 1)
            selection.definition = &definitions->front();
        else
            selection.definition =
                &definitions->at(urand(0, uint32(definitions->size() - 1)));

        selection.announcement = selection.definition->fallbackAnnouncement;
        selection.source = "fallback";
        return selection;
    }

    MissionSelection TrySelectMissionWithLlm(Map* map)
    {
        MissionSelection fallback = SelectMissionFallback(map->GetId());
        if (!fallback.definition)
            return fallback;

        if (!MissionStore::Instance().IsLlmEnabled())
            return fallback;

        auto definitions = MissionStore::Instance().GetByMap(map->GetId());
        if (!definitions || definitions->empty())
            return fallback;

        std::string host;
        std::string path;
        std::string endpoint = MissionStore::Instance().GetLlmUrl() +
            "/mission/select";

        if (!ParseHttpUrl(endpoint, host, path))
        {
            LOG_ERROR(
                "module.instance_bonus_mission",
                "Failed to parse LLM URL: {}",
                endpoint);
            return fallback;
        }

        std::ostringstream json;
        json << "{";
        json << "\"map_id\":" << map->GetId() << ",";
        json << "\"instance_name\":\""
             << EscapeJson(Acore::StringFormat("instance_{}", map->GetId()))
             << "\",";
        json << "\"difficulty\":\"normal\",";
        json << "\"party_size\":" << map->GetPlayersCountExceptGMs()
             << ",";
        json << "\"candidates\":[";

        bool first = true;
        for (MissionDefinition const& definition : *definitions)
        {
            if (!first)
                json << ",";

            first = false;
            json << "{";
            json << "\"mission_id\":" << definition.missionId << ",";
            json << "\"title\":\"" << EscapeJson(definition.title)
                 << "\",";
            json << "\"target_label\":\""
                 << EscapeJson(definition.targetLabel) << "\",";
            json << "\"target_count\":" << definition.targetCount << ",";
            json << "\"time_limit_sec\":" << definition.timeLimitSec;
            json << "}";
        }

        json << "]}";

        try
        {
            httplib::Client cli(host);
            uint32 timeoutMs = MissionStore::Instance().GetLlmTimeoutMs();
            time_t timeoutSec = std::max<time_t>(1, timeoutMs / 1000);
            time_t timeoutUsec = (timeoutMs % 1000) * 1000;
            cli.set_connection_timeout(timeoutSec, timeoutUsec);
            cli.set_read_timeout(timeoutSec, timeoutUsec);
            cli.set_write_timeout(timeoutSec, timeoutUsec);

            httplib::Headers headers = {
                { "Content-Type", "application/json; charset=utf-8" }
            };
            httplib::Result result = cli.Post(
                path.c_str(), headers, json.str(),
                "application/json; charset=utf-8");

            if (!result)
            {
                LOG_ERROR(
                    "module.instance_bonus_mission",
                    "LLM request failed: {}",
                    httplib::to_string(result.error()));
                return fallback;
            }

            if (result->status != 200)
            {
                LOG_ERROR(
                    "module.instance_bonus_mission",
                    "LLM request returned status {}",
                    result->status);
                return fallback;
            }

            uint32 selectedMissionId = 0;
            std::string announcement;
            if (!TryExtractUInt(
                    result->body, "selected_mission_id",
                    selectedMissionId))
            {
                LOG_ERROR(
                    "module.instance_bonus_mission",
                    "LLM response missing selected_mission_id: {}",
                    result->body);
                return fallback;
            }

            TryExtractString(result->body, "announcement", announcement);

            for (MissionDefinition const& definition : *definitions)
            {
                if (definition.missionId != selectedMissionId)
                    continue;

                MissionSelection selection;
                selection.definition = &definition;
                selection.announcement = announcement.empty()
                    ? definition.fallbackAnnouncement
                    : announcement;
                selection.source = "llm";
                return selection;
            }

            LOG_ERROR(
                "module.instance_bonus_mission",
                "LLM selected unknown mission_id {} for map {}",
                selectedMissionId, map->GetId());
            return fallback;
        }
        catch (std::exception const& ex)
        {
            LOG_ERROR(
                "module.instance_bonus_mission",
                "LLM request exception: {}",
                ex.what());
            return fallback;
        }
    }

    void RewardMissionToMap(Map* map, MissionState const& state)
    {
        if (!map || !state.rewardItem || !state.rewardCount)
            return;

        Map::PlayerList const& players = map->GetPlayers();
        for (auto const& ref : players)
        {
            Player* player = ref.GetSource();
            if (!player)
                continue;

            player->AddItem(state.rewardItem, state.rewardCount);
        }
    }

    void UpdateMissionProgress(Player* killer, Creature* killed)
    {
        if (!killer || !killed)
            return;

        Map* map = killer->GetMap();
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
            return;

        MissionState* state = MissionStateStore::Instance().Get(
            map->GetInstanceId());
        if (!state || state->completed || state->failed)
            return;

        if (state->targetEntry != killed->GetEntry())
            return;

        ++state->currentCount;

        uint32 remaining = 0;
        if (state->targetCount > state->currentCount)
            remaining = state->targetCount - state->currentCount;

        if (!state->announcedHalf &&
            state->targetCount > 1 &&
            state->currentCount >= (state->targetCount / 2))
        {
            state->announcedHalf = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 임무] {} 목표를 절반 달성했습니다. {} / {}",
                    state->targetLabel, state->currentCount,
                    state->targetCount));
        }

        if (!state->announcedFive && remaining == 5)
        {
            state->announcedFive = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 임무] {} {}마리 중 {}마리 남았습니다.",
                    state->targetLabel, state->targetCount, remaining));
        }

        if (!state->announcedThree && remaining == 3)
        {
            state->announcedThree = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 임무] {} {}마리 중 {}마리 남았습니다.",
                    state->targetLabel, state->targetCount, remaining));
        }

        if (!state->announcedOne && remaining == 1)
        {
            state->announcedOne = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 임무] {} {}마리 중 {}마리 남았습니다.",
                    state->targetLabel, state->targetCount, remaining));
        }

        if (state->currentCount < state->targetCount)
            return;

        state->completed = true;
        RewardMissionToMap(map, *state);
        SendMissionMessageToMap(
            map,
            Acore::StringFormat(
                "[추가 임무] {} 임무를 완수했습니다. 추가 보상이 지급됩니다.",
                state->title));
    }
}

class InstanceBonusMissionWorldScript : public WorldScript
{
public:
    InstanceBonusMissionWorldScript()
        : WorldScript("InstanceBonusMissionWorldScript")
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        MissionStore::Instance().Load();
    }
};

class InstanceBonusMissionPlayerScript : public PlayerScript
{
public:
    InstanceBonusMissionPlayerScript()
        : PlayerScript("InstanceBonusMissionPlayerScript")
    {
    }

    void OnPlayerMapChanged(Player* player) override
    {
        if (!player || !MissionStore::Instance().IsEnabled())
            return;

        Map* map = player->GetMap();
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
            return;

        uint32 instanceId = map->GetInstanceId();
        if (MissionStateStore::Instance().Get(instanceId))
            return;

        MissionSelection selection = TrySelectMissionWithLlm(map);
        if (!selection.definition)
            return;

        MissionState& state = MissionStateStore::Instance().Create(
            instanceId, selection);

        SendMissionMessageToGroup(
            player,
            Acore::StringFormat("[추가 임무] {}", state.announcement));
    }
};

class InstanceBonusMissionKillScript : public PlayerScript
{
public:
    InstanceBonusMissionKillScript()
        : PlayerScript("InstanceBonusMissionKillScript")
    {
    }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        UpdateMissionProgress(killer, killed);
    }
};

class InstanceBonusMissionMapScript : public AllMapScript
{
public:
    InstanceBonusMissionMapScript()
        : AllMapScript("InstanceBonusMissionMapScript")
    {
    }

    void OnMapUpdate(Map* map, uint32 /*diff*/) override
    {
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
            return;

        MissionState* state = MissionStateStore::Instance().Get(
            map->GetInstanceId());
        if (!state || state->completed || state->failed)
            return;

        time_t now = GameTime::GetGameTime().count();
        if (state->timeLimitSec == 0)
            return;

        if (now >= state->expireTime)
        {
            state->failed = true;
            SendMissionMessageToMap(
                map,
                Acore::StringFormat(
                    "[추가 임무] {} 임무가 시간 초과로 실패했습니다.",
                    state->title));
            return;
        }

        uint32 remaining = uint32(state->expireTime - now);

        if (!state->announcedTenMinute && remaining <= 600 &&
            state->timeLimitSec > 600)
        {
            state->announcedTenMinute = true;
            SendMissionMessageToMap(
                map, "[추가 임무] 시간이 10분 남았습니다.");
        }

        if (!state->announcedFiveMinute && remaining <= 300)
        {
            state->announcedFiveMinute = true;
            SendMissionMessageToMap(
                map, "[추가 임무] 시간이 5분 남았습니다.");
        }

        if (!state->announcedThreeMinute && remaining <= 180)
        {
            state->announcedThreeMinute = true;
            SendMissionMessageToMap(
                map, "[추가 임무] 시간이 3분 남았습니다.");
        }

        if (!state->announcedOneMinute && remaining <= 60)
        {
            state->announcedOneMinute = true;
            SendMissionMessageToMap(
                map, "[추가 임무] 시간이 1분 남았습니다.");
        }

        if (!state->announcedThirtySec && remaining <= 30)
        {
            state->announcedThirtySec = true;
            SendMissionMessageToMap(
                map, "[추가 임무] 시간이 30초 남았습니다.");
        }
    }
};

void AddInstanceBonusMissionScripts()
{
    new InstanceBonusMissionWorldScript();
    new InstanceBonusMissionPlayerScript();
    new InstanceBonusMissionKillScript();
    new InstanceBonusMissionMapScript();
}
