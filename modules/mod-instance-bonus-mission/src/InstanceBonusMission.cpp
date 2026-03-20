#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Group.h"
#include "InstanceMap.h"
#include "Log.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "StringFormat.h"
#include "WorldSession.h"

#include <unordered_map>
#include <vector>

namespace
{
    enum MissionType : uint8
    {
        MISSION_KILL_ENTRY = 1,
        MISSION_KILL_TYPE = 2,
        MISSION_KILL_BOSS = 3,
    };

    struct MissionDefinition
    {
        uint32 mapId = 0;
        uint32 missionId = 0;
        uint8 missionType = 0;
        uint32 targetEntry = 0;
        uint32 targetCount = 0;
        uint32 timeLimitSec = 0;
        std::string title;
        std::string fallbackAnnouncement;
        uint32 rewardItem = 0;
        uint32 rewardCount = 0;
    };

    struct MissionState
    {
        uint32 missionId = 0;
        uint32 mapId = 0;
        uint32 targetEntry = 0;
        uint32 targetCount = 0;
        uint32 currentCount = 0;
        uint32 timeLimitSec = 0;
        time_t startTime = 0;
        time_t expireTime = 0;
        bool completed = false;
        bool failed = false;
        bool announcedHalf = false;
        bool announcedFive = false;
        bool announcedThree = false;
        bool announcedOne = false;
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
            _definitions.clear();

            if (!_enabled)
                return;

            QueryResult result = WorldDatabase.Query(
                "SELECT map_id, mission_id, mission_type, target_entry, "
                "target_count, time_limit_sec, title, "
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
                definition.fallbackAnnouncement =
                    fields[7].Get<std::string>();
                definition.rewardItem = fields[8].Get<uint32>();
                definition.rewardCount = fields[9].Get<uint32>();

                _definitions[definition.mapId].push_back(definition);
            } while (result->NextRow());
        }

        bool IsEnabled() const
        {
            return _enabled;
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
            uint32 instanceId, MissionDefinition const& definition)
        {
            MissionState& state = _states[instanceId];
            state.missionId = definition.missionId;
            state.mapId = definition.mapId;
            state.targetEntry = definition.targetEntry;
            state.targetCount = definition.targetCount;
            state.currentCount = 0;
            state.timeLimitSec = definition.timeLimitSec;
            state.startTime = GameTime::GetGameTime().count();
            state.expireTime = state.startTime + state.timeLimitSec;
            state.completed = false;
            state.failed = false;
            state.announcedHalf = false;
            state.announcedFive = false;
            state.announcedThree = false;
            state.announcedOne = false;
            return state;
        }

        void Remove(uint32 instanceId)
        {
            _states.erase(instanceId);
        }

    private:
        std::unordered_map<uint32, MissionState> _states;
    };

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
                    ChatHandler(member->GetSession()).SendSysMessage(
                        msg.c_str());
                }
            }
            return;
        }

        ChatHandler(player->GetSession()).SendSysMessage(msg.c_str());
    }

    MissionDefinition const* SelectFallbackMission(uint32 mapId)
    {
        auto definitions = MissionStore::Instance().GetByMap(mapId);
        if (!definitions || definitions->empty())
            return nullptr;

        return &definitions->front();
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
        if (!map || !map->IsDungeon())
            return;

        if (!map->GetInstanceId())
            return;

        uint32 instanceId = map->GetInstanceId();
        if (MissionStateStore::Instance().Get(instanceId))
            return;

        MissionDefinition const* definition =
            SelectFallbackMission(map->GetId());
        if (!definition)
            return;

        MissionStateStore::Instance().Create(instanceId, *definition);
        SendMissionMessageToGroup(player, definition->fallbackAnnouncement);
    }
};

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
            state->currentCount >= (state->targetCount / 2))
        {
            state->announcedHalf = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "추가 보상 임무 진행 중: 목표의 절반을 달성했습니다."));
        }

        if (!state->announcedFive && remaining == 5)
        {
            state->announcedFive = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "현재 목표까지 {}마리 남았습니다.", remaining));
        }

        if (!state->announcedThree && remaining == 3)
        {
            state->announcedThree = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "현재 목표까지 {}마리 남았습니다.", remaining));
        }

        if (!state->announcedOne && remaining == 1)
        {
            state->announcedOne = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "현재 목표까지 {}마리 남았습니다.", remaining));
        }

        if (state->currentCount < state->targetCount)
            return;

        state->completed = true;
        SendMissionMessageToGroup(
            killer, "추가 보상 임무를 완수했습니다.");
    }
}

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
        if (state->timeLimitSec && now >= state->expireTime)
        {
            state->failed = true;
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
