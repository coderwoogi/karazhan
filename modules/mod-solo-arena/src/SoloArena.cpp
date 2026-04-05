#include "ArenaScript.h"
#include "AllSpellScript.h"
#include "Battleground.h"
#include "BattlegroundAB.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Duration.h"
#include "GameObject.h"
#include "LFGMgr.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Pet.h"
#include "PathGenerator.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Spell.h"
#include "SpellAuraDefines.h"
#include "SpellMgr.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "UnitScript.h"
#include "WorldPacket.h"
#include "WorldStateDefines.h"
#include "WorldSession.h"

#include "../../mod-instance-bonus-mission/src/thirdparty/httplib.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr uint32 DEFAULT_ARENA_MAP_ID = 572;
    constexpr BattlegroundTypeId DEFAULT_ARENA_BG_TYPE = BATTLEGROUND_RL;
    constexpr uint32 DEFAULT_OBJECTIVE_MAP_ID = 529;
    constexpr BattlegroundTypeId DEFAULT_OBJECTIVE_BG_TYPE = BATTLEGROUND_AB;
    constexpr uint32 ACTION_STAGE_BASE = 100;
    constexpr uint32 ACTION_ABANDON = 500;
    constexpr uint32 SOLO_ARENA_PREPARATION_MS = 20000;
    constexpr uint32 DEFAULT_COMBAT_LIMIT_MS = 180000;
    constexpr uint32 DEFAULT_OBJECTIVE_LIMIT_MS = 1200000;
    constexpr uint32 OBJECTIVE_WIN_RESOURCES = 1600;
    constexpr uint32 OBJECTIVE_RESPAWN_DELAY_SEC = 15;
    constexpr uint32 OBJECTIVE_FLAG_USE_DELAY_SEC = 5;
    constexpr float OBJECTIVE_BASE_RUN_RATE = 1.0f;
    constexpr float OBJECTIVE_MOUNT_RUN_RATE = 2.0f;
    constexpr float OBJECTIVE_SPEED_BOX_BONUS = 0.6f;
    constexpr uint32 TRIAL_MECHANIC_GOOD_ENTRY = 178187;
    constexpr uint32 TRIAL_MECHANIC_BAD_ENTRY = 20352;
    constexpr uint32 TRIAL_HELPER_ENTRY = 190023;
    constexpr uint32 TRIAL_HAZARD_ENTRY = 190024;
    constexpr uint32 TRIAL_MECHANIC_BUFF_AURA = 32182;
    constexpr uint32 TRIAL_RANDOM_SPELL_KNOCK_AWAY = 36512;
    constexpr uint32 TRIAL_RANDOM_SPELL_KICK = 75448;
    constexpr uint32 TRIAL_RANDOM_SPELL_ROCKET_BOOTS = 51582;
    constexpr uint32 TRIAL_RANDOM_SPELL_PARACHUTE_BUFF = 44795;
    constexpr uint32 TRIAL_TICKET_ITEM = 600022;
    constexpr uint32 TRIAL_PATH_MARKER_ENTRY = 190025;
    constexpr uint32 TRIAL_DAILY_LIMIT = 5;
    constexpr uint8 MAX_STAGE_MECHANIC_SLOTS = 16;
    constexpr float TRIAL_MARKER_LINK_DISTANCE = 65.0f;
    constexpr uint32 TRIAL_MARKER_MAX_NEIGHBORS = 8;
    constexpr float TRIAL_NODE_MARKER_MAX_DISTANCE = 35.0f;
    constexpr float TRIAL_MARKER_ENDPOINT_DISTANCE = 80.0f;
    constexpr std::array<std::array<float, 4>, BG_AB_DYNAMIC_NODES_COUNT>
        TRIAL_AB_MECHANIC_POSITIONS =
    {{
        {{1185.71f, 1185.24f, -56.36f, 2.56f}},
        {{990.75f, 1008.18f, -42.60f, 2.43f}},
        {{817.66f, 843.34f, -56.54f, 3.01f}},
        {{807.46f, 1189.16f, 11.92f, 5.44f}},
        {{1146.62f, 816.94f, -98.49f, 6.14f}}
    }};
    constexpr std::array<std::array<float, 4>, 20> TRIAL_AB_ROUTE_POSITIONS =
    {{
        {{1354.0f, 1275.0f, -11.3f, 4.77f}},    // 0 alliance base
        {{1278.0f, 1234.0f, -13.5f, 4.20f}},    // 1 alliance gate
        {{1220.0f, 1210.0f, -34.0f, 3.60f}},    // 2 stables north road
        {{1176.0f, 1191.0f, -56.7f, 0.90f}},    // 3 stables approach
        {{1148.0f, 1154.0f, -48.0f, 2.70f}},    // 4 upper west road
        {{1087.0f, 1101.0f, -48.0f, 2.45f}},    // 5 upper center road
        {{1014.0f, 1115.0f, -39.0f, 3.10f}},    // 6 blacksmith north road
        {{1012.0f, 1072.0f, -39.5f, 4.10f}},    // 7 blacksmith east bridge
        {{989.5f, 1041.0f, -44.8f, -2.60f}},    // 8 blacksmith approach
        {{916.0f, 959.0f, -49.5f, 2.90f}},      // 9 lower center road
        {{861.0f, 909.0f, -53.0f, -2.70f}},     // 10 farm west road
        {{816.0f, 882.0f, -56.0f, -2.30f}},     // 11 farm approach
        {{955.0f, 1140.0f, -26.0f, 4.30f}},     // 12 lumber south road
        {{892.0f, 1146.0f, -3.0f, 4.80f}},      // 13 lumber bridge
        {{849.0f, 1138.0f, 11.1f, -2.30f}},     // 14 lumber approach
        {{1019.0f, 918.0f, -59.0f, 5.40f}},     // 15 gold north road
        {{1091.0f, 900.0f, -86.0f, 5.80f}},     // 16 gold bridge
        {{1138.0f, 858.0f, -110.9f, -0.73f}},   // 17 gold approach
        {{786.0f, 728.0f, -14.0f, 0.90f}},      // 18 horde gate
        {{714.6f, 646.1f, -10.9f, 4.34f}}       // 19 horde base
    }};
    constexpr std::array<uint8, BG_AB_DYNAMIC_NODES_COUNT>
        TRIAL_AB_NODE_ROUTE_IDS = {{ 3, 8, 11, 14, 17 }};
    constexpr std::array<uint8, 2> TRIAL_AB_START_ROUTE_IDS = {{ 0, 19 }};
    constexpr std::array<std::array<uint8, 5>, 20> TRIAL_AB_ROUTE_GRAPH =
    {{
        {{ 1, 255, 255, 255, 255 }},
        {{ 0, 2, 5, 255, 255 }},
        {{ 1, 3, 4, 255, 255 }},
        {{ 2, 255, 255, 255, 255 }},
        {{ 2, 5, 12, 255, 255 }},
        {{ 1, 4, 6, 9, 255 }},
        {{ 5, 7, 8, 255, 255 }},
        {{ 6, 8, 15, 255, 255 }},
        {{ 6, 7, 9, 255, 255 }},
        {{ 5, 8, 10, 15, 18 }},
        {{ 9, 11, 255, 255, 255 }},
        {{ 10, 255, 255, 255, 255 }},
        {{ 4, 13, 255, 255, 255 }},
        {{ 12, 14, 255, 255, 255 }},
        {{ 13, 255, 255, 255, 255 }},
        {{ 7, 9, 16, 255, 255 }},
        {{ 15, 17, 255, 255, 255 }},
        {{ 16, 255, 255, 255, 255 }},
        {{ 9, 19, 255, 255, 255 }},
        {{ 18, 255, 255, 255, 255 }}
    }};

    enum class StageMechanicType : uint8
    {
        None = 0,
        HealingSurge = 1,
        ShadowBurnHazard = 2,
        GuardianSummon = 3,
        SpeedBoost = 4,
        ReturnToStart = 5,
        LaunchAway = 6
    };

    enum class ArenaResult : uint8
    {
        None = 0,
        Victory = 1,
        Failure = 2,
        Abandoned = 3
    };

    enum class SessionState : uint8
    {
        PendingSpawn = 0,
        WaitingForStart = 1,
        Active = 2,
        PendingFinish = 3,
        AwaitingReturn = 4
    };

    enum class TrialScenario : uint8
    {
        Duel = 0,
        Objective = 1
    };

    enum class ObjectiveNodeOwner : uint8
    {
        Neutral = 0,
        Player = 1,
        Shadow = 2
    };

    enum CloneVisualSpells : uint32
    {
        SPELL_COPY_MAINHAND = 41055,
        SPELL_COPY_OFFHAND = 45206,
        SPELL_COPY_RANGED = 57593
    };

    enum ShadowEvents : uint32
    {
        EVENT_PRIMARY = 1,
        EVENT_SECONDARY = 2,
        EVENT_TERTIARY = 3,
        EVENT_DEFENSIVE = 4,
        EVENT_BURST = 5,
        EVENT_UTILITY = 6,
        EVENT_CC = 7,
        EVENT_AOE = 8
    };

    struct StageConfig
    {
        uint8 StageId = 0;
        std::string Name;
        uint32 ArenaMapId = DEFAULT_ARENA_MAP_ID;
        float PlayerX = 1294.74f;
        float PlayerY = 1584.50f;
        float PlayerZ = 31.62f;
        float PlayerO = 1.66f;
        float BotX = 1277.50f;
        float BotY = 1751.07f;
        float BotZ = 31.61f;
        float BotO = 4.70f;
        float HealthMultiplier = 1.0f;
        float DamageMultiplier = 1.0f;
        uint32 AttackTimeMs = 2000;
        uint32 SpellIntervalMs = 4000;
        float MoveSpeedRate = 1.0f;
        bool Enabled = true;
        uint32 PreparationMs = 6000;
    };

    struct ArenaSession
    {
        uint64 RunUid = 0;
        ObjectGuid PlayerGuid = ObjectGuid::Empty;
        ObjectGuid BotGuid = ObjectGuid::Empty;
        ObjectGuid PetGuid = ObjectGuid::Empty;
        uint8 StageId = 0;
        TrialScenario Scenario = TrialScenario::Duel;
        BattlegroundTypeId BgTypeId = DEFAULT_ARENA_BG_TYPE;
        TeamId Team = TEAM_NEUTRAL;
        uint32 ArenaMapId = DEFAULT_ARENA_MAP_ID;
        uint32 ArenaInstanceId = 0;
        uint32 ReturnMapId = 0;
        float ReturnX = 0.0f;
        float ReturnY = 0.0f;
        float ReturnZ = 0.0f;
        float ReturnO = 0.0f;
        uint64 StartedAt = 0;
        uint64 PreparationEndsAt = 0;
        uint64 CombatStartedAt = 0;
        uint64 CombatEndsAt = 0;
        uint64 EndedAt = 0;
        uint64 NextTimePayloadAt = 0;
        uint64 PlayerRespawnAt = 0;
        uint64 ShadowRespawnAt = 0;
        uint64 CompletedAt = 0;
        uint64 FailedAt = 0;
        uint64 AbandonedAt = 0;
        uint32 CombatDurationSec = 0;
        uint32 SpawnDelayMs = 1000;
        uint32 FinishDelayMs = 3000;
        uint64 NextMovementNormalizeAt = 0;
        bool ObjectiveReadyAnnounced = false;
        bool ObjectiveLinkNotified = false;
        bool ObjectiveIntroSent = false;
        bool ObjectiveAllNodesCaptured = false;
        std::array<ObjectiveNodeOwner, BG_AB_DYNAMIC_NODES_COUNT>
            ObjectiveNodeOwners = {};
        std::array<ObjectiveNodeOwner, BG_AB_DYNAMIC_NODES_COUNT>
            ObjectiveVisualOwners = {};
        std::array<uint8, BG_AB_DYNAMIC_NODES_COUNT>
            ObjectiveClientStates =
                {{ 255, 255, 255, 255, 255 }};
        int8 ShadowTargetNode = -1;
        int8 ShadowCurrentSector = -1;
        std::array<uint64, BG_AB_DYNAMIC_NODES_COUNT>
            ShadowNodeActivationEndsAt = {};
        std::array<uint64, BG_AB_DYNAMIC_NODES_COUNT>
            ShadowNodeCaptureEndsAt = {};
        uint64 NextShadowNodeUpdateAt = 0;
        std::vector<uint8> ShadowRoute;
        uint8 ShadowRouteIndex = 0;
        std::vector<G3D::Vector3> ShadowMarkerRoute;
        uint32 ShadowMarkerRouteIndex = 0;
        Movement::PointsArray ShadowGroundPath;
        int8 ShadowGroundPathNode = -1;
        uint64 NextShadowGroundRepathAt = 0;
        uint64 NextObjectiveNodeScanAt = 0;
        uint64 NextObjectiveWorldStateSyncAt = 0;
        uint64 NextObjectiveMechanicUpdateAt = 0;
        bool ObjectiveWorldStateDirty = true;
        std::array<uint64, PVP_TEAMS_COUNT> ObjectiveNextResourceTickAt = {};
        std::array<uint32, PVP_TEAMS_COUNT> ObjectiveResourceScores = {};
        std::array<uint8, PVP_TEAMS_COUNT> ObjectiveDisplayedBases =
            {{ 255, 255 }};
        uint32 PetEntry = 0;
        uint32 PetDisplayId = 0;
        std::array<ObjectGuid, MAX_STAGE_MECHANIC_SLOTS> MechanicGuids = {};
        std::unordered_map<uint64, uint64> ExternalMechanicCooldowns;
        ObjectGuid HelperGuid = ObjectGuid::Empty;
        ObjectGuid HazardGuid = ObjectGuid::Empty;
        std::array<uint64, MAX_STAGE_MECHANIC_SLOTS> NextMechanicSpawnAt = {};
        uint64 PlayerDamageBuffEndsAt = 0;
        uint64 ObjectiveSpeedBuffEndsAt = 0;
        uint8 RankValue = 0;
        std::string RankLabel;
        ArenaResult Result = ArenaResult::None;
        SessionState State = SessionState::PendingSpawn;
    };

    struct StageMechanicConfig
    {
        uint8 StageId = 0;
        uint8 SlotId = 0;
        StageMechanicType MechanicType = StageMechanicType::None;
        uint32 ObjectEntry = TRIAL_MECHANIC_GOOD_ENTRY;
        float SpawnX = 1285.81f;
        float SpawnY = 1667.90f;
        float SpawnZ = 39.96f;
        float SpawnO = 0.0f;
        uint32 SpawnIntervalMs = 20000;
        uint32 DurationMs = 20000;
        float EffectValue1 = 0.0f;
        float EffectValue2 = 0.0f;
        uint32 SummonEntry = 0;
        bool Enabled = true;
        std::string Name;
    };

    struct ShadowProfile
    {
        ObjectGuid PlayerGuid = ObjectGuid::Empty;
        uint8 StageId = 0;
        uint8 PlayerClass = CLASS_WARRIOR;
        uint32 ActiveSpec = 0;
        std::string PlayerName;
        uint8 PlayerRace = RACE_HUMAN;
        uint8 PlayerGender = GENDER_MALE;
        uint8 SheathState = 1;
        uint8 Level = 1;
        Powers PowerType = POWER_MANA;
        uint32 MaxHealth = 1;
        uint32 MaxPower = 0;
        float ObjectScale = 1.0f;
        float RunSpeedRate = 1.0f;
        float CastSpeedRate = 1.0f;
        float AverageItemLevel = 1.0f;
        int32 Armor = 0;
        int32 SpellPowerBonus = 0;
        std::array<int32, MAX_STATS> Stats = {};
        std::array<int32, MAX_SPELL_SCHOOL> Resistances = {};
        std::array<uint32, MAX_ATTACK> AttackTimeMs = {};
        std::array<float, MAX_ATTACK> WeaponMinDamage = {};
        std::array<float, MAX_ATTACK> WeaponMaxDamage = {};
        uint32 MainHandEntry = 0;
        uint32 OffHandEntry = 0;
        uint32 RangedEntry = 0;
        bool CanDualWield = false;
        float DamageMultiplier = 1.0f;
        uint32 SpellIntervalMs = 4000;
    };

    struct SpellPackage
    {
        uint32 PrimarySpell = 0;
        uint32 SecondarySpell = 0;
        uint32 TertiarySpell = 0;
        uint32 DefensiveSpell = 0;
        SpellSchoolMask PrimarySchool = SPELL_SCHOOL_MASK_NORMAL;
        SpellSchoolMask SecondarySchool = SPELL_SCHOOL_MASK_NORMAL;
        SpellSchoolMask TertiarySchool = SPELL_SCHOOL_MASK_NORMAL;
        SpellSchoolMask DefensiveSchool = SPELL_SCHOOL_MASK_NORMAL;
        float PrimaryDamageFactor = 1.0f;
        float SecondaryDamageFactor = 1.2f;
        float TertiaryDamageFactor = 1.05f;
        float DefensiveDamageFactor = 0.0f;
        float PrimaryRange = 30.0f;
        float SecondaryRange = 20.0f;
        float TertiaryRange = 20.0f;
        float DefensiveRange = 0.0f;
        uint32 PrimaryCooldownMs = 3500;
        uint32 SecondaryCooldownMs = 5200;
        uint32 TertiaryCooldownMs = 7000;
        uint32 DefensiveCooldownMs = 12000;
        float DefensiveHealthPct = 0.45f;
    };

    struct TacticalSpell
    {
        uint32 SpellId = 0;
        SpellSchoolMask School = SPELL_SCHOOL_MASK_NORMAL;
        float DamageFactor = 1.0f;
        float Range = 20.0f;
        uint32 CooldownMs = 8000;
        bool SelfCast = false;
        float SelfHealthPct = 1.0f;
        float TargetHealthPct = 1.0f;
        bool RequiresCastingTarget = false;
        bool RequiresNearbySecondary = false;
    };

    struct TacticalPackage
    {
        TacticalSpell Burst;
        TacticalSpell Utility;
        TacticalSpell CrowdControl;
        TacticalSpell Area;
    };

    bool IsObjectiveTrialStage(uint8 stageId)
    {
        return stageId >= 4 && stageId <= 6;
    }

    std::string GetObjectiveNodeName(uint8 nodeId)
    {
        switch (nodeId)
        {
            case BG_AB_NODE_STABLES:
                return "마굿간";
            case BG_AB_NODE_BLACKSMITH:
                return "대장간";
            case BG_AB_NODE_FARM:
                return "농장";
            case BG_AB_NODE_LUMBER_MILL:
                return "제재소";
            case BG_AB_NODE_GOLD_MINE:
                return "금광";
            default:
                return "거점";
        }
    }

    void GetObjectiveStartLocation(TeamId teamId, float& x, float& y,
        float& z, float& o)
    {
        uint8 index = teamId == TEAM_HORDE ?
            BG_AB_SPIRIT_HORDE : BG_AB_SPIRIT_ALIANCE;
        x = BG_AB_SpiritGuidePos[index][0];
        y = BG_AB_SpiritGuidePos[index][1];
        z = BG_AB_SpiritGuidePos[index][2];
        o = BG_AB_SpiritGuidePos[index][3];
    }

    void GetObjectiveShadowLocation(float& x, float& y, float& z, float& o)
    {
        x = BG_AB_NodePositions[BG_AB_NODE_BLACKSMITH][0];
        y = BG_AB_NodePositions[BG_AB_NODE_BLACKSMITH][1];
        z = BG_AB_NodePositions[BG_AB_NODE_BLACKSMITH][2];
        o = BG_AB_NodePositions[BG_AB_NODE_BLACKSMITH][3];
    }

    void GetObjectiveNodeApproachLocation(uint8 nodeId, float& x, float& y,
        float& z, float& o)
    {
        if (nodeId >= BG_AB_DYNAMIC_NODES_COUNT)
            nodeId = 0;

        uint8 routeId = TRIAL_AB_NODE_ROUTE_IDS[nodeId];
        x = TRIAL_AB_ROUTE_POSITIONS[routeId][0];
        y = TRIAL_AB_ROUTE_POSITIONS[routeId][1];
        z = TRIAL_AB_ROUTE_POSITIONS[routeId][2];
        o = TRIAL_AB_ROUTE_POSITIONS[routeId][3];
    }

    uint8 CountObjectiveNodesOwned(
        std::array<ObjectiveNodeOwner, BG_AB_DYNAMIC_NODES_COUNT> const& owners,
        ObjectiveNodeOwner owner)
    {
        uint8 count = 0;
        for (ObjectiveNodeOwner current : owners)
            if (current == owner)
                ++count;
        return count;
    }

    TeamId GetObjectiveShadowTeam(TeamId playerTeam)
    {
        return Battleground::GetOtherTeamId(playerTeam);
    }

    uint32 GetObjectiveOccupiedBannerType(uint8 node, TeamId teamId)
    {
        return node * BG_AB_OBJECTS_PER_NODE +
            (teamId == TEAM_ALLIANCE ?
                BG_AB_OBJECT_BANNER_ALLY :
                BG_AB_OBJECT_BANNER_HORDE);
    }

    uint32 GetObjectiveOccupiedAuraType(uint8 node, TeamId teamId)
    {
        return node * BG_AB_OBJECTS_PER_NODE +
            (teamId == TEAM_ALLIANCE ?
                BG_AB_OBJECT_AURA_ALLY :
                BG_AB_OBJECT_AURA_HORDE);
    }

    void ApplyObjectiveNodeVisual(BattlegroundAB* bg, uint8 node,
        ObjectiveNodeOwner owner, TeamId playerTeam)
    {
        if (!bg || node >= BG_AB_DYNAMIC_NODES_COUNT)
            return;

        for (uint8 offset = 0; offset < BG_AB_OBJECTS_PER_NODE; ++offset)
            bg->SpawnBGObject(node * BG_AB_OBJECTS_PER_NODE + offset,
                RESPAWN_ONE_DAY);

        if (owner == ObjectiveNodeOwner::Neutral)
        {
            bg->SpawnBGObject(node * BG_AB_OBJECTS_PER_NODE +
                BG_AB_OBJECT_BANNER_NEUTRAL, RESPAWN_IMMEDIATELY);
            return;
        }

        TeamId visualTeam = owner == ObjectiveNodeOwner::Player ?
            playerTeam : GetObjectiveShadowTeam(playerTeam);

        bg->SpawnBGObject(GetObjectiveOccupiedBannerType(node, visualTeam),
            RESPAWN_IMMEDIATELY);
        bg->SpawnBGObject(GetObjectiveOccupiedAuraType(node, visualTeam),
            RESPAWN_IMMEDIATELY);
    }

    void SyncObjectiveNodeVisuals(ArenaSession& session, BattlegroundAB* bg)
    {
        if (!bg || session.Team == TEAM_NEUTRAL)
            return;

        for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        {
            if (session.ObjectiveVisualOwners[node] ==
                session.ObjectiveNodeOwners[node])
                continue;

            ApplyObjectiveNodeVisual(bg, node,
                session.ObjectiveNodeOwners[node], session.Team);
            session.ObjectiveVisualOwners[node] =
                session.ObjectiveNodeOwners[node];
        }
    }

    GameObject* GetObjectiveNodeBanner(BattlegroundAB* bg, uint8 node)
    {
        if (!bg || node >= BG_AB_DYNAMIC_NODES_COUNT)
            return nullptr;

        static constexpr std::array<uint32, 5> bannerOffsets =
        {
            BG_AB_OBJECT_BANNER_NEUTRAL,
            BG_AB_OBJECT_BANNER_ALLY,
            BG_AB_OBJECT_BANNER_HORDE,
            BG_AB_OBJECT_BANNER_CONT_A,
            BG_AB_OBJECT_BANNER_CONT_H
        };

        for (uint32 offset : bannerOffsets)
        {
            if (GameObject* go = bg->GetBGObject(
                node * BG_AB_OBJECTS_PER_NODE + offset))
            {
                if (go->isSpawned())
                    return go;
            }
        }

        return nullptr;
    }

    uint32 GetObjectiveNodeIconWorldState(uint8 node)
    {
        switch (node)
        {
            case BG_AB_NODE_STABLES:
                return WORLD_STATE_BATTLEGROUND_AB_STABLE_ICON;
            case BG_AB_NODE_BLACKSMITH:
                return WORLD_STATE_BATTLEGROUND_AB_BLACKSMITH_ICON;
            case BG_AB_NODE_FARM:
                return WORLD_STATE_BATTLEGROUND_AB_FARM_ICON;
            case BG_AB_NODE_LUMBER_MILL:
                return WORLD_STATE_BATTLEGROUND_AB_LUMBERMILL_ICON;
            case BG_AB_NODE_GOLD_MINE:
                return WORLD_STATE_BATTLEGROUND_AB_GOLDMINE_ICON;
            default:
                return 0;
        }
    }

    uint32 GetObjectiveNodeStateWorldStateBase(uint8 node)
    {
        switch (node)
        {
            case BG_AB_NODE_STABLES:
                return WORLD_STATE_BATTLEGROUND_AB_STABLE_STATE_ALLIANCE;
            case BG_AB_NODE_BLACKSMITH:
                return WORLD_STATE_BATTLEGROUND_AB_BLACKSMITH_STATE_ALLIANCE;
            case BG_AB_NODE_FARM:
                return WORLD_STATE_BATTLEGROUND_AB_FARM_STATE_ALLIANCE;
            case BG_AB_NODE_LUMBER_MILL:
                return WORLD_STATE_BATTLEGROUND_AB_LUMBERMILL_STATE_ALLIANCE;
            case BG_AB_NODE_GOLD_MINE:
                return WORLD_STATE_BATTLEGROUND_AB_GOLDMINE_STATE_ALLIANCE;
            default:
                return 0;
        }
    }

    uint8 GetObjectiveNodeStateValue(ObjectiveNodeOwner owner, TeamId playerTeam,
        bool capturing)
    {
        if (owner == ObjectiveNodeOwner::Neutral)
            return BG_AB_NODE_STATE_NEUTRAL;

        TeamId nodeTeam = owner == ObjectiveNodeOwner::Player ? playerTeam :
            GetObjectiveShadowTeam(playerTeam);
        if (capturing)
            return nodeTeam == TEAM_ALLIANCE ?
                BG_AB_NODE_STATE_ALLY_CONTESTED :
                BG_AB_NODE_STATE_HORDE_CONTESTED;

        return nodeTeam == TEAM_ALLIANCE ?
            BG_AB_NODE_STATE_ALLY_OCCUPIED :
            BG_AB_NODE_STATE_HORDE_OCCUPIED;
    }

    bool IsObjectiveNodeShadowCapturing(ArenaSession const& session, uint8 node,
        uint64 now)
    {
        if (node >= BG_AB_DYNAMIC_NODES_COUNT)
            return false;

        return session.ShadowNodeCaptureEndsAt[node] != 0 &&
            now < session.ShadowNodeCaptureEndsAt[node];
    }

    void SyncObjectiveWorldStates(Player* player, ArenaSession& session,
        BattlegroundAB* bg, uint64 now)
    {
        if (!player || !bg || session.Team == TEAM_NEUTRAL)
            return;

        std::array<uint8, PVP_TEAMS_COUNT> controlledBases = {};

        for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        {
            bool capturing = IsObjectiveNodeShadowCapturing(session, node, now);
            ObjectiveNodeOwner owner = session.ObjectiveNodeOwners[node];
            if (capturing)
                owner = ObjectiveNodeOwner::Shadow;

            uint8 state = GetObjectiveNodeStateValue(owner, session.Team,
                capturing);
            uint32 iconState = GetObjectiveNodeIconWorldState(node);
            uint32 stateBase = GetObjectiveNodeStateWorldStateBase(node);
            if (!iconState || !stateBase)
                continue;

            if (session.ObjectiveClientStates[node] != state)
            {
                bg->UpdateWorldState(iconState,
                    state == BG_AB_NODE_STATE_NEUTRAL ? 1 : 0);
                for (uint8 i = BG_AB_NODE_STATE_ALLY_OCCUPIED;
                     i <= BG_AB_NODE_STATE_HORDE_CONTESTED; ++i)
                {
                    bg->UpdateWorldState(stateBase + i - 1, state == i ? 1 : 0);
                }
                session.ObjectiveClientStates[node] = state;
            }

            if (capturing)
                continue;

            if (owner == ObjectiveNodeOwner::Player)
            {
                if (session.Team == TEAM_ALLIANCE)
                    ++controlledBases[TEAM_ALLIANCE];
                else
                    ++controlledBases[TEAM_HORDE];
            }
            else if (owner == ObjectiveNodeOwner::Shadow)
            {
                if (session.Team == TEAM_ALLIANCE)
                    ++controlledBases[TEAM_HORDE];
                else
                    ++controlledBases[TEAM_ALLIANCE];
            }
        }

        for (uint8 teamIndex = TEAM_ALLIANCE; teamIndex < PVP_TEAMS_COUNT;
             ++teamIndex)
        {
            if (session.ObjectiveDisplayedBases[teamIndex] !=
                controlledBases[teamIndex])
            {
                bg->UpdateWorldState(
                    teamIndex == TEAM_ALLIANCE ?
                        WORLD_STATE_BATTLEGROUND_AB_OCCUPIED_BASES_ALLIANCE :
                        WORLD_STATE_BATTLEGROUND_AB_OCCUPIED_BASES_HORDE,
                    controlledBases[teamIndex]);
                session.ObjectiveDisplayedBases[teamIndex] =
                    controlledBases[teamIndex];
                session.ObjectiveNextResourceTickAt[teamIndex] = now +
                    BG_AB_TickIntervals[std::min<uint8>(
                        controlledBases[teamIndex],
                        BG_AB_DYNAMIC_NODES_COUNT)].count() / IN_MILLISECONDS;
            }
        }

        for (uint8 teamIndex = TEAM_ALLIANCE; teamIndex < PVP_TEAMS_COUNT;
             ++teamIndex)
        {
            uint8 bases = controlledBases[teamIndex];
            if (bases == 0)
                continue;

            if (session.ObjectiveNextResourceTickAt[teamIndex] == 0)
            {
                session.ObjectiveNextResourceTickAt[teamIndex] = now +
                    BG_AB_TickIntervals[bases].count() / IN_MILLISECONDS;
                continue;
            }

            if (now < session.ObjectiveNextResourceTickAt[teamIndex])
                continue;

            session.ObjectiveResourceScores[teamIndex] += BG_AB_TickPoints[bases];
            bg->UpdateWorldState(
                teamIndex == TEAM_ALLIANCE ?
                    WORLD_STATE_BATTLEGROUND_AB_RESOURCES_ALLIANCE :
                    WORLD_STATE_BATTLEGROUND_AB_RESOURCES_HORDE,
                session.ObjectiveResourceScores[teamIndex]);
            session.ObjectiveNextResourceTickAt[teamIndex] = now +
                BG_AB_TickIntervals[bases].count() / IN_MILLISECONDS;
        }
    }

    void GetRandomObjectiveFlagLocation(float& x, float& y, float& z, float& o)
    {
        uint8 node = urand(0, BG_AB_DYNAMIC_NODES_COUNT - 1);
        GetObjectiveNodeApproachLocation(node, x, y, z, o);
        float angle = frand(0.0f, float(M_PI) * 2.0f);
        float distance = frand(1.0f, 3.0f);
        x += std::cos(angle) * distance;
        y += std::sin(angle) * distance;
        o = Position::NormalizeOrientation(std::atan2(
            BG_AB_NodePositions[node][1] - y,
            BG_AB_NodePositions[node][0] - x));
    }

    bool GetObjectiveTravelPoint(float currentX, float currentY,
        uint8 targetNode, float& x, float& y, float& z)
    {
        auto distance2d = [](float ax, float ay, float bx, float by)
        {
            float dx = ax - bx;
            float dy = ay - by;
            return std::sqrt((dx * dx) + (dy * dy));
        };

        uint8 startRoute = 0;
        float bestStartDistance = std::numeric_limits<float>::max();
        for (uint8 i = 0; i < TRIAL_AB_ROUTE_POSITIONS.size(); ++i)
        {
            float distance = distance2d(currentX, currentY,
                TRIAL_AB_ROUTE_POSITIONS[i][0], TRIAL_AB_ROUTE_POSITIONS[i][1]);
            if (distance < bestStartDistance)
            {
                bestStartDistance = distance;
                startRoute = i;
            }
        }

        uint8 targetRoute = TRIAL_AB_NODE_ROUTE_IDS[targetNode];
        if (startRoute == targetRoute || bestStartDistance <= 6.0f)
            return false;

        std::array<int8, TRIAL_AB_ROUTE_POSITIONS.size()> parent = {};
        parent.fill(-1);
        std::array<bool, TRIAL_AB_ROUTE_POSITIONS.size()> visited = {};
        std::vector<uint8> queue;
        queue.push_back(startRoute);
        visited[startRoute] = true;

        for (size_t idx = 0; idx < queue.size(); ++idx)
        {
            uint8 current = queue[idx];
            if (current == targetRoute)
                break;

            for (uint8 next : TRIAL_AB_ROUTE_GRAPH[current])
            {
                if (next == 255 || visited[next])
                    continue;

                visited[next] = true;
                parent[next] = int8(current);
                queue.push_back(next);
            }
        }

        if (!visited[targetRoute])
            return false;

        std::vector<uint8> route;
        for (int8 node = int8(targetRoute); node >= 0; node = parent[node])
            route.push_back(uint8(node));
        std::reverse(route.begin(), route.end());

        if (route.size() <= 1)
            return false;

        uint8 nextRoute = route[1];
        x = TRIAL_AB_ROUTE_POSITIONS[nextRoute][0];
        y = TRIAL_AB_ROUTE_POSITIONS[nextRoute][1];
        z = TRIAL_AB_ROUTE_POSITIONS[nextRoute][2];
        return true;
    }

    uint8 GetNearestObjectiveRouteIndex(float x, float y)
    {
        float bestDistance = std::numeric_limits<float>::max();
        uint8 bestIndex = 0;
        for (uint8 i = 0; i < TRIAL_AB_ROUTE_POSITIONS.size(); ++i)
        {
            float distance = std::hypot(TRIAL_AB_ROUTE_POSITIONS[i][0] - x,
                TRIAL_AB_ROUTE_POSITIONS[i][1] - y);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestIndex = i;
            }
        }

        return bestIndex;
    }

    uint8 GetNearestObjectiveLandSector(float x, float y)
    {
        float bestDistance = std::numeric_limits<float>::max();
        uint8 bestSector = 0;
        for (uint8 sector = 0; sector < TRIAL_AB_START_ROUTE_IDS.size() +
            BG_AB_DYNAMIC_NODES_COUNT; ++sector)
        {
            uint8 routeIndex = 0;
            if (sector == 0)
                routeIndex = TRIAL_AB_START_ROUTE_IDS[0];
            else if (sector == 6)
                routeIndex = TRIAL_AB_START_ROUTE_IDS[1];
            else
                routeIndex = TRIAL_AB_NODE_ROUTE_IDS[sector - 1];

            float distance = std::hypot(TRIAL_AB_ROUTE_POSITIONS[routeIndex][0] - x,
                TRIAL_AB_ROUTE_POSITIONS[routeIndex][1] - y);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestSector = sector;
            }
        }

        return bestSector;
    }

    uint8 GetObjectiveTargetSector(uint8 targetNode)
    {
        switch (targetNode)
        {
            case BG_AB_NODE_STABLES:
                return 1;
            case BG_AB_NODE_BLACKSMITH:
                return 2;
            case BG_AB_NODE_FARM:
                return 3;
            case BG_AB_NODE_LUMBER_MILL:
                return 4;
            case BG_AB_NODE_GOLD_MINE:
                return 5;
            default:
                return 2;
        }
    }

    void AppendObjectiveRouteIds(std::vector<uint8>& route,
        std::initializer_list<uint8> routeIds)
    {
        for (uint8 routeId : routeIds)
        {
            if (route.empty() || route.back() != routeId)
                route.push_back(routeId);
        }
    }

    bool AppendObjectiveLandSegment(uint8 fromSector, uint8 toSector,
        std::vector<uint8>& route)
    {
        if (fromSector == toSector)
            return true;

        if (fromSector == 0 && toSector == 1)
        {
            AppendObjectiveRouteIds(route, { 1, 2, 3 });
            return true;
        }
        if (fromSector == 1 && toSector == 0)
        {
            AppendObjectiveRouteIds(route, { 2, 1, 0 });
            return true;
        }
        if (fromSector == 0 && toSector == 2)
        {
            AppendObjectiveRouteIds(route, { 1, 5, 6, 7, 8 });
            return true;
        }
        if (fromSector == 2 && toSector == 0)
        {
            AppendObjectiveRouteIds(route, { 7, 6, 5, 1, 0 });
            return true;
        }
        if (fromSector == 1 && toSector == 2)
        {
            AppendObjectiveRouteIds(route, { 4, 5, 6, 7, 8 });
            return true;
        }
        if (fromSector == 2 && toSector == 1)
        {
            AppendObjectiveRouteIds(route, { 6, 5, 4, 3 });
            return true;
        }
        if (fromSector == 1 && toSector == 4)
        {
            AppendObjectiveRouteIds(route, { 4, 12, 13, 14 });
            return true;
        }
        if (fromSector == 4 && toSector == 1)
        {
            AppendObjectiveRouteIds(route, { 13, 12, 4, 3 });
            return true;
        }
        if (fromSector == 2 && toSector == 4)
        {
            AppendObjectiveRouteIds(route, { 5, 12, 13, 14 });
            return true;
        }
        if (fromSector == 4 && toSector == 2)
        {
            AppendObjectiveRouteIds(route, { 13, 12, 5, 6, 7, 8 });
            return true;
        }
        if (fromSector == 2 && toSector == 3)
        {
            AppendObjectiveRouteIds(route, { 9, 10, 11 });
            return true;
        }
        if (fromSector == 3 && toSector == 2)
        {
            AppendObjectiveRouteIds(route, { 10, 9, 8 });
            return true;
        }
        if (fromSector == 2 && toSector == 5)
        {
            AppendObjectiveRouteIds(route, { 7, 15, 16, 17 });
            return true;
        }
        if (fromSector == 5 && toSector == 2)
        {
            AppendObjectiveRouteIds(route, { 16, 15, 7, 8 });
            return true;
        }
        if (fromSector == 6 && toSector == 3)
        {
            AppendObjectiveRouteIds(route, { 18, 10, 11 });
            return true;
        }
        if (fromSector == 3 && toSector == 6)
        {
            AppendObjectiveRouteIds(route, { 10, 18, 19 });
            return true;
        }
        if (fromSector == 6 && toSector == 5)
        {
            AppendObjectiveRouteIds(route, { 18, 9, 15, 16, 17 });
            return true;
        }
        if (fromSector == 5 && toSector == 6)
        {
            AppendObjectiveRouteIds(route, { 16, 15, 9, 18, 19 });
            return true;
        }
        if (fromSector == 6 && toSector == 2)
        {
            AppendObjectiveRouteIds(route, { 18, 9, 8 });
            return true;
        }
        if (fromSector == 2 && toSector == 6)
        {
            AppendObjectiveRouteIds(route, { 9, 18, 19 });
            return true;
        }

        return false;
    }

    bool BuildObjectiveTravelRouteFromSector(uint8 startSector,
        uint8 targetNode, std::vector<uint8>& route)
    {
        route.clear();

        if (targetNode >= BG_AB_DYNAMIC_NODES_COUNT || startSector > 6)
            return false;

        uint8 targetSector = GetObjectiveTargetSector(targetNode);
        if (startSector == targetSector)
        {
            route.push_back(TRIAL_AB_NODE_ROUTE_IDS[targetNode]);
            return true;
        }
        switch (startSector)
        {
            case 0: // alliance base
                switch (targetNode)
                {
                    case BG_AB_NODE_STABLES:
                        AppendObjectiveRouteIds(route, { 1, 2, 3 });
                        return true;
                    case BG_AB_NODE_BLACKSMITH:
                        AppendObjectiveRouteIds(route, { 1, 5, 6, 7, 8 });
                        return true;
                    case BG_AB_NODE_FARM:
                        AppendObjectiveRouteIds(route, { 1, 5, 9, 10, 11 });
                        return true;
                    case BG_AB_NODE_LUMBER_MILL:
                        AppendObjectiveRouteIds(route, { 1, 4, 12, 13, 14 });
                        return true;
                    case BG_AB_NODE_GOLD_MINE:
                        AppendObjectiveRouteIds(route,
                            { 1, 5, 7, 15, 16, 17 });
                        return true;
                }
                break;
            case 1: // stables
                switch (targetNode)
                {
                    case BG_AB_NODE_BLACKSMITH:
                        AppendObjectiveRouteIds(route, { 4, 5, 6, 7, 8 });
                        return true;
                    case BG_AB_NODE_FARM:
                        AppendObjectiveRouteIds(route, { 4, 5, 9, 10, 11 });
                        return true;
                    case BG_AB_NODE_LUMBER_MILL:
                        AppendObjectiveRouteIds(route, { 4, 12, 13, 14 });
                        return true;
                    case BG_AB_NODE_GOLD_MINE:
                        AppendObjectiveRouteIds(route,
                            { 4, 5, 7, 15, 16, 17 });
                        return true;
                }
                break;
            case 2: // blacksmith
                switch (targetNode)
                {
                    case BG_AB_NODE_STABLES:
                        AppendObjectiveRouteIds(route, { 6, 5, 4, 3 });
                        return true;
                    case BG_AB_NODE_FARM:
                        AppendObjectiveRouteIds(route, { 9, 10, 11 });
                        return true;
                    case BG_AB_NODE_LUMBER_MILL:
                        AppendObjectiveRouteIds(route, { 6, 5, 12, 13, 14 });
                        return true;
                    case BG_AB_NODE_GOLD_MINE:
                        AppendObjectiveRouteIds(route, { 7, 15, 16, 17 });
                        return true;
                }
                break;
            case 3: // farm
                switch (targetNode)
                {
                    case BG_AB_NODE_STABLES:
                        AppendObjectiveRouteIds(route,
                            { 10, 9, 5, 4, 3 });
                        return true;
                    case BG_AB_NODE_BLACKSMITH:
                        AppendObjectiveRouteIds(route, { 10, 9, 8 });
                        return true;
                    case BG_AB_NODE_LUMBER_MILL:
                        AppendObjectiveRouteIds(route,
                            { 10, 9, 5, 12, 13, 14 });
                        return true;
                    case BG_AB_NODE_GOLD_MINE:
                        AppendObjectiveRouteIds(route,
                            { 10, 9, 15, 16, 17 });
                        return true;
                }
                break;
            case 4: // lumber mill
                switch (targetNode)
                {
                    case BG_AB_NODE_STABLES:
                        AppendObjectiveRouteIds(route, { 13, 12, 4, 3 });
                        return true;
                    case BG_AB_NODE_BLACKSMITH:
                        AppendObjectiveRouteIds(route,
                            { 13, 12, 5, 6, 7, 8 });
                        return true;
                    case BG_AB_NODE_FARM:
                        AppendObjectiveRouteIds(route,
                            { 13, 12, 5, 9, 10, 11 });
                        return true;
                    case BG_AB_NODE_GOLD_MINE:
                        AppendObjectiveRouteIds(route,
                            { 13, 12, 5, 7, 15, 16, 17 });
                        return true;
                }
                break;
            case 5: // gold mine
                switch (targetNode)
                {
                    case BG_AB_NODE_STABLES:
                        AppendObjectiveRouteIds(route,
                            { 16, 15, 7, 6, 5, 4, 3 });
                        return true;
                    case BG_AB_NODE_BLACKSMITH:
                        AppendObjectiveRouteIds(route, { 16, 15, 7, 8 });
                        return true;
                    case BG_AB_NODE_FARM:
                        AppendObjectiveRouteIds(route,
                            { 16, 15, 9, 10, 11 });
                        return true;
                    case BG_AB_NODE_LUMBER_MILL:
                        AppendObjectiveRouteIds(route,
                            { 16, 15, 7, 6, 5, 12, 13, 14 });
                        return true;
                }
                break;
            case 6: // horde base
                switch (targetNode)
                {
                    case BG_AB_NODE_STABLES:
                        AppendObjectiveRouteIds(route,
                            { 18, 9, 5, 4, 3 });
                        return true;
                    case BG_AB_NODE_BLACKSMITH:
                        AppendObjectiveRouteIds(route, { 18, 9, 8 });
                        return true;
                    case BG_AB_NODE_FARM:
                        AppendObjectiveRouteIds(route, { 18, 10, 11 });
                        return true;
                    case BG_AB_NODE_LUMBER_MILL:
                        AppendObjectiveRouteIds(route,
                            { 18, 9, 5, 12, 13, 14 });
                        return true;
                    case BG_AB_NODE_GOLD_MINE:
                        AppendObjectiveRouteIds(route,
                            { 18, 9, 15, 16, 17 });
                        return true;
                }
                break;
        }

        return false;
    }

    bool BuildObjectiveTravelRoute(float currentX, float currentY,
        uint8 targetNode, std::vector<uint8>& route)
    {
        uint8 startSector = GetNearestObjectiveLandSector(currentX, currentY);
        return BuildObjectiveTravelRouteFromSector(startSector, targetNode,
            route);
    }

    float ResolveArenaGroundZ(Map* map, float x, float y, float fallbackZ);

    bool BuildObjectiveGroundPath(Creature* bot, float destX, float destY,
        float destZ, Movement::PointsArray& pathPoints)
    {
        pathPoints.clear();

        if (!bot || !bot->GetMap())
            return false;

        destZ = ResolveArenaGroundZ(bot->GetMap(), destX, destY, destZ);

        PathGenerator path(bot);
        path.SetUseStraightPath(false);
        path.SetSlopeCheck(true);

        if (!path.CalculatePath(destX, destY, destZ, true))
            return false;

        PathType pathType = path.GetPathType();
        if (pathType & (PATHFIND_NOPATH | PATHFIND_NOT_USING_PATH |
                        PATHFIND_SHORTCUT))
        {
            return false;
        }

        Movement::PointsArray const& generatedPath = path.GetPath();
        if (generatedPath.size() < 2)
            return false;

        if (path.IsWaterPath(generatedPath))
            return false;

        pathPoints = generatedPath;
        return true;
    }

    struct ObjectivePathMarker
    {
        uint32 SpawnId = 0;
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
    };

    struct ObjectiveMarkerGraph
    {
        std::vector<ObjectivePathMarker> Markers;
        std::vector<std::vector<std::pair<size_t, float>>> Links;
    };

    std::vector<ObjectivePathMarker> GetObjectivePathMarkers()
    {
        std::vector<ObjectivePathMarker> markers;

        CreatureDataContainer const& allCreatures =
            sObjectMgr->GetAllCreatureData();
        for (auto const& [spawnId, data] : allCreatures)
        {
            if (data.id1 != TRIAL_PATH_MARKER_ENTRY ||
                data.mapid != DEFAULT_OBJECTIVE_MAP_ID)
            {
                continue;
            }

            markers.push_back({
                spawnId,
                data.posX,
                data.posY,
                data.posZ
            });
        }

        return markers;
    }

    int32 GetNearestObjectiveMarkerIndex(
        std::vector<ObjectivePathMarker> const& markers,
        float x, float y)
    {
        float bestDistance = std::numeric_limits<float>::max();
        int32 bestIndex = -1;

        for (size_t i = 0; i < markers.size(); ++i)
        {
            float dx = x - markers[i].X;
            float dy = y - markers[i].Y;
            float distance = std::sqrt(dx * dx + dy * dy);
            if (distance >= bestDistance)
                continue;

            bestDistance = distance;
            bestIndex = int32(i);
        }

        return bestIndex;
    }

    int32 GetNearestObjectiveMarkerIndexWithin(
        std::vector<ObjectivePathMarker> const& markers,
        float x, float y, float maxDistance)
    {
        int32 bestIndex = GetNearestObjectiveMarkerIndex(markers, x, y);
        if (bestIndex < 0)
            return -1;

        float dx = x - markers[size_t(bestIndex)].X;
        float dy = y - markers[size_t(bestIndex)].Y;
        float distance = std::sqrt(dx * dx + dy * dy);
        return distance <= maxDistance ? bestIndex : -1;
    }

    bool IsObjectiveMarkerLinkOnLand(Map* map,
        ObjectivePathMarker const& a,
        ObjectivePathMarker const& b)
    {
        if (!map)
            return true;

        constexpr uint32 sampleCount = 8;
        for (uint32 step = 1; step < sampleCount; ++step)
        {
            float t = float(step) / float(sampleCount);
            float sampleX = a.X + ((b.X - a.X) * t);
            float sampleY = a.Y + ((b.Y - a.Y) * t);
            float sampleZ = a.Z + ((b.Z - a.Z) * t);
            float groundZ = ResolveArenaGroundZ(map, sampleX, sampleY, sampleZ);
            if (map->IsInWater(1, sampleX, sampleY, groundZ + 1.0f, 2.0f))
                return false;
        }

        return true;
    }

    ObjectiveMarkerGraph BuildObjectiveMarkerGraph()
    {
        ObjectiveMarkerGraph graph;
        graph.Markers = GetObjectivePathMarkers();
        Map* baseMap = sMapMgr->CreateBaseMap(DEFAULT_OBJECTIVE_MAP_ID);

        size_t const markerCount = graph.Markers.size();
        graph.Links.resize(markerCount);

        auto linkNodes = [&](size_t a, size_t b)
        {
            float dx = graph.Markers[a].X - graph.Markers[b].X;
            float dy = graph.Markers[a].Y - graph.Markers[b].Y;
            float dz = graph.Markers[a].Z - graph.Markers[b].Z;
            float cost = std::sqrt(dx * dx + dy * dy + dz * dz);
            graph.Links[a].push_back({ b, cost });
            graph.Links[b].push_back({ a, cost });
        };

        std::set<std::pair<size_t, size_t>> linkedPairs;
        for (size_t i = 0; i < markerCount; ++i)
        {
            std::vector<std::pair<float, size_t>> neighbors;
            neighbors.reserve(markerCount);

            for (size_t j = 0; j < markerCount; ++j)
            {
                if (i == j)
                    continue;

                float dx = graph.Markers[i].X - graph.Markers[j].X;
                float dy = graph.Markers[i].Y - graph.Markers[j].Y;
                float dz = std::fabs(graph.Markers[i].Z - graph.Markers[j].Z);
                float distance2d = std::sqrt(dx * dx + dy * dy);
                if (distance2d > TRIAL_MARKER_LINK_DISTANCE || dz > 18.0f)
                    continue;

                neighbors.push_back({ distance2d, j });
            }

            std::sort(neighbors.begin(), neighbors.end(),
                [](auto const& left, auto const& right)
                {
                    return left.first < right.first;
                });

            uint32 linked = 0;
            for (auto const& [_, j] : neighbors)
            {
                size_t a = std::min(i, j);
                size_t b = std::max(i, j);
                if (!linkedPairs.insert({ a, b }).second)
                    continue;

                if (!IsObjectiveMarkerLinkOnLand(baseMap,
                    graph.Markers[i], graph.Markers[j]))
                {
                    continue;
                }

                linkNodes(i, j);
                ++linked;
                if (linked >= TRIAL_MARKER_MAX_NEIGHBORS)
                    break;
            }
        }

        return graph;
    }

    bool BuildObjectiveMarkerIndexRoute(ObjectiveMarkerGraph const& graph,
        int32 startIndex, int32 endIndex, std::vector<size_t>& indexRoute,
        float* outCost = nullptr)
    {
        indexRoute.clear();

        if (startIndex < 0 || endIndex < 0)
            return false;

        size_t const markerCount = graph.Markers.size();
        if (size_t(startIndex) >= markerCount || size_t(endIndex) >= markerCount)
            return false;

        if (startIndex == endIndex)
        {
            indexRoute.push_back(size_t(startIndex));
            if (outCost)
                *outCost = 0.0f;
            return true;
        }

        std::vector<float> dist(markerCount,
            std::numeric_limits<float>::max());
        std::vector<int32> prev(markerCount, -1);
        using QueueEntry = std::pair<float, size_t>;
        std::priority_queue<QueueEntry, std::vector<QueueEntry>,
            std::greater<QueueEntry>> queue;

        dist[size_t(startIndex)] = 0.0f;
        queue.push({ 0.0f, size_t(startIndex) });

        while (!queue.empty())
        {
            auto const [currentCost, current] = queue.top();
            queue.pop();

            if (currentCost > dist[current])
                continue;

            if (current == size_t(endIndex))
                break;

            for (auto const& edge : graph.Links[current])
            {
                size_t next = edge.first;
                float nextCost = currentCost + edge.second;
                if (nextCost >= dist[next])
                    continue;

                dist[next] = nextCost;
                prev[next] = int32(current);
                queue.push({ nextCost, next });
            }
        }

        if (prev[size_t(endIndex)] < 0)
            return false;

        for (int32 cursor = endIndex; cursor >= 0;
             cursor = prev[size_t(cursor)])
        {
            indexRoute.push_back(size_t(cursor));
            if (cursor == startIndex)
                break;
        }

        if (indexRoute.empty() || int32(indexRoute.back()) != startIndex)
            return false;

        std::reverse(indexRoute.begin(), indexRoute.end());
        if (outCost)
            *outCost = dist[size_t(endIndex)];
        return true;
    }

    int32 GetObjectiveNodeMarkerIndex(ObjectiveMarkerGraph const& graph,
        uint8 node)
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float o = 0.0f;
        GetObjectiveNodeApproachLocation(node, x, y, z, o);
        return GetNearestObjectiveMarkerIndexWithin(graph.Markers, x, y,
            TRIAL_NODE_MARKER_MAX_DISTANCE);
    }

    bool GetObjectiveNodeMarkerLocation(uint8 node, float& x, float& y, float& z)
    {
        ObjectiveMarkerGraph graph = BuildObjectiveMarkerGraph();
        if (graph.Markers.empty())
            return false;

        int32 markerIndex = GetObjectiveNodeMarkerIndex(graph, node);
        if (markerIndex < 0)
            return false;

        ObjectivePathMarker const& marker = graph.Markers[size_t(markerIndex)];
        x = marker.X;
        y = marker.Y;
        z = marker.Z;
        return true;
    }

    bool GetObjectiveStartMarkerLocation(TeamId teamId, float& x, float& y,
        float& z, float& o)
    {
        GetObjectiveStartLocation(teamId, x, y, z, o);

        ObjectiveMarkerGraph graph = BuildObjectiveMarkerGraph();
        if (graph.Markers.empty())
            return false;

        int32 markerIndex = GetNearestObjectiveMarkerIndex(graph.Markers, x, y);
        if (markerIndex < 0)
            return false;

        ObjectivePathMarker const& marker = graph.Markers[size_t(markerIndex)];
        x = marker.X;
        y = marker.Y;
        z = marker.Z;
        return true;
    }

    bool GetObjectiveNextMarkerTowardNode(ObjectiveMarkerGraph const& graph,
        float currentX, float currentY, uint8 node,
        float& nextX, float& nextY, float& nextZ, bool& atTargetMarker)
    {
        atTargetMarker = false;

        if (graph.Markers.empty())
            return false;

        int32 currentIndex = GetNearestObjectiveMarkerIndex(graph.Markers,
            currentX, currentY);
        int32 targetIndex = GetObjectiveNodeMarkerIndex(graph, node);
        if (currentIndex < 0 || targetIndex < 0)
            return false;

        ObjectivePathMarker const& targetMarker =
            graph.Markers[size_t(targetIndex)];

        if (currentIndex == targetIndex)
        {
            nextX = targetMarker.X;
            nextY = targetMarker.Y;
            nextZ = targetMarker.Z;
            atTargetMarker = true;
            return true;
        }

        std::vector<size_t> indexRoute;
        if (BuildObjectiveMarkerIndexRoute(graph, currentIndex, targetIndex,
            indexRoute) && indexRoute.size() >= 2)
        {
            ObjectivePathMarker const& nextMarker =
                graph.Markers[indexRoute[1]];
            nextX = nextMarker.X;
            nextY = nextMarker.Y;
            nextZ = nextMarker.Z;
            return true;
        }

        float bestDistance = std::numeric_limits<float>::max();
        int32 bestNeighbor = -1;
        for (auto const& [neighborIndex, _] : graph.Links[size_t(currentIndex)])
        {
            ObjectivePathMarker const& neighbor =
                graph.Markers[neighborIndex];
            float dx = neighbor.X - targetMarker.X;
            float dy = neighbor.Y - targetMarker.Y;
            float distance = std::sqrt(dx * dx + dy * dy);
            if (distance >= bestDistance)
                continue;

            bestDistance = distance;
            bestNeighbor = int32(neighborIndex);
        }

        if (bestNeighbor < 0)
        {
            for (size_t i = 0; i < graph.Markers.size(); ++i)
            {
                if (i == size_t(currentIndex))
                    continue;

                ObjectivePathMarker const& neighbor = graph.Markers[i];
                float currentDx = currentX - neighbor.X;
                float currentDy = currentY - neighbor.Y;
                float currentDistance = std::sqrt(
                    currentDx * currentDx + currentDy * currentDy);
                if (currentDistance > TRIAL_MARKER_LINK_DISTANCE)
                    continue;

                float dx = neighbor.X - targetMarker.X;
                float dy = neighbor.Y - targetMarker.Y;
                float distance = std::sqrt(dx * dx + dy * dy);
                if (distance >= bestDistance)
                    continue;

                bestDistance = distance;
                bestNeighbor = int32(i);
            }
        }

        if (bestNeighbor < 0)
            return false;

        ObjectivePathMarker const& nextMarker =
            graph.Markers[size_t(bestNeighbor)];
        nextX = nextMarker.X;
        nextY = nextMarker.Y;
        nextZ = nextMarker.Z;
        return true;
    }

    bool BuildObjectiveMarkerRoute(float startX, float startY, float startZ,
        float endX, float endY, float endZ, std::vector<G3D::Vector3>& route)
    {
        route.clear();
        ObjectiveMarkerGraph graph = BuildObjectiveMarkerGraph();
        if (graph.Markers.empty())
            return false;

        int32 startIndex = GetNearestObjectiveMarkerIndex(graph.Markers,
            startX, startY);
        int32 endIndex = GetNearestObjectiveMarkerIndex(graph.Markers,
            endX, endY);
        std::vector<size_t> indexRoute;
        if (!BuildObjectiveMarkerIndexRoute(graph, startIndex, endIndex,
            indexRoute))
        {
            return false;
        }

        for (size_t idx : indexRoute)
            route.emplace_back(graph.Markers[idx].X, graph.Markers[idx].Y,
                graph.Markers[idx].Z);

        if (!route.empty())
        {
            G3D::Vector3 const& first = route.front();
            if (std::fabs(first.x - startX) <= 6.0f &&
                std::fabs(first.y - startY) <= 6.0f)
            {
                if (route.size() > 1)
                    route.erase(route.begin());
            }
        }

        return !route.empty();
    }

    Unit* SelectShadowPetTarget(Player* player)
    {
        if (!player)
            return nullptr;

        if (Pet* pet = player->GetPet())
            if (pet->IsAlive())
                return pet;

        return player;
    }

    float ResolveArenaGroundZ(Map* map, float x, float y, float fallbackZ)
    {
        if (!map)
            return fallbackZ;

        float probeZ = fallbackZ + 5.0f;
        float groundZ = map->GetHeight(x, y, probeZ, true, 50.0f);
        if (!std::isfinite(groundZ) || groundZ <= INVALID_HEIGHT)
            return fallbackZ;

        return groundZ + 0.15f;
    }

    int32 CaptureShadowSpellPower(Player* player)
    {
        if (!player)
            return 0;

        int32 best = 0;
        for (uint32 school = SPELL_SCHOOL_HOLY;
             school < MAX_SPELL_SCHOOL; ++school)
        {
            best = std::max(best,
                player->SpellBaseDamageBonusDone(
                    SpellSchoolMask(1u << school)));
        }

        return best;
    }

    ShadowProfile CaptureShadowProfile(Player* player, StageConfig const& stage)
    {
        ShadowProfile profile;
        if (!player)
            return profile;

        profile.PlayerGuid = player->GetGUID();
        profile.StageId = stage.StageId;
        profile.PlayerClass = player->getClass();
        profile.ActiveSpec = player->GetSpec(player->GetActiveSpec());
        profile.PlayerName = player->GetName();
        profile.PlayerRace = player->getRace();
        profile.PlayerGender = player->getGender();
        profile.SheathState = player->GetSheath();
        profile.Level = player->GetLevel();
        profile.PowerType = player->getPowerType();
        profile.MaxHealth = std::max<uint32>(1u, player->GetMaxHealth());
        profile.MaxPower = player->GetMaxPower(profile.PowerType);
        profile.ObjectScale = player->GetObjectScale();
        profile.RunSpeedRate = player->GetSpeedRate(MOVE_RUN);
        profile.CastSpeedRate = player->GetFloatValue(UNIT_MOD_CAST_SPEED);
        profile.AverageItemLevel = std::max<float>(1.0f,
            player->GetAverageItemLevel());
        profile.Armor = int32(player->GetArmor());
        profile.SpellPowerBonus = CaptureShadowSpellPower(player);
        profile.DamageMultiplier = stage.DamageMultiplier;
        profile.SpellIntervalMs = stage.SpellIntervalMs;

        for (uint32 stat = STAT_STRENGTH; stat < MAX_STATS; ++stat)
            profile.Stats[stat] = int32(player->GetStat(Stats(stat)));

        for (uint32 school = SPELL_SCHOOL_NORMAL;
             school < MAX_SPELL_SCHOOL; ++school)
        {
            profile.Resistances[school] =
                int32(player->GetResistance(SpellSchools(school)));
        }

        for (uint32 attack = BASE_ATTACK; attack < MAX_ATTACK; ++attack)
        {
            profile.AttackTimeMs[attack] =
                player->GetAttackTime(WeaponAttackType(attack));
            profile.WeaponMinDamage[attack] =
                player->GetWeaponDamageRange(
                    WeaponAttackType(attack), MINDAMAGE);
            profile.WeaponMaxDamage[attack] =
                player->GetWeaponDamageRange(
                    WeaponAttackType(attack), MAXDAMAGE);
        }

        if (Item* mainHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0,
                EQUIPMENT_SLOT_MAINHAND))
        {
            profile.MainHandEntry = mainHand->GetEntry();
        }

        if (Item* offHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0,
                EQUIPMENT_SLOT_OFFHAND))
        {
            profile.OffHandEntry = offHand->GetEntry();
            if (offHand->GetTemplate() &&
                offHand->GetTemplate()->Class == ITEM_CLASS_WEAPON)
            {
                profile.CanDualWield = true;
            }
        }

        if (Item* ranged = player->GetItemByPos(INVENTORY_SLOT_BAG_0,
                EQUIPMENT_SLOT_RANGED))
        {
            profile.RangedEntry = ranged->GetEntry();
        }

        return profile;
    }

    void ApplyShadowCloneVisual(Player* player, Creature* summon)
    {
        if (!player || !summon)
            return;

        summon->SetDisplayId(player->GetNativeDisplayId(),
            player->GetObjectScale());
        summon->SetNativeDisplayId(player->GetNativeDisplayId());
        summon->SetUnitFlag2(UNIT_FLAG2_MIRROR_IMAGE);
        player->CastSpell(summon, SPELL_COPY_MAINHAND, true);
        player->CastSpell(summon, SPELL_COPY_OFFHAND, true);
        player->CastSpell(summon, SPELL_COPY_RANGED, true);
    }

    void BuildMirrorImagePacket(WorldPacket& data,
        ObjectGuid const& shadowGuid, Player* player)
    {
        data.Initialize(SMSG_MIRRORIMAGE_DATA, 68);
        data << shadowGuid;
        data << uint32(player->GetDisplayId());
        data << uint8(player->getRace());
        data << uint8(player->getGender());
        data << uint8(player->getClass());
        data << uint8(player->GetByteValue(PLAYER_BYTES, 0));
        data << uint8(player->GetByteValue(PLAYER_BYTES, 1));
        data << uint8(player->GetByteValue(PLAYER_BYTES, 2));
        data << uint8(player->GetByteValue(PLAYER_BYTES, 3));
        data << uint8(player->GetByteValue(PLAYER_BYTES_2, 0));
        data << uint32(player->GetGuildId());

        static EquipmentSlots const itemSlots[] =
        {
            EQUIPMENT_SLOT_HEAD,
            EQUIPMENT_SLOT_SHOULDERS,
            EQUIPMENT_SLOT_BODY,
            EQUIPMENT_SLOT_CHEST,
            EQUIPMENT_SLOT_WAIST,
            EQUIPMENT_SLOT_LEGS,
            EQUIPMENT_SLOT_FEET,
            EQUIPMENT_SLOT_WRISTS,
            EQUIPMENT_SLOT_HANDS,
            EQUIPMENT_SLOT_BACK,
            EQUIPMENT_SLOT_TABARD,
            EQUIPMENT_SLOT_END
        };

        for (EquipmentSlots const* itr = &itemSlots[0];
             *itr != EQUIPMENT_SLOT_END; ++itr)
        {
            if (*itr == EQUIPMENT_SLOT_HEAD &&
                player->HasPlayerFlag(PLAYER_FLAGS_HIDE_HELM))
            {
                data << uint32(0);
            }
            else if (*itr == EQUIPMENT_SLOT_BACK &&
                player->HasPlayerFlag(PLAYER_FLAGS_HIDE_CLOAK))
            {
                data << uint32(0);
            }
            else if (Item const* item =
                player->GetItemByPos(INVENTORY_SLOT_BAG_0, *itr))
            {
                uint32 displayInfoId = item->GetTemplate()->DisplayInfoID;
                sScriptMgr->OnGlobalMirrorImageDisplayItem(item,
                    displayInfoId);
                data << uint32(displayInfoId);
            }
            else
            {
                data << uint32(0);
            }
        }
    }

    void StartShadowCombat(Player* player, Creature* bot)
    {
        if (!player || !bot)
            return;

        bot->SetReactState(REACT_AGGRESSIVE);
        bot->SetInCombatWith(player);
        player->SetInCombatWith(bot);

        if (CreatureAI* ai = bot->AI())
            ai->AttackStart(player);
    }

    void CopyShadowPetStats(Creature* shadowPet, Pet* playerPet)
    {
        if (!shadowPet || !playerPet)
            return;

        shadowPet->SetLevel(playerPet->GetLevel());
        shadowPet->SetObjectScale(playerPet->GetObjectScale());
        shadowPet->SetDisplayId(playerPet->GetDisplayId());
        shadowPet->SetNativeDisplayId(playerPet->GetDisplayId());
        shadowPet->setPowerType(playerPet->getPowerType());

        shadowPet->SetMaxHealth(playerPet->GetMaxHealth());
        shadowPet->SetHealth(std::min(playerPet->GetHealth(),
            playerPet->GetMaxHealth()));

        for (uint8 stat = STAT_STRENGTH; stat < MAX_STATS; ++stat)
        {
            float statValue = playerPet->GetStat(Stats(stat));
            shadowPet->SetCreateStat(Stats(stat), statValue);
            shadowPet->SetStat(Stats(stat), int32(statValue));
        }

        shadowPet->SetArmor(playerPet->GetArmor());
        for (uint8 school = SPELL_SCHOOL_NORMAL;
            school < MAX_SPELL_SCHOOL; ++school)
            shadowPet->SetResistance(SpellSchools(school),
                playerPet->GetResistance(SpellSchools(school)));

        Powers const trackedPowers[] =
        {
            POWER_MANA,
            POWER_RAGE,
            POWER_FOCUS,
            POWER_ENERGY,
            POWER_HAPPINESS
        };

        for (Powers power : trackedPowers)
        {
            uint32 maxPower = playerPet->GetMaxPower(power);
            shadowPet->SetMaxPower(power, maxPower);
            shadowPet->SetPower(power,
                std::min(playerPet->GetPower(power), maxPower));
        }

        shadowPet->SetAttackTime(BASE_ATTACK,
            playerPet->GetAttackTime(BASE_ATTACK));
        shadowPet->SetAttackTime(OFF_ATTACK,
            playerPet->GetAttackTime(OFF_ATTACK));
        shadowPet->SetAttackTime(RANGED_ATTACK,
            playerPet->GetAttackTime(RANGED_ATTACK));

        shadowPet->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE,
            playerPet->GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE));
        shadowPet->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE,
            playerPet->GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE));
        shadowPet->SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE,
            playerPet->GetWeaponDamageRange(OFF_ATTACK, MINDAMAGE));
        shadowPet->SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE,
            playerPet->GetWeaponDamageRange(OFF_ATTACK, MAXDAMAGE));
        shadowPet->SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE,
            playerPet->GetWeaponDamageRange(RANGED_ATTACK, MINDAMAGE));
        shadowPet->SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE,
            playerPet->GetWeaponDamageRange(RANGED_ATTACK, MAXDAMAGE));
        shadowPet->UpdateDamagePhysical(BASE_ATTACK);
        shadowPet->UpdateDamagePhysical(OFF_ATTACK);
        shadowPet->UpdateDamagePhysical(RANGED_ATTACK);
    }

    class SoloArenaConfig
    {
    public:
        static SoloArenaConfig& Instance()
        {
            static SoloArenaConfig instance;
            return instance;
        }

        void Load()
        {
            _enabled = sConfigMgr->GetOption<bool>("SoloArena.Enable", true);
            _npcEntry = sConfigMgr->GetOption<uint32>("SoloArena.NpcEntry",
                910000);
            _shadowEntry = sConfigMgr->GetOption<uint32>(
                "SoloArena.ShadowEntry", 910001);
            _debug = sConfigMgr->GetOption<bool>("SoloArena.Debug", false);
        }

        bool IsEnabled() const { return _enabled; }
        uint32 GetNpcEntry() const { return _npcEntry; }
        uint32 GetShadowEntry() const { return _shadowEntry; }
        bool IsDebug() const { return _debug; }

    private:
        bool _enabled = true;
        uint32 _npcEntry = 910000;
        uint32 _shadowEntry = 910001;
        bool _debug = false;
    };

    class SoloArenaMgr
    {
    public:
        static SoloArenaMgr& Instance()
        {
            static SoloArenaMgr instance;
            return instance;
        }

        void LoadStages();
        void LoadMechanics();
        std::vector<StageConfig> GetStages() const;
        StageConfig const* GetStage(uint8 stageId) const;
        std::vector<StageMechanicConfig> GetMechanics(uint8 stageId) const;
        bool HasSession(ObjectGuid const& playerGuid) const;
        ArenaSession const* GetSession(ObjectGuid const& playerGuid) const;
        bool IsCombatActive(ObjectGuid const& playerGuid) const;
        bool IsObjectiveRaceActive(ObjectGuid const& playerGuid) const;
        bool StartChallenge(Player* player, uint8 stageId);
        bool ReturnPlayer(Player* player);
        void SendUi(Player* player);
        void Update(uint32 diff);
        void OnPlayerMapChanged(Player* player);
        void MarkVictory(ObjectGuid const& playerGuid);
        void MarkFailure(ObjectGuid const& playerGuid);
        void MarkAbandoned(ObjectGuid const& playerGuid);
        bool RespawnObjectivePlayer(ObjectGuid const& playerGuid);
        bool RespawnObjectiveShadow(ObjectGuid const& playerGuid);
        void ProcessObjectiveRespawns(Player* player, ArenaSession& session,
            uint64 now);
        void EnsureObjectiveShadowGrounded(Player* player, Creature* bot,
            ArenaSession& session);
        ShadowProfile const* GetShadowProfile(
            ObjectGuid const& creatureGuid) const;
        void UnregisterShadow(ObjectGuid const& creatureGuid);
        bool IsManagedShadow(Creature const* creature) const;
        bool IsManagedArenaInstance(uint32 instanceId) const;

    private:
        bool SpawnShadow(Player* player, ArenaSession& session);
        bool UpdateObjectiveTrial(Player* player, ArenaSession& session);
        void NormalizeObjectiveMovement(Player* player,
            ArenaSession& session);
        bool SyncShadowPet(Player* player, ArenaSession& session,
            bool startCombat);
        void ConfigureShadow(Creature* summon, Player* player,
            ShadowProfile const& profile, StageConfig const& stage);
        void FinishSession(Player* player, ArenaSession& session);
        void CleanupPet(ArenaSession& session);
        void CleanupBot(ArenaSession const& session);
        std::string RequestTrialTaunt(Player* player,
            ArenaSession const& session, std::string const& eventType) const;
        void SpeakTrialTaunt(Player* player, ArenaSession const& session,
            std::string const& eventType) const;
        uint8 GetHighestStageCleared(Player* player) const;
        bool IsStageUnlocked(Player* player, uint8 stageId) const;
        void SaveProgress(Player* player, uint8 stageId);
        void SaveStageRecord(Player* player, ArenaSession const& session);
        void GrantStageRewards(Player* player, ArenaSession const& session);
        uint32 GetTodayEntryCount(Player* player) const;
        void LogRun(Player* player, ArenaSession const& session);
        void LogEvent(Player* player, ArenaSession const& session,
            std::string const& eventType, std::string const& note = "");
        void LogReward(Player* player, ArenaSession const& session,
            uint32 itemEntry, uint32 itemCount, float chance,
            std::string const& status);
        void GrantDeserterImmunity(Player* player);
        void UpdateDeserterImmunity();
        std::string GetStageName(uint8 stageId) const;
        std::string BuildStageRewardPayload(uint8 stageId) const;
        std::string BuildStageRankPayload(Player* player, uint8 stageId) const;
        std::string GetStageMechanicName(uint8 stageId) const;
        std::pair<uint8, std::string> ComputeTrialRank(
            ArenaSession const& session) const;
        void SendResultPayload(Player* player, ArenaSession const& session);
        void NotifyObjectiveFinishReason(Player* player,
            char const* reason) const;
        void LoadDefaultStages();
        void LoadDefaultMechanics();
        void UpdateMechanics(Player* player, ArenaSession& session);
        bool SpawnMechanicObject(Player* player, ArenaSession& session,
            StageMechanicConfig const& mechanic);
        void ClearMechanicObject(ArenaSession& session);
        void ClearMechanicSummons(ArenaSession& session);
        void ApplyMechanicEffect(Player* player, ArenaSession& session,
            StageMechanicConfig const& mechanic);
        bool SummonMechanicHelper(Player* player, ArenaSession& session,
            StageMechanicConfig const& mechanic);
        bool SummonMechanicHazard(Player* player, ArenaSession& session,
            StageMechanicConfig const& mechanic);

        template <typename... Args>
        void Debug(std::string const& fmt, Args&&... args) const
        {
            (void)fmt;
            (void)sizeof...(args);
        }

        static void SendSystem(Player* player, std::string const& message)
        {
            if (!player || !player->GetSession())
                return;

            ChatHandler(player->GetSession()).PSendSysMessage("{}", message);
        }

        std::unordered_map<uint8, StageConfig> _stages;
        std::unordered_multimap<uint8, StageMechanicConfig> _mechanics;
        std::unordered_map<uint64, ArenaSession> _sessions;
        std::unordered_map<uint64, ShadowProfile> _shadowProfiles;
        std::unordered_map<uint64, uint64> _deserterImmuneUntil;
        std::unordered_set<uint32> _managedArenaInstances;
    };

    uint64 GenerateRunUid()
    {
        static std::atomic<uint64> seed { 1 };
        uint64 now = uint64(std::time(nullptr));
        return (now << 20) | (seed.fetch_add(1) & 0xFFFFF);
    }

    SpellPackage GetSpellPackage(uint8 playerClass, uint32 activeSpec);
    TacticalPackage GetTacticalPackage(uint8 playerClass, uint32 activeSpec);
    uint32 ComputeShadowSpellDamage(Creature* me,
        ShadowProfile const& profile,
        float factor);
    void BuildMirrorImagePacket(WorldPacket& data,
        ObjectGuid const& shadowGuid, Player* player);

    std::string SanitizeAddonField(std::string text, size_t maxLen)
    {
        std::replace(text.begin(), text.end(), '\t', ' ');
        std::replace(text.begin(), text.end(), '\n', ' ');
        std::replace(text.begin(), text.end(), '\r', ' ');
        std::replace(text.begin(), text.end(), '|', '/');
        std::replace(text.begin(), text.end(), '~', '-');

        if (text.size() > maxLen)
            text.resize(maxLen);

        return text;
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

    std::string GetTrialClassLabel(uint8 classId)
    {
        switch (classId)
        {
            case CLASS_WARRIOR: return "warrior";
            case CLASS_PALADIN: return "paladin";
            case CLASS_HUNTER: return "hunter";
            case CLASS_ROGUE: return "rogue";
            case CLASS_PRIEST: return "priest";
            case CLASS_DEATH_KNIGHT: return "death knight";
            case CLASS_SHAMAN: return "shaman";
            case CLASS_MAGE: return "mage";
            case CLASS_WARLOCK: return "warlock";
            case CLASS_DRUID: return "druid";
            default: return "adventurer";
        }
    }

    std::string GetTrialClassDisplayName(uint8 classId)
    {
        switch (classId)
        {
            case CLASS_WARRIOR: return "전사";
            case CLASS_PALADIN: return "성기사";
            case CLASS_HUNTER: return "사냥꾼";
            case CLASS_ROGUE: return "도적";
            case CLASS_PRIEST: return "사제";
            case CLASS_DEATH_KNIGHT: return "죽음의 기사";
            case CLASS_SHAMAN: return "주술사";
            case CLASS_MAGE: return "마법사";
            case CLASS_WARLOCK: return "흑마법사";
            case CLASS_DRUID: return "드루이드";
            default: return "모험가";
        }
    }

    std::string EscapeJson(std::string text)
    {
        std::string out;
        out.reserve(text.size() + 8);
        for (char ch : text)
        {
            switch (ch)
            {
                case '\\': out += "\\\\"; break;
                case '"': out += "\\\""; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out += ch; break;
            }
        }
        return out;
    }

    std::string ExtractJsonStringField(std::string const& body,
        std::string const& field)
    {
        std::string key = "\"" + field + "\"";
        std::size_t keyPos = body.find(key);
        if (keyPos == std::string::npos)
            return "";

        std::size_t colonPos = body.find(':', keyPos + key.size());
        if (colonPos == std::string::npos)
            return "";

        std::size_t quotePos = body.find('"', colonPos + 1);
        if (quotePos == std::string::npos)
            return "";

        std::string value;
        bool escaped = false;
        for (std::size_t i = quotePos + 1; i < body.size(); ++i)
        {
            char ch = body[i];
            if (escaped)
            {
                switch (ch)
                {
                    case 'n': value += '\n'; break;
                    case 'r': value += '\r'; break;
                    case 't': value += '\t'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    default: value += ch; break;
                }
                escaped = false;
                continue;
            }

            if (ch == '\\')
            {
                escaped = true;
                continue;
            }

            if (ch == '"')
                return value;

            value += ch;
        }

        return "";
    }

    std::string BuildTrialTauntFallback(Player* player,
        uint8 playerClass, uint8 stageId, std::string const& eventType)
    {
        std::string classLabel = GetTrialClassDisplayName(playerClass);
        std::string playerName = player ? player->GetName() : "도전자";
        std::string stageLabel = Acore::StringFormat(
            "그림자 시련 {}단계", uint32(stageId));

        auto stageLine = [&](std::string const& s1,
                             std::string const& s2,
                             std::string const& s3,
                             std::string const& s4,
                             std::string const& s5) -> std::string
        {
            switch (stageId)
            {
                case 1: return s1;
                case 2: return s2;
                case 3: return s3;
                case 4:
                case 5: return s4;
                default: return s5;
            }
        };

        if (eventType == "spawn")
            return stageLine(
                Acore::StringFormat(
                    "{}, {}의 그림자가 이미 기다리고 있다.",
                    playerName, classLabel),
                Acore::StringFormat(
                    "{}, {}의 손놀림까지도 내가 지켜보고 있었다.",
                    playerName, classLabel),
                Acore::StringFormat(
                    "{}, 네 익숙한 기술이 여기선 전부 읽힌다.",
                    playerName),
                Acore::StringFormat(
                    "{}, {}의 힘을 조금 더 끌어낸 그림자가 왔다.",
                    playerName, classLabel),
                Acore::StringFormat(
                    "{}, {}의 끝을 시험할 그림자가 기다린다.",
                    playerName, classLabel));
        if (eventType == "combat_start")
            return stageLine(
                Acore::StringFormat(
                    "{}답게 끝까지 버텨 봐라. 지금부터 시작이다.",
                    classLabel),
                Acore::StringFormat(
                    "이번엔 네 기술이 얼마나 다양한지 내가 먼저 보여 주지.",
                    classLabel),
                Acore::StringFormat(
                    "{}의 빈틈까지 모두 찌를 테니 정신 차려라.",
                    classLabel),
                Acore::StringFormat(
                    "{}의 힘이 커진 만큼, 내 그림자도 더 무거워졌다.",
                    classLabel),
                Acore::StringFormat(
                    "여기부터는 네가 아는 {}와는 다를 것이다.",
                    classLabel));
        if (eventType == "victory")
            return stageLine(
                Acore::StringFormat(
                    "{}, 이번엔 네가 한 수 위였군.",
                    playerName),
                Acore::StringFormat(
                    "{}, 제법이다. 다음 단계의 그림자는 더 날카롭다.",
                    playerName),
                Acore::StringFormat(
                    "{}, 이 정도로 끝낼 생각은 하지 마라.",
                    playerName),
                Acore::StringFormat(
                    "{}, 더 무거운 그림자가 아직 남아 있다.",
                    playerName),
                Acore::StringFormat(
                    "{}, {}의 시련은 아직 끝나지 않았다.",
                    playerName, stageLabel));
        if (eventType == "failure")
            return stageLine(
                Acore::StringFormat(
                    "{}의 힘이 이 정도라면 아직 그림자를 넘기 어렵다.",
                    classLabel),
                Acore::StringFormat(
                    "{}의 기술이 늘어도, 나를 넘기엔 아직 부족하다.",
                    classLabel),
                Acore::StringFormat(
                    "네 빈틈을 읽는 것만으로도 이 정도다, {}.",
                    classLabel),
                Acore::StringFormat(
                    "{}의 힘이 커질수록 그림자도 더 짙어진다.",
                    classLabel),
                Acore::StringFormat(
                    "{}의 끝은 아직 멀다. 다시 올라와라.",
                    stageLabel));
        if (eventType == "abandoned")
            return stageLine(
                Acore::StringFormat(
                    "{}, 이번엔 물러가도 된다. 다음엔 끝까지 버텨 봐라.",
                    playerName),
                Acore::StringFormat(
                    "{}, 손을 뗐군. 하지만 내 그림자는 남아 있다.",
                    playerName),
                Acore::StringFormat(
                    "{}, 기술을 감춘다고 그림자가 사라지진 않는다.",
                    playerName),
                Acore::StringFormat(
                    "{}, 더 짙은 그림자를 보기 전에 돌아섰군.",
                    playerName),
                Acore::StringFormat(
                    "{}, {}의 끝에서 다시 만나자.",
                    playerName, stageLabel));

        return stageLine(
            Acore::StringFormat(
                "{}, {}의 그림자는 언제나 너를 기다린다.",
                playerName, classLabel),
            Acore::StringFormat(
                "{}, 이번 단계의 그림자는 네 기술을 더 많이 닮았다.",
                playerName),
            Acore::StringFormat(
                "{}, 여기선 네 습관까지도 그림자가 된다.",
                playerName),
            Acore::StringFormat(
                "{}, {}의 그림자는 점점 더 무거워진다.",
                playerName, classLabel),
            Acore::StringFormat(
                "{}, {}는 끝으로 갈수록 더 선명해진다.",
                playerName, stageLabel));
    }

    void SendTrialTimePayload(Player* player, ArenaSession& session,
        bool force = false)
    {
        if (!player)
            return;

        uint64 now = std::time(nullptr);
        if (!force && session.NextTimePayloadAt != 0 &&
            now < session.NextTimePayloadAt)
            return;

        session.NextTimePayloadAt = now + 1;

        std::ostringstream payload;
        payload << "TIME\t";
        payload << uint64(session.PreparationEndsAt) << "\t";
        payload << uint64(session.CombatStartedAt) << "\t";
        payload << uint64(session.CombatEndsAt) << "\t";
        payload << uint64(session.EndedAt) << "\t";
        payload << uint32(session.State) << "\t";
        payload << uint64(session.PlayerRespawnAt) << "\t";
        payload << uint64(session.ShadowRespawnAt);
        SendAddonPayload(player, "TRIAL_UI", payload.str());
    }

    uint32 GetCombatDurationSec(ArenaSession const& session)
    {
        if (session.CombatStartedAt == 0 || session.EndedAt == 0 ||
            session.EndedAt < session.CombatStartedAt)
            return 0;

        return uint32(session.EndedAt - session.CombatStartedAt);
    }

    uint32 GetTrialDurationSec(ArenaSession const& session)
    {
        if (session.StartedAt == 0 || session.EndedAt == 0 ||
            session.EndedAt < session.StartedAt)
            return 0;

        return uint32(session.EndedAt - session.StartedAt);
    }

    uint32 GetRankDurationSec(ArenaSession const& session)
    {
        return IsObjectiveTrialStage(session.StageId) ?
            GetTrialDurationSec(session) : GetCombatDurationSec(session);
    }

    std::string GetFixedStageLabel(uint8 stageId)
    {
        switch (stageId)
        {
            case 1: return "그림자 시련 1단계";
            case 2: return "그림자 시련 2단계";
            case 3: return "그림자 시련 3단계";
            case 4: return "그림자 시련 4단계";
            case 5: return "그림자 시련 5단계";
            case 6: return "그림자 시련 6단계";
            case 7: return "그림자 시련 7단계";
            case 8: return "그림자 시련 8단계";
            case 9: return "그림자 시련 9단계";
            case 10: return "그림자 시련 10단계";
            default: return "";
        }
    }

    std::string GetFixedMechanicLabel(uint8 stageId)
    {
        switch (stageId)
        {
            case 1: return "시련의 숨결";
            case 2: return "뒤틀린 파편";
            case 3: return "균열의 제단";
            default: return "";
        }
    }

    bool StartsWith(std::string const& text, std::string const& token)
    {
        return text.compare(0, token.size(), token) == 0;
    }

    std::string EscapeCharacterDb(std::string text)
    {
        CharacterDatabase.EscapeString(text);
        return text;
    }

    bool HandleTrialAddonCommand(Player* player, std::string const& msg)
    {
        if (!player || !StartsWith(msg, "TRIAL_CMD\t"))
            return false;

        if (StartsWith(msg, "TRIAL_CMD\tSTART\t"))
        {
            std::string stageText = msg.substr(16);
            uint8 stageId = uint8(std::max(0, atoi(stageText.c_str())));
            SoloArenaMgr::Instance().StartChallenge(player, stageId);
            return true;
        }

        if (msg == "TRIAL_CMD\tOPEN")
        {
            SoloArenaMgr::Instance().SendUi(player);
            return true;
        }

        if (msg == "TRIAL_CMD\tABANDON")
        {
            SoloArenaMgr::Instance().MarkAbandoned(player->GetGUID());
            return true;
        }

        if (msg == "TRIAL_CMD\tRETURN")
        {
            SoloArenaMgr::Instance().ReturnPlayer(player);
            return true;
        }

        return true;
    }
}

