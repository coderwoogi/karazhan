#include "ArenaScript.h"
#include "AllSpellScript.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Duration.h"
#include "LFGMgr.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Pet.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include "../../mod-instance-bonus-mission/src/thirdparty/httplib.h"

#include <algorithm>
#include <array>
#include <atomic>
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
    constexpr uint32 SOLO_ARENA_PREPARATION_MS = 20000;
    constexpr uint32 DEFAULT_COMBAT_LIMIT_MS = 180000;

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
        uint64 CompletedAt = 0;
        uint64 FailedAt = 0;
        uint64 AbandonedAt = 0;
        uint32 SpawnDelayMs = 1000;
        uint32 FinishDelayMs = 3000;
        uint32 PetEntry = 0;
        uint32 PetDisplayId = 0;
        ArenaResult Result = ArenaResult::None;
        SessionState State = SessionState::PendingSpawn;
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
        std::vector<StageConfig> GetStages() const;
        StageConfig const* GetStage(uint8 stageId) const;
        bool HasSession(ObjectGuid const& playerGuid) const;
        ArenaSession const* GetSession(ObjectGuid const& playerGuid) const;
        bool IsCombatActive(ObjectGuid const& playerGuid) const;
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
        void GrantStageRewards(Player* player, ArenaSession const& session);
        void LogRun(Player* player, ArenaSession const& session);
        void LogEvent(Player* player, ArenaSession const& session,
            std::string const& eventType, std::string const& note = "");
        void LogReward(Player* player, ArenaSession const& session,
            uint32 itemEntry, uint32 itemCount, float chance,
            std::string const& status);
        std::string GetStageName(uint8 stageId) const;
        std::string BuildStageRewardPayload(uint8 stageId) const;
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

    uint64 GenerateRunUid()
    {
        static std::atomic<uint64> seed { 1 };
        uint64 now = uint64(std::time(nullptr));
        return (now << 20) | (seed.fetch_add(1) & 0xFFFFF);
    }

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
        uint8 playerClass, std::string const& eventType)
    {
        std::string classLabel = GetTrialClassDisplayName(playerClass);
        std::string playerName = player ? player->GetName() : "도전자";

        if (eventType == "spawn")
            return Acore::StringFormat(
                "{}, {}의 그림자가 너를 기다리고 있다.",
                playerName, classLabel);
        if (eventType == "combat_start")
            return Acore::StringFormat(
                "{}답게 끝까지 버텨 봐라. 지금부터 시작이다.",
                classLabel);
        if (eventType == "victory")
            return Acore::StringFormat(
                "{}, 이번 승부는 네가 가져갔다.",
                playerName);
        if (eventType == "failure")
            return Acore::StringFormat(
                "{}의 힘이 이 정도라면 아직 내 그림자를 넘기 어렵다.",
                classLabel);
        if (eventType == "abandoned")
            return Acore::StringFormat(
                "{}, 이번엔 물러섰군. 다음에는 끝까지 버텨 봐라.",
                playerName);

        return Acore::StringFormat(
            "{}, {}의 그림자는 언제든 너를 기다린다.",
            playerName, classLabel);
    }

    void SendTrialTimePayload(Player* player, ArenaSession const& session)
    {
        if (!player)
            return;

        std::ostringstream payload;
        payload << "TIME\t";
        payload << uint64(session.PreparationEndsAt) << "\t";
        payload << uint64(session.CombatStartedAt) << "\t";
        payload << uint64(session.CombatEndsAt) << "\t";
        payload << uint64(session.EndedAt) << "\t";
        payload << uint32(session.State);
        SendAddonPayload(player, "TRIAL_UI", payload.str());
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

        LOG_INFO("module",
            "SoloArena addon command: player='{}' msg='{}'",
            player->GetName(), msg);

        if (StartsWith(msg, "TRIAL_CMD\tSTART\t"))
        {
            std::string stageText = msg.substr(16);
            uint8 stageId = uint8(std::max(0, atoi(stageText.c_str())));
            LOG_INFO("module",
                "SoloArena start request: player='{}' stage={}",
                player->GetName(), uint32(stageId));
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

bool SoloArenaMgr::IsCombatActive(ObjectGuid const& playerGuid) const
{
    ArenaSession const* session = GetSession(playerGuid);
    if (!session)
        return false;

    return session->State == SessionState::Active;
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
    session.RunUid = GenerateRunUid();
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
    session.PreparationEndsAt = session.StartedAt +
        (SOLO_ARENA_PREPARATION_MS / 1000);

    _sessions[player->GetGUID().GetCounter()] = session;
    _managedArenaInstances.insert(session.ArenaInstanceId);
    LogEvent(player, _sessions[player->GetGUID().GetCounter()],
        "RUN_CREATED");

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

    if (Battleground* playerBg = player->GetBattleground())
    {
        ArenaSession& activeSession = _sessions[player->GetGUID().GetCounter()];
        activeSession.PreparationEndsAt = std::time(nullptr) +
            (SOLO_ARENA_PREPARATION_MS / 1000);
        SendTrialTimePayload(player, activeSession);
    }
    else
    {
        ArenaSession& activeSession = _sessions[player->GetGUID().GetCounter()];
        activeSession.PreparationEndsAt = std::time(nullptr) +
            (SOLO_ARENA_PREPARATION_MS / 1000);
        SendTrialTimePayload(player, activeSession);
    }

    float playerZ = stage->PlayerZ;
    if (Map* map = sMapMgr->CreateBaseMap(stage->ArenaMapId))
        playerZ = ResolveArenaGroundZ(map, stage->PlayerX, stage->PlayerY,
            stage->PlayerZ);

    if (!player->TeleportTo(stage->ArenaMapId, stage->PlayerX, stage->PlayerY,
        playerZ, stage->PlayerO, TELE_TO_GM_MODE))
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
    LogEvent(player, _sessions[player->GetGUID().GetCounter()],
        "PLAYER_TELEPORTED");
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
        entries << stage.MoveSpeedRate << "~";
        entries << BuildStageRewardPayload(stage.StageId);
    }

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

void SoloArenaMgr::Update(uint32 diff)
{
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
                }
                break;
            case SessionState::WaitingForStart:
                SyncShadowPet(player, session, false);

                if (player->GetMapId() != session.ArenaMapId)
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.AbandonedAt = std::time(nullptr);
                    session.EndedAt = session.AbandonedAt;
                    session.FinishDelayMs = 1;
                    LogEvent(player, session, "LEFT_ARENA_BEFORE_START");
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
                        SendTrialTimePayload(player, session);
                        SpeakTrialTaunt(player, session, "combat_start");
                        SendSystem(player,
                            "문이 열렸습니다. 그림자와의 결투가 시작됩니다.");
                    }
                }
                break;
            case SessionState::Active:
                SyncShadowPet(player, session, true);

                if (player->GetMapId() != session.ArenaMapId)
                {
                    session.Result = ArenaResult::Abandoned;
                    session.State = SessionState::PendingFinish;
                    session.AbandonedAt = std::time(nullptr);
                    session.EndedAt = session.AbandonedAt;
                    session.FinishDelayMs = 1;
                    LogEvent(player, session, "LEFT_ARENA");
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
                    SendTrialTimePayload(player, session);
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
    itr->second.EndedAt = std::time(nullptr);
    itr->second.CompletedAt = itr->second.EndedAt;
    itr->second.CombatEndsAt = itr->second.EndedAt;
    itr->second.FinishDelayMs = 3000;

    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid))
    {
        LogEvent(player, itr->second, "VICTORY");
        SendTrialTimePayload(player, itr->second);
        SpeakTrialTaunt(player, itr->second, "victory");
    }
}

