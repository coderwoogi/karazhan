#include "ScriptMgr.h"
#include "Player.h"
#include "RandomQuestSystem.h"

class RandomQuest_PlayerScript : public PlayerScript
{
public:
    RandomQuest_PlayerScript() : PlayerScript("RandomQuest_PlayerScript") { }

    void OnPlayerQuestRewardItem(Player* player, Item* /*item*/, uint32 /*count*/) override
    {
        // This hook is for items, but we need OnPlayerCompleteQuest or similar.
        // Checking PlayerScript.h... 
        // PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST -> OnPlayerCompleteQuest
    }


    void OnPlayerQuestAbandon(Player* player, uint32 questId) override
    {
        sRandomQuestSystem->HandleQuestAbandon(player, questId);
    }
};

void AddSC_RandomQuest_PlayerScript()
{
    new RandomQuest_PlayerScript();
}
