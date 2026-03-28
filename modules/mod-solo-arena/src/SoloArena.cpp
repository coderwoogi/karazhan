#include "ArenaScript.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Duration.h"
#include "LFGMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellMgr.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr uint32 DEFAULT_ARENA_MAP_ID = 572;
    constexpr BattlegroundTypeId DEFAULT_ARENA_BG_TYPE = BATTLEGROUND_RL;
    constexpr uint32 ACTION_STAGE_BASE = 100;
    constexpr uint32 ACTION_ABANDON = 500;

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
        PendingFinish = 3
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
        EVENT_DEFENSIVE = 4
    };

    struct StageConfig
    {
        uint8 StageId = 0;
        std::string Name;
        uint32 ArenaMapId = DEFAULT_ARENA_MAP_ID;
        float PlayerX = 1298.61f;
        float PlayerY = 1598.59f;
        float PlayerZ = 31.62f;
        float PlayerO = 1.57f;
        float BotX = 1273.71f;
        float BotY = 1734.05f;
        float BotZ = 31.61f;
        float BotO = 4.71f;
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
        ObjectGuid PlayerGuid = ObjectGuid::Empty;
        ObjectGuid BotGuid = ObjectGuid::Empty;
        uint8 StageId = 0;
        uint32 ArenaMapId = DEFAULT_ARENA_MAP_ID;
        uint32 ArenaInstanceId = 0;
        uint32 ReturnMapId = 0;
        float ReturnX = 0.0f;
        float ReturnY = 0.0f;
        float ReturnZ = 0.0f;
        float ReturnO = 0.0f;
        uint64 StartedAt = 0;
        uint32 SpawnDelayMs = 1000;
        uint32 FinishDelayMs = 3000;
        ArenaResult Result = ArenaResult::None;
        SessionState State = SessionState::PendingSpawn;
    };

    struct ShadowProfile
    {
        ObjectGuid PlayerGuid = ObjectGuid::Empty;
        uint8 StageId = 0;
        uint8 PlayerClass = CLASS_WARRIOR;
        uint32 ActiveSpec = 0;
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
        std::vector<StageConfig> GetStages() const;
        StageConfig const* GetStage(uint8 stageId) const;
        bool HasSession(ObjectGuid const& playerGuid) const;
        ArenaSession const* GetSession(ObjectGuid const& playerGuid) const;
        bool StartChallenge(Player* player, uint8 stageId);
        void SendUi(Player* player);
        void Update(uint32 diff);
        void OnPlayerMapChanged(Player* player);
        void MarkVictory(ObjectGuid const& playerGuid);
        void MarkFailure(ObjectGuid const& playerGuid);
        void MarkAbandoned(ObjectGuid const& playerGuid);
        ShadowProfile const* GetShadowProfile(
            ObjectGuid const& creatureGuid) const;
        void UnregisterShadow(ObjectGuid const& creatureGuid);
        bool IsManagedShadow(Creature const* creature) const;
        bool IsManagedArenaInstance(uint32 instanceId) const;

    private:
        bool SpawnShadow(Player* player, ArenaSession& session);
        void ConfigureShadow(Creature* summon, Player* player,
            StageConfig const& stage);
        void FinishSession(Player* player, ArenaSession& session);
        void CleanupBot(ArenaSession const& session);
        uint8 GetHighestStageCleared(Player* player) const;
        bool IsStageUnlocked(Player* player, uint8 stageId) const;
        void SaveProgress(Player* player, uint8 stageId);
        void LogRun(Player* player, ArenaSession const& session);
        std::string GetStageName(uint8 stageId) const;
        void LoadDefaultStages();

        template <typename... Args>
        void Debug(std::string const& fmt, Args&&... args) const
        {
            if (!SoloArenaConfig::Instance().IsDebug())
                return;

            LOG_INFO("module", "{}", Acore::StringFormat(
                fmt, std::forward<Args>(args)...));
        }

        static void SendSystem(Player* player, std::string const& message)
        {
            if (!player || !player->GetSession())
                return;

            ChatHandler(player->GetSession()).PSendSysMessage("{}", message);
        }

        std::unordered_map<uint8, StageConfig> _stages;
        std::unordered_map<uint64, ArenaSession> _sessions;
        std::unordered_map<uint64, ShadowProfile> _shadowProfiles;
        std::unordered_set<uint32> _managedArenaInstances;
    };

    SpellPackage GetSpellPackage(uint8 playerClass, uint32 activeSpec);
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

    bool StartsWith(std::string const& text, std::string const& token)
    {
        return text.compare(0, token.size(), token) == 0;
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

        _stages[stage.StageId] = stage;
    } while (result->NextRow());

    if (_stages.empty())
        LoadDefaultStages();
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

    Battleground* arenaTemplate =
        sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
    if (!arenaTemplate)
    {
        SendSystem(player, "솔로 투기장 템플릿을 찾지 못했습니다.");
        return false;
    }

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketById(
        arenaTemplate->GetMapId(), arenaTemplate->GetBracketId());
    if (!bracketEntry)
    {
        SendSystem(player, "솔로 투기장 등급 정보를 찾지 못했습니다.");
        return false;
    }

    Battleground* arena = sBattlegroundMgr->CreateNewBattleground(
        DEFAULT_ARENA_BG_TYPE, bracketEntry, ARENA_TYPE_2v2, false);
    if (!arena)
    {
        SendSystem(player, "투기장 인스턴스를 만들지 못했습니다.");
        return false;
    }

    arena->StartBattleground();

    ArenaSession session;
    session.PlayerGuid = player->GetGUID();
    session.StageId = stageId;
    session.ArenaMapId = stage->ArenaMapId;
    session.ArenaInstanceId = arena->GetInstanceID();
    session.ReturnMapId = player->GetMapId();
    session.ReturnX = player->GetPositionX();
    session.ReturnY = player->GetPositionY();
    session.ReturnZ = player->GetPositionZ();
    session.ReturnO = player->GetOrientation();
    session.StartedAt = std::time(nullptr);

    _sessions[player->GetGUID().GetCounter()] = session;
    _managedArenaInstances.insert(session.ArenaInstanceId);

    TeamId teamId = Player::TeamIdForRace(player->getRace());
    uint32 queueSlot = 0;

    player->SetEntryPoint();

    WorldPacket data;
    sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, arena, queueSlot,
        STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME, 0,
        arena->GetArenaType(), teamId);
    player->SendDirectMessage(&data);

    sLFGMgr->LeaveAllLfgQueues(player->GetGUID(), false);

    player->SetBattlegroundId(arena->GetInstanceID(), arena->GetBgTypeID(),
        queueSlot, true, false, teamId);

    if (!player->TeleportTo(stage->ArenaMapId, stage->PlayerX, stage->PlayerY,
        stage->PlayerZ, stage->PlayerO, TELE_TO_GM_MODE))
    {
        _sessions.erase(player->GetGUID().GetCounter());
        _managedArenaInstances.erase(session.ArenaInstanceId);
        player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE,
            PLAYER_MAX_BATTLEGROUND_QUEUES, false, false, TEAM_NEUTRAL);
        SendSystem(player, "투기장으로 이동하지 못했습니다.");
        return false;
    }

    SendSystem(player, Acore::StringFormat(
        "{} 시작. 언더시티 투기장으로 이동합니다.", stage->Name));
    Debug("Solo arena started: player='{}' stage={} map={} instance={}",
        player->GetName(), stageId, stage->ArenaMapId, session.ArenaInstanceId);
    return true;
}