void SoloArenaMgr::MarkFailure(ObjectGuid const& playerGuid)
{
    auto itr = _sessions.find(playerGuid.GetCounter());
    if (itr == _sessions.end())
        return;

    itr->second.Result = ArenaResult::Failure;
    itr->second.State = SessionState::PendingFinish;
    itr->second.EndedAt = std::time(nullptr);
    itr->second.FailedAt = itr->second.EndedAt;
    itr->second.CombatEndsAt = itr->second.EndedAt;
    itr->second.FinishDelayMs = 3000;

    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid))
    {
        LogEvent(player, itr->second, "FAILURE");
        SendTrialTimePayload(player, itr->second);
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
    itr->second.FinishDelayMs = 1;

    if (Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid))
    {
        LogEvent(player, itr->second, "ABANDONED");
        SendTrialTimePayload(player, itr->second);
        SpeakTrialTaunt(player, itr->second, "abandoned");
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

    ShadowProfile profile = CaptureShadowProfile(player, *stage);

    float botZ = stage->BotZ;
    if (Map* map = player->GetMap())
        botZ = ResolveArenaGroundZ(map, stage->BotX, stage->BotY, stage->BotZ);

    TempSummon* summon = player->SummonCreature(
        SoloArenaConfig::Instance().GetShadowEntry(),
        stage->BotX, stage->BotY, botZ, stage->BotO,
        TEMPSUMMON_MANUAL_DESPAWN, 0);

    if (!summon)
    {
        LOG_ERROR("module",
            "SoloArena shadow spawn failed: player='{}' stage={} map={} "
            "instance={} entry={} pos=({}, {}, {}, {})",
            player->GetName(), stage->StageId, player->GetMapId(),
            player->GetInstanceId(),
            SoloArenaConfig::Instance().GetShadowEntry(),
            stage->BotX, stage->BotY, botZ, stage->BotO);
        return false;
    }

    ConfigureShadow(summon, player, profile, *stage);

    session.BotGuid = summon->GetGUID();
    session.ArenaInstanceId = player->GetInstanceId();
    session.State = SessionState::WaitingForStart;

    _shadowProfiles[summon->GetGUID().GetCounter()] = profile;

    summon->ClearInCombat();
    player->ClearInCombat();
    SyncShadowPet(player, session, false);

    if (WorldSession* playerSession = player->GetSession())
    {
        WorldPacket data;
        BuildMirrorImagePacket(data, summon->GetGUID(), player);
        playerSession->SendPacket(&data);
    }

    SendSystem(player,
        "그림자가 모습을 드러냈습니다. 문이 열리면 전투가 시작됩니다.");
    Debug("Solo arena shadow spawned: player='{}' stage={} botGuid={}",
        player->GetName(), stage->StageId, summon->GetGUID().ToString());
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
    summon->SetReactState(REACT_PASSIVE);
    summon->SetFaction(14);
    summon->SetWalk(false);
    summon->StopMoving();
    summon->SetHomePosition(stage.BotX, stage.BotY, stage.BotZ, stage.BotO);
}