void SoloArenaMgr::LoadStages()
{
    _stages.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT stage_id, name, arena_map_id, player_x, player_y, "
        "player_z, player_o, bot_x, bot_y, bot_z, bot_o, "
        "health_multiplier, damage_multiplier, attack_time_ms, "
        "spell_interval_ms, move_speed_rate, preparation_ms, enabled "
        "FROM solo_arena_stage ORDER BY stage_id");

    if (!result)
    {
        LoadDefaultStages();
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        StageConfig stage;
        stage.StageId = fields[0].Get<uint8>();
        stage.Name = fields[1].Get<std::string>();
        stage.ArenaMapId = fields[2].Get<uint32>();
        stage.PlayerX = fields[3].Get<float>();
        stage.PlayerY = fields[4].Get<float>();
        stage.PlayerZ = fields[5].Get<float>();
        stage.PlayerO = fields[6].Get<float>();
        stage.BotX = fields[7].Get<float>();
        stage.BotY = fields[8].Get<float>();
        stage.BotZ = fields[9].Get<float>();
        stage.BotO = fields[10].Get<float>();
        stage.HealthMultiplier = fields[11].Get<float>();
        stage.DamageMultiplier = fields[12].Get<float>();
        stage.AttackTimeMs = fields[13].Get<uint32>();
        stage.SpellIntervalMs = fields[14].Get<uint32>();
        stage.MoveSpeedRate = fields[15].Get<float>();
        stage.PreparationMs = fields[16].Get<uint32>();
        stage.Enabled = fields[17].Get<uint8>() != 0;
        if (IsObjectiveTrialStage(stage.StageId))
            stage.ArenaMapId = DEFAULT_OBJECTIVE_MAP_ID;
        if (std::string fixedName = GetFixedStageLabel(stage.StageId);
            !fixedName.empty())
            stage.Name = fixedName;

        _stages[stage.StageId] = stage;
    } while (result->NextRow());

    if (_stages.empty())
        LoadDefaultStages();
}