void SoloArenaMgr::SendUi(Player* player)
{
    if (!SoloArenaConfig::Instance().IsEnabled() || !player)
        return;

    uint8 highestStage = GetHighestStageCleared(player);
    uint8 maxUnlockedStage = highestStage + 1;

    std::ostringstream entries;
    bool first = true;

    for (StageConfig const& stage : GetStages())
    {
        if (!stage.Enabled)
            continue;

        if (stage.StageId > maxUnlockedStage)
            continue;

        if (!first)
            entries << "|";

        first = false;
        entries << uint32(stage.StageId) << "~";
        entries << SanitizeAddonField(stage.Name, 64) << "~";
        entries << stage.HealthMultiplier << "~";
        entries << stage.DamageMultiplier << "~";
        entries << stage.SpellIntervalMs << "~";
        entries << stage.MoveSpeedRate;
    }

    std::ostringstream payload;
    payload << "OPEN\t";
    payload << uint32(highestStage) << "\t";
    payload << entries.str();
    SendAddonPayload(player, "TRIAL_UI", payload.str());
}

void SoloArenaMgr::Update(uint32 diff)
{
    std::vector<uint64> toErase;

    for (auto& [playerKey, session] : _sessions)
    {
        Player* player = ObjectAccessor::FindConnectedPlayer(session.PlayerGuid);
        if (!player)
        {
            CleanupBot(session);
            toErase.push_back(playerKey);
            continue;
        }

        switch (session.State)
        {
            case SessionState::PendingSpawn:
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
                    session.FinishDelayMs = 1;
                }
                break;
            case SessionState::WaitingForStart:
                if (player->GetMapId() != session.ArenaMapId)
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.FinishDelayMs = 1;
                    break;
                }

                if (session.BotGuid.IsEmpty())
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.FinishDelayMs = 1;
                    break;
                }

                if (Battleground* bg = player->GetBattleground())
                {
                    if (bg->GetStatus() == STATUS_IN_PROGRESS)
                    {
                        if (Creature* bot = ObjectAccessor::GetCreature(
                                *player, session.BotGuid))
                            StartShadowCombat(player, bot);

                        session.State = SessionState::Active;
                        SendSystem(player,
                            "문이 열렸습니다. 그림자와의 결투가 시작됩니다.");
                    }
                }
                break;
            case SessionState::Active:
                if (player->GetMapId() != session.ArenaMapId)
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.FinishDelayMs = 1;
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
                toErase.push_back(playerKey);
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
    }
}

