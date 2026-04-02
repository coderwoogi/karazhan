
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
#include "SharedDefines.h"
#include "StringFormat.h"
#include "WorldSession.h"

#include "thirdparty/httplib.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    std::string const kBriefingPrefix = "[작전 브리핑] ";
    std::string const kMissionPrefix = "[추가 미션] ";

    struct MissionDefinition
    {
        uint32 mapId = 0;
        uint32 missionId = 0;
        uint8 missionType = 0;
        uint32 difficultyMask = 0;
        uint32 targetEntry = 0;
        uint32 targetCount = 0;
        uint32 timeLimitSec = 0;
        uint32 rewardProfileId = 0;
        std::string title;
        std::string description;
        std::string briefingText;
        std::string targetLabel;
        std::string fallbackAnnouncement;
        uint32 rewardItem = 0;
        uint32 rewardCount = 0;
    };

    struct MapConfigDefinition
    {
        uint32 mapId = 0;
        std::string mapName;
        uint32 difficultyMask = 0;
        bool enabled = true;
        bool allowVote = true;
        bool allowLlm = true;
        uint32 dailyLimitPerPlayer = 0;
        uint32 defaultTimeLimitSec = 0;
        uint8 minPartySize = 1;
        uint8 maxPartySize = 5;
        uint8 maxConcurrentMissions = 1;
        std::string notes;
    };

    struct RewardProfileItem
    {
        uint32 itemEntry = 0;
        uint32 itemCount = 0;
        std::string grade;
    };

    struct ThemeDefinition
    {
        uint32 mapId = 0;
        uint32 themeId = 0;
        std::string themeKey;
        std::string name;
        std::string description;
        uint32 difficultyMask = 0;
        uint8 minPartySize = 1;
        uint8 maxPartySize = 5;
        uint32 minAvgItemLevel = 0;
        uint32 maxAvgItemLevel = 9999;
        bool requiredTank = false;
        bool requiredHealer = false;
        uint32 weight = 100;
    };

    struct PartyContext
    {
        uint32 partySize = 0;
        uint32 averageItemLevel = 0;
        bool hasTank = false;
        bool hasHealer = false;
        uint32 likelyTankCount = 0;
        uint32 likelyHealerCount = 0;
        uint32 difficultyMask = 0;
        std::string difficulty = "normal";
        std::unordered_map<uint8, uint32> classCounts;
    };

    enum MissionDifficultyMask : uint32
    {
        DIFFICULTY_MASK_DUNGEON_NORMAL = 1 << 0,
        DIFFICULTY_MASK_DUNGEON_HEROIC = 1 << 1,
        DIFFICULTY_MASK_RAID_10_NORMAL = 1 << 2,
        DIFFICULTY_MASK_RAID_10_HEROIC = 1 << 3,
        DIFFICULTY_MASK_RAID_25_NORMAL = 1 << 4,
        DIFFICULTY_MASK_RAID_25_HEROIC = 1 << 5
    };

    struct MissionSelection
    {
        MissionDefinition const* definition = nullptr;
        std::string announcement;
        std::string source = "fallback";
    };

    struct MissionState
    {
        uint64 runId = 0;
        uint32 missionId = 0;
        uint32 mapId = 0;
        uint32 themeId = 0;
        uint8 missionType = 0;
        uint32 targetEntry = 0;
        uint32 targetCount = 0;
        uint32 currentCount = 0;
        uint32 timeLimitSec = 0;
        uint32 rewardProfileId = 0;
        uint32 rewardItem = 0;
        uint32 rewardCount = 0;
        uint32 difficultyMask = 0;
        std::string title;
        std::string description;
        std::string targetLabel;
        std::string themeKey;
        std::string themeName;
        std::string briefing;
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
        bool voteApproved = false;
        std::unordered_map<uint32, bool> votes;
    };

    struct VoteStatus
    {
        uint32 eligible = 0;
        uint32 required = 1;
        uint32 yes = 0;
        uint32 no = 0;
        std::string state = "pending";
    };

    char const* GetClassToken(uint8 classId)
    {
        switch (classId)
        {
        case CLASS_WARRIOR:
            return "warrior";
        case CLASS_PALADIN:
            return "paladin";
        case CLASS_HUNTER:
            return "hunter";
        case CLASS_ROGUE:
            return "rogue";
        case CLASS_PRIEST:
            return "priest";
        case CLASS_DEATH_KNIGHT:
            return "death_knight";
        case CLASS_SHAMAN:
            return "shaman";
        case CLASS_MAGE:
            return "mage";
        case CLASS_WARLOCK:
            return "warlock";
        case CLASS_DRUID:
            return "druid";
        default:
            return "unknown";
        }
    }

    bool IsLikelyTankClass(uint8 classId)
    {
        switch (classId)
        {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
        case CLASS_DEATH_KNIGHT:
        case CLASS_DRUID:
            return true;
        default:
            return false;
        }
    }

    bool IsLikelyHealerClass(uint8 classId)
    {
        switch (classId)
        {
        case CLASS_PALADIN:
        case CLASS_PRIEST:
        case CLASS_SHAMAN:
        case CLASS_DRUID:
            return true;
        default:
            return false;
        }
    }

    uint32 GetDifficultyMaskForMap(Map* map)
    {
        if (!map)
            return 0;

        if (!map->IsRaid())
            return map->IsHeroic()
                ? DIFFICULTY_MASK_DUNGEON_HEROIC
                : DIFFICULTY_MASK_DUNGEON_NORMAL;

        if (map->Is25ManRaid())
            return map->IsHeroic()
                ? DIFFICULTY_MASK_RAID_25_HEROIC
                : DIFFICULTY_MASK_RAID_25_NORMAL;

        return map->IsHeroic()
            ? DIFFICULTY_MASK_RAID_10_HEROIC
            : DIFFICULTY_MASK_RAID_10_NORMAL;
    }

    std::string GetDifficultyTokenForMap(Map* map)
    {
        if (!map)
            return "unknown";

        if (!map->IsRaid())
            return map->IsHeroic()
                ? "dungeon_heroic"
                : "dungeon_normal";

        if (map->Is25ManRaid())
            return map->IsHeroic()
                ? "raid_25_heroic"
                : "raid_25_normal";

        return map->IsHeroic()
            ? "raid_10_heroic"
            : "raid_10_normal";
    }

    bool IsDifficultyAllowed(uint32 allowedMask, uint32 currentMask)
    {
        if (!allowedMask || !currentMask)
            return true;

        return (allowedMask & currentMask) != 0;
    }

    uint64 GenerateRunId(uint32 instanceId)
    {
        uint64 timePart = uint64(GameTime::GetGameTime().count()) & 0xFFFFFFFF;
        uint64 saltPart = uint64(urand(1, 0xFFFF));
        return (uint64(instanceId) << 32) | ((timePart ^ saltPart) & 0xFFFFFFFF);
    }

    std::string NormalizeDbToken(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char ch) { return char(std::tolower(ch)); });
        return value;
    }

    uint8 ParseMissionType(std::string value)
    {
        value = NormalizeDbToken(value);
        if (value == "1" || value == "kill" || value == "general")
            return 1;
        if (value == "2" || value == "survival" || value == "clean_run")
            return 2;
        if (value == "3" || value == "speed")
            return 3;
        if (value == "4" || value == "boss")
            return 4;
        return 1;
    }

    std::string EscapeSql(std::string text)
    {
        WorldDatabase.EscapeString(text);
        return text;
    }

    bool TableExists(char const* tableName)
    {
        QueryResult result = WorldDatabase.Query(Acore::StringFormat(
            "SELECT 1 "
            "FROM INFORMATION_SCHEMA.TABLES "
            "WHERE TABLE_SCHEMA = DATABASE() "
            "  AND TABLE_NAME = '{}' "
            "LIMIT 1",
            tableName));
        return result != nullptr;
    }

    bool ColumnExists(char const* tableName, char const* columnName)
    {
        QueryResult result = WorldDatabase.Query(Acore::StringFormat(
            "SELECT 1 "
            "FROM INFORMATION_SCHEMA.COLUMNS "
            "WHERE TABLE_SCHEMA = DATABASE() "
            "  AND TABLE_NAME = '{}' "
            "  AND COLUMN_NAME = '{}' "
            "LIMIT 1",
            tableName, columnName));
        return result != nullptr;
    }

    std::string GetColumnDataType(char const* tableName, char const* columnName)
    {
        QueryResult result = WorldDatabase.Query(Acore::StringFormat(
            "SELECT LOWER(DATA_TYPE) "
            "FROM INFORMATION_SCHEMA.COLUMNS "
            "WHERE TABLE_SCHEMA = DATABASE() "
            "  AND TABLE_NAME = '{}' "
            "  AND COLUMN_NAME = '{}' "
            "LIMIT 1",
            tableName, columnName));
        if (!result)
            return {};

        return result->Fetch()[0].Get<std::string>();
    }

    std::string MakeTimeValueExpr(
        char const* tableName,
        char const* columnName,
        time_t value)
    {
        std::string type = GetColumnDataType(tableName, columnName);
        if (type == "datetime" || type == "timestamp" || type == "date")
            return Acore::StringFormat("FROM_UNIXTIME({})", uint64(value));

        return Acore::StringFormat("{}", uint64(value));
    }

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
            _mapConfigs.clear();
            _themes.clear();
            _themeMissionIds.clear();

            if (!_enabled)
                return;

            LoadMapConfigs();
            LoadMissions();
            LOG_INFO(
                "module.instance_bonus_mission",
                "InstanceBonusMission loaded: mapConfigs={}, missionMaps={}",
                _mapConfigs.size(),
                _definitions.size());
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

        MapConfigDefinition GetMapConfig(uint32 mapId) const
        {
            auto itr = _mapConfigs.find(mapId);
            if (itr == _mapConfigs.end())
                return {};

            return itr->second;
        }

        std::vector<ThemeDefinition> const* GetThemesByMap(uint32 mapId) const
        {
            auto itr = _themes.find(mapId);
            if (itr == _themes.end())
                return nullptr;

            return &itr->second;
        }

        std::vector<uint32> const* GetMissionIdsForTheme(
            uint32 mapId, uint32 themeId) const
        {
            auto mapItr = _themeMissionIds.find(mapId);
            if (mapItr == _themeMissionIds.end())
                return nullptr;

            auto themeItr = mapItr->second.find(themeId);
            if (themeItr == mapItr->second.end())
                return nullptr;

            return &themeItr->second;
        }

    private:
        void LoadMapConfigs()
        {
            if (!TableExists("instance_bonus_map_config"))
                return;

            std::string maxConcurrentExpr = "1";
            if (ColumnExists(
                    "instance_bonus_map_config",
                    "max_concurrent_missions"))
            {
                maxConcurrentExpr =
                    "IFNULL(max_concurrent_missions, 1)";
            }
            else if (ColumnExists(
                         "instance_bonus_map_config",
                         "max_active_missions"))
            {
                maxConcurrentExpr =
                    "IFNULL(max_active_missions, 1)";
            }

            std::string difficultyExpr = "0";
            if (ColumnExists("instance_bonus_map_config", "difficulty_mask"))
                difficultyExpr = "IFNULL(difficulty_mask, 0)";

            std::string notesExpr = "''";
            if (ColumnExists("instance_bonus_map_config", "notes"))
                notesExpr = "IFNULL(notes, '')";

            QueryResult result = WorldDatabase.Query(Acore::StringFormat(
                "SELECT map_id, "
                "IFNULL(map_name, ''), "
                "{}, "
                "IFNULL(enabled, 1), "
                "IFNULL(allow_vote, 1), "
                "IFNULL(allow_llm, 1), "
                "IFNULL(daily_limit_per_player, 0), "
                "IFNULL(default_time_limit_sec, 0), "
                "IFNULL(min_party_size, 1), "
                "IFNULL(max_party_size, 5), "
                "{}, "
                "{} "
                "FROM instance_bonus_map_config",
                difficultyExpr,
                maxConcurrentExpr,
                notesExpr));

            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();

                MapConfigDefinition definition;
                definition.mapId = fields[0].Get<uint32>();
                definition.mapName = fields[1].Get<std::string>();
                definition.difficultyMask = fields[2].Get<uint32>();
                definition.enabled = fields[3].Get<uint8>() != 0;
                definition.allowVote = fields[4].Get<uint8>() != 0;
                definition.allowLlm = fields[5].Get<uint8>() != 0;
                definition.dailyLimitPerPlayer = fields[6].Get<uint32>();
                definition.defaultTimeLimitSec = fields[7].Get<uint32>();
                definition.minPartySize = fields[8].Get<uint8>();
                definition.maxPartySize = fields[9].Get<uint8>();
                definition.maxConcurrentMissions = fields[10].Get<uint8>();
                definition.notes = fields[11].Get<std::string>();
                _mapConfigs[definition.mapId] = definition;
            } while (result->NextRow());
        }

        void LoadMissions()
        {
            bool loadedV2 = false;

            if (TableExists("instance_bonus_mission"))
            {
                QueryResult result = WorldDatabase.Query(
                    "SELECT mission_id, map_id, "
                    "CAST(mission_type AS CHAR), "
                    "difficulty_mask, "
                    "target_entry, target_count, time_limit_sec, "
                    "IFNULL(name, ''), "
                    "IFNULL(description, ''), "
                    "IFNULL(briefing_text, ''), "
                    "IFNULL(target_label, ''), "
                    "reward_profile_id "
                    "FROM instance_bonus_mission "
                    "WHERE enabled = 1 "
                    "ORDER BY map_id, mission_id");

                if (result)
                {
                    do
                    {
                        Field* fields = result->Fetch();

                        MissionDefinition definition;
                        definition.missionId = fields[0].Get<uint32>();
                        definition.mapId = fields[1].Get<uint32>();
                        definition.missionType = ParseMissionType(
                            fields[2].Get<std::string>());
                        definition.difficultyMask = fields[3].Get<uint32>();
                        definition.targetEntry = fields[4].Get<uint32>();
                        definition.targetCount = fields[5].Get<uint32>();
                        definition.timeLimitSec = fields[6].Get<uint32>();
                        definition.title = fields[7].Get<std::string>();
                        definition.description = fields[8].Get<std::string>();
                        definition.briefingText = fields[9].Get<std::string>();
                        definition.targetLabel = fields[10].Get<std::string>();
                        definition.rewardProfileId = fields[11].Get<uint32>();

                        if (definition.targetLabel.empty())
                            definition.targetLabel = definition.title;

                        if (definition.briefingText.empty())
                            definition.briefingText = definition.description;

                        definition.fallbackAnnouncement =
                            definition.briefingText.empty()
                                ? definition.title
                                : definition.briefingText;
                        definition.rewardItem = _defaultRewardItem;
                        definition.rewardCount = _defaultRewardCount;

                        _definitions[definition.mapId].push_back(definition);
                    } while (result->NextRow());

                    loadedV2 = !_definitions.empty();
                }
            }

            if (loadedV2)
                return;

            QueryResult result = WorldDatabase.Query(
                "SELECT map_id, mission_id, mission_type, difficulty_mask, "
                "target_entry, target_count, time_limit_sec, title, "
                "target_label, fallback_announcement, reward_item, "
                "reward_count "
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
                definition.difficultyMask = fields[3].Get<uint32>();
                definition.targetEntry = fields[4].Get<uint32>();
                definition.targetCount = fields[5].Get<uint32>();
                definition.timeLimitSec = fields[6].Get<uint32>();
                definition.title = fields[7].Get<std::string>();
                definition.targetLabel = fields[8].Get<std::string>();
                definition.fallbackAnnouncement =
                    fields[9].Get<std::string>();
                definition.rewardItem = fields[10].Get<uint32>();
                definition.rewardCount = fields[11].Get<uint32>();

                if (definition.targetLabel.empty())
                    definition.targetLabel = definition.title;

                if (!definition.rewardItem)
                    definition.rewardItem = _defaultRewardItem;

                if (!definition.rewardCount)
                    definition.rewardCount = _defaultRewardCount;

                _definitions[definition.mapId].push_back(definition);
            } while (result->NextRow());
        }

        void LoadThemes()
        {
            QueryResult result = WorldDatabase.Query(
                "SELECT map_id, theme_id, theme_key, name, description, "
                "difficulty_mask, min_party_size, max_party_size, "
                "min_avg_item_level, max_avg_item_level, required_tank, "
                "required_healer, weight "
                "FROM instance_bonus_theme_pool "
                "WHERE enabled = 1 "
                "ORDER BY map_id, theme_id");

            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();

                ThemeDefinition definition;
                definition.mapId = fields[0].Get<uint32>();
                definition.themeId = fields[1].Get<uint32>();
                definition.themeKey = fields[2].Get<std::string>();
                definition.name = fields[3].Get<std::string>();
                definition.description = fields[4].Get<std::string>();
                definition.difficultyMask = fields[5].Get<uint32>();
                definition.minPartySize = fields[6].Get<uint8>();
                definition.maxPartySize = fields[7].Get<uint8>();
                definition.minAvgItemLevel = fields[8].Get<uint32>();
                definition.maxAvgItemLevel = fields[9].Get<uint32>();
                definition.requiredTank = fields[10].Get<uint8>() != 0;
                definition.requiredHealer = fields[11].Get<uint8>() != 0;
                definition.weight = fields[12].Get<uint32>();

                _themes[definition.mapId].push_back(definition);
            } while (result->NextRow());
        }

        void LoadThemeMissionLinks()
        {
            QueryResult result = WorldDatabase.Query(
                "SELECT map_id, theme_id, mission_id "
                "FROM instance_bonus_theme_mission "
                "ORDER BY map_id, theme_id, slot, mission_id");

            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();
                uint32 mapId = fields[0].Get<uint32>();
                uint32 themeId = fields[1].Get<uint32>();
                uint32 missionId = fields[2].Get<uint32>();
                _themeMissionIds[mapId][themeId].push_back(missionId);
            } while (result->NextRow());
        }

        bool _enabled = true;
        bool _announceEnabled = true;
        bool _llmEnabled = false;
        std::string _llmUrl;
        uint32 _llmTimeoutMs = 3000;
        uint32 _defaultRewardItem = 49426;
        uint32 _defaultRewardCount = 1;
        std::unordered_map<uint32, MapConfigDefinition> _mapConfigs;
        std::unordered_map<uint32, std::vector<MissionDefinition>>
            _definitions;
        std::unordered_map<uint32, std::vector<ThemeDefinition>> _themes;
        std::unordered_map<
            uint32,
            std::unordered_map<uint32, std::vector<uint32>>>
            _themeMissionIds;
    };

    void SaveLiveState(uint32 instanceId, MissionState const& state);
    void LogVote(MissionState const& state, Player* player, bool agree);
    void LogReward(
        MissionState const& state,
        Player* player,
        uint32 itemEntry,
        uint32 itemCount);

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
            uint32 instanceId,
            MissionSelection const& missionSelection)
        {
            MissionDefinition const& mission = *missionSelection.definition;
            MapConfigDefinition config =
                MissionStore::Instance().GetMapConfig(mission.mapId);
            MissionState& state = _states[instanceId];
            state.runId = GenerateRunId(instanceId);
            state.missionId = mission.missionId;
            state.mapId = mission.mapId;
            state.themeId = 0;
            state.missionType = mission.missionType;
            state.difficultyMask = mission.difficultyMask;
            state.targetEntry = mission.targetEntry;
            state.targetCount = mission.targetCount;
            state.currentCount = 0;
            state.timeLimitSec = mission.timeLimitSec
                ? mission.timeLimitSec
                : config.defaultTimeLimitSec;
            state.rewardProfileId = mission.rewardProfileId;
            state.rewardItem = mission.rewardItem;
            state.rewardCount = mission.rewardCount;
            state.title = mission.title;
            state.description = mission.description;
            state.targetLabel = mission.targetLabel;
            state.themeKey.clear();
            state.themeName.clear();
            state.briefing = mission.briefingText;
            state.announcement = missionSelection.announcement.empty()
                ? mission.fallbackAnnouncement
                : missionSelection.announcement;
            state.source = missionSelection.source;
            state.startTime = 0;
            state.expireTime = 0;
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
            state.voteApproved = false;
            state.votes.clear();
            SaveLiveState(instanceId, state);
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

    std::string SanitizeAddonField(std::string text, std::size_t maxLen)
    {
        for (char& ch : text)
        {
            if (ch == '\t' || ch == '\r' || ch == '\n')
                ch = ' ';
        }

        if (text.length() > maxLen)
            text.resize(maxLen);

        return text;
    }

    uint32 GetRemainingSeconds(MissionState const& state)
    {
        if (!state.voteApproved)
            return state.timeLimitSec;

        if (!state.timeLimitSec || !state.expireTime)
            return 0;

        time_t now = GameTime::GetGameTime().count();
        if (now >= state.expireTime)
            return 0;

        return uint32(state.expireTime - now);
    }

    std::string GetMissionStatusToken(MissionState const& state)
    {
        if (!state.voteApproved)
            return "pending";

        if (state.completed)
            return "complete";

        if (state.failed)
            return "failed";

        return "active";
    }

    uint32 GetVoteKey(Player* player)
    {
        if (!player)
            return 0;

        return uint32(player->GetGUID().GetCounter());
    }

    uint32 GetDailyLimitForMap(uint32 mapId)
    {
        return MissionStore::Instance().GetMapConfig(mapId)
            .dailyLimitPerPlayer;
    }

    MapConfigDefinition GetMapConfigForMap(uint32 mapId)
    {
        return MissionStore::Instance().GetMapConfig(mapId);
    }

    bool IsMapConfigEnabledForContext(
        MapConfigDefinition const& config,
        PartyContext const& context)
    {
        if (!config.mapId)
            return true;

        if (!config.enabled)
            return false;

        if (!IsDifficultyAllowed(config.difficultyMask, context.difficultyMask))
            return false;

        if (context.partySize < config.minPartySize)
            return false;

        if (context.partySize > config.maxPartySize)
            return false;

        return true;
    }

    std::vector<RewardProfileItem> GetRewardProfileItems(uint32 rewardProfileId)
    {
        std::vector<RewardProfileItem> items;
        if (!rewardProfileId ||
            !TableExists("instance_bonus_reward_profile_item"))
            return items;

        QueryResult result = WorldDatabase.Query(Acore::StringFormat(
            "SELECT item_entry, item_count, CAST(grade AS CHAR) "
            "FROM instance_bonus_reward_profile_item "
            "WHERE reward_profile_id = {} "
            "ORDER BY grade ASC, item_entry ASC",
            rewardProfileId));

        if (!result)
            return items;

        do
        {
            Field* fields = result->Fetch();
            RewardProfileItem item;
            item.itemEntry = fields[0].Get<uint32>();
            item.itemCount = fields[1].Get<uint32>();
            item.grade = fields[2].Get<std::string>();
            if (item.itemEntry && item.itemCount)
                items.push_back(item);
        } while (result->NextRow());

        return items;
    }

    uint32 GetPlayerDailySuccessCount(uint32 mapId, Player* player)
    {
        if (!player)
            return 0;

        QueryResult result = WorldDatabase.Query(Acore::StringFormat(
            "SELECT `success_count` "
            "FROM `instance_bonus_player_daily_usage` "
            "WHERE `usage_date` = CURDATE() "
            "  AND `map_id` = {} "
            "  AND `guid` = {} "
            "LIMIT 1",
            mapId, player->GetGUID().GetCounter()));

        if (!result)
            return 0;

        return result->Fetch()[0].Get<uint32>();
    }

    bool HasRemainingDailyMissionCount(uint32 mapId, Player* player)
    {
        uint32 limit = GetDailyLimitForMap(mapId);
        if (!limit)
            return true;

        return GetPlayerDailySuccessCount(mapId, player) < limit;
    }

    bool MapHasAnyEligibleMissionPlayer(Map* map, Player* seedPlayer = nullptr)
    {
        if (!map)
            return false;

        uint32 limit = GetDailyLimitForMap(map->GetId());
        if (!limit)
            return true;

        Map::PlayerList const& players = map->GetPlayers();
        for (auto const& ref : players)
        {
            Player* player = ref.GetSource();
            if (!player)
                continue;

            if (HasRemainingDailyMissionCount(map->GetId(), player))
                return true;
        }

        if (seedPlayer && HasRemainingDailyMissionCount(map->GetId(), seedPlayer))
            return true;

        return false;
    }

    void IncrementDailyMissionCount(uint32 mapId, Player* player)
    {
        if (!player)
            return;

        uint32 limit = GetDailyLimitForMap(mapId);
        if (!limit)
            return;

        WorldDatabase.Execute(Acore::StringFormat(
            "INSERT INTO `instance_bonus_player_daily_usage` "
            "(`usage_date`, `map_id`, `guid`, `success_count`, `updated_at`) "
            "VALUES (CURDATE(), {}, {}, 1, UNIX_TIMESTAMP()) "
            "ON DUPLICATE KEY UPDATE "
            "`success_count` = `success_count` + 1, "
            "`updated_at` = UNIX_TIMESTAMP()",
            mapId, player->GetGUID().GetCounter()));
    }

    VoteStatus BuildVoteStatus(Map* map, MissionState const& state)
    {
        VoteStatus vote;
        if (!map)
            return vote;

        Map::PlayerList const& players = map->GetPlayers();
        for (auto const& ref : players)
        {
            Player* player = ref.GetSource();
            if (!player)
                continue;

            if (!HasRemainingDailyMissionCount(map->GetId(), player))
                continue;

            ++vote.eligible;
            auto itr = state.votes.find(GetVoteKey(player));
            if (itr == state.votes.end())
                continue;

            if (itr->second)
                ++vote.yes;
            else
                ++vote.no;
        }

        vote.required = std::max<uint32>(1, vote.eligible / 2 + 1);

        if (vote.yes >= vote.required)
            vote.state = "approved";
        else if (vote.no >= vote.required)
            vote.state = "rejected";
        else
            vote.state = "pending";

        return vote;
    }

    std::string GetPlayerVoteToken(MissionState const& state, Player* player)
    {
        auto itr = state.votes.find(GetVoteKey(player));
        if (itr == state.votes.end())
            return "none";

        return itr->second ? "yes" : "no";
    }

    void SendAddonPayload(
        Player* player,
        std::string const& prefix,
        std::string const& payload)
    {
        if (!player || !player->GetSession())
            return;

        std::string fullMessage = prefix + "\t" + payload;

        WorldPacket data(SMSG_MESSAGECHAT, 100);
        data << uint8(CHAT_MSG_WHISPER);
        data << int32(LANG_ADDON);
        data << player->GetGUID();
        data << uint32(0);
        data << player->GetGUID();
        data << uint32(fullMessage.length() + 1);
        data << fullMessage;
        data << uint8(0);
        player->GetSession()->SendPacket(&data);
    }

    void SendMissionUiClear(Player* player)
    {
        SendAddonPayload(player, "KBM_UI", "CLEAR");
    }

    void SendMissionUiAlert(Player* player, std::string const& message)
    {
        SendAddonPayload(
            player,
            "KBM_UI",
            Acore::StringFormat(
                "ALERT\t{}",
                SanitizeAddonField(message, 180)));
    }

    void SendMissionUiState(Player* player, MissionState const& state)
    {
        VoteStatus vote = BuildVoteStatus(player ? player->GetMap() : nullptr, state);

        std::ostringstream payload;
        payload << "STATE\t";
        payload << state.mapId << "\t";
        payload << state.themeId << "\t";
        payload << SanitizeAddonField(state.themeKey, 24) << "\t";
        payload << SanitizeAddonField(state.themeName, 48) << "\t";
        payload << SanitizeAddonField(state.title, 64) << "\t";
        payload << SanitizeAddonField(state.targetLabel, 64) << "\t";
        payload << state.currentCount << "\t";
        payload << state.targetCount << "\t";
        payload << GetRemainingSeconds(state) << "\t";
        payload << state.timeLimitSec << "\t";
        payload << GetMissionStatusToken(state) << "\t";
        payload << uint32(state.missionType) << "\t";
        payload << vote.state << "\t";
        payload << vote.yes << "\t";
        payload << vote.no << "\t";
        payload << vote.required << "\t";
        payload << GetPlayerVoteToken(state, player);

        SendAddonPayload(player, "KBM_UI", payload.str());
        SendAddonPayload(
            player,
            "KBM_UI",
            Acore::StringFormat(
                "BRIEFING\t{}",
                SanitizeAddonField(state.briefing, 180)));
        SendAddonPayload(
            player,
            "KBM_UI",
            Acore::StringFormat(
                "ANNOUNCEMENT\t{}",
                SanitizeAddonField(state.announcement, 180)));
    }

    void SendMissionUiStateToMap(Map* map, MissionState const& state)
    {
        if (!map)
            return;

        Map::PlayerList const& players = map->GetPlayers();
        for (auto const& ref : players)
        {
            if (Player* player = ref.GetSource())
                SendMissionUiState(player, state);
        }
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
                {
                    SendMissionMessageToPlayer(member, msg);
                    SendMissionUiAlert(member, msg);
                }
            }
            return;
        }

        SendMissionMessageToPlayer(player, msg);
        SendMissionUiAlert(player, msg);
    }

    void SendMissionMessageToMap(Map* map, std::string const& msg)
    {
        if (!map)
            return;

        Map::PlayerList const& players = map->GetPlayers();
        for (auto const& ref : players)
        {
            if (Player* player = ref.GetSource())
            {
                SendMissionMessageToPlayer(player, msg);
                SendMissionUiAlert(player, msg);
            }
        }
    }

    void ApproveMission(Map* map, MissionState& state)
    {
        if (!map || state.voteApproved)
            return;

        state.voteApproved = true;
        state.startTime = GameTime::GetGameTime().count();
        state.expireTime = state.startTime + state.timeLimitSec;
        state.announcement = "추가 미션이 과반수 찬성으로 시작되었습니다.";
        SaveLiveState(map->GetInstanceId(), state);
        SendMissionUiStateToMap(map, state);
        SendMissionMessageToMap(
            map,
            "[추가 미션] 과반수 찬성으로 미션이 시작되었습니다.");
    }

    bool ApplyVote(Player* player, bool agree)
    {
        if (!player)
            return false;

        Map* map = player->GetMap();
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
            return false;

        MissionState* state = MissionStateStore::Instance().Get(
            map->GetInstanceId());
        if (!state || state->completed || state->failed)
            return false;

        state->votes[GetVoteKey(player)] = agree;
        VoteStatus vote = BuildVoteStatus(map, *state);

        if (vote.state == "approved")
            ApproveMission(map, *state);
        else
        {
            state->announcement = Acore::StringFormat(
                "찬성 {} / {}, 반대 {}",
                vote.yes, vote.required, vote.no);
            SaveLiveState(map->GetInstanceId(), *state);
            SendMissionUiStateToMap(map, *state);
        }

        SendMissionMessageToMap(
            map,
            Acore::StringFormat(
                "[추가 미션] {}님이 {}를 선택했습니다. 찬성 {} / {}, 반대 {}",
                player->GetName(),
                agree ? "찬성" : "반대",
                vote.yes,
                vote.required,
                vote.no));
        return true;
    }

    void ApproveMissionVote(Map* map, MissionState& state)
    {
        if (!map || state.voteApproved)
            return;

        state.voteApproved = true;
        state.startTime = GameTime::GetGameTime().count();
        state.expireTime = state.startTime + state.timeLimitSec;
        state.announcement = "과반수 찬성으로 추가 미션이 시작되었습니다.";
        SaveLiveState(map->GetInstanceId(), state);
        SendMissionUiStateToMap(map, state);
        SendMissionMessageToMap(
            map,
            "[추가 미션] 과반수 찬성으로 미션이 시작되었습니다.");
    }

    bool ApplyMissionVote(Player* player, bool agree)
    {
        if (!player)
            return false;

        Map* map = player->GetMap();
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
            return false;

        MissionState* state = MissionStateStore::Instance().Get(
            map->GetInstanceId());
        if (!state || state->completed || state->failed)
            return false;

        if (state->voteApproved)
            return false;

        if (!HasRemainingDailyMissionCount(map->GetId(), player))
        {
            SendMissionMessageToPlayer(
                player,
                "[추가 미션] 오늘 이 던전의 추가 미션 참여 횟수를 모두 사용했습니다.");
            return false;
        }

        state->votes[GetVoteKey(player)] = agree;
        LogVote(*state, player, agree);
        VoteStatus vote = BuildVoteStatus(map, *state);

        if (vote.state == "approved")
            ApproveMissionVote(map, *state);
        else
        {
            if (vote.state == "rejected")
                state->announcement =
                    "과반수 반대로 추가 미션이 잠금 상태가 되었습니다.";
            else
            {
                state->announcement = Acore::StringFormat(
                    "과반수 찬성이 필요합니다. 찬성 {} / {}, 반대 {}",
                    vote.yes, vote.required, vote.no);
            }

            SaveLiveState(map->GetInstanceId(), *state);
            SendMissionUiStateToMap(map, *state);
        }

        SendMissionMessageToMap(
            map,
            Acore::StringFormat(
                "[추가 미션] {}님이 {}를 선택했습니다. 찬성 {} / {}, 반대 {}",
                player->GetName(),
                agree ? "찬성" : "반대",
                vote.yes,
                vote.required,
                vote.no));
        return true;
    }

    void SaveRunHistory(MissionState const& state)
    {
        if (!TableExists("instance_bonus_run_history"))
            return;

        std::string status = "pending";
        if (state.completed)
            status = "success";
        else if (state.failed)
            status = "failed";
        else if (state.voteApproved)
            status = "active";

        if (ColumnExists("instance_bonus_run_history", "theme_key"))
        {
            WorldDatabase.Execute(Acore::StringFormat(
                "REPLACE INTO instance_bonus_run_history ("
                "run_id, instance_id, map_id, difficulty, leader_guid, "
                "theme_id, theme_key, mission_id, mission_name, status, "
                "started_at, ended_at, clear_time_sec, deaths, wipes, "
                "failure_reason, score, grade, vote_yes, vote_no, llm_used, "
                "llm_source, reward_profile_id) VALUES ("
                "{}, {}, {}, {}, 0, 0, '', {}, '{}', {}, {}, {}, {}, 0, 0, "
                "'', 0, '', {}, {}, {}, '{}', {})",
                state.runId,
                state.runId >> 32,
                state.mapId,
                state.difficultyMask,
                state.missionId,
                EscapeSql(state.title),
                state.completed ? 2 : (state.failed ? 3 : (state.voteApproved ? 1 : 0)),
                MakeTimeValueExpr(
                    "instance_bonus_run_history",
                    "started_at",
                    state.startTime ? state.startTime : GameTime::GetGameTime().count()),
                MakeTimeValueExpr(
                    "instance_bonus_run_history",
                    "ended_at",
                    (state.completed || state.failed)
                        ? GameTime::GetGameTime().count()
                        : 0),
                state.completed && state.startTime
                    ? uint32(GameTime::GetGameTime().count() - state.startTime)
                    : 0,
                uint32(std::count_if(
                    state.votes.begin(),
                    state.votes.end(),
                    [](auto const& entry) { return entry.second; })),
                uint32(std::count_if(
                    state.votes.begin(),
                    state.votes.end(),
                    [](auto const& entry) { return !entry.second; })),
                state.source == "llm" ? 1 : 0,
                EscapeSql(state.source),
                state.rewardProfileId));
            return;
        }

        WorldDatabase.Execute(Acore::StringFormat(
            "INSERT INTO instance_bonus_run_history ("
            "instance_id, map_id, theme_id, theme_name, mission_id, "
            "mission_name, status, grade, started_at, ended_at, "
            "clear_time_sec, deaths, wipes, score, vote_yes, vote_no, "
            "llm_used, fallback_used, failure_reason) VALUES ("
            "{}, {}, 0, '', {}, '{}', '{}', '', {}, {}, {}, 0, 0, 0, {}, "
            "{}, {}, {}, '')",
            state.runId >> 32,
            state.mapId,
            state.missionId,
            EscapeSql(state.title),
            EscapeSql(status),
            MakeTimeValueExpr(
                "instance_bonus_run_history",
                "started_at",
                state.startTime ? state.startTime : GameTime::GetGameTime().count()),
            (state.completed || state.failed)
                ? MakeTimeValueExpr(
                    "instance_bonus_run_history",
                    "ended_at",
                    GameTime::GetGameTime().count())
                : "NULL",
            state.completed && state.startTime
                ? uint32(GameTime::GetGameTime().count() - state.startTime)
                : 0,
            uint32(std::count_if(
                state.votes.begin(),
                state.votes.end(),
                [](auto const& entry) { return entry.second; })),
            uint32(std::count_if(
                state.votes.begin(),
                state.votes.end(),
                [](auto const& entry) { return !entry.second; })),
            state.source == "llm" ? 1 : 0,
            state.source == "fallback" ? 1 : 0));
    }

    void SaveRunLive(uint32 instanceId, MissionState const& state)
    {
        if (!TableExists("instance_bonus_run_live"))
            return;

        if (ColumnExists("instance_bonus_run_live", "difficulty"))
        {
            WorldDatabase.Execute(Acore::StringFormat(
                "REPLACE INTO instance_bonus_run_live ("
                "run_id, instance_id, map_id, difficulty, status, theme_id, "
                "theme_key, theme_name, mission_id, mission_name, objective_type, "
                "target_entry, target_label, target_count, current_count, "
                "time_limit_sec, start_time, expire_time, deaths, wipes, "
                "briefing, announcement, source, vote_required, vote_yes, "
                "vote_no, completed, failed, failure_reason, reward_profile_id, "
                "updated_at) VALUES ("
                "{}, {}, {}, {}, {}, 0, '', '', {}, '{}', {}, {}, '{}', {}, {}, "
                "{}, {}, {}, 0, 0, '{}', '{}', '{}', {}, {}, {}, {}, {}, '', "
                "{}, {})",
                state.runId,
                instanceId,
                state.mapId,
                state.difficultyMask,
                state.completed ? 2 : (state.failed ? 3 : (state.voteApproved ? 1 : 0)),
                state.missionId,
                EscapeSql(state.title),
                uint32(state.missionType),
                state.targetEntry,
                EscapeSql(state.targetLabel),
                state.targetCount,
                state.currentCount,
                state.timeLimitSec,
                MakeTimeValueExpr(
                    "instance_bonus_run_live", "start_time", state.startTime),
                MakeTimeValueExpr(
                    "instance_bonus_run_live", "expire_time", state.expireTime),
                EscapeSql(state.briefing),
                EscapeSql(state.announcement),
                EscapeSql(state.source),
                1,
                uint32(std::count_if(
                    state.votes.begin(),
                    state.votes.end(),
                    [](auto const& entry) { return entry.second; })),
                uint32(std::count_if(
                    state.votes.begin(),
                    state.votes.end(),
                    [](auto const& entry) { return !entry.second; })),
                state.completed ? 1 : 0,
                state.failed ? 1 : 0,
                state.rewardProfileId,
                MakeTimeValueExpr(
                    "instance_bonus_run_live",
                    "updated_at",
                    GameTime::GetGameTime().count())));
            return;
        }

        WorldDatabase.Execute(Acore::StringFormat(
            "INSERT INTO instance_bonus_run_live ("
            "instance_id, map_id, theme_id, mission_id, status, started_at, "
            "score, grade, llm_used, fallback_used) VALUES ("
            "{}, {}, 0, {}, '{}', {}, 0, '', {}, {})",
            instanceId,
            state.mapId,
            state.missionId,
            EscapeSql(
                state.completed ? "success"
                    : (state.failed ? "failed"
                        : (state.voteApproved ? "active" : "live"))),
            MakeTimeValueExpr(
                "instance_bonus_run_live",
                "started_at",
                state.startTime ? state.startTime : GameTime::GetGameTime().count()),
            state.source == "llm" ? 1 : 0,
            state.source == "fallback" ? 1 : 0));
    }

    void LogVote(MissionState const& state, Player* player, bool agree)
    {
        if (!player || !TableExists("instance_bonus_vote_log"))
            return;

        if (ColumnExists("instance_bonus_vote_log", "character_guid"))
        {
            WorldDatabase.Execute(Acore::StringFormat(
                "INSERT INTO instance_bonus_vote_log "
                "(run_id, character_guid, character_name, vote_value, voted_at) "
                "VALUES ({}, {}, '{}', '{}', {})",
                state.runId,
                player->GetGUID().GetCounter(),
                EscapeSql(player->GetName()),
                agree ? "yes" : "no",
                MakeTimeValueExpr(
                    "instance_bonus_vote_log",
                    "voted_at",
                    GameTime::GetGameTime().count())));
            return;
        }

        WorldDatabase.Execute(Acore::StringFormat(
            "INSERT INTO instance_bonus_vote_log "
            "(run_id, guid, name, vote, voted_at, vote_round) VALUES "
            "({}, {}, '{}', {}, {}, 1) "
            "ON DUPLICATE KEY UPDATE vote = VALUES(vote), voted_at = VALUES(voted_at)",
            state.runId,
            player->GetGUID().GetCounter(),
            EscapeSql(player->GetName()),
            agree ? 1 : 2,
            uint64(GameTime::GetGameTime().count())));
    }

    void LogReward(
        MissionState const& state,
        Player* player,
        uint32 itemEntry,
        uint32 itemCount)
    {
        if (!player || !TableExists("instance_bonus_reward_log"))
            return;

        if (ColumnExists("instance_bonus_reward_log", "character_guid"))
        {
            WorldDatabase.Execute(Acore::StringFormat(
                "INSERT INTO instance_bonus_reward_log "
                "(run_id, character_guid, character_name, grade, item_entry, "
                "item_count, granted_at) VALUES "
                "({}, {}, '{}', 'A', {}, {}, {})",
                state.runId,
                player->GetGUID().GetCounter(),
                EscapeSql(player->GetName()),
                itemEntry,
                itemCount,
                MakeTimeValueExpr(
                    "instance_bonus_reward_log",
                    "granted_at",
                    GameTime::GetGameTime().count())));
            return;
        }

        WorldDatabase.Execute(Acore::StringFormat(
            "INSERT INTO instance_bonus_reward_log "
            "(run_id, guid, name, grade, reward_profile_id, item_entry, "
            "item_count, granted_at, grant_status) VALUES "
            "({}, {}, '{}', 'A', {}, {}, {}, {}, 1)",
            state.runId,
            player->GetGUID().GetCounter(),
            EscapeSql(player->GetName()),
            state.rewardProfileId,
            itemEntry,
            itemCount,
            uint64(GameTime::GetGameTime().count())));
    }

    void SaveLiveState(uint32 instanceId, MissionState const& state)
    {
        std::string themeKey = state.themeKey;
        std::string themeName = state.themeName;
        std::string title = state.title;
        std::string targetLabel = state.targetLabel;
        std::string briefing = state.briefing;
        std::string announcement = state.announcement;
        std::string source = state.source;

        WorldDatabase.EscapeString(themeKey);
        WorldDatabase.EscapeString(themeName);
        WorldDatabase.EscapeString(title);
        WorldDatabase.EscapeString(targetLabel);
        WorldDatabase.EscapeString(briefing);
        WorldDatabase.EscapeString(announcement);
        WorldDatabase.EscapeString(source);

        WorldDatabase.Execute(Acore::StringFormat(
            "REPLACE INTO `instance_bonus_mission_live` "
            "(`instance_id`, `map_id`, `theme_id`, `theme_key`, "
            "`theme_name`, `mission_id`, `mission_type`, `title`, "
            "`target_label`, `target_entry`, `target_count`, "
            "`current_count`, `time_limit_sec`, `start_time`, "
            "`expire_time`, `briefing`, `announcement`, `source`, "
            "`completed`, `failed`, `updated_at`) "
            "VALUES ({}, {}, {}, '{}', '{}', {}, {}, '{}', '{}', {}, "
            "{}, {}, {}, {}, {}, '{}', '{}', '{}', {}, {}, "
            "UNIX_TIMESTAMP())",
            instanceId, state.mapId, state.themeId, themeKey, themeName,
            state.missionId, state.missionType, title, targetLabel,
            state.targetEntry, state.targetCount, state.currentCount,
            state.timeLimitSec, static_cast<uint64>(state.startTime),
            static_cast<uint64>(state.expireTime), briefing, announcement,
            source, state.completed ? 1 : 0, state.failed ? 1 : 0));

        SaveRunLive(instanceId, state);
        SaveRunHistory(state);
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
            Acore::StringFormat("\\\"{}\\\"\\s*:\\s*(\\d+)", key));
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

    std::vector<Player*> GetInstancePlayers(Map* map, Player* seedPlayer = nullptr)
    {
        std::vector<Player*> players;
        if (!map)
            return players;

        Map::PlayerList const& refs = map->GetPlayers();
        for (auto const& ref : refs)
        {
            Player* player = ref.GetSource();
            if (!player || player->IsGameMaster())
                continue;

            players.push_back(player);
        }

        if (seedPlayer && !seedPlayer->IsGameMaster())
        {
            bool found = std::any_of(
                players.begin(),
                players.end(),
                [seedPlayer](Player* entry)
                {
                    return entry && entry->GetGUID() == seedPlayer->GetGUID();
                });

            if (!found)
                players.push_back(seedPlayer);
        }

        return players;
    }

    PartyContext BuildPartyContext(Map* map, Player* seedPlayer = nullptr)
    {
        PartyContext context;
        context.difficultyMask = GetDifficultyMaskForMap(map);
        context.difficulty = GetDifficultyTokenForMap(map);

        std::vector<Player*> players = GetInstancePlayers(map, seedPlayer);
        context.partySize = uint32(players.size());

        // Treat the player who triggered the entry hook as the minimum
        // context source, even for GM-led test runs where the map player list
        // may still be empty or filtered.
        if (!context.partySize && seedPlayer)
            context.partySize = 1;

        if (players.empty())
            return context;

        float totalItemLevel = 0.0f;

        for (Player* player : players)
        {
            uint8 classId = player->getClass();
            ++context.classCounts[classId];
            totalItemLevel += player->GetAverageItemLevel();

            if (IsLikelyTankClass(classId))
                ++context.likelyTankCount;
            if (IsLikelyHealerClass(classId))
                ++context.likelyHealerCount;
        }

        context.averageItemLevel = uint32(
            (totalItemLevel / float(players.size())) + 0.5f);
        context.hasTank = context.likelyTankCount > 0;
        context.hasHealer = context.likelyHealerCount > 0;
        return context;
    }

    bool IsThemeEligible(
        ThemeDefinition const& definition,
        PartyContext const& context)
    {
        if (context.partySize < definition.minPartySize)
            return false;

        if (context.partySize > definition.maxPartySize)
            return false;

        if (!IsDifficultyAllowed(
                definition.difficultyMask, context.difficultyMask))
            return false;

        if (context.averageItemLevel < definition.minAvgItemLevel)
            return false;

        if (context.averageItemLevel > definition.maxAvgItemLevel)
            return false;

        if (definition.requiredTank && !context.hasTank)
            return false;

        if (definition.requiredHealer && !context.hasHealer)
            return false;

        return true;
    }

    std::string BuildClassCountsJson(PartyContext const& context)
    {
        std::ostringstream json;
        json << "{";
        bool first = true;
        for (auto const& entry : context.classCounts)
        {
            if (!first)
                json << ",";

            first = false;
            json << "\"" << GetClassToken(entry.first) << "\":"
                 << entry.second;
        }
        json << "}";
        return json.str();
    }

    std::vector<MissionDefinition const*> CollectEligibleMissions(
        uint32 mapId,
        PartyContext const& context)
    {
        std::vector<MissionDefinition const*> missions;
        auto definitions = MissionStore::Instance().GetByMap(mapId);
        if (!definitions)
            return missions;

        MapConfigDefinition config = GetMapConfigForMap(mapId);
        if (!IsMapConfigEnabledForContext(config, context))
            return missions;

        for (MissionDefinition const& definition : *definitions)
        {
            if (IsDifficultyAllowed(
                    definition.difficultyMask, context.difficultyMask))
                missions.push_back(&definition);
        }

        return missions;
    }

    MissionSelection SelectMissionFallback(
        uint32 mapId,
        PartyContext const& context)
    {
        MissionSelection selection;
        std::vector<MissionDefinition const*> candidates =
            CollectEligibleMissions(mapId, context);
        if (candidates.empty())
            return selection;

        if (candidates.size() == 1)
            selection.definition = candidates.front();
        else
            selection.definition = candidates.at(
                urand(0, uint32(candidates.size() - 1)));

        selection.announcement = selection.definition->fallbackAnnouncement;
        selection.source = "fallback";
        return selection;
    }

    MissionSelection TrySelectMissionWithLlm(
        Map* map,
        PartyContext const& ctx)
    {
        MissionSelection fallback = SelectMissionFallback(
            map->GetId(), ctx);
        if (!fallback.definition)
            return fallback;

        MapConfigDefinition config = GetMapConfigForMap(map->GetId());
        if (!MissionStore::Instance().IsLlmEnabled() || !config.allowLlm)
            return fallback;

        std::vector<MissionDefinition const*> definitions =
            CollectEligibleMissions(map->GetId(), ctx);
        if (definitions.empty())
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
        json << "\"difficulty\":\"" << EscapeJson(ctx.difficulty)
             << "\",";
        json << "\"party_size\":" << ctx.partySize << ",";
        json << "\"candidates\":[";

        bool first = true;
        for (MissionDefinition const* definition : definitions)
        {
            if (!first)
                json << ",";

            first = false;
            json << "{";
            json << "\"mission_id\":" << definition->missionId << ",";
            json << "\"mission_type\":"
                 << uint32(definition->missionType) << ",";
            json << "\"title\":\"" << EscapeJson(definition->title)
                 << "\",";
            json << "\"target_label\":\""
                 << EscapeJson(definition->targetLabel) << "\",";
            json << "\"target_count\":" << definition->targetCount
                 << ",";
            json << "\"time_limit_sec\":"
                 << definition->timeLimitSec;
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
                    "Mission LLM request failed: {}",
                    httplib::to_string(result.error()));
                return fallback;
            }

            if (result->status != 200)
            {
                LOG_ERROR(
                    "module.instance_bonus_mission",
                    "Mission LLM request returned status {}",
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
                    "Mission LLM response missing selected_mission_id: {}",
                    result->body);
                return fallback;
            }

            TryExtractString(result->body, "announcement", announcement);

            for (MissionDefinition const* definition : definitions)
            {
                if (definition->missionId != selectedMissionId)
                    continue;

                MissionSelection selection;
                selection.definition = definition;
                selection.announcement = announcement.empty()
                    ? definition->fallbackAnnouncement
                    : announcement;
                selection.source = "llm";
                return selection;
            }

            LOG_ERROR(
                "module.instance_bonus_mission",
                "Mission LLM selected unknown mission_id {} for map {}",
                selectedMissionId, map->GetId());
            return fallback;
        }
        catch (std::exception const& ex)
        {
            LOG_ERROR(
                "module.instance_bonus_mission",
                "Mission LLM request exception: {}",
                ex.what());
            return fallback;
        }
    }

    void RewardMissionToMap(Map* map, MissionState const& state)
    {
        if (!map)
            return;

        std::vector<RewardProfileItem> profileItems =
            GetRewardProfileItems(state.rewardProfileId);

        Map::PlayerList const& players = map->GetPlayers();
        for (auto const& ref : players)
        {
            Player* player = ref.GetSource();
            if (!player)
                continue;

            if (!HasRemainingDailyMissionCount(map->GetId(), player))
            {
                SendMissionMessageToPlayer(
                    player,
                    "[추가 미션] 오늘 이 던전의 추가 미션 보상 횟수를 모두 사용했습니다.");
                continue;
            }

            bool rewarded = false;
            if (!profileItems.empty())
            {
                for (RewardProfileItem const& item : profileItems)
                {
                    player->AddItem(item.itemEntry, item.itemCount);
                    LogReward(state, player, item.itemEntry, item.itemCount);
                    rewarded = true;
                }
            }
            else if (state.rewardItem && state.rewardCount)
            {
                player->AddItem(state.rewardItem, state.rewardCount);
                LogReward(state, player, state.rewardItem, state.rewardCount);
                rewarded = true;
            }

            if (!rewarded)
                continue;

            IncrementDailyMissionCount(map->GetId(), player);
        }
    }

    void CompleteMission(Map* map, MissionState& state)
    {
        if (!map || state.completed || state.failed)
            return;

        state.completed = true;
        SaveLiveState(map->GetInstanceId(), state);
        SendMissionUiStateToMap(map, state);
        RewardMissionToMap(map, state);
        SendMissionMessageToMap(
            map,
            Acore::StringFormat(
                "[추가 미션] {} 임무를 완료했습니다. 추가 보상이 지급됩니다.",
                state.title));
    }

    void FailMission(Map* map, MissionState& state, std::string const& reason)
    {
        if (!map || state.completed || state.failed)
            return;

        state.failed = true;
        SaveLiveState(map->GetInstanceId(), state);
        SendMissionUiStateToMap(map, state);
        SendMissionMessageToMap(
            map,
            Acore::StringFormat(
                "[추가 미션] {} 임무에 실패했습니다. {}",
                state.title, reason));
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

        if (!state->voteApproved)
            return;

        if (state->targetEntry != killed->GetEntry())
            return;

        ++state->currentCount;
        SaveLiveState(map->GetInstanceId(), *state);
        SendMissionUiStateToMap(map, *state);

        uint32 remaining = 0;
        if (state->targetCount > state->currentCount)
            remaining = state->targetCount - state->currentCount;

        if (state->missionType == 2)
        {
            if (state->currentCount >= std::max<uint32>(1, state->targetCount))
                CompleteMission(map, *state);
            return;
        }

        if (!state->announcedHalf &&
            state->targetCount > 1 &&
            state->currentCount >= (state->targetCount / 2))
        {
            state->announcedHalf = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 미션] {} 목표를 절반 달성했습니다. {} / {}",
                    state->targetLabel,
                    state->currentCount,
                    state->targetCount));
        }

        if (!state->announcedFive && remaining == 5)
        {
            state->announcedFive = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 미션] {} {}마리 중 {}마리 남았습니다.",
                    state->targetLabel,
                    state->targetCount,
                    remaining));
        }

        if (!state->announcedThree && remaining == 3)
        {
            state->announcedThree = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 미션] {} {}마리 중 {}마리 남았습니다.",
                    state->targetLabel,
                    state->targetCount,
                    remaining));
        }

        if (!state->announcedOne && remaining == 1)
        {
            state->announcedOne = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 미션] {} {}마리 중 {}마리 남았습니다.",
                    state->targetLabel,
                    state->targetCount,
                    remaining));
        }

        if (state->currentCount >= state->targetCount)
            CompleteMission(map, *state);
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
        : PlayerScript(
            "InstanceBonusMissionPlayerScript",
            {
                PLAYERHOOK_CAN_PLAYER_USE_GROUP_CHAT,
                PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
            })
    {
    }

    void OnPlayerMapChanged(Player* player) override
    {
        if (!player || !MissionStore::Instance().IsEnabled())
            return;

        Map* map = player->GetMap();
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
        {
            SendMissionUiClear(player);
            return;
        }

        LOG_INFO(
            "module.instance_bonus_mission",
            "추가미션 모듈 실행: player='{}' mapId={} instanceId={}",
            player->GetName(),
            map->GetId(),
            map->GetInstanceId());

        SendMissionMessageToPlayer(
            player,
            "[추가 미션] 추가미션 모듈 실행");
        SendMissionMessageToPlayer(
            player,
            "[추가 미션] 추가미션 모듈 작동");

        uint32 instanceId = map->GetInstanceId();
        if (MissionState* state = MissionStateStore::Instance().Get(instanceId))
        {
            SendMissionUiState(player, *state);
            return;
        }

        if (!MapHasAnyEligibleMissionPlayer(map, player))
        {
            SendMissionMessageToPlayer(
                player,
                "[추가 미션] 오늘 이 던전의 추가 미션 가능 횟수를 모두 사용했습니다.");
            SendMissionUiClear(player);
            return;
        }

        PartyContext context = BuildPartyContext(map, player);
        MapConfigDefinition config = GetMapConfigForMap(map->GetId());
        LOG_INFO(
            "module.instance_bonus_mission",
            "Entry context: mapId={} instanceId={} partySize={} diffMask={} cfgEnabled={} cfgDiffMask={} minParty={} maxParty={}",
            map->GetId(),
            instanceId,
            context.partySize,
            context.difficultyMask,
            config.enabled ? 1 : 0,
            config.difficultyMask,
            config.minPartySize,
            config.maxPartySize);
        if (!IsMapConfigEnabledForContext(config, context))
        {
            LOG_INFO(
                "module.instance_bonus_mission",
                "Entry blocked by map config: mapId={} instanceId={}",
                map->GetId(),
                instanceId);
            SendMissionUiClear(player);
            return;
        }

        std::vector<MissionDefinition const*> eligibleMissions =
            CollectEligibleMissions(map->GetId(), context);
        LOG_INFO(
            "module.instance_bonus_mission",
            "Eligible missions: mapId={} instanceId={} count={}",
            map->GetId(),
            instanceId,
            eligibleMissions.size());

        MissionSelection missionSelection = TrySelectMissionWithLlm(
            map, context);
        if (!missionSelection.definition)
        {
            LOG_INFO(
                "module.instance_bonus_mission",
                "No mission selected: mapId={} instanceId={}",
                map->GetId(),
                instanceId);
            return;
        }

        LOG_INFO(
            "module.instance_bonus_mission",
            "Mission selected: mapId={} instanceId={} missionId={} title='{}' source={}",
            map->GetId(),
            instanceId,
            missionSelection.definition->missionId,
            missionSelection.definition->title,
            missionSelection.source);

        MissionState& state = MissionStateStore::Instance().Create(
            instanceId,
            missionSelection);
        state.difficultyMask = context.difficultyMask;

        state.announcement = "추가 미션이 제안되었습니다. 과반수 찬성이 필요합니다.";
        SaveLiveState(instanceId, state);

        if (!state.briefing.empty())
        {
            SendMissionMessageToGroup(
                player,
                std::string(kBriefingPrefix) + state.briefing);
        }

        SendMissionMessageToGroup(
            player,
            std::string(kMissionPrefix) + state.announcement);
        SendMissionUiStateToMap(map, state);

        if (!config.allowVote)
            ApproveMissionVote(map, state);
    }

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;

        Map* map = player->GetMap();
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
        {
            SendMissionUiClear(player);
            return;
        }

        if (MissionState* state = MissionStateStore::Instance().Get(
                map->GetInstanceId()))
            SendMissionUiState(player, *state);
    }

    void OnPlayerJustDied(Player* player) override
    {
        if (!player)
            return;

        Map* map = player->GetMap();
        if (!map || !map->IsDungeon() || !map->GetInstanceId())
            return;

        MissionState* state = MissionStateStore::Instance().Get(
            map->GetInstanceId());
        if (!state || state->completed || state->failed)
            return;

        if (!state->voteApproved)
            return;

        if (state->missionType != 2)
            return;

        FailMission(map, *state, "파티 사망이 발생했습니다.");
    }
    bool OnPlayerCanUseChat(
        Player* player,
        uint32 /*type*/,
        uint32 language,
        std::string& msg,
        Player* /*receiver*/) override
    {
        if (language != LANG_ADDON)
            return true;

        if (msg == "KBM_VOTE\tYES")
            return !ApplyMissionVote(player, true);

        if (msg == "KBM_VOTE\tNO")
            return !ApplyMissionVote(player, false);

        return true;
    }

    bool OnPlayerCanUseChat(
        Player* player,
        uint32 /*type*/,
        uint32 language,
        std::string& msg,
        Group* /*group*/) override
    {
        if (language != LANG_ADDON)
            return true;

        if (msg == "KBM_VOTE\tYES")
            return !ApplyMissionVote(player, true);

        if (msg == "KBM_VOTE\tNO")
            return !ApplyMissionVote(player, false);

        return true;
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

        if (!state->voteApproved)
            return;

        time_t now = GameTime::GetGameTime().count();
        if (state->timeLimitSec == 0)
            return;

        if (now >= state->expireTime)
        {
            FailMission(map, *state, "제한 시간이 지났습니다.");
            return;
        }

        uint32 remaining = uint32(state->expireTime - now);

        if (!state->announcedTenMinute && remaining <= 600 &&
            state->timeLimitSec > 600)
        {
            state->announcedTenMinute = true;
            SendMissionMessageToMap(
                map, "[추가 미션] 시간이 10분 남았습니다.");
        }

        if (!state->announcedFiveMinute && remaining <= 300)
        {
            state->announcedFiveMinute = true;
            SendMissionMessageToMap(
                map, "[추가 미션] 시간이 5분 남았습니다.");
        }

        if (!state->announcedThreeMinute && remaining <= 180)
        {
            state->announcedThreeMinute = true;
            SendMissionMessageToMap(
                map, "[추가 미션] 시간이 3분 남았습니다.");
        }

        if (!state->announcedOneMinute && remaining <= 60)
        {
            state->announcedOneMinute = true;
            SendMissionMessageToMap(
                map, "[추가 미션] 시간이 1분 남았습니다.");
        }

        if (!state->announcedThirtySec && remaining <= 30)
        {
            state->announcedThirtySec = true;
            SendMissionMessageToMap(
                map, "[추가 미션] 시간이 30초 남았습니다.");
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