void SoloArenaMgr::LoadMechanics()
{
    _mechanics.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT stage_id, slot_id, mechanic_type, object_entry, "
        "spawn_x, spawn_y, spawn_z, spawn_o, spawn_interval_ms, "
        "duration_ms, effect_value_1, effect_value_2, summon_entry, "
        "enabled, name "
        "FROM solo_arena_stage_mechanic "
        "ORDER BY stage_id, slot_id");

    if (!result)
    {
        LoadDefaultMechanics();
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        StageMechanicConfig mechanic;
        mechanic.StageId = fields[0].Get<uint8>();
        mechanic.SlotId = fields[1].Get<uint8>();
        mechanic.MechanicType = StageMechanicType(fields[2].Get<uint8>());
        mechanic.ObjectEntry = fields[3].Get<uint32>();
        mechanic.SpawnX = fields[4].Get<float>();
        mechanic.SpawnY = fields[5].Get<float>();
        mechanic.SpawnZ = fields[6].Get<float>();
        mechanic.SpawnO = fields[7].Get<float>();
        mechanic.SpawnIntervalMs = fields[8].Get<uint32>();
        mechanic.DurationMs = fields[9].Get<uint32>();
        mechanic.EffectValue1 = fields[10].Get<float>();
        mechanic.EffectValue2 = fields[11].Get<float>();
        mechanic.SummonEntry = fields[12].Get<uint32>();
        mechanic.Enabled = fields[13].Get<uint8>() != 0;
        mechanic.Name = fields[14].Get<std::string>();
        if (std::string fixedName = GetFixedMechanicLabel(mechanic.StageId);
            !fixedName.empty())
            mechanic.Name = fixedName;

        if (mechanic.Enabled)
            _mechanics.emplace(mechanic.StageId, mechanic);
    } while (result->NextRow());

    if (_mechanics.empty())
        LoadDefaultMechanics();
}

