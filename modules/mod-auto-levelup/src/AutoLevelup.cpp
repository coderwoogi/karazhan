#include "Config.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
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

    uint32 GetCurrentCharacterCount(uint32 accountId)
    {
        std::string query = Acore::StringFormat(
            "SELECT COUNT(*) FROM characters WHERE account = {}",
            accountId);

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
        PlayerScript("AutoLevelupPlayerScript", { PLAYERHOOK_ON_CREATE })
    {
    }

    void OnPlayerCreate(Player* player) override
    {
        AutoLevelupConfig const& config = AutoLevelupConfig::Instance();
        if (!config.IsEnabled())
            return;

        uint32 accountId = player->GetSession()->GetAccountId();
        uint32 characterCount = GetCurrentCharacterCount(accountId);

        uint8 targetLevel = characterCount == 1
            ? config.GetFirstCharacterLevel()
            : config.GetOtherCharacterLevel();

        ApplyStartLevel(player, targetLevel);
    }
};

void AddAutoLevelupScripts()
{
    new AutoLevelupWorldScript();
    new AutoLevelupPlayerScript();
}
