#include "Config.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "StringFormat.h"

namespace
{
    class AutoLevelupConfig
    {
    public:
        static AutoLevelupConfig& Instance()
        {
            static AutoLevelupConfig instance;
            return instance;
        }

        void Load()
        {
            _enabled = sConfigMgr->GetOption<bool>("AutoLevelup.Enable",
                true);
            _firstCharacterLevel = sConfigMgr->GetOption<uint32>(
                "AutoLevelup.FirstCharacterLevel", 70);
            _otherCharacterLevel = sConfigMgr->GetOption<uint32>(
                "AutoLevelup.OtherCharacterLevel", 1);
        }

        bool IsEnabled() const
        {
            return _enabled;
        }

        uint8 GetFirstCharacterLevel() const
        {
            return NormalizeLevel(_firstCharacterLevel);
        }

        uint8 GetOtherCharacterLevel() const
        {
            return NormalizeLevel(_otherCharacterLevel);
        }

    private:
        static uint8 NormalizeLevel(uint32 level)
        {
            if (level < 1)
                return 1;

            if (level > STRONG_MAX_LEVEL)
                return STRONG_MAX_LEVEL;

            return static_cast<uint8>(level);
        }

        bool _enabled = true;
        uint32 _firstCharacterLevel = 70;
        uint32 _otherCharacterLevel = 1;
    };

    uint32 GetCurrentNonDeathKnightCharacterCount(uint32 accountId)
    {
        std::string query = Acore::StringFormat(
            "SELECT COUNT(*) FROM characters WHERE account = {} AND class <> {}",
            accountId,
            CLASS_DEATH_KNIGHT);

        QueryResult result = CharacterDatabase.Query(query);
        if (!result)
            return 0;

        return result->Fetch()[0].Get<uint64>();
    }

    void ApplyStartLevel(Player* player, uint8 level)
    {
        if (player->GetLevel() != level)
            player->GiveLevel(level);

        player->SetUInt32Value(PLAYER_XP, 0);
        player->SaveToDB(false, false);
    }

    // 첫 캐릭터에게 부여할 스펠 (Hira Snowdawn / TrainerId 36 의 승마·한랭비행과 동일)
    constexpr uint32 FIRST_CHARACTER_SPELLS[] =
    {
        33388, // Apprentice Riding (견습 승마)
        33391, // Journeyman Riding (숙련 승마)
        34090, // Expert Riding (전문가 승마, 비행 150%)
        34091, // Artisan Riding (장인 승마, 비행 280%)
        54197  // Cold Weather Flying (한랭 비행)
    };

    // 부족한 스펠만 학습(이미 가진 건 건너뜀). 하나라도 배우면 true.
    bool LearnFirstCharacterSpells(Player* player)
    {
        bool learnedAny = false;
        for (uint32 spellId : FIRST_CHARACTER_SPELLS)
        {
            if (!player->HasSpell(spellId))
            {
                player->learnSpell(spellId);
                learnedAny = true;
            }
        }

        if (learnedAny)
            player->SaveToDB(false, false);

        return learnedAny;
    }

    // 이 캐릭터가 계정에서 가장 먼저 생성된 비-죽음의기사 캐릭터인가
    // (가장 낮은 guid = 가장 먼저 생성됨 → mod-auto-levelup 의 "첫 캐릭터" 정의와 일치)
    bool IsFirstNonDeathKnightCharacter(Player* player)
    {
        uint32 accountId = player->GetSession()->GetAccountId();
        std::string query = Acore::StringFormat(
            "SELECT MIN(guid) FROM characters "
            "WHERE account = {} AND class <> {}",
            accountId,
            CLASS_DEATH_KNIGHT);

        QueryResult result = CharacterDatabase.Query(query);
        if (!result)
            return false;

        uint32 firstGuid = result->Fetch()[0].Get<uint32>();
        return firstGuid != 0 && firstGuid == player->GetGUID().GetCounter();
    }
}

class AutoLevelupWorldScript : public WorldScript
{
public:
    AutoLevelupWorldScript() : WorldScript("AutoLevelupWorldScript")
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        AutoLevelupConfig::Instance().Load();
    }
};

class AutoLevelupPlayerScript : public PlayerScript
{
public:
    AutoLevelupPlayerScript() :
        PlayerScript("AutoLevelupPlayerScript",
            { PLAYERHOOK_ON_CREATE, PLAYERHOOK_ON_LOGIN })
    {
    }

    void OnPlayerCreate(Player* player) override
    {
        AutoLevelupConfig const& config = AutoLevelupConfig::Instance();
        if (!config.IsEnabled())
            return;

        if (player->IsClass(CLASS_DEATH_KNIGHT))
            return;

        uint32 accountId = player->GetSession()->GetAccountId();
        uint32 characterCount = GetCurrentNonDeathKnightCharacterCount(accountId);

        bool isFirstCharacter = characterCount == 1;
        uint8 targetLevel = isFirstCharacter
            ? config.GetFirstCharacterLevel()
            : config.GetOtherCharacterLevel();

        ApplyStartLevel(player, targetLevel);

        // 첫 캐릭터에게만 승마/한랭비행 스펠 부여 (2번째 캐릭터부터는 미부여)
        if (isFirstCharacter)
            LearnFirstCharacterSpells(player);
    }

    // 이미 생성된 첫 캐릭터가 스펠을 갖고 있지 않으면 로그인 시 자동 학습
    void OnPlayerLogin(Player* player) override
    {
        AutoLevelupConfig const& config = AutoLevelupConfig::Instance();
        if (!config.IsEnabled())
            return;

        if (player->IsClass(CLASS_DEATH_KNIGHT))
            return;

        // 이미 5개 모두 보유하면 DB 조회 없이 종료
        bool missingAny = false;
        for (uint32 spellId : FIRST_CHARACTER_SPELLS)
        {
            if (!player->HasSpell(spellId))
            {
                missingAny = true;
                break;
            }
        }

        if (!missingAny)
            return;

        // 부족한 경우, 이 캐릭터가 계정의 첫 비-DK 캐릭터일 때만 학습
        if (IsFirstNonDeathKnightCharacter(player))
            LearnFirstCharacterSpells(player);
    }
};

void AddAutoLevelupScripts()
{
    new AutoLevelupWorldScript();
    new AutoLevelupPlayerScript();
}