std::vector<StageConfig> SoloArenaMgr::GetStages() const
{
    std::vector<StageConfig> stages;
    for (auto const& [id, stage] : _stages)
    {
        if (stage.Enabled)
            stages.push_back(stage);
    }

    std::sort(stages.begin(), stages.end(),
        [](StageConfig const& lhs, StageConfig const& rhs)
        {
            return lhs.StageId < rhs.StageId;
        });

    return stages;
}

StageConfig const* SoloArenaMgr::GetStage(uint8 stageId) const
{
    auto itr = _stages.find(stageId);
    if (itr == _stages.end())
        return nullptr;

    return &itr->second;
}

bool SoloArenaMgr::HasSession(ObjectGuid const& playerGuid) const
{
    return _sessions.find(playerGuid.GetCounter()) != _sessions.end();
}

ArenaSession const* SoloArenaMgr::GetSession(ObjectGuid const& playerGuid) const
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return nullptr;

    return &itr->second;
}

std::vector<StageMechanicConfig> SoloArenaMgr::GetMechanics(
    uint8 stageId) const
{
    std::vector<StageMechanicConfig> mechanics;
    auto [itr, end] = _mechanics.equal_range(stageId);
    for (; itr != end; ++itr)
        mechanics.push_back(itr->second);

    std::sort(mechanics.begin(), mechanics.end(),
        [](StageMechanicConfig const& lhs, StageMechanicConfig const& rhs)
        {
            return lhs.SlotId < rhs.SlotId;
        });

    return mechanics;
}

bool SoloArenaMgr::IsCombatActive(ObjectGuid const& playerGuid) const
{
    ArenaSession const* session = GetSession(playerGuid);
    if (!session)
        return false;

    return session->State == SessionState::Active;
}

bool SoloArenaMgr::IsObjectiveRaceActive(ObjectGuid const& playerGuid) const
{
    ArenaSession const* session = GetSession(playerGuid);
    if (!session)
        return false;

    return session->Scenario == TrialScenario::Objective &&
        session->StageId >= 4 && session->StageId <= 6 &&
        session->State == SessionState::Active;
}

bool SoloArenaMgr::StartChallenge(Player* player, uint8 stageId)
{
    if (!SoloArenaConfig::Instance().IsEnabled() || !player)
        return false;

    if (HasSession(player->GetGUID()))
    {
        SendSystem(player, "이미 진행 중인 시련이 있습니다.");
        return false;
    }

    StageConfig const* stage = GetStage(stageId);
    if (!stage || !stage->Enabled)
    {
        SendSystem(player, "해당 단계는 아직 사용할 수 없습니다.");
        return false;
    }

    if (player->IsInCombat())
    {
        SendSystem(player, "전투 중에는 시련을 시작할 수 없습니다.");
        return false;
    }

    if (!IsStageUnlocked(player, stageId))
    {
        SendSystem(player, "이전 단계를 먼저 클리어해야 합니다.");
        return false;
    }

    if (GetTodayEntryCount(player) >= TRIAL_DAILY_LIMIT)
    {
        SendSystem(player,
            "오늘의 시련 입장 가능 횟수 5회를 모두 사용했습니다.");
        return false;
    }

    if (!player->HasItemCount(TRIAL_TICKET_ITEM, 1, false))
    {
        SendSystem(player,
            "시련 입장권이 필요합니다.");
        return false;
    }

    bool const objectiveTrial = IsObjectiveTrialStage(stageId);
    BattlegroundTypeId const bgTypeId = objectiveTrial ?
        DEFAULT_OBJECTIVE_BG_TYPE : DEFAULT_ARENA_BG_TYPE;

    Battleground* battleground = nullptr;
    if (objectiveTrial)
    {
        Battleground* battlegroundTemplate =
            sBattlegroundMgr->GetBattlegroundTemplate(
                DEFAULT_OBJECTIVE_BG_TYPE);
        if (!battlegroundTemplate)
        {
            SendSystem(player, "시련 전장 템플릿을 찾지 못했습니다.");
            return false;
        }

        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketById(
            battlegroundTemplate->GetMapId(),
            battlegroundTemplate->GetBracketId());
        if (!bracketEntry)
        {
            SendSystem(player, "시련 전장 등급 정보를 찾지 못했습니다.");
            return false;
        }

        battleground = sBattlegroundMgr->CreateNewBattleground(
            bgTypeId, bracketEntry, 0, false);
        if (!battleground)
        {
            SendSystem(player, "시련 전장 인스턴스를 만들지 못했습니다.");
            return false;
        }

        battleground->SetMinPlayersPerTeam(0);
        battleground->SetMaxPlayersPerTeam(1);
        battleground->StartBattleground();
        battleground->SetStartDelayTime(SOLO_ARENA_PREPARATION_MS);
    }
    else
    {
        Battleground* battlegroundTemplate =
            sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
        if (!battlegroundTemplate)
        {
            SendSystem(player, "솔로 투기장 템플릿을 찾지 못했습니다.");
            return false;
        }

        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketById(
            battlegroundTemplate->GetMapId(),
            battlegroundTemplate->GetBracketId());
        if (!bracketEntry)
        {
            SendSystem(player, "솔로 투기장 등급 정보를 찾지 못했습니다.");
            return false;
        }

        battleground = sBattlegroundMgr->CreateNewBattleground(
            bgTypeId, bracketEntry, ARENA_TYPE_2v2, false);
        if (!battleground)
        {
            SendSystem(player,
                "투기장 인스턴스를 만들지 못했습니다.");
            return false;
        }

        battleground->StartBattleground();
    }

    ArenaSession session;
    session.RunUid = GenerateRunUid();
    session.PlayerGuid = player->GetGUID();
    session.StageId = stageId;
    session.Scenario = objectiveTrial ?
        TrialScenario::Objective : TrialScenario::Duel;
    session.BgTypeId = bgTypeId;
    session.Team = Player::TeamIdForRace(player->getRace());
    session.ArenaMapId = objectiveTrial ?
        DEFAULT_OBJECTIVE_MAP_ID : stage->ArenaMapId;
    session.ArenaInstanceId = battleground ? battleground->GetInstanceID() : 0;
    session.ReturnMapId = player->GetMapId();
    session.ReturnX = player->GetPositionX();
    session.ReturnY = player->GetPositionY();
    session.ReturnZ = player->GetPositionZ();
    session.ReturnO = player->GetOrientation();
    session.StartedAt = std::time(nullptr);
    session.PreparationEndsAt = session.StartedAt +
        (SOLO_ARENA_PREPARATION_MS / 1000);
    session.NextMovementNormalizeAt = session.StartedAt;
    if (objectiveTrial)
        session.SpawnDelayMs = 1500;

    _sessions[player->GetGUID().GetCounter()] = session;
    if (session.ArenaInstanceId != 0)
        _managedArenaInstances.insert(session.ArenaInstanceId);
    LogEvent(player, _sessions[player->GetGUID().GetCounter()],
        "RUN_CREATED");

    TeamId teamId = session.Team;
    BattlegroundQueueTypeId queueTypeId{};
    uint32 queueSlot = PLAYER_MAX_BATTLEGROUND_QUEUES;
    if (objectiveTrial)
    {
        queueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, 0);
        queueSlot = player->AddBattlegroundQueueId(queueTypeId);
        if (queueSlot >= PLAYER_MAX_BATTLEGROUND_QUEUES)
        {
            _sessions.erase(player->GetGUID().GetCounter());
            _managedArenaInstances.erase(session.ArenaInstanceId);
            SendSystem(player, "시련 전장 대기열을 만들지 못했습니다.");
            return false;
        }

        player->SetEntryPoint();
        player->SetInviteForBattlegroundQueueType(queueTypeId,
            battleground->GetInstanceID());

        WorldPacket data;
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, battleground,
            queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME, 0, 0,
            teamId);
        player->SendDirectMessage(&data);

        sLFGMgr->LeaveAllLfgQueues(player->GetGUID(), false);

        player->SetBattlegroundId(session.ArenaInstanceId,
            bgTypeId, queueSlot, true, false, teamId);
    }
    else
    {
        queueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, ARENA_TYPE_2v2);
        queueSlot = player->AddBattlegroundQueueId(queueTypeId);
        if (queueSlot >= PLAYER_MAX_BATTLEGROUND_QUEUES)
        {
            _sessions.erase(player->GetGUID().GetCounter());
            _managedArenaInstances.erase(session.ArenaInstanceId);
            SendSystem(player, "시련 투기장 대기열을 만들지 못했습니다.");
            return false;
        }

        player->SetEntryPoint();
        player->SetInviteForBattlegroundQueueType(queueTypeId,
            battleground->GetInstanceID());

        WorldPacket data;
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, battleground,
            queueSlot,
            STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME, 0,
            battleground->GetArenaType(), teamId);
        player->SendDirectMessage(&data);

        sLFGMgr->LeaveAllLfgQueues(player->GetGUID(), false);

        player->SetBattlegroundId(battleground->GetInstanceID(),
            battleground->GetBgTypeID(),
            queueSlot, true, false, teamId);
    }

    if (Battleground* playerBg = player->GetBattleground())
    {
        ArenaSession& activeSession = _sessions[player->GetGUID().GetCounter()];
        activeSession.PreparationEndsAt = std::time(nullptr) +
            (SOLO_ARENA_PREPARATION_MS / 1000);
        SendTrialTimePayload(player, activeSession, true);
    }
    else
    {
        ArenaSession& activeSession = _sessions[player->GetGUID().GetCounter()];
        activeSession.PreparationEndsAt = std::time(nullptr) +
            (SOLO_ARENA_PREPARATION_MS / 1000);
        SendTrialTimePayload(player, activeSession, true);
    }

    if (objectiveTrial)
    {
        sBattlegroundMgr->SendToBattleground(player,
            session.ArenaInstanceId, bgTypeId);
    }
    else
    {
        float playerX = stage->PlayerX;
        float playerY = stage->PlayerY;
        float playerZ = stage->PlayerZ;
        float playerO = stage->PlayerO;

        if (Map* map = sMapMgr->CreateBaseMap(session.ArenaMapId))
            playerZ = ResolveArenaGroundZ(map, playerX, playerY, playerZ);

        if (!player->TeleportTo(session.ArenaMapId, playerX, playerY,
            playerZ, playerO, TELE_TO_GM_MODE))
        {
            _sessions.erase(player->GetGUID().GetCounter());
            _managedArenaInstances.erase(session.ArenaInstanceId);
            player->RemoveBattlegroundQueueId(queueTypeId);
            player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE,
                PLAYER_MAX_BATTLEGROUND_QUEUES, false, false, TEAM_NEUTRAL);
            SendSystem(player, "투기장으로 이동하지 못했습니다.");
            return false;
        }
    }

    player->DestroyItemCount(TRIAL_TICKET_ITEM, 1, true, false);
    LogEvent(player, _sessions[player->GetGUID().GetCounter()],
        "TICKET_CONSUMED", Acore::StringFormat(
            "item={} remaining={}", TRIAL_TICKET_ITEM,
            player->GetItemCount(TRIAL_TICKET_ITEM, false)));

    SendSystem(player, Acore::StringFormat(
        "{} 시작. {}로 이동합니다.", stage->Name,
        objectiveTrial ? "아라시 분지" : "언더시티 투기장"));
    LogEvent(player, _sessions[player->GetGUID().GetCounter()],
        "PLAYER_TELEPORTED");
    return true;
}

bool SoloArenaMgr::ReturnPlayer(Player* player)
{
    if (!player)
        return false;

    auto itr = _sessions.find(player->GetGUID().GetCounter());
    if (itr == _sessions.end())
        return false;

    ArenaSession session = itr->second;

    if (Battleground* battleground = player->GetBattleground())
        battleground->RemovePlayerAtLeave(player);
    else if (session.ArenaInstanceId)
        if (Battleground* arena = sBattlegroundMgr->GetBattleground(
                session.ArenaInstanceId, session.BgTypeId))
            arena->RemovePlayerAtLeave(player);

    _managedArenaInstances.erase(session.ArenaInstanceId);
    if (session.Scenario == TrialScenario::Objective)
        GrantDeserterImmunity(player);
    player->TeleportTo(session.ReturnMapId, session.ReturnX, session.ReturnY,
        session.ReturnZ, session.ReturnO);
    if (session.Scenario == TrialScenario::Objective)
    {
        player->RemoveBattlegroundQueueId(
            BattlegroundMgr::BGQueueTypeId(session.BgTypeId, 0));
        player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE,
            PLAYER_MAX_BATTLEGROUND_QUEUES, false, false, TEAM_NEUTRAL);
    }
    if (session.Scenario == TrialScenario::Objective)
        player->RemoveAura(26013);
    _sessions.erase(itr);
    return true;
}

void SoloArenaMgr::SendUi(Player* player)
{
    if (!SoloArenaConfig::Instance().IsEnabled() || !player)
        return;

    if (ArenaSession const* session = GetSession(player->GetGUID()))
        if (session->State == SessionState::AwaitingReturn)
        {
            SendResultPayload(player, *session);
            return;
        }

    uint8 highestStage = GetHighestStageCleared(player);
    uint8 maxUnlockedStage = highestStage + 1;

    std::ostringstream entries;
    bool first = true;
    uint32 sentStages = 0;

    for (StageConfig const& stage : GetStages())
    {
        if (!stage.Enabled)
            continue;

        if (stage.StageId > maxUnlockedStage)
            continue;

        if (!first)
            entries << "|";

        first = false;
        std::string rewardPayload = BuildStageRewardPayload(stage.StageId);
        std::string rankPayload = BuildStageRankPayload(player, stage.StageId);
        std::string mechanicName = SanitizeAddonField(
            GetStageMechanicName(stage.StageId), 48);
        entries << uint32(stage.StageId) << "~";
        entries << SanitizeAddonField(stage.Name, 64) << "~";
        entries << stage.HealthMultiplier << "~";
        entries << stage.DamageMultiplier << "~";
        entries << stage.SpellIntervalMs << "~";
        entries << stage.MoveSpeedRate << "~";
        entries << rewardPayload << "~";
        entries << rankPayload << "~";
        entries << mechanicName;
        ++sentStages;
    }

    if (sentStages == 0)
        SendSystem(player, "시련 UI에 표시할 단계 데이터가 없습니다.");

    std::ostringstream payload;
    payload << "OPEN\t";
    payload << uint32(highestStage) << "\t";
    payload << entries.str() << "\t";
    payload << (HasSession(player->GetGUID()) ? 1 : 0) << "\t";
    if (ArenaSession const* session = GetSession(player->GetGUID()))
    {
        payload << uint64(session->PreparationEndsAt) << "\t";
        payload << uint64(session->CombatStartedAt) << "\t";
        payload << uint64(session->CombatEndsAt) << "\t";
        payload << uint64(session->EndedAt) << "\t";
        payload << uint32(session->State);
    }
    else
    {
        payload << uint64(0) << "\t" << uint64(0) << "\t"
                << uint64(0) << "\t" << uint64(0) << "\t"
                << uint32(SessionState::PendingSpawn);
    }
    SendAddonPayload(player, "TRIAL_UI", payload.str());
}

void SoloArenaMgr::SendResultPayload(Player* player,
    ArenaSession const& session)
{
    if (!player)
        return;

    std::ostringstream payload;
    payload << "RESULT\t";
    payload << uint32(session.StageId) << "\t";
    payload << SanitizeAddonField(GetStageName(session.StageId), 64) << "\t";
    payload << SanitizeAddonField(
        session.Result == ArenaResult::Victory ? "성공" :
        session.Result == ArenaResult::Failure ? "실패" :
        session.Result == ArenaResult::Abandoned ? "포기" : "종료", 16)
        << "\t";
    payload << SanitizeAddonField(
        session.RankLabel.empty() ? "-" : session.RankLabel, 8) << "\t";
    payload << uint32(session.CombatDurationSec);
    SendAddonPayload(player, "TRIAL_UI", payload.str());
}

void SoloArenaMgr::NotifyObjectiveFinishReason(Player* player,
    char const* reason) const
{
    if (!player || !reason)
        return;

    SendSystem(player, Acore::StringFormat(
        "시련 종료 원인: {}", reason));
}

void SoloArenaMgr::Update(uint32 diff)
{
    UpdateDeserterImmunity();

    std::vector<uint64> toErase;

    for (auto& [playerKey, session] : _sessions)
    {
        Player* player = ObjectAccessor::FindConnectedPlayer(session.PlayerGuid);
        if (!player)
        {
            CleanupPet(session);
            CleanupBot(session);
            toErase.push_back(playerKey);
            continue;
        }

        switch (session.State)
        {
            case SessionState::PendingSpawn:
                if (session.Scenario == TrialScenario::Objective)
                {
                    if (player->GetMapId() == session.ArenaMapId)
                    {
                        session.State = SessionState::WaitingForStart;
                        session.SpawnDelayMs = 0;
                    }
                    else if (session.SpawnDelayMs > diff)
                    {
                        session.SpawnDelayMs -= diff;
                    }
                    else
                    {
                        session.Result = ArenaResult::Abandoned;
                        session.State = SessionState::PendingFinish;
                        session.AbandonedAt = std::time(nullptr);
                        session.EndedAt = session.AbandonedAt;
                        session.FinishDelayMs = 1;
                        LogEvent(player, session,
                            "OBJECTIVE_JOIN_TIMEOUT");
                        NotifyObjectiveFinishReason(player,
                            "아라시 분지 입장 실패");
                    }
                    break;
                }

                if (player->GetMapId() != session.ArenaMapId)
                    break;

                if (session.SpawnDelayMs > diff)
                {
                    session.SpawnDelayMs -= diff;
                    break;
                }

                session.SpawnDelayMs = 0;
                if (!SpawnShadow(player, session))
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.AbandonedAt = std::time(nullptr);
                    session.EndedAt = session.AbandonedAt;
                    session.FinishDelayMs = 1;
                    LogEvent(player, session, "SHADOW_SPAWN_FAILED");
                    if (session.Scenario == TrialScenario::Objective)
                        NotifyObjectiveFinishReason(player,
                            "그림자 소환 실패");
                }
                break;
            case SessionState::WaitingForStart:
                if (session.Scenario == TrialScenario::Objective)
                {
                    UpdateObjectiveTrial(player, session);
                    break;
                }

                SyncShadowPet(player, session, false);

                if (player->GetMapId() != session.ArenaMapId)
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.AbandonedAt = std::time(nullptr);
                    session.EndedAt = session.AbandonedAt;
                    session.FinishDelayMs = 1;
                    LogEvent(player, session, "LEFT_ARENA_BEFORE_START");
                    if (session.Scenario == TrialScenario::Objective)
                        NotifyObjectiveFinishReason(player,
                            "시작 전 맵 이탈");
                    break;
                }

                if (session.BotGuid.IsEmpty())
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.AbandonedAt = std::time(nullptr);
                    session.EndedAt = session.AbandonedAt;
                    session.FinishDelayMs = 1;
                    LogEvent(player, session, "BOT_MISSING_BEFORE_START");
                    if (session.Scenario == TrialScenario::Objective)
                        NotifyObjectiveFinishReason(player,
                            "그림자 개체 없음");
                    break;
                }

                if (Battleground* bg = player->GetBattleground())
                {
                    if (bg->GetStatus() == STATUS_WAIT_JOIN &&
                        bg->GetStartDelayTime() > int32(SOLO_ARENA_PREPARATION_MS))
                    {
                        bg->SetStartDelayTime(SOLO_ARENA_PREPARATION_MS);
                    }

                    if (bg->GetStatus() == STATUS_WAIT_JOIN)
                    {
                        session.PreparationEndsAt = std::time(nullptr) +
                            std::max<int32>(0, bg->GetStartDelayTime()) / 1000;
                    }

                    if (bg->GetStatus() == STATUS_IN_PROGRESS)
                    {
                        if (Creature* bot = ObjectAccessor::GetCreature(
                                *player, session.BotGuid))
                            StartShadowCombat(player, bot);

                        SyncShadowPet(player, session, true);
                        session.State = SessionState::Active;
                        session.CombatStartedAt = std::time(nullptr);
                        session.CombatEndsAt = session.CombatStartedAt +
                            (DEFAULT_COMBAT_LIMIT_MS / 1000);
                        session.EndedAt = 0;
                        LogEvent(player, session, "COMBAT_STARTED");
                        SendTrialTimePayload(player, session, true);
                        SpeakTrialTaunt(player, session, "combat_start");
                        SendSystem(player,
                            "문이 열렸습니다. 그림자와의 결투가 시작됩니다.");
                    }
                }
                break;
            case SessionState::Active:
                if (session.Scenario == TrialScenario::Objective)
                {
                    if (!UpdateObjectiveTrial(player, session))
                        break;
                }
                else
                {
                    SyncShadowPet(player, session, true);
                    UpdateMechanics(player, session);
                }

                if (session.PlayerDamageBuffEndsAt != 0 &&
                    uint64(std::time(nullptr)) >= session.PlayerDamageBuffEndsAt)
                {
                    player->RemoveAurasDueToSpell(TRIAL_MECHANIC_BUFF_AURA);
                    session.PlayerDamageBuffEndsAt = 0;
                    SendSystem(player,
                        "시련의 숨결 효과가 사라졌습니다.");
                }

                if (session.ObjectiveSpeedBuffEndsAt != 0 &&
                    uint64(std::time(nullptr)) >= session.ObjectiveSpeedBuffEndsAt)
                {
                    session.ObjectiveSpeedBuffEndsAt = 0;
                    if (session.Scenario == TrialScenario::Objective)
                        SendSystem(player,
                            "질주의 상자 효과가 사라졌습니다.");
                }

                if (player->GetMapId() != session.ArenaMapId)
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.AbandonedAt = std::time(nullptr);
                    session.EndedAt = session.AbandonedAt;
                    session.FinishDelayMs = 1;
                    LogEvent(player, session, "LEFT_ARENA");
                    if (session.Scenario == TrialScenario::Objective)
                        NotifyObjectiveFinishReason(player,
                            "진행 중 맵 이탈");
                    break;
                }

                if (session.CombatEndsAt != 0 &&
                    std::time(nullptr) >= time_t(session.CombatEndsAt))
                {
                    session.Result = ArenaResult::Failure;
                    session.State = SessionState::PendingFinish;
                    session.FailedAt = std::time(nullptr);
                    session.EndedAt = std::time(nullptr);
                    session.FinishDelayMs = 1;
                    LogEvent(player, session, "COMBAT_TIMEOUT");
                    SendTrialTimePayload(player, session, true);
                    if (session.Scenario == TrialScenario::Objective)
                        NotifyObjectiveFinishReason(player,
                            "시련 제한 시간 초과");
                    break;
                }

                if (!session.BotGuid.IsEmpty())
                {
                    Creature* bot = ObjectAccessor::GetCreature(*player,
                        session.BotGuid);
                    if (!bot || !bot->IsAlive())
                    {
                        if (session.Result == ArenaResult::None)
                            session.Result = ArenaResult::Victory;

                        session.State = SessionState::PendingFinish;
                    }
                }
                break;
            case SessionState::PendingFinish:
                if (session.FinishDelayMs > diff)
                {
                    session.FinishDelayMs -= diff;
                    break;
                }

                FinishSession(player, session);
                if (session.State != SessionState::AwaitingReturn)
                    toErase.push_back(playerKey);
                break;
            case SessionState::AwaitingReturn:
                break;
        }
    }

    for (uint64 key : toErase)
        _sessions.erase(key);
}

void SoloArenaMgr::OnPlayerMapChanged(Player* player)
{
    if (!player)
        return;

    if (ArenaSession const* session = GetSession(player->GetGUID()))
    {
        Debug("Solo arena map change: player='{}' map={} stage={}",
            player->GetName(), player->GetMapId(), session->StageId);

        if (session->State == SessionState::AwaitingReturn &&
            player->GetMapId() != session->ArenaMapId)
        {
            _managedArenaInstances.erase(session->ArenaInstanceId);
            _sessions.erase(player->GetGUID().GetCounter());
            return;
        }
    }
}

void SoloArenaMgr::GrantDeserterImmunity(Player* player)
{
    if (!player)
        return;

    _deserterImmuneUntil[player->GetGUID().GetCounter()] =
        uint64(std::time(nullptr)) + 15;
    player->RemoveAura(26013);
}

void SoloArenaMgr::UpdateDeserterImmunity()
{
    uint64 now = std::time(nullptr);
    std::vector<uint64> expired;

    for (auto const& [playerKey, until] : _deserterImmuneUntil)
    {
        Player* player = ObjectAccessor::FindConnectedPlayer(
            ObjectGuid::Create<HighGuid::Player>(playerKey));
        if (!player || now >= until)
        {
            expired.push_back(playerKey);
            continue;
        }

        if (player->HasAura(26013))
            player->RemoveAura(26013);
    }

    for (uint64 playerKey : expired)
        _deserterImmuneUntil.erase(playerKey);
}

void SoloArenaMgr::NormalizeObjectiveMovement(Player* player,
    ArenaSession& session)
{
    if (!player)
        return;

    uint64 now = std::time(nullptr);
    if (session.NextMovementNormalizeAt > now)
        return;

    session.NextMovementNormalizeAt = now + 1;

    if (!player->IsMounted())
        return;

    float mountedRunRate = OBJECTIVE_MOUNT_RUN_RATE;
    if (session.ObjectiveSpeedBuffEndsAt != 0 &&
        uint64(std::time(nullptr)) < session.ObjectiveSpeedBuffEndsAt)
        mountedRunRate += OBJECTIVE_SPEED_BOX_BONUS;

    player->SetSpeed(MOVE_RUN, mountedRunRate, true);
    player->SetSpeed(MOVE_RUN_BACK, mountedRunRate, true);
    player->SetSpeed(MOVE_SWIM, mountedRunRate, true);
}

void MatchObjectiveShadowSpeed(Player* player, Creature* bot)
{
    if (!player || !bot)
        return;

    float runRate = player->GetSpeedRate(MOVE_RUN);
    float backRate = player->GetSpeedRate(MOVE_RUN_BACK);
    float swimRate = player->GetSpeedRate(MOVE_SWIM);

    bot->SetSpeed(MOVE_RUN, std::max(1.0f, runRate), true);
    bot->SetSpeed(MOVE_RUN_BACK, std::max(1.0f, backRate), true);
    bot->SetSpeed(MOVE_SWIM, std::max(1.0f, swimRate), true);
}