void SoloArenaMgr::FinishSession(Player* player, ArenaSession& session)
{
    CleanupPet(session);
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
            GrantStageRewards(player, session);
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
            SendSystem(player, "시련을 종료했습니다.");
            break;
        default:
            break;
    }

    LogRun(player, session);
    LogEvent(player, session, "RUN_FINISHED");
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

    return BuildTrialTauntFallback(player, player->getClass(), eventType);
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

void SoloArenaMgr::GrantStageRewards(Player* player, ArenaSession const& session)
{
    if (!player)
        return;

    QueryResult result = WorldDatabase.Query(
        "SELECT item_entry, item_count, chance "
        "FROM solo_arena_stage_reward "
        "WHERE stage_id = {} AND enabled = 1 "
        "ORDER BY sort_order, id",
        session.StageId);

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 itemEntry = fields[0].Get<uint32>();
        uint32 itemCount = std::max<uint32>(1, fields[1].Get<uint32>());
        float chance = fields[2].Get<float>();
        float roll = float(std::rand() % 10000) / 100.0f;

        if (chance > 0.0f && roll > chance)
        {
            LogReward(player, session, itemEntry, itemCount, chance, "SKIPPED");
            continue;
        }

        if (player->AddItem(itemEntry, itemCount))
        {
            LogReward(player, session, itemEntry, itemCount, chance, "GRANTED");
            continue;
        }

        LogReward(player, session, itemEntry, itemCount, chance, "FAILED");
    } while (result->NextRow());
}