void SoloArenaMgr::MarkVictory(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return;

    itr->second.Result = ArenaResult::Victory;
    itr->second.State = SessionState::PendingFinish;
    itr->second.FinishDelayMs = 3000;
}

void SoloArenaMgr::MarkFailure(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return;

    itr->second.Result = ArenaResult::Failure;
    itr->second.State = SessionState::PendingFinish;
    itr->second.FinishDelayMs = 3000;
}

void SoloArenaMgr::MarkAbandoned(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return;

    itr->second.Result = ArenaResult::Abandoned;
    itr->second.State = SessionState::PendingFinish;
    itr->second.FinishDelayMs = 1;
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

    TempSummon* summon = player->SummonCreature(
        SoloArenaConfig::Instance().GetShadowEntry(),
        stage->BotX, stage->BotY, stage->BotZ, stage->BotO,
        TEMPSUMMON_MANUAL_DESPAWN, 0);

    if (!summon)
    {
        LOG_ERROR("module",
            "SoloArena shadow spawn failed: player='{}' stage={} map={} "
            "instance={} entry={} pos=({}, {}, {}, {})",
            player->GetName(), stage->StageId, player->GetMapId(),
            player->GetInstanceId(),
            SoloArenaConfig::Instance().GetShadowEntry(),
            stage->BotX, stage->BotY, stage->BotZ, stage->BotO);
        return false;
    }

    ConfigureShadow(summon, player, *stage);

    session.BotGuid = summon->GetGUID();
    session.ArenaInstanceId = player->GetInstanceId();
    session.State = SessionState::WaitingForStart;

    ShadowProfile profile;
    profile.PlayerGuid = player->GetGUID();
    profile.StageId = stage->StageId;
    profile.PlayerClass = player->getClass();
    profile.ActiveSpec = player->GetSpec(player->GetActiveSpec());
    profile.DamageMultiplier = stage->DamageMultiplier;
    profile.SpellIntervalMs = stage->SpellIntervalMs;
    _shadowProfiles[summon->GetGUID().GetCounter()] = profile;

    summon->ClearInCombat();
    player->ClearInCombat();

    if (WorldSession* playerSession = player->GetSession())
    {
        WorldPacket data;
        BuildMirrorImagePacket(data, summon->GetGUID(), player);
        playerSession->SendPacket(&data);
    }

    SendSystem(player,
        "Shadow spawned. Combat begins when the gates open.");
    Debug("Solo arena shadow spawned: player='{}' stage={} botGuid={}",
        player->GetName(), stage->StageId, summon->GetGUID().ToString());
    return true;
}