bool SoloArenaMgr::UpdateObjectiveTrial(Player* player, ArenaSession& session)
{
    if (!player)
        return false;

    uint64 now = std::time(nullptr);
    ProcessObjectiveRespawns(player, session, now);
    NormalizeObjectiveMovement(player, session);

    if (player->GetMapId() != session.ArenaMapId)
    {
        session.Result = ArenaResult::Abandoned;
        session.State = SessionState::PendingFinish;
        session.AbandonedAt = std::time(nullptr);
        session.EndedAt = session.AbandonedAt;
        session.FinishDelayMs = 1;
        LogEvent(player, session, "LEFT_OBJECTIVE_TRIAL");
        NotifyObjectiveFinishReason(player, "아라시 분지 맵 이탈");
        return false;
    }

    if (Battleground* bg = player->GetBattleground())
    {
        if (bg->GetStatus() == STATUS_WAIT_JOIN &&
            bg->GetStartDelayTime() > int32(SOLO_ARENA_PREPARATION_MS))
        {
            bg->SetStartDelayTime(SOLO_ARENA_PREPARATION_MS);
        }

        if (bg->GetStatus() == STATUS_WAIT_JOIN)
        {
            session.PreparationEndsAt = now +
                (std::max<int32>(0, bg->GetStartDelayTime()) / 1000);
            SendTrialTimePayload(player, session);
            return true;
        }

        if (bg->GetStatus() != STATUS_IN_PROGRESS)
        {
            session.PreparationEndsAt = now + (SOLO_ARENA_PREPARATION_MS / 1000);
            SendTrialTimePayload(player, session);
            return true;
        }
    }
    else
    {
        if (session.PreparationEndsAt == 0)
            session.PreparationEndsAt = now +
                (SOLO_ARENA_PREPARATION_MS / 1000);

        if (now < session.PreparationEndsAt)
        {
            SendTrialTimePayload(player, session);
            return true;
        }
    }

    if (session.CombatStartedAt == 0)
    {
        session.CombatStartedAt = now;
        session.CombatEndsAt = session.CombatStartedAt +
            (DEFAULT_OBJECTIVE_LIMIT_MS / 1000);
        LogEvent(player, session, "OBJECTIVE_STARTED");
        SendTrialTimePayload(player, session, true);

        if (!session.ObjectiveIntroSent)
        {
            session.ObjectiveIntroSent = true;
            if (!SpawnShadow(player, session))
            {
                session.Result = ArenaResult::Failure;
                session.State = SessionState::PendingFinish;
                session.FailedAt = now;
                session.EndedAt = session.FailedAt;
                session.FinishDelayMs = 1;
                LogEvent(player, session, "OBJECTIVE_SHADOW_SPAWN_FAILED");
                NotifyObjectiveFinishReason(player, "초기 그림자 소환 실패");
                return false;
            }

            SendSystem(player,
                "그림자가 동시에 아라시 분지에 진입했습니다. 먼저 1600점을 달성하십시오.");
            SpeakTrialTaunt(player, session, "combat_start");
        }
    }

    BattlegroundAB* objectiveBg = nullptr;
    if (Battleground* bg = player->GetBattleground())
        objectiveBg = dynamic_cast<BattlegroundAB*>(bg);

    if (!session.ObjectiveReadyAnnounced)
    {
        session.ObjectiveReadyAnnounced = true;
        SendSystem(player,
            "각 거점의 깃발을 직접 활성화하십시오. 그림자도 거점을 점령합니다.");
    }

    if (session.CombatEndsAt != 0 &&
        now >= time_t(session.CombatEndsAt))
    {
        session.Result = ArenaResult::Failure;
        session.State = SessionState::PendingFinish;
        session.FailedAt = now;
        session.EndedAt = session.FailedAt;
        session.FinishDelayMs = 1;
        LogEvent(player, session, "OBJECTIVE_TIMEOUT");
        SendTrialTimePayload(player, session, true);
        NotifyObjectiveFinishReason(player, "거점 점령 시간 초과");
        return false;
    }

    if (session.NextObjectiveMechanicUpdateAt <= now)
    {
        session.NextObjectiveMechanicUpdateAt = now + 1;
        UpdateMechanics(player, session);
    }

    if (objectiveBg && session.NextObjectiveNodeScanAt <= now)
    {
        session.NextObjectiveNodeScanAt = now + 1;
        for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
        {
            GameObject* playerBanner = objectiveBg->GetBGObject(
                node * BG_AB_OBJECTS_PER_NODE +
                (player->GetTeamId() == TEAM_ALLIANCE ?
                    BG_AB_NODE_STATE_ALLY_OCCUPIED :
                    BG_AB_NODE_STATE_HORDE_OCCUPIED));
            if (playerBanner && playerBanner->isSpawned() &&
                session.ObjectiveNodeOwners[node] != ObjectiveNodeOwner::Player)
            {
                session.ObjectiveNodeOwners[node] = ObjectiveNodeOwner::Player;
                session.ObjectiveWorldStateDirty = true;
            }
        }
    }

    if (objectiveBg)
    {
        SyncObjectiveNodeVisuals(session, objectiveBg);
        if (session.ObjectiveWorldStateDirty ||
            session.NextObjectiveWorldStateSyncAt <= now)
        {
            session.NextObjectiveWorldStateSyncAt = now + 1;
            SyncObjectiveWorldStates(player, session, objectiveBg, now);
            session.ObjectiveWorldStateDirty = false;
        }
    }

    TeamId playerObjectiveTeam = session.Team == TEAM_ALLIANCE ?
        TEAM_ALLIANCE : TEAM_HORDE;
    TeamId shadowObjectiveTeam = playerObjectiveTeam == TEAM_ALLIANCE ?
        TEAM_HORDE : TEAM_ALLIANCE;

    if (session.ObjectiveResourceScores[playerObjectiveTeam] >=
        OBJECTIVE_WIN_RESOURCES)
    {
        session.ObjectiveAllNodesCaptured = true;
        MarkVictory(player->GetGUID());
        SendSystem(player,
            "1600점을 먼저 달성했습니다. 시련에서 승리했습니다.");
        return false;
    }

    Creature* bot = session.BotGuid.IsEmpty() ? nullptr :
        ObjectAccessor::GetCreature(*player, session.BotGuid);
    if (!bot || !bot->IsAlive())
        return true;

    EnsureObjectiveShadowGrounded(player, bot, session);
    SyncShadowPet(player, session, true);
    MatchObjectiveShadowSpeed(player, bot);
    session.State = SessionState::Active;

    bool inShadowCombat = (bot->GetVictim() == player ||
        player->GetVictim() == bot || bot->IsInCombatWith(player) ||
        player->IsInCombatWith(bot));

    if (inShadowCombat && !bot->IsWithinDistInMap(player, 35.0f))
    {
        bot->CombatStop(true);
        bot->SetReactState(REACT_AGGRESSIVE);
        bot->ClearEmoteState();
        player->CombatStopWithPets(true);
        session.ShadowMarkerRoute.clear();
        session.ShadowMarkerRouteIndex = 0;
        inShadowCombat = false;
    }

    for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
    {
        if (session.ShadowNodeActivationEndsAt[node] != 0)
        {
            if (now >= time_t(session.ShadowNodeActivationEndsAt[node]))
            {
                session.ShadowNodeActivationEndsAt[node] = 0;
                session.ShadowNodeCaptureEndsAt[node] = now +
                    BG_AB_FLAG_CAPTURING_TIME.count() / IN_MILLISECONDS;
                session.ObjectiveWorldStateDirty = true;
                SyncObjectiveWorldStates(player, session, objectiveBg, now);
            }
            continue;
        }

        if (session.ShadowNodeCaptureEndsAt[node] == 0)
            continue;

        if (now >= time_t(session.ShadowNodeCaptureEndsAt[node]))
        {
            session.ObjectiveNodeOwners[node] = ObjectiveNodeOwner::Shadow;
            if (objectiveBg)
            {
                objectiveBg->PlaySoundToAll(
                    player->GetTeamId() == TEAM_ALLIANCE ?
                        BG_AB_SOUND_NODE_CAPTURED_HORDE :
                        BG_AB_SOUND_NODE_CAPTURED_ALLIANCE);
            }
            SendSystem(player, Acore::StringFormat(
                "[전장 알림] 그림자가 {} 거점을 점령했습니다.",
                GetObjectiveNodeName(node)));
            LogEvent(player, session, "OBJ_NODE_CAP_SHADOW",
                GetObjectiveNodeName(node));
            session.ShadowNodeCaptureEndsAt[node] = 0;
            session.ShadowTargetNode = -1;
            session.ShadowCurrentSector = int8(GetObjectiveTargetSector(node));
            session.ShadowRoute.clear();
            session.ShadowRouteIndex = 0;
            session.ShadowMarkerRoute.clear();
            session.ShadowMarkerRouteIndex = 0;
            session.ShadowGroundPath.clear();
            session.ShadowGroundPathNode = -1;
            session.NextShadowGroundRepathAt = 0;
            bot->ClearEmoteState();
            bot->SetReactState(REACT_AGGRESSIVE);
            session.ObjectiveWorldStateDirty = true;
            SyncObjectiveNodeVisuals(session, objectiveBg);
            SyncObjectiveWorldStates(player, session, objectiveBg, now);
        }
    }

    if (inShadowCombat)
        return true;

    if (bot->IsWithinDistInMap(player, 18.0f) && bot->IsWithinLOSInMap(player))
    {
        bot->ClearEmoteState();
        StartShadowCombat(player, bot);
        return true;
    }

    bot->CombatStop(true);
    bot->SetReactState(REACT_AGGRESSIVE);
    bot->ClearEmoteState();

    if (session.NextShadowNodeUpdateAt <= now)
    {
        session.NextShadowNodeUpdateAt = now + 1;

        ObjectiveMarkerGraph markerGraph = BuildObjectiveMarkerGraph();

        if (session.ShadowTargetNode >= 0 &&
            session.ShadowTargetNode < BG_AB_DYNAMIC_NODES_COUNT)
        {
            uint8 node = uint8(session.ShadowTargetNode);
            float approachX = 0.0f;
            float approachY = 0.0f;
            float approachZ = 0.0f;
            bool atTargetMarker = false;
            if (!GetObjectiveNextMarkerTowardNode(markerGraph,
                bot->GetPositionX(), bot->GetPositionY(), node,
                approachX, approachY, approachZ, atTargetMarker))
            {
                session.ShadowTargetNode = -1;
                session.ShadowMarkerRoute.clear();
                session.ShadowMarkerRouteIndex = 0;
                return true;
            }
            if (atTargetMarker && bot->GetDistance2d(approachX, approachY) <= 5.0f)
            {
                if (session.ShadowNodeActivationEndsAt[node] == 0 &&
                    session.ShadowNodeCaptureEndsAt[node] == 0)
                {
                    session.ShadowNodeActivationEndsAt[node] = now +
                        OBJECTIVE_FLAG_USE_DELAY_SEC;
                    if (GameObject* banner =
                        GetObjectiveNodeBanner(objectiveBg, node))
                        bot->SetFacingToObject(banner);
                    SendSystem(player, Acore::StringFormat(
                        "[전장 알림] 그림자가 {} 거점 깃발을 사용하고 있습니다.",
                        GetObjectiveNodeName(node)));
                    LogEvent(player, session, "OBJ_NODE_CAP_START",
                        GetObjectiveNodeName(node));
                }

                session.ShadowCurrentSector = int8(GetObjectiveTargetSector(node));
                session.ShadowTargetNode = -1;
                session.ShadowRoute.clear();
                session.ShadowRouteIndex = 0;
                session.ShadowMarkerRoute.clear();
                session.ShadowMarkerRouteIndex = 0;
                session.ShadowGroundPath.clear();
                session.ShadowGroundPathNode = -1;
                session.NextShadowGroundRepathAt = 0;
            }
        }

        if (session.ShadowTargetNode < 0)
        {
            float bestCost = std::numeric_limits<float>::max();
            int8 bestNode = -1;
            int32 startMarkerIndex = GetNearestObjectiveMarkerIndex(
                markerGraph.Markers, bot->GetPositionX(), bot->GetPositionY());

            for (uint8 node = 0; node < BG_AB_DYNAMIC_NODES_COUNT; ++node)
            {
                if (session.ObjectiveNodeOwners[node] ==
                        ObjectiveNodeOwner::Shadow ||
                    session.ShadowNodeActivationEndsAt[node] != 0 ||
                    IsObjectiveNodeShadowCapturing(session, node, now))
                {
                    continue;
                }

                float cost = std::numeric_limits<float>::max();
                int32 nodeMarkerIndex = GetObjectiveNodeMarkerIndex(markerGraph,
                    node);

                if (startMarkerIndex >= 0 && nodeMarkerIndex >= 0)
                {
                    std::vector<size_t> indexRoute;
                    if (BuildObjectiveMarkerIndexRoute(markerGraph,
                        startMarkerIndex, nodeMarkerIndex, indexRoute, &cost))
                    {
                    }
                }

                if (cost < bestCost)
                {
                    bestCost = cost;
                    bestNode = int8(node);
                }
            }

            session.ShadowTargetNode = bestNode;
            session.ShadowRoute.clear();
            session.ShadowRouteIndex = 0;
            session.ShadowMarkerRoute.clear();
            session.ShadowMarkerRouteIndex = 0;
            session.ShadowGroundPath.clear();
            session.ShadowGroundPathNode = -1;
            session.NextShadowGroundRepathAt = 0;
        }

            if (session.ShadowTargetNode >= 0)
        {
            uint8 node = uint8(session.ShadowTargetNode);
            float targetX = 0.0f;
            float targetY = 0.0f;
            float targetZ = 0.0f;
            bool atTargetMarker = false;
            if (!GetObjectiveNextMarkerTowardNode(markerGraph,
                bot->GetPositionX(), bot->GetPositionY(), node,
                targetX, targetY, targetZ, atTargetMarker))
            {
                session.ShadowTargetNode = -1;
                session.ShadowMarkerRoute.clear();
                session.ShadowMarkerRouteIndex = 0;
                return true;
            }

            if (Map* map = player->GetMap())
            {
                map->CanReachPositionAndGetValidCoords(bot,
                    targetX, targetY, targetZ, true, true);
                targetZ = ResolveArenaGroundZ(map,
                    targetX, targetY, targetZ);
            }

            bot->GetMotionMaster()->MovePoint(9000 + node,
                targetX, targetY, targetZ);

            MatchObjectiveShadowSpeed(player, bot);
        }
    }

    if (session.ObjectiveResourceScores[shadowObjectiveTeam] >=
        OBJECTIVE_WIN_RESOURCES)
    {
        session.Result = ArenaResult::Failure;
        session.State = SessionState::PendingFinish;
        session.FailedAt = now;
        session.EndedAt = session.FailedAt;
        session.FinishDelayMs = 1;
        LogEvent(player, session, "OBJECTIVE_SHADOW_WON");
        NotifyObjectiveFinishReason(player,
            "그림자가 먼저 1600점을 달성했습니다");
        return false;
    }

    SyncObjectiveNodeVisuals(session, objectiveBg);
    if (objectiveBg &&
        (session.ObjectiveWorldStateDirty ||
         session.NextObjectiveWorldStateSyncAt <= now))
    {
        session.NextObjectiveWorldStateSyncAt = now + 1;
        SyncObjectiveWorldStates(player, session, objectiveBg, now);
        session.ObjectiveWorldStateDirty = false;
    }

    return true;
}

void SoloArenaMgr::MarkVictory(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return;

    itr->second.Result = ArenaResult::Victory;
    itr->second.State = SessionState::PendingFinish;
    itr->second.EndedAt = std::time(nullptr);
    itr->second.CompletedAt = itr->second.EndedAt;
    itr->second.CombatEndsAt = itr->second.EndedAt;
    itr->second.CombatDurationSec = GetRankDurationSec(itr->second);
    std::tie(itr->second.RankValue, itr->second.RankLabel) =
        ComputeTrialRank(itr->second);
    itr->second.FinishDelayMs = 3000;

    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid))
    {
        LogEvent(player, itr->second, "VICTORY");
        SendTrialTimePayload(player, itr->second, true);
        SpeakTrialTaunt(player, itr->second, "victory");
    }
}

void SoloArenaMgr::MarkFailure(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return;

    bool immediateFinish = false;
    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid))
        immediateFinish = !player->IsAlive();

    itr->second.Result = ArenaResult::Failure;
    itr->second.State = SessionState::PendingFinish;
    itr->second.EndedAt = std::time(nullptr);
    itr->second.FailedAt = itr->second.EndedAt;
    itr->second.CombatEndsAt = itr->second.EndedAt;
    itr->second.CombatDurationSec = GetRankDurationSec(itr->second);
    std::tie(itr->second.RankValue, itr->second.RankLabel) =
        ComputeTrialRank(itr->second);
    itr->second.FinishDelayMs = immediateFinish ? 1 : 3000;

    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid))
    {
        LogEvent(player, itr->second, "FAILURE");
        SendTrialTimePayload(player, itr->second, true);
        SpeakTrialTaunt(player, itr->second, "failure");
    }
}

void SoloArenaMgr::MarkAbandoned(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return;

    itr->second.Result = ArenaResult::Abandoned;
    itr->second.State = SessionState::PendingFinish;
    itr->second.EndedAt = std::time(nullptr);
    itr->second.AbandonedAt = itr->second.EndedAt;
    if (itr->second.CombatStartedAt != 0)
        itr->second.CombatEndsAt = itr->second.EndedAt;
    itr->second.CombatDurationSec = GetRankDurationSec(itr->second);
    std::tie(itr->second.RankValue, itr->second.RankLabel) =
        ComputeTrialRank(itr->second);
    itr->second.FinishDelayMs = 1;

    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid))
    {
        LogEvent(player, itr->second, "ABANDONED");
        SendTrialTimePayload(player, itr->second, true);
        SpeakTrialTaunt(player, itr->second, "abandoned");
    }
}

bool SoloArenaMgr::RespawnObjectivePlayer(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return false;

    ArenaSession& session = itr->second;
    if (session.Scenario != TrialScenario::Objective)
        return false;

    Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid);
    if (!player)
        return false;

    if (player->IsAlive())
        return true;

    if (session.PlayerRespawnAt != 0)
        return true;

    session.PlayerRespawnAt = uint64(std::time(nullptr)) +
        OBJECTIVE_RESPAWN_DELAY_SEC;
    SendSystem(player, "당신은 15초 후 시작 위치에서 다시 부활합니다.");
    LogEvent(player, session, "OBJ_PLAYER_RESPAWN_WAIT");
    SendTrialTimePayload(player, session, true);
    return true;
}

bool SoloArenaMgr::RespawnObjectiveShadow(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return false;

    ArenaSession& session = itr->second;
    if (session.Scenario != TrialScenario::Objective)
        return false;

    Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid);
    if (!player)
        return false;

    if (session.ShadowRespawnAt != 0)
        return true;

    session.BotGuid = ObjectGuid::Empty;
    session.PetGuid = ObjectGuid::Empty;
    session.ShadowTargetNode = -1;
    session.ShadowNodeActivationEndsAt.fill(0);
    session.ShadowNodeCaptureEndsAt.fill(0);
    session.ShadowMarkerRoute.clear();
    session.ShadowMarkerRouteIndex = 0;
    session.ShadowGroundPath.clear();
    session.ShadowGroundPathNode = -1;
    session.NextShadowGroundRepathAt = 0;
    session.ShadowRespawnAt = uint64(std::time(nullptr)) +
        OBJECTIVE_RESPAWN_DELAY_SEC;
    SendSystem(player, "그림자가 15초 후 시작 위치에서 다시 부활합니다.");
    LogEvent(player, session, "OBJ_SHADOW_RESPAWN_WAIT");
    SendTrialTimePayload(player, session, true);
    return true;
}

void SoloArenaMgr::ProcessObjectiveRespawns(Player* player,
    ArenaSession& session, uint64 now)
{
    if (!player || session.Scenario != TrialScenario::Objective)
        return;

    if (session.PlayerRespawnAt != 0 && now >= session.PlayerRespawnAt)
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float o = 0.0f;
        GetObjectiveStartLocation(session.Team, x, y, z, o);
        if (Map* map = sMapMgr->CreateBaseMap(session.ArenaMapId))
            z = ResolveArenaGroundZ(map, x, y, z);

        player->ResurrectPlayer(1.0f);
        player->SpawnCorpseBones();
        player->SetFullHealth();
        if (Powers powerType = player->getPowerType();
            powerType != POWER_HEALTH)
        {
            player->SetPower(powerType, player->GetMaxPower(powerType));
        }
        player->CombatStopWithPets(true);
        player->TeleportTo(session.ArenaMapId, x, y, z, o, TELE_TO_GM_MODE);
        session.PlayerRespawnAt = 0;
        SendSystem(player, "당신이 시작 위치에서 다시 부활했습니다.");
        LogEvent(player, session, "OBJ_PLAYER_RESPAWN");
        SendTrialTimePayload(player, session, true);
    }

    if (session.ShadowRespawnAt != 0 && now >= session.ShadowRespawnAt)
    {
        CleanupBot(session);
        session.BotGuid = ObjectGuid::Empty;
        session.PetGuid = ObjectGuid::Empty;
        session.ShadowTargetNode = -1;
        session.ShadowNodeActivationEndsAt.fill(0);
        session.ShadowNodeCaptureEndsAt.fill(0);
        session.ShadowMarkerRoute.clear();
        session.ShadowMarkerRouteIndex = 0;
        session.ShadowGroundPath.clear();
        session.ShadowGroundPathNode = -1;
        session.NextShadowGroundRepathAt = 0;
        session.ShadowRespawnAt = 0;

        if (SpawnShadow(player, session))
        {
            SendSystem(player, "그림자가 시작 위치에서 다시 부활했습니다.");
            LogEvent(player, session, "OBJ_SHADOW_RESPAWN");
        }
        else
        {
            session.ShadowRespawnAt = now + 5;
            SendSystem(player,
                "그림자 재소환이 지연되었습니다. 5초 후 다시 시도합니다.");
            LogEvent(player, session, "OBJ_SHADOW_RESPAWN_RETRY");
        }
        SendTrialTimePayload(player, session, true);
    }
}

void SoloArenaMgr::EnsureObjectiveShadowGrounded(Player* player, Creature* bot,
    ArenaSession& session)
{
    if (!player || !bot || session.Scenario != TrialScenario::Objective)
        return;

    bot->SetCanFly(false);
    bot->SetDisableGravity(false);
    bot->SetHover(false);
    bot->SetWaterWalking(false);
    bot->SetSwim(false);
    bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING |
        MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY);

    float expectedX = bot->GetPositionX();
    float expectedY = bot->GetPositionY();
    float expectedZ = bot->GetPositionZ();

    if (!session.ShadowMarkerRoute.empty())
    {
        G3D::Vector3 const& marker =
            session.ShadowMarkerRoute.back();
        expectedX = marker.x;
        expectedY = marker.y;
        expectedZ = marker.z;
    }
    else if (!session.ShadowGroundPath.empty())
    {
        G3D::Vector3 const& end = session.ShadowGroundPath.back();
        expectedX = end.x;
        expectedY = end.y;
        expectedZ = end.z;
    }
    else if (session.ShadowTargetNode >= 0 &&
        session.ShadowTargetNode < BG_AB_DYNAMIC_NODES_COUNT)
    {
        float o = 0.0f;
        if (!GetObjectiveNodeMarkerLocation(uint8(session.ShadowTargetNode),
            expectedX, expectedY, expectedZ))
        {
            GetObjectiveStartLocation(Battleground::GetOtherTeamId(session.Team),
                expectedX, expectedY, expectedZ, o);
        }
    }

    float groundZ = ResolveArenaGroundZ(player->GetMap(), bot->GetPositionX(),
        bot->GetPositionY(), bot->GetPositionZ());
    float expectedGroundZ = ResolveArenaGroundZ(player->GetMap(), expectedX,
        expectedY, expectedZ);

    bool isFlyingState = bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) ||
        bot->HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY) ||
        bot->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
    bool tooHigh = std::fabs(bot->GetPositionZ() - groundZ) > 12.0f;
    if (bot->IsInWater() || isFlyingState || tooHigh)
    {
        if (Map* map = player->GetMap())
            map->CanReachPositionAndGetValidCoords(bot,
                expectedX, expectedY, expectedGroundZ, true, true);

        // Objective shadow should always recover by running back to the
        // nearest road point, never by teleporting to a node.
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        bot->CombatStop(true);
        bot->SetReactState(REACT_AGGRESSIVE);
        bot->ClearEmoteState();
        session.ShadowRoute.clear();
        session.ShadowRouteIndex = 0;
        session.ShadowMarkerRoute.clear();
        session.ShadowMarkerRouteIndex = 0;
        session.ShadowGroundPath.clear();
        session.ShadowGroundPathNode = -1;
        session.NextShadowGroundRepathAt = 0;
        bot->GetMotionMaster()->MovePoint(9800, expectedX, expectedY,
            expectedGroundZ);
    }
}

ShadowProfile const* SoloArenaMgr::GetShadowProfile(
    ObjectGuid const& creatureGuid) const
{
    auto itr = _shadowProfiles.find(creatureGuid.GetCounter());
    if (itr == _shadowProfiles.end())
        return nullptr;

    return &itr->second;
}

void SoloArenaMgr::UnregisterShadow(ObjectGuid const& creatureGuid)
{
    _shadowProfiles.erase(creatureGuid.GetCounter());
}

bool SoloArenaMgr::IsManagedShadow(Creature const* creature) const
{
    if (!creature)
        return false;

    return _shadowProfiles.find(creature->GetGUID().GetCounter()) !=
        _shadowProfiles.end();
}

bool SoloArenaMgr::IsManagedArenaInstance(uint32 instanceId) const
{
    return _managedArenaInstances.find(instanceId) !=
        _managedArenaInstances.end();
}

bool SoloArenaMgr::SpawnShadow(Player* player, ArenaSession& session)
{
    StageConfig const* stage = GetStage(session.StageId);
    if (!stage)
        return false;

    StageConfig shadowStage = *stage;
    if (session.Scenario == TrialScenario::Objective)
    {
        shadowStage.ArenaMapId = DEFAULT_OBJECTIVE_MAP_ID;
        if (!GetObjectiveStartMarkerLocation(
                Battleground::GetOtherTeamId(session.Team),
                shadowStage.BotX, shadowStage.BotY,
                shadowStage.BotZ, shadowStage.BotO))
        {
            GetObjectiveStartLocation(Battleground::GetOtherTeamId(session.Team),
                shadowStage.BotX, shadowStage.BotY,
                shadowStage.BotZ, shadowStage.BotO);
        }
    }

    ShadowProfile profile = CaptureShadowProfile(player, shadowStage);

    float botZ = shadowStage.BotZ;
    if (Map* map = player->GetMap())
        botZ = ResolveArenaGroundZ(map, shadowStage.BotX, shadowStage.BotY,
            shadowStage.BotZ);

    TempSummon* summon = player->SummonCreature(
        SoloArenaConfig::Instance().GetShadowEntry(),
        shadowStage.BotX, shadowStage.BotY, botZ, shadowStage.BotO,
        TEMPSUMMON_MANUAL_DESPAWN, 0);

    if (!summon)
    {
        return false;
    }

    ConfigureShadow(summon, player, profile, shadowStage);

    session.BotGuid = summon->GetGUID();
    session.ArenaInstanceId = player->GetInstanceId();
    if (session.Scenario == TrialScenario::Duel)
        session.State = SessionState::WaitingForStart;

    _shadowProfiles[summon->GetGUID().GetCounter()] = profile;

    summon->ClearInCombat();
    player->ClearInCombat();
    if (session.Scenario == TrialScenario::Objective)
        EnsureObjectiveShadowGrounded(player, summon, session);
    SyncShadowPet(player, session, false);

    if (WorldSession* playerSession = player->GetSession())
    {
        WorldPacket data;
        BuildMirrorImagePacket(data, summon->GetGUID(), player);
        playerSession->SendPacket(&data);
    }

    if (session.Scenario == TrialScenario::Objective)
    {
        session.ShadowTargetNode = -1;
        session.ShadowCurrentSector =
            Battleground::GetOtherTeamId(session.Team) == TEAM_ALLIANCE ?
                0 : 6;
        session.ShadowNodeActivationEndsAt.fill(0);
        session.ShadowNodeCaptureEndsAt.fill(0);
        session.ShadowRoute.clear();
        session.ShadowRouteIndex = 0;
        session.ShadowMarkerRoute.clear();
        session.ShadowMarkerRouteIndex = 0;
        session.ShadowGroundPath.clear();
        session.ShadowGroundPathNode = -1;
        session.NextShadowGroundRepathAt = 0;
        SendSystem(player,
            "그림자가 반대 진영 기지에서 움직이기 시작했습니다.");
    }
    else
    {
        SendSystem(player,
            "그림자가 모습을 드러냈습니다. 문이 열리면 전투가 시작됩니다.");
    }
    LogEvent(player, session, "SHADOW_SPAWNED");
    SpeakTrialTaunt(player, session, "spawn");
    return true;
}

void SoloArenaMgr::ConfigureShadow(Creature* summon, Player* player,
    ShadowProfile const& profile, StageConfig const& stage)
{
    summon->SetName(profile.PlayerName);
    ApplyShadowCloneVisual(player, summon);
    summon->SetLevel(profile.Level);
    summon->SetByteValue(UNIT_FIELD_BYTES_0, 0, profile.PlayerRace);
    summon->SetByteValue(UNIT_FIELD_BYTES_0, 1, profile.PlayerClass);
    summon->SetByteValue(UNIT_FIELD_BYTES_0, 2, profile.PlayerGender);
    summon->SetByteValue(UNIT_FIELD_BYTES_2, 0, profile.SheathState);
    summon->SetObjectScale(profile.ObjectScale);

    summon->SetVirtualItem(0, profile.MainHandEntry);
    summon->SetVirtualItem(1, profile.OffHandEntry);
    summon->SetVirtualItem(2, profile.RangedEntry);
    summon->SetCanDualWield(profile.MainHandEntry != 0 &&
        profile.CanDualWield);

    for (uint32 stat = STAT_STRENGTH; stat < MAX_STATS; ++stat)
        summon->SetStat(Stats(stat), profile.Stats[stat]);

    summon->SetArmor(profile.Armor);
    for (uint32 school = SPELL_SCHOOL_HOLY;
         school < MAX_SPELL_SCHOOL; ++school)
    {
        summon->SetResistance(SpellSchools(school),
            profile.Resistances[school]);
    }

    summon->setPowerType(profile.PowerType);
    summon->SetMaxPower(profile.PowerType, profile.MaxPower);
    summon->SetPower(profile.PowerType, profile.MaxPower);

    uint32 maxHealth = std::max<uint32>(5000u,
        static_cast<uint32>(profile.MaxHealth *
            stage.HealthMultiplier));
    summon->SetMaxHealth(maxHealth);
    summon->SetHealth(maxHealth);

    uint32 baseAttackTime = profile.AttackTimeMs[BASE_ATTACK] ?
        std::min(profile.AttackTimeMs[BASE_ATTACK], stage.AttackTimeMs) :
        stage.AttackTimeMs;
    summon->SetAttackTime(BASE_ATTACK, baseAttackTime);
    summon->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE,
        std::max(1.0f, profile.WeaponMinDamage[BASE_ATTACK] *
            stage.DamageMultiplier));
    summon->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE,
        std::max(2.0f, profile.WeaponMaxDamage[BASE_ATTACK] *
            stage.DamageMultiplier));
    summon->UpdateDamagePhysical(BASE_ATTACK);

    if (profile.AttackTimeMs[OFF_ATTACK] != 0)
    {
        summon->SetAttackTime(OFF_ATTACK,
            std::min(profile.AttackTimeMs[OFF_ATTACK], stage.AttackTimeMs));
        summon->SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE,
            std::max(1.0f, profile.WeaponMinDamage[OFF_ATTACK] *
                stage.DamageMultiplier));
        summon->SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE,
            std::max(2.0f, profile.WeaponMaxDamage[OFF_ATTACK] *
                stage.DamageMultiplier));
        summon->UpdateDamagePhysical(OFF_ATTACK);
    }

    if (profile.AttackTimeMs[RANGED_ATTACK] != 0)
    {
        summon->SetAttackTime(RANGED_ATTACK,
            std::min(profile.AttackTimeMs[RANGED_ATTACK], stage.AttackTimeMs));
        summon->SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE,
            std::max(1.0f, profile.WeaponMinDamage[RANGED_ATTACK] *
                stage.DamageMultiplier));
        summon->SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE,
            std::max(2.0f, profile.WeaponMaxDamage[RANGED_ATTACK] *
                stage.DamageMultiplier));
        summon->UpdateDamagePhysical(RANGED_ATTACK);
    }

    summon->SetFloatValue(UNIT_MOD_CAST_SPEED,
        std::max(0.5f, profile.CastSpeedRate));
    summon->SetSpeed(MOVE_RUN, std::max(profile.RunSpeedRate,
        stage.MoveSpeedRate), true);
    summon->SetCanFly(false);
    summon->SetDisableGravity(false);
    summon->SetHover(false);
    summon->SetReactState(REACT_PASSIVE);
    summon->SetFaction(14);
    summon->SetWalk(false);
    summon->StopMoving();
    summon->SetHomePosition(stage.BotX, stage.BotY, stage.BotZ, stage.BotO);
}