void SoloArenaMgr::LogRun(Player* player, ArenaSession const& session)
{
    CharacterDatabase.Execute(
        "INSERT INTO solo_arena_run_log "
        "(run_uid, guid, account_id, player_name, stage_id, stage_name, "
        "result, result_label, session_state, started_at, "
        "preparation_ends_at, combat_started_at, combat_ended_at, "
        "ended_at, completed_at, failed_at, abandoned_at, duration_sec, "
        "arena_map_id, arena_instance_id, return_map_id) "
        "VALUES ({}, {}, {}, '{}', {}, '{}', {}, '{}', {}, {}, {}, {}, "
        "{}, {}, {}, {}, {}, {}, {}, {}, {})",
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

std::string SoloArenaMgr::BuildStageRewardPayload(uint8 stageId) const
{
    QueryResult result = WorldDatabase.Query(
        "SELECT item_entry, item_count, chance "
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

        if (!first)
            rewards << ",";

        first = false;
        rewards << itemEntry << "^" << itemCount << "^" << chance;
    } while (result->NextRow());

    return rewards.str();
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
            }
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

        void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
        {
            InitializeProfile();

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
            float cooldownScale = std::clamp(_profile.CastSpeedRate,
                0.55f, 1.25f);
            _package.PrimaryCooldownMs = std::max<uint32>(900u,
                uint32(float(_package.PrimaryCooldownMs) * cooldownScale));
            _package.SecondaryCooldownMs = std::max<uint32>(1200u,
                uint32(float(_package.SecondaryCooldownMs) * cooldownScale));
            _package.TertiaryCooldownMs = std::max<uint32>(1500u,
                uint32(float(_package.TertiaryCooldownMs) * cooldownScale));
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
                me->GetMotionMaster()->MoveChase(victim,
                    IsMeleeProfile() ? 1.5f : GetDesiredCombatRange());
                return;
            }

            if (!me->IsWithinLOSInMap(victim))
            {
                RepositionForCombat(victim, range);
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
                    RepositionForCombat(victim, range);
                return;
            }

            SpellCastResult result = me->CastSpell(victim, spellId, false);
            if (result != SPELL_FAILED_SUCCESS)
            {
                RepositionForCombat(victim, range);
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

        void MaintainCombatSpacing()
        {
            Unit* victim = me->GetVictim();
            if (!victim || !victim->IsAlive())
                return;

            if (victim->ToPet())
            {
                if (!me->IsWithinMeleeRange(victim))
                    me->GetMotionMaster()->MoveChase(victim, 1.5f);
                return;
            }

            float distance = me->GetDistance(victim);
            if (IsMeleeProfile())
            {
                if (!me->IsWithinMeleeRange(victim))
                    me->GetMotionMaster()->MoveChase(victim, 1.5f);
                return;
            }

            float desiredRange = GetDesiredCombatRange();
            if (distance < (desiredRange * 0.55f))
            {
                RepositionForCombat(victim, desiredRange);
                return;
            }

            if (distance > (desiredRange + 4.0f))
                me->GetMotionMaster()->MoveChase(victim, desiredRange);
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
            if (!victim)
                return;

            float range = IsMeleeProfile() ? 2.5f :
                std::max(12.0f, preferredRange - 2.0f);
            float angle = victim->GetAbsoluteAngle(me) + float(M_PI) * 0.5f;
            float x = victim->GetPositionX() + std::cos(angle) * range;
            float y = victim->GetPositionY() + std::sin(angle) * range;
            float z = victim->GetPositionZ();
            if (Map* map = me->GetMap())
                z = ResolveArenaGroundZ(map, x, y, z);

            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(2, x, y, z);
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
        bool _initialized = false;
        uint32 _petInterceptMs = 0;
        uint32 _petInterceptCooldownMs = 0;
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
}

void AddSoloArenaScripts()
{
    new SoloArenaWorldScript();
    new SoloArenaPlayerScript();
    new SoloArenaArenaScript();
    new SoloArenaServerScript();
    new SoloArenaAllSpellScript();
    new npc_solo_arena_master();
    new npc_solo_arena_shadow();
}