void SoloArenaMgr::ConfigureShadow(Creature* summon, Player* player,
    StageConfig const& stage)
{
    summon->SetName(player->GetName());
    ApplyShadowCloneVisual(player, summon);
    summon->SetLevel(player->GetLevel());
    summon->SetByteValue(UNIT_FIELD_BYTES_0, 0, player->getRace());
    summon->SetByteValue(UNIT_FIELD_BYTES_0, 1, player->getClass());
    summon->SetByteValue(UNIT_FIELD_BYTES_0, 2, player->getGender());
    summon->SetByteValue(UNIT_FIELD_BYTES_2, 0, player->GetSheath());
    summon->SetObjectScale(player->GetObjectScale());

    bool hasMainHand = false;
    bool hasOffHandWeapon = false;

    if (Item* mainHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0,
        EQUIPMENT_SLOT_MAINHAND))
    {
        summon->SetVirtualItem(0, mainHand->GetEntry());
        hasMainHand = true;
    }
    else
        summon->SetVirtualItem(0, 0);

    if (Item* offHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0,
        EQUIPMENT_SLOT_OFFHAND))
    {
        summon->SetVirtualItem(1, offHand->GetEntry());
        if (offHand->GetTemplate() &&
            offHand->GetTemplate()->Class == ITEM_CLASS_WEAPON)
        {
            hasOffHandWeapon = true;
        }
    }
    else
        summon->SetVirtualItem(1, 0);

    if (Item* ranged = player->GetItemByPos(INVENTORY_SLOT_BAG_0,
        EQUIPMENT_SLOT_RANGED))
    {
        summon->SetVirtualItem(2, ranged->GetEntry());
    }
    else
        summon->SetVirtualItem(2, 0);

    summon->SetCanDualWield(hasMainHand && hasOffHandWeapon);

    uint32 maxHealth = std::max<uint32>(5000u,
        static_cast<uint32>(player->GetMaxHealth() *
            stage.HealthMultiplier));
    summon->SetMaxHealth(maxHealth);
    summon->SetHealth(maxHealth);

    float averageItemLevel = std::max<float>(1.0f,
        player->GetAverageItemLevel());
    float baseDamage =
        ((averageItemLevel * 6.5f) +
        (static_cast<float>(player->GetLevel()) * 12.0f)) *
        stage.DamageMultiplier;

    summon->SetAttackTime(BASE_ATTACK, stage.AttackTimeMs);
    summon->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, baseDamage * 0.8f);
    summon->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, baseDamage * 1.2f);
    summon->UpdateDamagePhysical(BASE_ATTACK);
    summon->SetSpeed(MOVE_RUN, stage.MoveSpeedRate, true);
    summon->SetReactState(REACT_PASSIVE);
    summon->SetFaction(14);
    summon->SetWalk(false);
    summon->StopMoving();
}