void SoloArenaMgr::FinishSession(Player* player, ArenaSession& session)
{
    ClearMechanicObject(session);
    ClearMechanicSummons(session);
    player->RemoveAurasDueToSpell(TRIAL_MECHANIC_BUFF_AURA);
    CleanupPet(session);
    CleanupBot(session);

    session.CombatDurationSec = GetRankDurationSec(session);
    if (session.RankLabel.empty())
        std::tie(session.RankValue, session.RankLabel) =
            ComputeTrialRank(session);

    if (session.Result == ArenaResult::Failure && !player->IsAlive())
    {
        player->ResurrectPlayer(1.0f);
        player->SpawnCorpseBones();
    }

    switch (session.Result)
    {
        case ArenaResult::Victory:
            SaveStageRecord(player, session);
            GrantStageRewards(player, session);
            if (session.RankValue >= 3)
            {
                SaveProgress(player, session.StageId);
                SendSystem(player, Acore::StringFormat(
                    "{} 성공. B랭크 이상 달성으로 다음 단계가 해금됩니다. 결과를 확인한 뒤 복귀할 수 있습니다.",
                    GetStageName(session.StageId)));
            }
            else
            {
                SendSystem(player, Acore::StringFormat(
                    "{} 승리. 하지만 B랭크 이상을 달성하지 못해 다음 단계는 해금되지 않습니다.",
                    GetStageName(session.StageId)));
            }
            break;
        case ArenaResult::Failure:
            SendSystem(player, Acore::StringFormat(
                "{} 실패. 결과를 확인한 뒤 복귀할 수 있습니다.",
                GetStageName(session.StageId)));
            break;
        case ArenaResult::Abandoned:
            SendSystem(player, "시련을 포기했습니다. 결과를 확인한 뒤 복귀할 수 있습니다.");
            break;
        default:
            break;
    }

    LogRun(player, session);
    LogEvent(player, session, "RUN_FINISHED");
    session.State = SessionState::AwaitingReturn;
    SendTrialTimePayload(player, session, true);
    SendResultPayload(player, session);
}

std::string SoloArenaMgr::RequestTrialTaunt(Player* player,
    ArenaSession const& session, std::string const& eventType) const
{
    if (!player)
        return "";

    Pet* pet = player->GetPet();
    std::string petName = pet ? pet->GetName() : "";
    std::string payload = Acore::StringFormat(
        "{{\"player_name\":\"{}\",\"player_class\":\"{}\","
        "\"stage_id\":{},\"event_type\":\"{}\",\"has_pet\":{},"
        "\"pet_name\":\"{}\"}}",
        EscapeJson(player->GetName()),
        EscapeJson(GetTrialClassLabel(player->getClass())),
        uint32(session.StageId),
        EscapeJson(eventType),
        pet ? "true" : "false",
        EscapeJson(petName));

    httplib::Client client("127.0.0.1", 8000);
    client.set_connection_timeout(0, 300000);
    client.set_read_timeout(0, 500000);
    client.set_write_timeout(0, 300000);

    if (auto response = client.Post("/solo-arena/taunt", payload,
            "application/json"))
    {
        if (response->status == 200)
        {
            std::string line = ExtractJsonStringField(response->body, "line");
            if (!line.empty())
                return line;
        }
    }

    return BuildTrialTauntFallback(player, player->getClass(),
        session.StageId, eventType);
}

void SoloArenaMgr::SpeakTrialTaunt(Player* player, ArenaSession const& session,
    std::string const& eventType) const
{
    std::string line = RequestTrialTaunt(player, session, eventType);
    if (line.empty())
        return;

    if (!session.BotGuid.IsEmpty() && player)
        if (Creature* bot = ObjectAccessor::GetCreature(*player,
                session.BotGuid))
        {
            bot->SetFacingToObject(player);
            bot->Yell(line, LANG_UNIVERSAL);
            return;
        }

    if (!session.PetGuid.IsEmpty() && player)
        if (Creature* pet = ObjectAccessor::GetCreature(*player,
                session.PetGuid))
        {
            pet->SetFacingToObject(player);
            pet->Yell(line, LANG_UNIVERSAL);
            return;
        }

    SendSystem(player, line);
}

bool SoloArenaMgr::SyncShadowPet(Player* player, ArenaSession& session,
    bool startCombat)
{
    if (!player)
        return false;

    StageConfig const* stage = GetStage(session.StageId);
    if (!stage)
        return false;

    Pet* playerPet = player->GetPet();
    if (!playerPet || !playerPet->IsAlive())
    {
        if (startCombat && !session.PetGuid.IsEmpty())
            if (Creature* shadowPet = ObjectAccessor::GetCreature(*player,
                    session.PetGuid))
            {
                Unit* desiredTarget = player;
                shadowPet->SetReactState(REACT_AGGRESSIVE);
                shadowPet->SetHomePosition(shadowPet->GetPositionX(),
                    shadowPet->GetPositionY(), shadowPet->GetPositionZ(),
                    shadowPet->GetOrientation());
                shadowPet->SetInCombatWith(desiredTarget);
                desiredTarget->SetInCombatWith(shadowPet);
                shadowPet->Attack(desiredTarget, true);
                shadowPet->GetMotionMaster()->Clear();
                shadowPet->GetMotionMaster()->MoveChase(desiredTarget, 1.5f);
            }

        return true;
    }

    uint32 petEntry = playerPet->GetEntry();
    uint32 petDisplayId = playerPet->GetDisplayId();
    if (!petEntry || !petDisplayId)
        return true;

    if (!session.PetGuid.IsEmpty() &&
        session.PetEntry == petEntry &&
        session.PetDisplayId == petDisplayId)
    {
        if (Creature* shadowPet = ObjectAccessor::GetCreature(*player,
                session.PetGuid))
        {
            CopyShadowPetStats(shadowPet, playerPet);
            if (startCombat)
            {
                Unit* desiredTarget = SelectShadowPetTarget(player);
                if (!desiredTarget)
                    desiredTarget = player;

                if (shadowPet->GetDistance(desiredTarget) > 35.0f ||
                    !shadowPet->IsWithinLOSInMap(desiredTarget))
                {
                    float angle = desiredTarget->GetOrientation() +
                        float(M_PI) * 0.35f;
                    float x = desiredTarget->GetPositionX() +
                        std::cos(angle) * 3.0f;
                    float y = desiredTarget->GetPositionY() +
                        std::sin(angle) * 3.0f;
                    float z = desiredTarget->GetPositionZ();
                    if (Map* map = shadowPet->GetMap())
                        z = ResolveArenaGroundZ(map, x, y, z);
                    shadowPet->GetMotionMaster()->Clear();
                    shadowPet->GetMotionMaster()->MovePoint(1, x, y, z);
                }

                shadowPet->SetReactState(REACT_AGGRESSIVE);
                shadowPet->SetHomePosition(shadowPet->GetPositionX(),
                    shadowPet->GetPositionY(), shadowPet->GetPositionZ(),
                    shadowPet->GetOrientation());
                shadowPet->SetInCombatWith(desiredTarget);
                desiredTarget->SetInCombatWith(shadowPet);
                shadowPet->Attack(desiredTarget, true);
                shadowPet->GetMotionMaster()->Clear();
                shadowPet->GetMotionMaster()->MoveChase(desiredTarget, 1.5f);
            }
        }

        return true;
    }

    if (!session.PetGuid.IsEmpty())
    {
        if (Creature* shadowPet = ObjectAccessor::GetCreature(*player,
                session.PetGuid))
        {
            if (!shadowPet->IsAlive())
            {
                CleanupPet(session);
                LogEvent(player, session, "SHADOW_PET_DESPAWNED");
            }
            else
                CleanupPet(session);
        }
        else
            CleanupPet(session);
    }

    float petZ = stage->BotZ;
    if (Map* map = player->GetMap())
        petZ = ResolveArenaGroundZ(map, stage->BotX + 2.0f, stage->BotY,
            stage->BotZ);

    TempSummon* shadowPet = player->SummonCreature(
        petEntry,
        stage->BotX + 2.0f, stage->BotY, petZ, stage->BotO,
        TEMPSUMMON_MANUAL_DESPAWN, 0);
    if (!shadowPet)
    {
        Debug("Solo arena pet sync failed: player='{}' petEntry={}",
            player->GetName(), petEntry);
        LogEvent(player, session, "SHADOW_PET_SYNC_FAILED",
            Acore::StringFormat("entry={}", petEntry));
        return false;
    }

    shadowPet->SetName(playerPet->GetName());
    shadowPet->SetDisplayId(petDisplayId);
    shadowPet->SetNativeDisplayId(petDisplayId);
    shadowPet->SetObjectScale(playerPet->GetObjectScale());
    shadowPet->SetFaction(14);
    shadowPet->SetWalk(false);
    shadowPet->SetReactState(startCombat ? REACT_AGGRESSIVE : REACT_PASSIVE);
    shadowPet->SetHomePosition(shadowPet->GetPositionX(),
        shadowPet->GetPositionY(), shadowPet->GetPositionZ(),
        shadowPet->GetOrientation());
    CopyShadowPetStats(shadowPet, playerPet);
    shadowPet->StopMoving();

    session.PetGuid = shadowPet->GetGUID();
    session.PetEntry = petEntry;
    session.PetDisplayId = petDisplayId;

    if (startCombat)
    {
        Unit* desiredTarget = SelectShadowPetTarget(player);
        if (!desiredTarget)
            desiredTarget = player;

        shadowPet->SetInCombatWith(desiredTarget);
        desiredTarget->SetInCombatWith(shadowPet);
        shadowPet->Attack(desiredTarget, true);
        shadowPet->GetMotionMaster()->Clear();
        shadowPet->GetMotionMaster()->MoveChase(desiredTarget, 1.5f);
    }

    LogEvent(player, session, "SHADOW_PET_SUMMONED",
        Acore::StringFormat("entry={}", petEntry));
    return true;
}

void SoloArenaMgr::CleanupPet(ArenaSession& session)
{
    if (session.PetGuid.IsEmpty())
        return;

    if (Player* player = ObjectAccessor::FindPlayer(session.PlayerGuid))
        if (Creature* pet = ObjectAccessor::GetCreature(*player,
                session.PetGuid))
            pet->DespawnOrUnsummon(Milliseconds(1));

    session.PetGuid = ObjectGuid::Empty;
    session.PetEntry = 0;
    session.PetDisplayId = 0;
}

void SoloArenaMgr::CleanupBot(ArenaSession const& session)
{
    if (session.BotGuid.IsEmpty())
        return;

    Player* player = ObjectAccessor::FindPlayer(session.PlayerGuid);
    if (!player)
    {
        _shadowProfiles.erase(session.BotGuid.GetCounter());
        return;
    }

    if (Creature* bot = ObjectAccessor::GetCreature(*player, session.BotGuid))
        bot->DespawnOrUnsummon(Milliseconds(1));

    _shadowProfiles.erase(session.BotGuid.GetCounter());
}

uint8 SoloArenaMgr::GetHighestStageCleared(Player* player) const
{
    if (!player)
        return 0;

    QueryResult result = CharacterDatabase.Query(
        "SELECT highest_stage_cleared "
        "FROM solo_arena_progress WHERE guid = {}",
        player->GetGUID().GetCounter());

    if (!result)
        return 0;

    return result->Fetch()[0].Get<uint8>();
}

bool SoloArenaMgr::IsStageUnlocked(Player* player, uint8 stageId) const
{
    return stageId <= GetHighestStageCleared(player) + 1;
}

std::pair<uint8, std::string> SoloArenaMgr::ComputeTrialRank(
    ArenaSession const& session) const
{
    if (session.Result != ArenaResult::Victory)
        return { 0, "F" };

    uint32 durationSec = GetRankDurationSec(session);
    if (IsObjectiveTrialStage(session.StageId))
    {
        if (durationSec <= 360)
            return { 5, "S" };
        if (durationSec <= 480)
            return { 4, "A" };
        if (durationSec <= 600)
            return { 3, "B" };
        if (durationSec <= 720)
            return { 2, "C" };
        return { 1, "D" };
    }

    if (durationSec <= 45)
        return { 5, "S" };
    if (durationSec <= 75)
        return { 4, "A" };
    if (durationSec <= 105)
        return { 3, "B" };
    if (durationSec <= 135)
        return { 2, "C" };
    return { 1, "D" };
}

uint32 SoloArenaMgr::GetTodayEntryCount(Player* player) const
{
    if (!player)
        return 0;

    QueryResult result = CharacterDatabase.Query(
        "SELECT COUNT(*) "
        "FROM solo_arena_run_log "
        "WHERE guid = {} "
        "AND started_at >= UNIX_TIMESTAMP(CURDATE()) "
        "AND started_at < UNIX_TIMESTAMP(DATE_ADD(CURDATE(), INTERVAL 1 DAY))",
        player->GetGUID().GetCounter());

    if (!result)
        return 0;

    return result->Fetch()[0].Get<uint32>();
}

void SoloArenaMgr::SaveProgress(Player* player, uint8 stageId)
{
    uint8 highestStage = GetHighestStageCleared(player);
    if (stageId <= highestStage)
        return;

    CharacterDatabase.Execute(
        "REPLACE INTO solo_arena_progress "
        "(guid, highest_stage_cleared, updated_at) "
        "VALUES ({}, {}, {})",
        player->GetGUID().GetCounter(), stageId, std::time(nullptr));
}

void SoloArenaMgr::SaveStageRecord(Player* player, ArenaSession const& session)
{
    if (!player || session.Result != ArenaResult::Victory ||
        session.RankLabel.empty())
        return;

    uint32 durationSec = GetRankDurationSec(session);
    QueryResult result = CharacterDatabase.Query(
        "SELECT best_rank, best_time_sec "
        "FROM solo_arena_stage_record "
        "WHERE guid = {} AND stage_id = {}",
        player->GetGUID().GetCounter(), session.StageId);

    bool shouldSave = true;
    if (result)
    {
        Field* fields = result->Fetch();
        uint8 bestRank = fields[0].Get<uint8>();
        uint32 bestTime = fields[1].Get<uint32>();
        shouldSave = session.RankValue > bestRank ||
            (session.RankValue == bestRank &&
             (bestTime == 0 || durationSec < bestTime));
    }

    if (!shouldSave)
        return;

    CharacterDatabase.Execute(
        "REPLACE INTO solo_arena_stage_record "
        "(guid, stage_id, best_rank, best_rank_label, best_time_sec, "
        "updated_at) VALUES ({}, {}, {}, '{}', {}, {})",
        player->GetGUID().GetCounter(),
        session.StageId,
        session.RankValue,
        EscapeCharacterDb(session.RankLabel),
        durationSec,
        std::time(nullptr));
}

void SoloArenaMgr::GrantStageRewards(Player* player, ArenaSession const& session)
{
    if (!player)
        return;

    QueryResult result = WorldDatabase.Query(
        "SELECT item_entry, item_count, chance, reward_rank_value, "
        "reward_rank_label "
        "FROM solo_arena_stage_reward "
        "WHERE stage_id = {} AND enabled = 1 "
        "AND (reward_rank_value = 0 OR reward_rank_value = {}) "
        "ORDER BY sort_order, id",
        session.StageId, session.RankValue);

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 itemEntry = fields[0].Get<uint32>();
        uint32 itemCount = std::max<uint32>(1, fields[1].Get<uint32>());
        float chance = fields[2].Get<float>();
        uint8 rewardRankValue = fields[3].Get<uint8>();
        std::string rewardRankLabel = fields[4].Get<std::string>();
        float roll = float(std::rand() % 10000) / 100.0f;

        if (chance > 0.0f && roll > chance)
        {
            LogReward(player, session, itemEntry, itemCount, chance,
                Acore::StringFormat("SKIPPED:{}:{}",
                    rewardRankValue, rewardRankLabel));
            continue;
        }

        if (player->AddItem(itemEntry, itemCount))
        {
            LogReward(player, session, itemEntry, itemCount, chance,
                Acore::StringFormat("GRANTED:{}:{}",
                    rewardRankValue, rewardRankLabel));
            continue;
        }

        LogReward(player, session, itemEntry, itemCount, chance,
            Acore::StringFormat("FAILED:{}:{}",
                rewardRankValue, rewardRankLabel));
    } while (result->NextRow());
}

void SoloArenaMgr::LogRun(Player* player, ArenaSession const& session)
{
    if (!player)
        return;

    CharacterDatabase.Execute(
        "INSERT INTO solo_arena_run_log ("
        "run_uid, guid, account_id, player_name, stage_id, stage_name, "
        "result, result_label, session_state, started_at, "
        "preparation_ends_at, combat_started_at, combat_ended_at, "
        "ended_at, completed_at, failed_at, abandoned_at, duration_sec, "
        "rank_value, rank_label, combat_duration_sec, arena_map_id, "
        "arena_instance_id, return_map_id"
        ") VALUES ("
        "{}, {}, {}, '{}', {}, '{}', {}, '{}', {}, {}, {}, {}, "
        "{}, {}, {}, {}, {}, {}, {}, '{}', {}, {}, {}, {}"
        ")",
        session.RunUid,
        player->GetGUID().GetCounter(),
        player->GetSession() ? player->GetSession()->GetAccountId() : 0,
        EscapeCharacterDb(player->GetName()),
        session.StageId,
        EscapeCharacterDb(GetStageName(session.StageId)),
        static_cast<uint8>(session.Result),
        EscapeCharacterDb(
            session.Result == ArenaResult::Victory ? "SUCCESS" :
            session.Result == ArenaResult::Failure ? "FAILURE" :
            session.Result == ArenaResult::Abandoned ? "ABANDONED" : "NONE"),
        static_cast<uint8>(session.State),
        session.StartedAt,
        session.PreparationEndsAt,
        session.CombatStartedAt,
        session.CombatEndsAt,
        session.EndedAt,
        session.CompletedAt,
        session.FailedAt,
        session.AbandonedAt,
        session.EndedAt > session.StartedAt ?
            uint32(session.EndedAt - session.StartedAt) : 0,
        session.RankValue,
        EscapeCharacterDb(session.RankLabel),
        session.CombatDurationSec,
        session.ArenaMapId,
        session.ArenaInstanceId,
        session.ReturnMapId);
}

void SoloArenaMgr::LogEvent(Player* player, ArenaSession const& session,
    std::string const& eventType, std::string const& note)
{
    if (!player)
        return;

    CharacterDatabase.Execute(
        "INSERT INTO solo_arena_event_log "
        "(run_uid, guid, account_id, player_name, stage_id, event_type, "
        "event_at, map_id, arena_instance_id, note) "
        "VALUES ({}, {}, {}, '{}', {}, '{}', {}, {}, {}, '{}')",
        session.RunUid,
        player->GetGUID().GetCounter(),
        player->GetSession() ? player->GetSession()->GetAccountId() : 0,
        EscapeCharacterDb(player->GetName()),
        session.StageId,
        EscapeCharacterDb(eventType),
        uint64(std::time(nullptr)),
        player->GetMapId(),
        session.ArenaInstanceId,
        EscapeCharacterDb(note));
}

void SoloArenaMgr::LogReward(Player* player, ArenaSession const& session,
    uint32 itemEntry, uint32 itemCount, float chance,
    std::string const& status)
{
    if (!player)
        return;

    CharacterDatabase.Execute(
        "INSERT INTO solo_arena_reward_log "
        "(run_uid, guid, account_id, player_name, stage_id, "
        "item_entry, item_count, chance, grant_status, granted_at) "
        "VALUES ({}, {}, {}, '{}', {}, {}, {}, {}, '{}', {})",
        session.RunUid,
        player->GetGUID().GetCounter(),
        player->GetSession() ? player->GetSession()->GetAccountId() : 0,
        EscapeCharacterDb(player->GetName()),
        session.StageId,
        itemEntry,
        itemCount,
        chance,
        EscapeCharacterDb(status),
        uint64(std::time(nullptr)));
}

std::string SoloArenaMgr::GetStageName(uint8 stageId) const
{
    StageConfig const* stage = GetStage(stageId);
    if (stage)
        return stage->Name;

    return Acore::StringFormat("시련 {}단계", stageId);
}

std::string SoloArenaMgr::GetStageMechanicName(uint8 stageId) const
{
    auto range = _mechanics.equal_range(stageId);
    for (auto itr = range.first; itr != range.second; ++itr)
    {
        if (itr->second.Enabled)
            return itr->second.Name;
    }

    return "";
}

std::string SoloArenaMgr::BuildStageRankPayload(Player* player,
    uint8 stageId) const
{
    if (!player)
        return "-^0";

    QueryResult result = CharacterDatabase.Query(
        "SELECT best_rank_label, best_time_sec "
        "FROM solo_arena_stage_record "
        "WHERE guid = {} AND stage_id = {}",
        player->GetGUID().GetCounter(), stageId);

    if (!result)
        return "-^0";

    Field* fields = result->Fetch();
    std::string rankLabel = fields[0].Get<std::string>();
    uint32 bestTime = fields[1].Get<uint32>();
    if (rankLabel.empty())
        rankLabel = "-";

    return Acore::StringFormat("{}^{}", rankLabel, bestTime);
}

std::string SoloArenaMgr::BuildStageRewardPayload(uint8 stageId) const
{
    QueryResult result = WorldDatabase.Query(
        "SELECT item_entry, item_count, chance, reward_rank_label "
        "FROM solo_arena_stage_reward "
        "WHERE stage_id = {} AND enabled = 1 "
        "ORDER BY sort_order, id",
        stageId);

    if (!result)
        return "0^0^0";

    std::ostringstream rewards;
    bool first = true;

    do
    {
        Field* fields = result->Fetch();
        uint32 itemEntry = fields[0].Get<uint32>();
        uint32 itemCount = std::max<uint32>(1, fields[1].Get<uint32>());
        float chance = fields[2].Get<float>();
        std::string rewardRankLabel = fields[3].Get<std::string>();
        std::string rewardItemName = "이름 로딩 중";

        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemEntry))
        {
            rewardItemName = itemTemplate->Name1;

            if (ItemLocale const* itemLocale = sObjectMgr->GetItemLocale(itemEntry))
            {
                if (itemLocale->Name.size() > std::size_t(LOCALE_koKR) &&
                    !itemLocale->Name[LOCALE_koKR].empty())
                    rewardItemName = itemLocale->Name[LOCALE_koKR];
            }
        }

        if (rewardItemName.empty() || rewardItemName == "이름 로딩 중")
        {
            if (QueryResult itemNameResult = WorldDatabase.Query(
                "SELECT COALESCE(("
                "SELECT Name FROM item_template_locale "
                "WHERE ID = {} AND locale = 'koKR' LIMIT 1"
                "), ("
                "SELECT name FROM item_template WHERE entry = {} LIMIT 1"
                "))",
                itemEntry, itemEntry))
            {
                Field* itemNameFields = itemNameResult->Fetch();
                if (itemNameFields && !itemNameFields[0].IsNull())
                    rewardItemName = itemNameFields[0].Get<std::string>();
            }
        }

        if (!first)
            rewards << ",";

        first = false;
        rewards << itemEntry << "^" << itemCount << "^" << chance
                << "^" << SanitizeAddonField(rewardRankLabel, 8)
                << "^" << SanitizeAddonField(rewardItemName, 64);
    } while (result->NextRow());

    return rewards.str();
}

void SoloArenaMgr::ClearMechanicObject(ArenaSession& session)
{
    Player* player = ObjectAccessor::FindConnectedPlayer(session.PlayerGuid);
    for (uint8 slot = 0; slot < MAX_STAGE_MECHANIC_SLOTS; ++slot)
    {
        if (session.MechanicGuids[slot].IsEmpty())
            continue;

        if (player)
            if (GameObject* go = ObjectAccessor::GetGameObject(*player,
                    session.MechanicGuids[slot]))
            {
                go->SetRespawnTime(0);
                go->Delete();
            }

        session.MechanicGuids[slot] = ObjectGuid::Empty;
        session.NextMechanicSpawnAt[slot] = 0;
    }
}

void SoloArenaMgr::ClearMechanicSummons(ArenaSession& session)
{
    if (!session.HelperGuid.IsEmpty())
        if (Player* player = ObjectAccessor::FindConnectedPlayer(
                session.PlayerGuid))
            if (Creature* helper = ObjectAccessor::GetCreature(*player,
                    session.HelperGuid))
                helper->DespawnOrUnsummon();

    if (!session.HazardGuid.IsEmpty())
        if (Player* player = ObjectAccessor::FindConnectedPlayer(
                session.PlayerGuid))
            if (Creature* hazard = ObjectAccessor::GetCreature(*player,
                    session.HazardGuid))
                hazard->DespawnOrUnsummon();

    session.HelperGuid = ObjectGuid::Empty;
    session.HazardGuid = ObjectGuid::Empty;
}

bool SoloArenaMgr::SpawnMechanicObject(Player* player,
    ArenaSession& session, StageMechanicConfig const& mechanic)
{
    if (!player)
        return false;

    Map* map = player->GetMap();
    if (!map || player->GetMapId() != session.ArenaMapId)
        return false;

    float z = ResolveArenaGroundZ(map, mechanic.SpawnX, mechanic.SpawnY,
        mechanic.SpawnZ);
    GameObject* go = map->SummonGameObject(mechanic.ObjectEntry,
        mechanic.SpawnX, mechanic.SpawnY, z, mechanic.SpawnO,
        0.0f, 0.0f, 0.0f, 0.0f,
        std::max<uint32>(10u, mechanic.DurationMs / 1000), true);
    if (!go)
        return false;

    if (mechanic.SlotId == 0 || mechanic.SlotId > MAX_STAGE_MECHANIC_SLOTS)
    {
        go->Delete();
        return false;
    }

    session.MechanicGuids[mechanic.SlotId - 1] = go->GetGUID();
    LogEvent(player, session, "MECHANIC_SPAWNED", mechanic.Name);
    return true;
}

bool SoloArenaMgr::SummonMechanicHelper(Player* player,
    ArenaSession& session, StageMechanicConfig const& mechanic)
{
    if (!player)
        return false;

    ClearMechanicSummons(session);

    Unit* target = nullptr;
    if (!session.BotGuid.IsEmpty())
        target = ObjectAccessor::GetCreature(*player, session.BotGuid);

    float angle = player->GetOrientation() + 0.8f;
    float x = player->GetPositionX() + std::cos(angle) * 2.5f;
    float y = player->GetPositionY() + std::sin(angle) * 2.5f;
    float z = ResolveArenaGroundZ(player->GetMap(), x, y,
        player->GetPositionZ());

    TempSummon* summon = player->SummonCreature(mechanic.SummonEntry,
        x, y, z, player->GetOrientation(),
        TEMPSUMMON_TIMED_DESPAWN, uint32(mechanic.EffectValue1) * 1000);
    if (!summon)
        return false;

    summon->SetFaction(player->GetFaction());
    summon->SetReactState(REACT_AGGRESSIVE);
    summon->SetWalk(false);
    summon->SetMaxHealth(std::max<uint32>(6000u,
        uint32(player->GetMaxHealth() * 0.20f)));
    summon->SetHealth(summon->GetMaxHealth());
    summon->SetAttackTime(BASE_ATTACK, 1800);
    summon->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE,
        std::max(120.0f, player->GetMaxHealth() * 0.010f));
    summon->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE,
        std::max(220.0f, player->GetMaxHealth() * 0.018f));
    summon->UpdateDamagePhysical(BASE_ATTACK);
    session.HelperGuid = summon->GetGUID();

    if (target)
    {
        summon->SetInCombatWith(target);
        target->SetInCombatWith(summon);
        summon->AI()->AttackStart(target);
    }

    return true;
}

bool SoloArenaMgr::SummonMechanicHazard(Player* player,
    ArenaSession& session, StageMechanicConfig const& mechanic)
{
    if (!player)
        return false;

    ClearMechanicSummons(session);

    float angle = player->GetOrientation() - 0.9f;
    float x = player->GetPositionX() + std::cos(angle) * 3.0f;
    float y = player->GetPositionY() + std::sin(angle) * 3.0f;
    float z = ResolveArenaGroundZ(player->GetMap(), x, y,
        player->GetPositionZ());

    TempSummon* summon = player->SummonCreature(mechanic.SummonEntry,
        x, y, z, player->GetOrientation(),
        TEMPSUMMON_TIMED_DESPAWN, 18000);
    if (!summon)
        return false;

    summon->SetFaction(14);
    summon->SetReactState(REACT_AGGRESSIVE);
    summon->SetWalk(false);
    summon->SetMaxHealth(std::max<uint32>(5000u,
        uint32(player->GetMaxHealth() * 0.16f)));
    summon->SetHealth(summon->GetMaxHealth());
    summon->SetAttackTime(BASE_ATTACK, 1900);
    summon->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE,
        std::max(90.0f, player->GetMaxHealth() * 0.008f));
    summon->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE,
        std::max(180.0f, player->GetMaxHealth() * 0.014f));
    summon->UpdateDamagePhysical(BASE_ATTACK);
    session.HazardGuid = summon->GetGUID();
    summon->SetInCombatWith(player);
    player->SetInCombatWith(summon);
    summon->AI()->AttackStart(player);
    return true;
}

