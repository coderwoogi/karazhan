#include "RandomQuestSystem.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "ScriptedGossip.h"
#include "Log.h"
#include "Chat.h"
#include "StringFormat.h"
#include "World.h"

enum RandomQuestGossipActions
{
    GOSSIP_ACTION_REQUEST_RANDOM_QUEST = GOSSIP_ACTION_INFO_DEF + 1,
    GOSSIP_ACTION_CHECK_DAILY_STATUS = GOSSIP_ACTION_INFO_DEF + 2
};

class npc_random_quest_giver : public CreatureScript
{
public:
    npc_random_quest_giver() : CreatureScript("npc_random_quest_giver") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->GetEntry() == NPC_RANDOM_QUEST_GIVER)
        {
            uint32 const maxPlayerLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
            if (player->GetLevel() < maxPlayerLevel)
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                    "랜덤 퀘스트는 만렙 달성 후 이용할 수 있습니다.",
                    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
                AddGossipItemFor(player, GOSSIP_ICON_DOT,
                    Acore::StringFormat("현재 레벨: {} / 필요 레벨: {}", player->GetLevel(), maxPlayerLevel),
                    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }
            // 플레이어의 일일 상태 조회
            uint8 questsGivenToday = sRandomQuestSystem->GetPlayerDailyQuestCount(player);
            uint8 questsCompletedToday = sRandomQuestSystem->GetPlayerDailyQuestCompletedCount(player);
            uint8 maxQuests = 3;

            // 진행 상황을 포함한 가십 메뉴
            std::string gossipText = Acore::StringFormat(
                "오늘의 랜덤 퀘스트를 받고 싶습니다.\n"
                "(진행: {}/{}, 완료: {}개)\n"
                "\n"
                "- 규칙: 하루 최대 {}개 수행 가능\n"
                "- 초기화: 매일 오전 06:00",
                questsGivenToday, maxQuests, questsCompletedToday, maxQuests
            );

            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                gossipText,
                GOSSIP_SENDER_MAIN, GOSSIP_ACTION_REQUEST_RANDOM_QUEST);

            // 추가: 일일 상태 확인 옵션
            AddGossipItemFor(player, GOSSIP_ICON_DOT,
                "오늘 받은 퀘스트 현황을 확인합니다.",
                GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CHECK_DAILY_STATUS);
        }

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);

        if (action == GOSSIP_ACTION_REQUEST_RANDOM_QUEST)
        {
            uint32 const maxPlayerLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
            if (player->GetLevel() < maxPlayerLevel)
            {
                CloseGossipMenuFor(player);
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "랜덤 퀘스트는 만렙({}) 달성 후 이용할 수 있습니다.", maxPlayerLevel);
                return true;
            }
            CloseGossipMenuFor(player);
            sRandomQuestSystem->HandleQuestGiverInteraction(player);
            return true;
        }
        else if (action == GOSSIP_ACTION_CHECK_DAILY_STATUS)
        {
            // CloseGossipMenuFor(player); // 메뉴를 닫지 않고 새로운 메뉴를 보여줌
            sRandomQuestSystem->ShowDailyStatusGossip(player, creature);
            return true;
        }
        else if (action == GOSSIP_ACTION_INFO_DEF + 99) // 돌아가기 버튼
        {
            OnGossipHello(player, creature);
            return true;
        }

        return false;
    }
};

class npc_random_quest_completer : public CreatureScript
{
public:
    npc_random_quest_completer() : CreatureScript("npc_random_quest_completer") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnQuestReward(Player* player, Creature* /*creature*/, Quest const* quest, uint32 /*opt*/) override
    {
        sRandomQuestSystem->HandleQuestReward(player, quest);
        return true;
    }
};

void AddSC_npc_random_quest_giver()
{
    new npc_random_quest_giver();
    new npc_random_quest_completer();
}