void SoloArenaMgr::FinishSession(Player* player, ArenaSession& session)
{
    CleanupBot(session);

    if (session.Result == ArenaResult::Failure && !player->IsAlive())
    {
        player->ResurrectPlayer(1.0f);
        player->SpawnCorpseBones();
    }

    if (Battleground* battleground = player->GetBattleground())
        battleground->RemovePlayerAtLeave(player);
    else if (session.ArenaInstanceId)
        if (Battleground* arena = sBattlegroundMgr->GetBattleground(
                session.ArenaInstanceId, DEFAULT_ARENA_BG_TYPE))
            arena->RemovePlayerAtLeave(player);

    _managedArenaInstances.erase(session.ArenaInstanceId);

    player->TeleportTo(session.ReturnMapId, session.ReturnX, session.ReturnY,
        session.ReturnZ, session.ReturnO);

    switch (session.Result)
    {
        case ArenaResult::Victory:
            SaveProgress(player, session.StageId);
            SendSystem(player, Acore::StringFormat(
                "{} 클리어. 다음 단계가 열렸습니다.",
                GetStageName(session.StageId)));
            break;
        case ArenaResult::Failure:
            SendSystem(player, Acore::StringFormat(
                "{} 실패. 다시 도전할 수 있습니다.",
                GetStageName(session.StageId)));
            break;
        case ArenaResult::Abandoned:
            SendSystem(player, "솔로 아레나 시련이 종료되었습니다.");
            break;
        default:
            break;
    }

    LogRun(player, session);
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

void SoloArenaMgr::LogRun(Player* player, ArenaSession const& session)
{
    CharacterDatabase.Execute(
        "INSERT INTO solo_arena_run_log "
        "(guid, stage_id, result, started_at, ended_at, "
        "arena_map_id, arena_instance_id) "
        "VALUES ({}, {}, {}, {}, {}, {}, {})",
        player->GetGUID().GetCounter(),
        session.StageId,
        static_cast<uint8>(session.Result),
        session.StartedAt,
        std::time(nullptr),
        session.ArenaMapId,
        session.ArenaInstanceId);
}

std::string SoloArenaMgr::GetStageName(uint8 stageId) const
{
    StageConfig const* stage = GetStage(stageId);
    if (stage)
        return stage->Name;

    return Acore::StringFormat("시련 {}단계", stageId);
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
    stage2.HealthMultiplier = 1.35f;
    stage2.DamageMultiplier = 1.25f;
    stage2.AttackTimeMs = 1600;
    stage2.SpellIntervalMs = 3000;
    stage2.MoveSpeedRate = 1.10f;
    _stages[stage2.StageId] = stage2;

    StageConfig stage3 = stage1;
    stage3.StageId = 3;
    stage3.Name = "그림자 시련 3단계";
    stage3.HealthMultiplier = 1.75f;
    stage3.DamageMultiplier = 1.55f;
    stage3.AttackTimeMs = 1300;
    stage3.SpellIntervalMs = 2100;
    stage3.MoveSpeedRate = 1.20f;
    _stages[stage3.StageId] = stage3;
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

    uint32 ComputeShadowSpellDamage(Creature* me,
        ShadowProfile const& profile,
        float factor)
    {
        if (!me)
            return 0;

        float baseDamage = (me->GetMaxHealth() / 18.0f) * factor;
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
            events.ScheduleEvent(EVENT_TERTIARY,
                Milliseconds(_package.TertiaryCooldownMs));
            events.ScheduleEvent(EVENT_DEFENSIVE,
                Milliseconds(_package.DefensiveCooldownMs));
        }

        void KilledUnit(Unit* victim) override
        {
            if (Player* player = victim->ToPlayer())
                SoloArenaMgr::Instance().MarkFailure(player->GetGUID());
        }

        void JustDied(Unit* killer) override
        {
            if (Player* player = killer ? killer->ToPlayer() : nullptr)
                SoloArenaMgr::Instance().MarkVictory(player->GetGUID());

            SoloArenaMgr::Instance().UnregisterShadow(me->GetGUID());
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            InitializeProfile();
            if (!_initialized)
                return;

            events.Update(diff);

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
                }
            }

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
                me->GetMotionMaster()->MoveChase(victim);
                return;
            }

            me->CastSpell(victim, spellId, true);

            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            {
                uint32 damage = ComputeShadowSpellDamage(me, _profile,
                    damageFactor);
                Unit::DealDamage(me, victim, damage, nullptr, DIRECT_DAMAGE,
                    schoolMask, spellInfo, false);
            }
        }

        ShadowProfile _profile;
        SpellPackage _package;
        bool _initialized = false;
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

            SoloArenaMgr::Instance().MarkFailure(killed->GetGUID());
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

            if (!receiver || receiver != player)
                return true;

            if (!StartsWith(msg, "TRIAL_CMD\t"))
                return true;

            if (StartsWith(msg, "TRIAL_CMD\tSTART\t"))
            {
                std::string stageText = msg.substr(16);
                uint8 stageId = uint8(std::max(0, atoi(stageText.c_str())));
                SoloArenaMgr::Instance().StartChallenge(player, stageId);
                return false;
            }

            if (msg == "TRIAL_CMD\tOPEN")
            {
                SoloArenaMgr::Instance().SendUi(player);
                return false;
            }

            return false;
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
            if (!session ||
                packet.GetOpcode() != CMSG_GET_MIRRORIMAGE_DATA)
                return true;

            Player* viewer = session->GetPlayer();
            if (!viewer)
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
}

void AddSoloArenaScripts()
{
    new SoloArenaWorldScript();
    new SoloArenaPlayerScript();
    new SoloArenaArenaScript();
    new SoloArenaServerScript();
    new npc_solo_arena_master();
    new npc_solo_arena_shadow();
}