void SoloArenaMgr::ApplyMechanicEffect(Player* player, ArenaSession& session,
    StageMechanicConfig const& mechanic)
{
    if (session.Scenario == TrialScenario::Objective &&
        session.StageId >= 4 && session.StageId <= 6)
    {
        switch (urand(1, 6))
        {
            case 1:
            {
                player->CastSpell(player, TRIAL_RANDOM_SPELL_KNOCK_AWAY, true);
                SendSystem(player,
                    "마법진이 폭주합니다. 날려버리기가 발동했습니다.");
                LogEvent(player, session, "MECHANIC_TRIGGERED",
                    "랜덤 마법진: 날려버리기");
                break;
            }
            case 2:
            {
                player->CastSpell(player, TRIAL_RANDOM_SPELL_KICK, true);
                SendSystem(player,
                    "마법진이 흔들립니다. 브원삼디의 발차기가 발동했습니다.");
                LogEvent(player, session, "MECHANIC_TRIGGERED",
                    "랜덤 마법진: 브원삼디의 발차기");
                break;
            }
            case 3:
            {
                player->CastSpell(player, TRIAL_RANDOM_SPELL_ROCKET_BOOTS,
                    true);
                SendSystem(player,
                    "마법진이 점화됩니다. 로켓 장화가 발동했습니다.");
                LogEvent(player, session, "MECHANIC_TRIGGERED",
                    "랜덤 마법진: 로켓 장화");
                break;
            }
            case 4:
            {
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                float o = 0.0f;
                GetObjectiveStartLocation(session.Team, x, y, z, o);
                z = ResolveArenaGroundZ(player->GetMap(), x, y, z);
                player->TeleportTo(session.ArenaMapId, x, y, z, o,
                    TELE_TO_GM_MODE);
                SendSystem(player,
                    "마법진이 뒤틀립니다. 시작 위치로 되돌아갑니다.");
                LogEvent(player, session, "MECHANIC_TRIGGERED",
                    "랜덤 마법진: 시작 위치 복귀");
                break;
            }
            case 5:
            {
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                float o = 0.0f;
                GetRandomObjectiveFlagLocation(x, y, z, o);
                z = ResolveArenaGroundZ(player->GetMap(), x, y, z) + 60.0f;
                player->TeleportTo(session.ArenaMapId, x, y, z, o,
                    TELE_TO_GM_MODE);
                player->CastSpell(player, TRIAL_RANDOM_SPELL_PARACHUTE_BUFF,
                    true);
                SendSystem(player,
                    "마법진이 폭발합니다. 거점 상공으로 이동하며 낙하산이 펼쳐집니다.");
                LogEvent(player, session, "MECHANIC_TRIGGERED",
                    "랜덤 마법진: 거점 상공 이동");
                break;
            }
            case 6:
            default:
            {
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                float o = 0.0f;
                GetRandomObjectiveFlagLocation(x, y, z, o);
                z = ResolveArenaGroundZ(player->GetMap(), x, y, z);
                player->TeleportTo(session.ArenaMapId, x, y, z, o,
                    TELE_TO_GM_MODE);
                SendSystem(player,
                    "마법진이 왜곡됩니다. 거점 깃발 위치로 이동합니다.");
                LogEvent(player, session, "MECHANIC_TRIGGERED",
                    "랜덤 마법진: 거점 깃발 이동");
                break;
            }
        }

        return;
    }

    switch (mechanic.MechanicType)
    {
        case StageMechanicType::HealingSurge:
        {
            uint32 heal = std::max<uint32>(1u,
                uint32(player->GetMaxHealth() * mechanic.EffectValue1));
            player->ModifyHealth(int32(heal));
            session.PlayerDamageBuffEndsAt = std::time(nullptr) + 12;
            player->RemoveAurasDueToSpell(TRIAL_MECHANIC_BUFF_AURA);
            player->CastSpell(player, TRIAL_MECHANIC_BUFF_AURA, true);
            SendSystem(player,
                "시련의 숨결이 당신을 감쌉니다. 잠시 더 강하게 싸웁니다.");
            LogEvent(player, session, "MECHANIC_TRIGGERED",
                "시련의 숨결");
            break;
        }
        case StageMechanicType::ShadowBurnHazard:
        {
            if (Creature* bot = ObjectAccessor::GetCreature(*player,
                    session.BotGuid))
            {
                uint32 damage = uint32(bot->GetMaxHealth() *
                    mechanic.EffectValue1);
                damage = std::max<uint32>(1u, damage);
                uint32 newHealth = (bot->GetHealth() > damage) ?
                    (bot->GetHealth() - damage) : 1;
                bot->SetHealth(newHealth);
            }

            uint32 selfDamage = std::max<uint32>(1u,
                uint32(player->GetMaxHealth() * mechanic.EffectValue2));
            if (player->GetHealth() > selfDamage)
                player->ModifyHealth(-int32(selfDamage));

            SummonMechanicHazard(player, session, mechanic);
            SendSystem(player,
                "뒤틀린 파편이 폭주합니다. 그림자는 흔들리지만 잔영이 덤벼듭니다.");
            LogEvent(player, session, "MECHANIC_TRIGGERED",
                "뒤틀린 파편");
            break;
        }
        case StageMechanicType::GuardianSummon:
        {
            SummonMechanicHelper(player, session, mechanic);
            SendSystem(player,
                "균열의 제단이 응답합니다. 시련 수호자가 당신을 돕습니다.");
            LogEvent(player, session, "MECHANIC_TRIGGERED",
                "균열의 제단");
            break;
        }
        case StageMechanicType::SpeedBoost:
        {
            session.ObjectiveSpeedBuffEndsAt = std::time(nullptr) +
                uint64(std::max<uint32>(3u, uint32(mechanic.EffectValue1)));
            SendSystem(player,
                "질주의 상자가 발동했습니다. 잠시 이동 속도가 빨라집니다.");
            LogEvent(player, session, "MECHANIC_TRIGGERED",
                "질주의 상자");
            break;
        }
        case StageMechanicType::ReturnToStart:
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            float o = 0.0f;
            GetObjectiveStartLocation(session.Team, x, y, z, o);
            z = ResolveArenaGroundZ(player->GetMap(), x, y, z);
            player->TeleportTo(session.ArenaMapId, x, y, z, o,
                TELE_TO_GM_MODE);
            SendSystem(player,
                "복귀의 상자가 발동했습니다. 시작 지점으로 되돌아갑니다.");
            LogEvent(player, session, "MECHANIC_TRIGGERED",
                "복귀의 상자");
            break;
        }
        case StageMechanicType::LaunchAway:
        {
            player->KnockbackFrom(mechanic.SpawnX, mechanic.SpawnY,
                std::max(10.0f, mechanic.EffectValue1),
                std::max(8.0f, mechanic.EffectValue2));
            SendSystem(player,
                "광폭의 상자가 폭주합니다. 거센 충격에 멀리 날아갑니다.");
            LogEvent(player, session, "MECHANIC_TRIGGERED",
                "광폭의 상자");
            break;
        }
        default:
            break;
    }
}

void SoloArenaMgr::UpdateMechanics(Player* player, ArenaSession& session)
{
    if (!player || session.StageId < 4 || session.StageId > 6)
        return;

    std::vector<StageMechanicConfig> mechanics = GetMechanics(session.StageId);
    uint64 now = std::time(nullptr);
    if (mechanics.empty())
    {
        std::vector<uint64> expiredCooldowns;
        for (auto const& pair : session.ExternalMechanicCooldowns)
            if (pair.second <= now)
                expiredCooldowns.push_back(pair.first);

        for (uint64 guid : expiredCooldowns)
            session.ExternalMechanicCooldowns.erase(guid);

        std::list<GameObject*> externalCircles;
        player->GetGameObjectListWithEntryInGrid(externalCircles,
            std::vector<uint32>{ TRIAL_MECHANIC_GOOD_ENTRY,
                TRIAL_MECHANIC_BAD_ENTRY }, 12.0f);

        for (GameObject* go : externalCircles)
        {
            if (!go || go->GetMapId() != session.ArenaMapId)
                continue;

            uint64 guid = go->GetGUID().GetRawValue();
            auto cooldown = session.ExternalMechanicCooldowns.find(guid);
            if (cooldown != session.ExternalMechanicCooldowns.end() &&
                cooldown->second > now)
                continue;

            if (player->GetDistance2d(go) > 6.0f)
                continue;

            if (std::abs(player->GetPositionZ() - go->GetPositionZ()) > 8.0f)
                continue;

            StageMechanicConfig externalMechanic;
            externalMechanic.StageId = session.StageId;
            externalMechanic.ObjectEntry = go->GetEntry();
            externalMechanic.SpawnX = go->GetPositionX();
            externalMechanic.SpawnY = go->GetPositionY();
            externalMechanic.SpawnZ = go->GetPositionZ();
            externalMechanic.SpawnO = go->GetOrientation();
            externalMechanic.Name = "직접 배치한 시련 마법진";

            ApplyMechanicEffect(player, session, externalMechanic);
            go->DespawnOrUnsummon(0ms, Seconds(30));
            session.ExternalMechanicCooldowns[guid] = now + 30;
            break;
        }

        return;
    }

    for (StageMechanicConfig const& mechanic : mechanics)
    {
        if (mechanic.SlotId == 0 || mechanic.SlotId > MAX_STAGE_MECHANIC_SLOTS)
            continue;

        uint8 slotIndex = mechanic.SlotId - 1;
        if (!session.MechanicGuids[slotIndex].IsEmpty())
        {
            GameObject* go = ObjectAccessor::GetGameObject(*player,
                session.MechanicGuids[slotIndex]);
            if (!go)
            {
                session.MechanicGuids[slotIndex] = ObjectGuid::Empty;
                if (session.NextMechanicSpawnAt[slotIndex] == 0)
                    session.NextMechanicSpawnAt[slotIndex] = now +
                        (mechanic.SpawnIntervalMs / 1000);
            }
            else if (player->GetDistance2d(
                    mechanic.SpawnX, mechanic.SpawnY) <= 6.0f &&
                std::abs(player->GetPositionZ() - mechanic.SpawnZ) <= 8.0f)
            {
                ApplyMechanicEffect(player, session, mechanic);
                go->DespawnOrUnsummon(0ms, Seconds(30));
                session.MechanicGuids[slotIndex] = ObjectGuid::Empty;
                session.NextMechanicSpawnAt[slotIndex] = now + 30;
            }

            continue;
        }

        if (session.NextMechanicSpawnAt[slotIndex] != 0 &&
            now < session.NextMechanicSpawnAt[slotIndex])
            continue;

        if (SpawnMechanicObject(player, session, mechanic))
            session.NextMechanicSpawnAt[slotIndex] = now +
                (mechanic.SpawnIntervalMs / 1000);
    }
}

void SoloArenaMgr::LoadDefaultStages()
{
    _stages.clear();

    StageConfig stage1;
    stage1.StageId = 1;
    stage1.Name = "그림자 시련 1단계";
    _stages[stage1.StageId] = stage1;

    StageConfig stage2 = stage1;
    stage2.StageId = 2;
    stage2.Name = "그림자 시련 2단계";
    stage2.HealthMultiplier = 1.10f;
    stage2.DamageMultiplier = 1.10f;
    stage2.AttackTimeMs = 1900;
    stage2.SpellIntervalMs = 4200;
    stage2.MoveSpeedRate = 1.00f;
    _stages[stage2.StageId] = stage2;

    StageConfig stage3 = stage1;
    stage3.StageId = 3;
    stage3.Name = "그림자 시련 3단계";
    stage3.HealthMultiplier = 1.20f;
    stage3.DamageMultiplier = 1.20f;
    stage3.AttackTimeMs = 1900;
    stage3.SpellIntervalMs = 4200;
    stage3.MoveSpeedRate = 1.00f;
    _stages[stage3.StageId] = stage3;

    StageConfig stage4 = stage1;
    stage4.StageId = 4;
    stage4.Name = "그림자 시련 4단계";
    stage4.ArenaMapId = DEFAULT_OBJECTIVE_MAP_ID;
    stage4.HealthMultiplier = 1.00f;
    stage4.DamageMultiplier = 1.00f;
    stage4.MoveSpeedRate = 1.00f;
    _stages[stage4.StageId] = stage4;

    StageConfig stage5 = stage1;
    stage5.StageId = 5;
    stage5.Name = "그림자 시련 5단계";
    stage5.ArenaMapId = DEFAULT_OBJECTIVE_MAP_ID;
    stage5.HealthMultiplier = 1.20f;
    stage5.DamageMultiplier = 1.20f;
    stage5.MoveSpeedRate = 1.20f;
    _stages[stage5.StageId] = stage5;

    StageConfig stage6 = stage1;
    stage6.StageId = 6;
    stage6.Name = "그림자 시련 6단계";
    stage6.ArenaMapId = DEFAULT_OBJECTIVE_MAP_ID;
    stage6.HealthMultiplier = 1.50f;
    stage6.DamageMultiplier = 1.50f;
    stage6.MoveSpeedRate = 1.50f;
    _stages[stage6.StageId] = stage6;

    StageConfig stage7 = stage1;
    stage7.StageId = 7;
    stage7.Name = "그림자 시련 7단계";
    stage7.HealthMultiplier = 1.60f;
    stage7.DamageMultiplier = 1.60f;
    _stages[stage7.StageId] = stage7;

    StageConfig stage8 = stage1;
    stage8.StageId = 8;
    stage8.Name = "그림자 시련 8단계";
    stage8.HealthMultiplier = 1.70f;
    stage8.DamageMultiplier = 1.70f;
    _stages[stage8.StageId] = stage8;

    StageConfig stage9 = stage1;
    stage9.StageId = 9;
    stage9.Name = "그림자 시련 9단계";
    stage9.HealthMultiplier = 1.80f;
    stage9.DamageMultiplier = 1.80f;
    _stages[stage9.StageId] = stage9;

    StageConfig stage10 = stage1;
    stage10.StageId = 10;
    stage10.Name = "그림자 시련 10단계";
    stage10.HealthMultiplier = 1.90f;
    stage10.DamageMultiplier = 1.90f;
    _stages[stage10.StageId] = stage10;
}

void SoloArenaMgr::LoadDefaultMechanics()
{
    _mechanics.clear();

    StageMechanicConfig stage1;
    stage1.StageId = 1;
    stage1.SlotId = 1;
    stage1.MechanicType = StageMechanicType::HealingSurge;
    stage1.ObjectEntry = TRIAL_MECHANIC_GOOD_ENTRY;
    stage1.SpawnX = 1281.90f;
    stage1.SpawnY = 1667.30f;
    stage1.SpawnZ = 39.96f;
    stage1.SpawnIntervalMs = 20000;
    stage1.DurationMs = 15000;
    stage1.EffectValue1 = 0.20f;
    stage1.EffectValue2 = 0.25f;
    stage1.Name = "시련의 숨결";
    _mechanics.emplace(stage1.StageId, stage1);

    StageMechanicConfig stage2;
    stage2.StageId = 2;
    stage2.SlotId = 1;
    stage2.MechanicType = StageMechanicType::ShadowBurnHazard;
    stage2.ObjectEntry = TRIAL_MECHANIC_BAD_ENTRY;
    stage2.SpawnX = 1289.60f;
    stage2.SpawnY = 1668.10f;
    stage2.SpawnZ = 39.96f;
    stage2.SpawnIntervalMs = 22000;
    stage2.DurationMs = 15000;
    stage2.EffectValue1 = 0.10f;
    stage2.EffectValue2 = 0.08f;
    stage2.SummonEntry = TRIAL_HAZARD_ENTRY;
    stage2.Name = "뒤틀린 파편";
    _mechanics.emplace(stage2.StageId, stage2);

    StageMechanicConfig stage3;
    stage3.StageId = 3;
    stage3.SlotId = 1;
    stage3.MechanicType = StageMechanicType::GuardianSummon;
    stage3.ObjectEntry = TRIAL_MECHANIC_GOOD_ENTRY;
    stage3.SpawnX = 1285.80f;
    stage3.SpawnY = 1662.80f;
    stage3.SpawnZ = 39.96f;
    stage3.SpawnIntervalMs = 25000;
    stage3.DurationMs = 15000;
    stage3.EffectValue1 = 18.0f;
    stage3.SummonEntry = TRIAL_HELPER_ENTRY;
    stage3.Name = "균열의 제단";
    _mechanics.emplace(stage3.StageId, stage3);
}

namespace
{
    SpellPackage GetSpellPackage(uint8 playerClass, uint32 activeSpec)
    {
        switch (playerClass)
        {
            case CLASS_WARRIOR:
                switch (activeSpec)
                {
                    case TALENT_TREE_WARRIOR_FURY:
                        return { 23881, 47450, 47520, 12975,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            0.95f, 1.10f, 1.25f, 0.0f,
                            5.0f, 5.0f, 8.0f, 0.0f,
                            3200, 4500, 6500, 15000, 0.40f };
                    case TALENT_TREE_WARRIOR_PROTECTION:
                        return { 47488, 57823, 47498, 871,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            0.90f, 1.00f, 1.05f, 0.0f,
                            5.0f, 5.0f, 5.0f, 0.0f,
                            3300, 4800, 7000, 18000, 0.45f };
                    default:
                        return { 47486, 47450, 7384, 871,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            1.00f, 0.95f, 1.10f, 0.0f,
                            5.0f, 5.0f, 5.0f, 0.0f,
                            3200, 4200, 6000, 18000, 0.45f };
                }
            case CLASS_PALADIN:
                switch (activeSpec)
                {
                    case TALENT_TREE_PALADIN_HOLY:
                        return { 48825, 48819, 48817, 48785,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            0.95f, 0.90f, 1.05f, 0.0f,
                            20.0f, 8.0f, 20.0f, 0.0f,
                            3200, 5200, 6500, 16000, 0.50f };
                    case TALENT_TREE_PALADIN_PROTECTION:
                        return { 53595, 48827, 48806, 642,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            0.95f, 1.00f, 1.10f, 0.0f,
                            5.0f, 10.0f, 20.0f, 0.0f,
                            3200, 5200, 7000, 18000, 0.45f };
                    default:
                        return { 35395, 48806, 48801, 642,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            1.00f, 1.10f, 1.05f, 0.0f,
                            5.0f, 20.0f, 20.0f, 0.0f,
                            3000, 5200, 7000, 18000, 0.40f };
                }
            case CLASS_HUNTER:
                return { 49045, 49052, 49050, 19263,
                    SPELL_SCHOOL_MASK_ARCANE,
                    SPELL_SCHOOL_MASK_NORMAL,
                    SPELL_SCHOOL_MASK_NORMAL,
                    SPELL_SCHOOL_MASK_NORMAL,
                    0.95f, 1.00f, 1.15f, 0.0f,
                    30.0f, 30.0f, 35.0f, 0.0f,
                    2800, 4200, 6500, 18000, 0.45f };
            case CLASS_ROGUE:
                return { 48638, 48668, 57993, 26669,
                    SPELL_SCHOOL_MASK_NORMAL,
                    SPELL_SCHOOL_MASK_NORMAL,
                    SPELL_SCHOOL_MASK_NORMAL,
                    SPELL_SCHOOL_MASK_NORMAL,
                    0.95f, 1.05f, 1.15f, 0.0f,
                    5.0f, 5.0f, 5.0f, 0.0f,
                    2600, 4300, 6200, 16000, 0.40f };
            case CLASS_PRIEST:
                switch (activeSpec)
                {
                    case TALENT_TREE_PRIEST_DISCIPLINE:
                        return { 48066, 53007, 48123, 47585,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            0.0f, 1.00f, 0.95f, 0.0f,
                            0.0f, 30.0f, 30.0f, 0.0f,
                            5000, 3500, 5200, 16000, 0.45f };
                    case TALENT_TREE_PRIEST_HOLY:
                        return { 48135, 48134, 48071, 47788,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            SPELL_SCHOOL_MASK_HOLY,
                            0.95f, 1.00f, 0.0f, 0.0f,
                            30.0f, 30.0f, 0.0f, 0.0f,
                            3200, 4500, 6200, 16000, 0.45f };
                    default:
                        return { 48127, 48156, 48300, 47585,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_HOLY,
                            1.00f, 0.95f, 1.10f, 0.0f,
                            30.0f, 20.0f, 30.0f, 0.0f,
                            3200, 4200, 6500, 16000, 0.40f };
                }
            case CLASS_DEATH_KNIGHT:
                switch (activeSpec)
                {
                    case TALENT_TREE_DEATH_KNIGHT_BLOOD:
                        return { 49924, 49930, 49941, 48743,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            1.00f, 1.05f, 1.15f, 0.0f,
                            5.0f, 20.0f, 5.0f, 0.0f,
                            3200, 4800, 6500, 16000, 0.45f };
                    case TALENT_TREE_DEATH_KNIGHT_UNHOLY:
                        return { 49909, 49895, 49938, 48792,
                            SPELL_SCHOOL_MASK_FROST,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_FROST,
                            0.95f, 1.05f, 1.10f, 0.0f,
                            20.0f, 30.0f, 5.0f, 0.0f,
                            3000, 4200, 6200, 18000, 0.45f };
                    default:
                        return { 51425, 49909, 49930, 48792,
                            SPELL_SCHOOL_MASK_FROST,
                            SPELL_SCHOOL_MASK_FROST,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_FROST,
                            1.10f, 0.95f, 1.00f, 0.0f,
                            5.0f, 20.0f, 20.0f, 0.0f,
                            3000, 4200, 6500, 18000, 0.45f };
                }
            case CLASS_SHAMAN:
                switch (activeSpec)
                {
                    case TALENT_TREE_SHAMAN_ENHANCEMENT:
                        return { 17364, 49231, 49238, 30823,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_NATURE,
                            1.00f, 1.05f, 0.90f, 0.0f,
                            5.0f, 20.0f, 25.0f, 0.0f,
                            2800, 4300, 6000, 15000, 0.45f };
                    case TALENT_TREE_SHAMAN_RESTORATION:
                        return { 49238, 49276, 49233, 30823,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_NATURE,
                            0.90f, 0.0f, 1.00f, 0.0f,
                            30.0f, 0.0f, 20.0f, 0.0f,
                            3200, 7000, 5200, 15000, 0.40f };
                    default:
                        return { 49238, 60043, 49233, 30823,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_NATURE,
                            0.95f, 1.15f, 1.00f, 0.0f,
                            30.0f, 30.0f, 20.0f, 0.0f,
                            3000, 4500, 6000, 15000, 0.40f };
                }
            case CLASS_MAGE:
                switch (activeSpec)
                {
                    case TALENT_TREE_MAGE_FIRE:
                        return { 42833, 55360, 42891, 45438,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_FROST,
                            1.00f, 1.15f, 0.95f, 0.0f,
                            30.0f, 30.0f, 30.0f, 0.0f,
                            2800, 5200, 4200, 16000, 0.40f };
                    case TALENT_TREE_MAGE_ARCANE:
                        return { 42897, 42846, 44781, 45438,
                            SPELL_SCHOOL_MASK_ARCANE,
                            SPELL_SCHOOL_MASK_ARCANE,
                            SPELL_SCHOOL_MASK_ARCANE,
                            SPELL_SCHOOL_MASK_FROST,
                            1.05f, 0.95f, 1.10f, 0.0f,
                            30.0f, 30.0f, 30.0f, 0.0f,
                            2600, 4200, 5600, 16000, 0.40f };
                    default:
                        return { 42842, 42917, 33395, 45438,
                            SPELL_SCHOOL_MASK_FROST,
                            SPELL_SCHOOL_MASK_FROST,
                            SPELL_SCHOOL_MASK_FROST,
                            SPELL_SCHOOL_MASK_FROST,
                            1.00f, 0.95f, 1.10f, 0.0f,
                            30.0f, 20.0f, 20.0f, 0.0f,
                            2800, 4500, 6500, 16000, 0.40f };
                }
            case CLASS_WARLOCK:
                switch (activeSpec)
                {
                    case TALENT_TREE_WARLOCK_DEMONOLOGY:
                        return { 47809, 47811, 47813, 48020,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_SHADOW,
                            0.95f, 1.00f, 1.10f, 0.0f,
                            30.0f, 30.0f, 30.0f, 0.0f,
                            2800, 4500, 6200, 17000, 0.45f };
                    case TALENT_TREE_WARLOCK_DESTRUCTION:
                        return { 47811, 17962, 47809, 48020,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_SHADOW,
                            1.00f, 1.15f, 0.95f, 0.0f,
                            30.0f, 20.0f, 30.0f, 0.0f,
                            3000, 5000, 4200, 17000, 0.45f };
                    default:
                        return { 47809, 47813, 47811, 48020,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_SHADOW,
                            SPELL_SCHOOL_MASK_FIRE,
                            SPELL_SCHOOL_MASK_SHADOW,
                            0.95f, 1.10f, 1.00f, 0.0f,
                            30.0f, 30.0f, 30.0f, 0.0f,
                            2800, 5000, 4200, 17000, 0.45f };
                }
            case CLASS_DRUID:
                switch (activeSpec)
                {
                    case TALENT_TREE_DRUID_FERAL_COMBAT:
                        return { 48566, 49800, 48570, 22812,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NORMAL,
                            SPELL_SCHOOL_MASK_NATURE,
                            1.00f, 1.10f, 0.95f, 0.0f,
                            5.0f, 5.0f, 5.0f, 0.0f,
                            2800, 4300, 6200, 15000, 0.40f };
                    case TALENT_TREE_DRUID_RESTORATION:
                        return { 48461, 48465, 48463, 22812,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_ARCANE,
                            SPELL_SCHOOL_MASK_ARCANE,
                            SPELL_SCHOOL_MASK_NATURE,
                            0.90f, 1.00f, 1.05f, 0.0f,
                            30.0f, 30.0f, 30.0f, 0.0f,
                            3200, 5000, 4200, 15000, 0.45f };
                    default:
                        return { 48461, 48465, 48463, 22812,
                            SPELL_SCHOOL_MASK_NATURE,
                            SPELL_SCHOOL_MASK_ARCANE,
                            SPELL_SCHOOL_MASK_ARCANE,
                            SPELL_SCHOOL_MASK_NATURE,
                            0.95f, 1.10f, 1.00f, 0.0f,
                            30.0f, 30.0f, 30.0f, 0.0f,
                            2800, 5000, 4200, 15000, 0.45f };
                }
            default:
                return { 585, 589, 48135, 17,
                    SPELL_SCHOOL_MASK_HOLY,
                    SPELL_SCHOOL_MASK_SHADOW,
                    SPELL_SCHOOL_MASK_HOLY,
                    SPELL_SCHOOL_MASK_HOLY,
                    0.80f, 1.0f, 0.9f, 0.0f,
                    30.0f, 30.0f, 30.0f, 0.0f,
                    3000, 4500, 6000, 15000, 0.45f };
        }
    }

