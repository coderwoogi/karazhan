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
#include "ScriptMgr.h"
#include "StringFormat.h"
#include "WorldSession.h"

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
        uint32 rewardItem = 0;
        uint32 rewardCount = 0;
        std::string title;
        std::string announcement;
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
            _llmEnabled = sConfigMgr->GetOption<bool>(
                "InstanceBonusMission.LLM.Enable", false);
            _llmUrl = sConfigMgr->GetOption<std::string>(
                "InstanceBonusMission.LLM.Url",
                "http://127.0.0.1:8000");
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

        bool IsLlmEnabled() const
        {
            return _llmEnabled;
        }

        std::string const& GetLlmUrl() const
        {
            return _llmUrl;
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
        bool _llmEnabled = false;
        std::string _llmUrl;
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
            state.rewardItem = definition.rewardItem;
            state.rewardCount = definition.rewardCount;
            state.title = definition.title;
            state.announcement = definition.fallbackAnnouncement;
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
        if (!player)
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

    MissionDefinition const* SelectMission(uint32 mapId)
    {
        auto definitions = MissionStore::Instance().GetByMap(mapId);
        if (!definitions || definitions->empty())
            return nullptr;

        if (MissionStore::Instance().IsLlmEnabled())
        {
            LOG_INFO(
                "module.instance_bonus_mission",
                "LLM is enabled. Fallback selection is used until HTTP "
                "bridge is connected. url={}",
                MissionStore::Instance().GetLlmUrl());
        }

        return &definitions->front();
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
                    "[추가 임무] 목표의 절반을 달성했습니다. {} / {}",
                    state->currentCount, state->targetCount));
        }

        if (!state->announcedFive && remaining == 5)
        {
            state->announcedFive = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 임무] {} 중 {}마리 남았습니다.",
                    state->targetCount, remaining));
        }

        if (!state->announcedThree && remaining == 3)
        {
            state->announcedThree = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 임무] {} 중 {}마리 남았습니다.",
                    state->targetCount, remaining));
        }

        if (!state->announcedOne && remaining == 1)
        {
            state->announcedOne = true;
            SendMissionMessageToGroup(
                killer,
                Acore::StringFormat(
                    "[추가 임무] {} 중 {}마리 남았습니다.",
                    state->targetCount, remaining));
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

        MissionDefinition const* definition = SelectMission(map->GetId());
        if (!definition)
            return;

        MissionState& state = MissionStateStore::Instance().Create(
            instanceId, *definition);

        SendMissionMessageToGroup(
            player,
            Acore::StringFormat(
                "[추가 임무] {}",
                state.announcement));
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