    TacticalPackage GetTacticalPackage(uint8 playerClass, uint32 activeSpec)
    {
        TacticalPackage package;

        switch (playerClass)
        {
            case CLASS_WARRIOR:
                package.Burst = { 12292, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                    0.0f, 24000, true, 1.0f, 1.0f, false, false };
                package.Utility = { 6552, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                    5.0f, 12000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 5246, SPELL_SCHOOL_MASK_NORMAL,
                    0.0f, 8.0f, 18000, false, 1.0f, 1.0f, false, false };
                package.Area = { 1680, SPELL_SCHOOL_MASK_NORMAL, 1.05f,
                    8.0f, 9000, true, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_PALADIN:
                package.Burst = { 31884, SPELL_SCHOOL_MASK_HOLY, 0.0f,
                    0.0f, 30000, true, 1.0f, 1.0f, false, false };
                package.Utility = { 20066, SPELL_SCHOOL_MASK_HOLY, 0.0f,
                    20.0f, 16000, false, 1.0f, 1.0f, false, false };
                package.CrowdControl = { 10308, SPELL_SCHOOL_MASK_HOLY,
                    0.0f, 10.0f, 18000, false, 1.0f, 1.0f, false, false };
                package.Area = { 48819, SPELL_SCHOOL_MASK_HOLY, 0.95f,
                    8.0f, 10000, true, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_HUNTER:
                package.Burst = { 3045, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                    0.0f, 26000, true, 1.0f, 1.0f, false, false };
                package.Utility = { 34490, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                    30.0f, 14000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 19503, SPELL_SCHOOL_MASK_NORMAL,
                    0.0f, 30.0f, 18000, false, 1.0f, 1.0f, false, false };
                package.Area = { 58434, SPELL_SCHOOL_MASK_NORMAL, 1.00f,
                    35.0f, 12000, false, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_ROGUE:
                package.Burst = { 13750, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                    0.0f, 25000, true, 1.0f, 1.0f, false, false };
                package.Utility = { 1766, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                    5.0f, 10000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 2094, SPELL_SCHOOL_MASK_NORMAL,
                    0.0f, 10.0f, 18000, false, 1.0f, 1.0f, false, false };
                package.Area = { 51723, SPELL_SCHOOL_MASK_NORMAL, 1.00f,
                    8.0f, 11000, true, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_PRIEST:
                package.Utility = { 15487, SPELL_SCHOOL_MASK_SHADOW, 0.0f,
                    30.0f, 14000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 10890, SPELL_SCHOOL_MASK_SHADOW,
                    0.0f, 8.0f, 18000, true, 1.0f, 1.0f, false, true };
                if (activeSpec == TALENT_TREE_PRIEST_SHADOW)
                {
                    package.Burst = { 34433, SPELL_SCHOOL_MASK_SHADOW,
                        0.0f, 30.0f, 24000, false, 1.0f, 0.60f, false,
                        false };
                    package.Area = { 53022, SPELL_SCHOOL_MASK_SHADOW, 0.95f,
                        30.0f, 12000, false, 1.0f, 1.0f, false, true };
                }
                else
                {
                    package.Burst = { 10060, SPELL_SCHOOL_MASK_HOLY, 0.0f,
                        0.0f, 26000, true, 1.0f, 1.0f, false, false };
                    package.Area = { 48078, SPELL_SCHOOL_MASK_HOLY, 0.85f,
                        10.0f, 11000, true, 1.0f, 1.0f, false, true };
                }
                return package;
            case CLASS_DEATH_KNIGHT:
                package.Burst = { 55268, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                    0.0f, 26000, true, 0.80f, 1.0f, false, false };
                package.Utility = { 47528, SPELL_SCHOOL_MASK_FROST, 0.0f,
                    5.0f, 12000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 47476, SPELL_SCHOOL_MASK_SHADOW,
                    0.0f, 20.0f, 16000, false, 1.0f, 1.0f, true, false };
                package.Area = { 49938, SPELL_SCHOOL_MASK_SHADOW, 1.00f,
                    20.0f, 12000, false, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_SHAMAN:
                package.Burst = { 2825, SPELL_SCHOOL_MASK_NATURE, 0.0f,
                    0.0f, 28000, true, 1.0f, 1.0f, false, false };
                package.Utility = { 57994, SPELL_SCHOOL_MASK_NATURE, 0.0f,
                    25.0f, 10000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 51514, SPELL_SCHOOL_MASK_NATURE,
                    0.0f, 30.0f, 18000, false, 1.0f, 1.0f, false, false };
                package.Area = { 49271, SPELL_SCHOOL_MASK_NATURE, 1.00f,
                    30.0f, 11000, false, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_MAGE:
                if (activeSpec == TALENT_TREE_MAGE_FIRE)
                    package.Burst = { 11129, SPELL_SCHOOL_MASK_FIRE, 0.0f,
                        0.0f, 26000, true, 1.0f, 1.0f, false, false };
                else if (activeSpec == TALENT_TREE_MAGE_ARCANE)
                    package.Burst = { 12042, SPELL_SCHOOL_MASK_ARCANE, 0.0f,
                        0.0f, 26000, true, 1.0f, 1.0f, false, false };
                else
                    package.Burst = { 12472, SPELL_SCHOOL_MASK_FROST, 0.0f,
                        0.0f, 26000, true, 1.0f, 1.0f, false, false };
                package.Utility = { 2139, SPELL_SCHOOL_MASK_ARCANE, 0.0f,
                    30.0f, 12000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 42917, SPELL_SCHOOL_MASK_FROST,
                    0.0f, 10.0f, 18000, false, 1.0f, 1.0f, false, false };
                package.Area = { 42926, SPELL_SCHOOL_MASK_FIRE, 1.05f,
                    30.0f, 12000, false, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_WARLOCK:
                package.Burst = { 17962, SPELL_SCHOOL_MASK_FIRE, 1.20f,
                    20.0f, 12000, false, 1.0f, 0.65f, false, false };
                package.Utility = { 47860, SPELL_SCHOOL_MASK_SHADOW, 0.90f,
                    20.0f, 16000, false, 1.0f, 1.0f, false, false };
                package.CrowdControl = { 6215, SPELL_SCHOOL_MASK_SHADOW,
                    0.0f, 30.0f, 18000, false, 1.0f, 1.0f, false, false };
                package.Area = { 47820, SPELL_SCHOOL_MASK_FIRE, 0.95f,
                    30.0f, 12000, false, 1.0f, 1.0f, false, true };
                return package;
            case CLASS_DRUID:
                if (activeSpec == TALENT_TREE_DRUID_FERAL_COMBAT)
                {
                    package.Burst = { 17116, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                        0.0f, 22000, true, 1.0f, 1.0f, false, false };
                    package.Utility = { 5211, SPELL_SCHOOL_MASK_NORMAL, 0.0f,
                        5.0f, 14000, false, 1.0f, 1.0f, false, false };
                    package.CrowdControl = { 33786, SPELL_SCHOOL_MASK_NATURE,
                        0.0f, 20.0f, 18000, false, 1.0f, 1.0f, false, false };
                    package.Area = { 62078, SPELL_SCHOOL_MASK_NORMAL, 1.00f,
                        8.0f, 11000, true, 1.0f, 1.0f, false, true };
                }
                else
                {
                    package.Burst = { 29166, SPELL_SCHOOL_MASK_NATURE, 0.0f,
                        0.0f, 26000, true, 1.0f, 1.0f, false, false };
                    package.Utility = { 16979, SPELL_SCHOOL_MASK_NATURE, 0.0f,
                        30.0f, 12000, false, 1.0f, 1.0f, true, false };
                    package.CrowdControl = { 33786, SPELL_SCHOOL_MASK_NATURE,
                        20.0f, 20.0f, 18000, false, 1.0f, 1.0f, false,
                        false };
                    package.Area = { 48467, SPELL_SCHOOL_MASK_ARCANE, 1.00f,
                        30.0f, 12000, false, 1.0f, 1.0f, false, true };
                }
                return package;
            default:
                package.Burst = { 48135, SPELL_SCHOOL_MASK_HOLY, 1.00f,
                    30.0f, 10000, false, 1.0f, 0.60f, false, false };
                package.Utility = { 15487, SPELL_SCHOOL_MASK_HOLY, 0.0f,
                    30.0f, 14000, false, 1.0f, 1.0f, true, false };
                package.CrowdControl = { 10890, SPELL_SCHOOL_MASK_SHADOW,
                    0.0f, 8.0f, 18000, true, 1.0f, 1.0f, false, true };
                package.Area = { 48078, SPELL_SCHOOL_MASK_HOLY, 0.80f,
                    10.0f, 12000, true, 1.0f, 1.0f, false, true };
                return package;
        }
    }

    uint32 ComputeShadowSpellDamage(Creature* me,
        ShadowProfile const& profile,
        float factor)
    {
        if (!me)
            return 0;

        float baseDamage = (me->GetMaxHealth() / 18.0f) * factor;
        baseDamage += float(profile.SpellPowerBonus) * 0.65f * factor;
        baseDamage *= profile.DamageMultiplier;
        return std::max<uint32>(150u, static_cast<uint32>(baseDamage));
    }

    class npc_solo_arena_master : public CreatureScript
    {
    public:
        npc_solo_arena_master() : CreatureScript("npc_solo_arena_master")
        {
        }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            ClearGossipMenuFor(player);

            if (!SoloArenaConfig::Instance().IsEnabled())
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                    "지금은 시련을 시작할 수 없습니다.",
                    GOSSIP_SENDER_MAIN, 0);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE,
                    creature->GetGUID());
                return true;
            }

            SoloArenaMgr::Instance().SendUi(player);
            CloseGossipMenuFor(player);
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature,
            uint32 /*sender*/, uint32 action) override
        {
            ClearGossipMenuFor(player);

            if (action == ACTION_ABANDON)
            {
                SoloArenaMgr::Instance().MarkAbandoned(player->GetGUID());
                CloseGossipMenuFor(player);
                return true;
            }

            if (action >= ACTION_STAGE_BASE &&
                action < (ACTION_STAGE_BASE + 100))
            {
                uint8 stageId = static_cast<uint8>(action - ACTION_STAGE_BASE);
                SoloArenaMgr::Instance().StartChallenge(player, stageId);
                CloseGossipMenuFor(player);
                return true;
            }

            return OnGossipHello(player, creature);
        }
    };

    struct npc_solo_arena_shadowAI : public ScriptedAI
    {
        npc_solo_arena_shadowAI(Creature* creature) :
            ScriptedAI(creature)
        {
        }

        void Reset() override
        {
            events.Reset();
            _initialized = false;
            _petInterceptMs = 0;
            _petInterceptCooldownMs = 0;
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->ClearInCombat();
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            InitializeProfile();
            if (!_initialized)
                return;

            events.ScheduleEvent(EVENT_PRIMARY,
                Milliseconds(_package.PrimaryCooldownMs));
            events.ScheduleEvent(EVENT_SECONDARY,
                Milliseconds(_profile.SpellIntervalMs / 2));

            if (_profile.StageId >= 2)
            {
                events.ScheduleEvent(EVENT_TERTIARY,
                    Milliseconds(_package.TertiaryCooldownMs));
            }

            if (_profile.StageId >= 3)
            {
                events.ScheduleEvent(EVENT_DEFENSIVE,
                    Milliseconds(_package.DefensiveCooldownMs));
                events.ScheduleEvent(EVENT_CC,
                    Milliseconds(_tactical.CrowdControl.CooldownMs));
            }

            if (_profile.StageId >= 2)
            {
                events.ScheduleEvent(EVENT_BURST,
                    Milliseconds(_tactical.Burst.CooldownMs));
                events.ScheduleEvent(EVENT_UTILITY,
                    Milliseconds(_tactical.Utility.CooldownMs / 2));
            }

            if (_profile.StageId >= 3)
            {
                events.ScheduleEvent(EVENT_AOE,
                    Milliseconds(_tactical.Area.CooldownMs));
            }
        }

        void KilledUnit(Unit* victim) override
        {
            if (Player* player = victim->ToPlayer())
            {
                if (!SoloArenaMgr::Instance().RespawnObjectivePlayer(
                        player->GetGUID()))
                {
                    SoloArenaMgr::Instance().MarkFailure(player->GetGUID());
                }
            }
        }

        void JustDied(Unit* killer) override
        {
            if (SoloArenaMgr::Instance().RespawnObjectiveShadow(
                    _profile.PlayerGuid))
            {
                SoloArenaMgr::Instance().UnregisterShadow(me->GetGUID());
                me->DespawnOrUnsummon(Milliseconds(1));
                return;
            }

            if (Player* player = killer ? killer->ToPlayer() : nullptr)
                SoloArenaMgr::Instance().MarkVictory(player->GetGUID());

            SoloArenaMgr::Instance().UnregisterShadow(me->GetGUID());
        }

        void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
        {
            InitializeProfile();

            if (SoloArenaMgr::Instance().IsObjectiveRaceActive(
                    _profile.PlayerGuid))
            {
                if (Unit* target = GetShadowTarget())
                    if (me->GetDistance(target) <= 28.0f)
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->Attack(target, true);
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveChase(target,
                            IsMeleeProfile() ? 1.5f :
                            GetDesiredCombatRange());
                        return;
                    }

                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->ClearInCombat();
                return;
            }

            if (!SoloArenaMgr::Instance().IsCombatActive(
                    _profile.PlayerGuid))
            {
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveTargetedHome();
                return;
            }

            if (Unit* target = GetShadowTarget())
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->Attack(target, true);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveChase(target,
                    IsMeleeProfile() ? 1.5f : GetDesiredCombatRange());
                return;
            }

            ScriptedAI::EnterEvadeMode(why);
        }

        void UpdateAI(uint32 diff) override
        {
            InitializeProfile();
            if (!_initialized)
                return;

            if (_movementLockMs > diff)
                _movementLockMs -= diff;
            else
                _movementLockMs = 0;

            if (_petInterceptMs > diff)
                _petInterceptMs -= diff;
            else
                _petInterceptMs = 0;

            if (_petInterceptCooldownMs > diff)
                _petInterceptCooldownMs -= diff;
            else
                _petInterceptCooldownMs = 0;

            if (!SoloArenaMgr::Instance().IsCombatActive(_profile.PlayerGuid))
            {
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                if (!me->IsInEvadeMode())
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveTargetedHome();
                }
                return;
            }

            if (SoloArenaMgr::Instance().IsObjectiveRaceActive(
                    _profile.PlayerGuid))
            {
                Unit* target = GetShadowTarget();
                if (!target || !target->IsAlive())
                {
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    return;
                }

                if (me->GetDistance(target) > 28.0f)
                {
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    return;
                }
            }

            if (!UpdateVictim())
            {
                if (Unit* target = GetShadowTarget())
                    AttackStart(target);
                return;
            }

            if (Unit* preferredTarget = GetShadowTarget())
                if (me->GetVictim() != preferredTarget)
                    AttackStart(preferredTarget);

            events.Update(diff);

            MaintainCombatSpacing();

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PRIMARY:
                        ExecuteSpell(_package.PrimarySpell,
                            _package.PrimarySchool,
                            _package.PrimaryDamageFactor,
                            _package.PrimaryRange, false);
                        events.ScheduleEvent(EVENT_PRIMARY,
                            Milliseconds(_package.PrimaryCooldownMs));
                        break;
                    case EVENT_SECONDARY:
                        ExecuteSpell(_package.SecondarySpell,
                            _package.SecondarySchool,
                            _package.SecondaryDamageFactor,
                            _package.SecondaryRange, false);
                        events.ScheduleEvent(EVENT_SECONDARY,
                            Milliseconds(_package.SecondaryCooldownMs));
                        break;
                    case EVENT_TERTIARY:
                        ExecuteSpell(_package.TertiarySpell,
                            _package.TertiarySchool,
                            _package.TertiaryDamageFactor,
                            _package.TertiaryRange, false);
                        events.ScheduleEvent(EVENT_TERTIARY,
                            Milliseconds(_package.TertiaryCooldownMs));
                        break;
                    case EVENT_DEFENSIVE:
                        ExecuteSpell(_package.DefensiveSpell,
                            _package.DefensiveSchool,
                            _package.DefensiveDamageFactor,
                            _package.DefensiveRange, true);
                        events.ScheduleEvent(EVENT_DEFENSIVE,
                            Milliseconds(_package.DefensiveCooldownMs));
                        break;
                    case EVENT_BURST:
                        ExecuteTacticalSpell(_tactical.Burst);
                        events.ScheduleEvent(EVENT_BURST,
                            Milliseconds(_tactical.Burst.CooldownMs));
                        break;
                    case EVENT_UTILITY:
                        ExecuteTacticalSpell(_tactical.Utility);
                        events.ScheduleEvent(EVENT_UTILITY,
                            Milliseconds(_tactical.Utility.CooldownMs));
                        break;
                    case EVENT_CC:
                        ExecuteTacticalSpell(_tactical.CrowdControl);
                        events.ScheduleEvent(EVENT_CC,
                            Milliseconds(_tactical.CrowdControl.CooldownMs));
                        break;
                    case EVENT_AOE:
                        ExecuteTacticalSpell(_tactical.Area);
                        events.ScheduleEvent(EVENT_AOE,
                            Milliseconds(_tactical.Area.CooldownMs));
                        break;
                }
            }

            if (IsMeleeProfile())
                DoMeleeAttackIfReady();
        }

    private:
        void InitializeProfile()
        {
            ShadowProfile const* profile =
                SoloArenaMgr::Instance().GetShadowProfile(me->GetGUID());
            if (!profile)
                return;

            _profile = *profile;
            _package = GetSpellPackage(_profile.PlayerClass,
                _profile.ActiveSpec);
            _tactical = GetTacticalPackage(_profile.PlayerClass,
                _profile.ActiveSpec);
            float cooldownScale = std::clamp(_profile.CastSpeedRate,
                0.55f, 1.25f);
            _package.PrimaryCooldownMs = std::max<uint32>(900u,
                uint32(float(_package.PrimaryCooldownMs) * cooldownScale));
            _package.SecondaryCooldownMs = std::max<uint32>(1200u,
                uint32(float(_package.SecondaryCooldownMs) * cooldownScale));
            _package.TertiaryCooldownMs = std::max<uint32>(1500u,
                uint32(float(_package.TertiaryCooldownMs) * cooldownScale));
            _tactical.Burst.CooldownMs = std::max<uint32>(2500u,
                uint32(float(_tactical.Burst.CooldownMs) * cooldownScale));
            _tactical.Utility.CooldownMs = std::max<uint32>(2500u,
                uint32(float(_tactical.Utility.CooldownMs) * cooldownScale));
            _tactical.CrowdControl.CooldownMs = std::max<uint32>(3500u,
                uint32(float(_tactical.CrowdControl.CooldownMs) *
                cooldownScale));
            _tactical.Area.CooldownMs = std::max<uint32>(3500u,
                uint32(float(_tactical.Area.CooldownMs) * cooldownScale));
            _initialized = true;
        }

        void ExecuteSpell(uint32 spellId, SpellSchoolMask schoolMask,
            float damageFactor, float range, bool selfCast)
        {
            if (!spellId)
                return;

            if (selfCast)
            {
                if (me->GetHealthPct() > (_package.DefensiveHealthPct * 100.0f))
                    return;

                me->CastSpell(me, spellId, true);
                return;
            }

            Unit* victim = me->GetVictim();
            if (!victim || !victim->IsAlive())
                return;

            if (me->GetDistance(victim) > range)
            {
                IssueChase(victim, IsMeleeProfile() ? 1.5f :
                    GetPreferredRangedRange(range), 900);
                return;
            }

            if (!me->IsWithinLOSInMap(victim))
            {
                IssueChase(victim, IsMeleeProfile() ? 1.5f :
                    GetPreferredRangedRange(range), 900);
                return;
            }

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                return;

            uint32 castTime = spellInfo->CalcCastTime(me);
            if (castTime > 0)
            {
                me->StopMoving();
                me->SetFacingToObject(victim);
                SpellCastResult result = me->CastSpell(victim, spellId, false);
                if (result != SPELL_FAILED_SUCCESS)
                    IssueChase(victim, IsMeleeProfile() ? 1.5f :
                        GetPreferredRangedRange(range), 900);
                return;
            }

            SpellCastResult result = me->CastSpell(victim, spellId, false);
            if (result != SPELL_FAILED_SUCCESS)
            {
                IssueChase(victim, IsMeleeProfile() ? 1.5f :
                    GetPreferredRangedRange(range), 900);
                return;
            }

            if (IsInstantDamageProfile())
            {
                uint32 damage = ComputeShadowSpellDamage(me, _profile,
                    damageFactor);
                Unit::DealDamage(me, victim, damage, nullptr, DIRECT_DAMAGE,
                    schoolMask, spellInfo, false);
            }
        }

        void ExecuteTacticalSpell(TacticalSpell const& spell)
        {
            if (!spell.SpellId)
                return;

            if (spell.SelfCast)
            {
                if (me->GetHealthPct() > (spell.SelfHealthPct * 100.0f))
                    return;

                me->CastSpell(me, spell.SpellId, true);
                return;
            }

            Unit* victim = me->GetVictim();
            if (!victim || !victim->IsAlive())
                return;

            if (spell.TargetHealthPct < 1.0f &&
                victim->GetHealthPct() > (spell.TargetHealthPct * 100.0f))
                return;

            if (spell.RequiresCastingTarget &&
                !victim->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (spell.RequiresNearbySecondary &&
                !HasNearbySecondaryTarget(victim, spell.Range))
                return;

            ExecuteSpell(spell.SpellId, spell.School,
                spell.DamageFactor, spell.Range, false);
        }

        void MaintainCombatSpacing()
        {
            Unit* victim = me->GetVictim();
            if (!victim || !victim->IsAlive())
                return;

            if (victim->ToPet())
            {
                if (!me->IsWithinMeleeRange(victim))
                    IssueChase(victim, 1.5f, 700);
                return;
            }

            float distance = me->GetDistance(victim);
            if (IsMeleeProfile())
            {
                if (!me->IsWithinMeleeRange(victim))
                    IssueChase(victim, 1.5f, 700);
                return;
            }

            float desiredRange = GetPreferredRangedRange(GetDesiredCombatRange());
            if (distance < 5.0f)
            {
                RepositionForCombat(victim, desiredRange);
                return;
            }

            if (distance > (desiredRange + 2.0f))
                IssueChase(victim, desiredRange, 900);
        }

        Unit* GetShadowTarget()
        {
            ShadowProfile const* profile =
                SoloArenaMgr::Instance().GetShadowProfile(me->GetGUID());
            if (!profile)
                return nullptr;

            Player* player = ObjectAccessor::FindConnectedPlayer(
                profile->PlayerGuid);
            if (!player || !player->IsAlive())
                return nullptr;

            if (Pet* pet = player->GetPet())
            {
                if (pet->IsAlive() && me->GetDistance(pet) <= 4.0f &&
                    _petInterceptMs == 0 && _petInterceptCooldownMs == 0)
                {
                    _petInterceptMs = 2200;
                    _petInterceptCooldownMs = 5500;
                }

                if (pet->IsAlive() && _petInterceptMs > 0)
                    return pet;
            }

            return player;
        }

        void RepositionForCombat(Unit* victim, float preferredRange)
        {
            if (!victim || _movementLockMs != 0)
                return;

            float range = IsMeleeProfile() ? 2.5f :
                std::max(7.5f, preferredRange - 1.0f);
            float angle = victim->GetAbsoluteAngle(me) + float(M_PI) * 0.5f;
            float x = victim->GetPositionX() + std::cos(angle) * range;
            float y = victim->GetPositionY() + std::sin(angle) * range;
            float z = victim->GetPositionZ();
            if (Map* map = me->GetMap())
                z = ResolveArenaGroundZ(map, x, y, z);

            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(2, x, y, z);
            _movementLockMs = 1200;
        }

        float GetPreferredRangedRange(float baseRange) const
        {
            return std::clamp(baseRange - 4.0f, 11.0f, 22.0f);
        }

        void IssueChase(Unit* victim, float distance, uint32 lockMs)
        {
            if (!victim || _movementLockMs != 0)
                return;

            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(victim, distance);
            _movementLockMs = lockMs;
        }

        bool HasNearbySecondaryTarget(Unit* primaryVictim, float radius) const
        {
            ShadowProfile const* profile =
                SoloArenaMgr::Instance().GetShadowProfile(me->GetGUID());
            if (!profile)
                return false;

            Player* player = ObjectAccessor::FindConnectedPlayer(
                profile->PlayerGuid);
            if (!player)
                return false;

            if (Pet* pet = player->GetPet())
            {
                if (pet->IsAlive() && pet != primaryVictim &&
                    me->GetDistance(pet) <= std::max(10.0f, radius))
                    return true;
            }

            return false;
        }

        bool IsMeleeProfile() const
        {
            switch (_profile.PlayerClass)
            {
                case CLASS_WARRIOR:
                case CLASS_ROGUE:
                    return true;
                case CLASS_DEATH_KNIGHT:
                    return _profile.ActiveSpec !=
                        TALENT_TREE_DEATH_KNIGHT_UNHOLY;
                case CLASS_DRUID:
                    return _profile.ActiveSpec ==
                        TALENT_TREE_DRUID_FERAL_COMBAT;
                case CLASS_PALADIN:
                    return _profile.ActiveSpec != TALENT_TREE_PALADIN_HOLY;
                case CLASS_SHAMAN:
                    return _profile.ActiveSpec ==
                        TALENT_TREE_SHAMAN_ENHANCEMENT;
                default:
                    return false;
            }
        }

        bool IsInstantDamageProfile() const
        {
            if (IsMeleeProfile())
                return true;

            switch (_profile.PlayerClass)
            {
                case CLASS_HUNTER:
                case CLASS_PRIEST:
                case CLASS_MAGE:
                case CLASS_WARLOCK:
                    return false;
                case CLASS_SHAMAN:
                    return _profile.ActiveSpec ==
                        TALENT_TREE_SHAMAN_ENHANCEMENT;
                case CLASS_DRUID:
                    return _profile.ActiveSpec ==
                        TALENT_TREE_DRUID_FERAL_COMBAT;
                default:
                    return true;
            }
        }

        float GetDesiredCombatRange() const
        {
            float range = std::max(_package.PrimaryRange,
                std::max(_package.SecondaryRange, _package.TertiaryRange));

            if (IsMeleeProfile())
                return 4.5f;

            return std::max(18.0f, range - 2.0f);
        }

        ShadowProfile _profile;
        SpellPackage _package;
        TacticalPackage _tactical;
        bool _initialized = false;
        uint32 _petInterceptMs = 0;
        uint32 _petInterceptCooldownMs = 0;
        uint32 _movementLockMs = 0;
    };

    class npc_solo_arena_shadow : public CreatureScript
    {
    public:
        npc_solo_arena_shadow() : CreatureScript("npc_solo_arena_shadow")
        {
        }

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_solo_arena_shadowAI(creature);
        }
    };

    struct npc_solo_arena_mechanicAI : public ScriptedAI
    {
        npc_solo_arena_mechanicAI(Creature* creature, bool ally) :
            ScriptedAI(creature), _ally(ally)
        {
        }

        void Reset() override
        {
            me->SetWalk(false);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterEvadeMode(EvadeReason /*why*/ = EVADE_REASON_OTHER) override
        {
            if (Unit* target = ResolveTarget())
            {
                me->Attack(target, true);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveChase(target, 1.5f);
                return;
            }

            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            Player* owner = ObjectAccessor::FindConnectedPlayer(
                me->GetSummonerGUID());
            if (!owner)
            {
                me->DespawnOrUnsummon();
                return;
            }

            ArenaSession const* session = SoloArenaMgr::Instance().GetSession(
                owner->GetGUID());
            if (!session || !SoloArenaMgr::Instance().IsCombatActive(
                    owner->GetGUID()))
            {
                me->DespawnOrUnsummon();
                return;
            }

            Unit* target = ResolveTarget();
            if (!target)
            {
                me->DespawnOrUnsummon();
                return;
            }

            if (!UpdateVictim())
            {
                AttackStart(target);
                return;
            }

            if (me->GetVictim() != target)
                AttackStart(target);

            DoMeleeAttackIfReady();
        }

    private:
        Unit* ResolveTarget() const
        {
            Player* owner = ObjectAccessor::FindConnectedPlayer(
                me->GetSummonerGUID());
            if (!owner)
                return nullptr;

            ArenaSession const* session = SoloArenaMgr::Instance().GetSession(
                owner->GetGUID());
            if (!session)
                return nullptr;

            if (_ally)
                return ObjectAccessor::GetCreature(*owner, session->BotGuid);

            return owner;
        }

        bool _ally = false;
    };

    class npc_solo_arena_helper : public CreatureScript
    {
    public:
        npc_solo_arena_helper() : CreatureScript("npc_solo_arena_helper")
        {
        }

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_solo_arena_mechanicAI(creature, true);
        }
    };

    class npc_solo_arena_hazard : public CreatureScript
    {
    public:
        npc_solo_arena_hazard() : CreatureScript("npc_solo_arena_hazard")
        {
        }

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_solo_arena_mechanicAI(creature, false);
        }
    };

    class SoloArenaWorldScript : public WorldScript
    {
    public:
        SoloArenaWorldScript() : WorldScript("SoloArenaWorldScript")
        {
        }

        void OnAfterConfigLoad(bool /*reload*/) override
        {
            SoloArenaConfig::Instance().Load();
        }

        void OnStartup() override
        {
            SoloArenaMgr::Instance().LoadStages();
            SoloArenaMgr::Instance().LoadMechanics();
        }

        void OnUpdate(uint32 diff) override
        {
            SoloArenaMgr::Instance().Update(diff);
        }
    };

    class SoloArenaPlayerScript : public PlayerScript
    {
    public:
        SoloArenaPlayerScript() :
            PlayerScript("SoloArenaPlayerScript",
                { PLAYERHOOK_ON_MAP_CHANGED,
                  PLAYERHOOK_ON_PLAYER_KILLED_BY_CREATURE })
        {
        }

        void OnPlayerMapChanged(Player* player) override
        {
            SoloArenaMgr::Instance().OnPlayerMapChanged(player);
        }

        void OnPlayerKilledByCreature(Creature* killer, Player* killed)
            override
        {
            if (!killer || !killed)
                return;

            if (!SoloArenaMgr::Instance().IsManagedShadow(killer))
                return;

            if (!SoloArenaMgr::Instance().RespawnObjectivePlayer(
                    killed->GetGUID()))
            {
                SoloArenaMgr::Instance().MarkFailure(killed->GetGUID());
            }
        }

        bool OnPlayerCanUseChat(
            Player* player,
            uint32 /*type*/,
            uint32 language,
            std::string& msg) override
        {
            if (!player || language != LANG_ADDON)
                return true;

            if (!StartsWith(msg, "TRIAL_CMD\t"))
                return true;

            return !HandleTrialAddonCommand(player, msg);
        }

        bool OnPlayerCanUseChat(
            Player* player,
            uint32 /*type*/,
            uint32 language,
            std::string& msg,
            Player* receiver) override
        {
            if (!player || language != LANG_ADDON)
                return true;

            if (!StartsWith(msg, "TRIAL_CMD\t"))
                return true;

            return !HandleTrialAddonCommand(player, msg);
        }
    };

    class SoloArenaArenaScript : public ArenaScript
    {
    public:
        SoloArenaArenaScript() : ArenaScript("SoloArenaArenaScript")
        {
        }

        bool OnBeforeArenaCheckWinConditions(Battleground* const bg) override
        {
            if (!bg)
                return true;

            if (!SoloArenaMgr::Instance().IsManagedArenaInstance(
                    bg->GetInstanceID()))
                return true;

            return false;
        }
    };

    class SoloArenaServerScript : public ServerScript
    {
    public:
        SoloArenaServerScript() : ServerScript("SoloArenaServerScript")
        {
        }

        bool CanPacketReceive(WorldSession* session,
            WorldPacket& packet) override
        {
            if (!session)
                return true;

            Player* viewer = session->GetPlayer();
            if (!viewer)
                return true;

            if (packet.GetOpcode() == CMSG_MESSAGECHAT)
            {
                WorldPacket copy(packet);
                uint32 type = 0;
                uint32 language = 0;
                copy >> type;
                copy >> language;

                if (language == LANG_ADDON && type == CHAT_MSG_WHISPER)
                {
                    std::string to;
                    copy >> to;
                    std::string msg = copy.ReadCString(false);

                    if (StartsWith(msg, "TRIAL_CMD\t"))
                    {
                        HandleTrialAddonCommand(viewer, msg);
                        return false;
                    }
                }

                return true;
            }

            if (packet.GetOpcode() != CMSG_GET_MIRRORIMAGE_DATA)
                return true;

            ObjectGuid shadowGuid;
            packet >> shadowGuid;

            ShadowProfile const* profile =
                SoloArenaMgr::Instance().GetShadowProfile(shadowGuid);
            if (!profile)
                return true;

            Player* player = ObjectAccessor::FindConnectedPlayer(
                profile->PlayerGuid);
            if (!player)
                return false;

            WorldPacket data;
            BuildMirrorImagePacket(data, shadowGuid, player);
            session->SendPacket(&data);
            return false;
        }
    };

    class SoloArenaAllSpellScript : public AllSpellScript
    {
    public:
        SoloArenaAllSpellScript() :
            AllSpellScript("SoloArenaAllSpellScript",
                { ALLSPELLHOOK_CAN_SELECT_SPEC_TALENT })
        {
        }

        bool CanSelectSpecTalent(Spell* spell) override
        {
            if (!spell)
                return true;

            Player* player = spell->GetCaster() ?
                spell->GetCaster()->ToPlayer() : nullptr;
            if (!player)
                return true;

            if (!SoloArenaMgr::Instance().HasSession(player->GetGUID()) &&
                !SoloArenaMgr::Instance().IsManagedArenaInstance(
                    player->GetInstanceId()))
                return true;

            if (player->GetSession())
                ChatHandler(player->GetSession()).PSendSysMessage("{}",
                    "시련 안에서는 이중특성을 변경할 수 없습니다.");
            return false;
        }
    };

    class SoloArenaUnitScript : public UnitScript
    {
    public:
        SoloArenaUnitScript() : UnitScript("SoloArenaUnitScript", true,
            { UNITHOOK_ON_DAMAGE })
        {
        }

        void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override
        {
            if (!attacker || !victim || damage == 0)
                return;

            Player* player = attacker->ToPlayer();
            if (!player)
                player = ObjectAccessor::FindConnectedPlayer(
                    attacker->GetOwnerGUID());

            if (!player)
                return;

            ArenaSession const* session = SoloArenaMgr::Instance().GetSession(
                player->GetGUID());
            if (!session || session->PlayerDamageBuffEndsAt == 0)
                return;

            if (uint64(std::time(nullptr)) >= session->PlayerDamageBuffEndsAt)
                return;

            if (victim->GetGUID() != session->BotGuid)
                return;

            damage = std::max<uint32>(damage + 1,
                uint32(float(damage) * 1.25f));
        }
    };
}

void AddSoloArenaScripts()
{
    new SoloArenaWorldScript();
    new SoloArenaPlayerScript();
    new SoloArenaArenaScript();
    new SoloArenaServerScript();
    new SoloArenaAllSpellScript();
    new SoloArenaUnitScript();
    new npc_solo_arena_master();
    new npc_solo_arena_shadow();
    new npc_solo_arena_helper();
    new npc_solo_arena_hazard();
}

