#include "Chat.h"
#include "CommandScript.h"
#include "DBCStores.h"
#include "Item.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "ReputationMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

using namespace Acore::ChatCommands;

namespace
{
    constexpr char const* KARAZHAN_WHISPER_SENDER = "선술집";

    void SendKarazhanMonsterWhisper(Player* target, std::string const& message)
    {
        if (!target || !target->GetSession())
            return;

        WorldPacket data;
        ChatHandler::BuildChatPacket(
            data,
            CHAT_MSG_MONSTER_WHISPER,
            LANG_UNIVERSAL,
            ObjectGuid::Empty,
            target->GetGUID(),
            message,
            0,
            KARAZHAN_WHISPER_SENDER,
            target->GetName());

        target->GetSession()->SendPacket(&data);
    }
}

class KarazhanCommandScript : public CommandScript
{
public:
    KarazhanCommandScript() : CommandScript("KarazhanCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable karazhanCommandTable =
        {
            { "whisper",    HandleKarazhanWhisperCommand,    SEC_ADMINISTRATOR, Console::Yes },
            { "questclear", HandleKarazhanQuestClearCommand, SEC_ADMINISTRATOR, Console::Yes }
        };

        static ChatCommandTable commandTable =
        {
            { "karazhan", karazhanCommandTable }
        };

        return commandTable;
    }

    static bool HandleKarazhanWhisperCommand(
        ChatHandler* handler, QuotedString characterName, QuotedString message)
    {
        if (characterName.empty() || message.empty())
        {
            handler->PSendSysMessage(
                "Usage: .karazhan whisper \"캐릭터명\" \"메시지\"");
            return false;
        }

        std::string normalizedName = characterName;
        if (!normalizePlayerName(normalizedName))
        {
            handler->PSendSysMessage(
                "[Karazhan] Invalid character name: {}", characterName);
            return false;
        }

        Player* target = ObjectAccessor::FindPlayerByName(normalizedName, false);
        if (!target || !target->GetSession() || !target->IsInWorld())
        {
            handler->PSendSysMessage(
                "[Karazhan] Character is offline or not ready: {}",
                normalizedName);
            return false;
        }

        SendKarazhanMonsterWhisper(target, message);
        handler->PSendSysMessage(
            "[Karazhan] Whisper sent to {}.", target->GetName());
        return true;
    }

    // .karazhan questclear "캐릭터명" 퀘스트ID
    // 선술집 프리미엄 퀘스트 아이템 구매 시 호출. 대상(온라인) 캐릭터가 해당 퀘스트를
    // 수락하지 않았으면 강제로 추가한 뒤, 목표를 모두 충족시켜 완료 가능 상태로 만든다.
    // (코어 .quest complete 로직을 미러링)
    static bool HandleKarazhanQuestClearCommand(
        ChatHandler* handler, QuotedString characterName, uint32 questId)
    {
        if (characterName.empty() || !questId)
        {
            handler->PSendSysMessage(
                "Usage: .karazhan questclear \"캐릭터명\" 퀘스트ID");
            return false;
        }

        std::string normalizedName = characterName;
        if (!normalizePlayerName(normalizedName))
        {
            handler->PSendSysMessage(
                "[Karazhan] Invalid character name: {}", characterName);
            return false;
        }

        Player* player = ObjectAccessor::FindPlayerByName(normalizedName, false);
        if (!player || !player->GetSession() || !player->IsInWorld())
        {
            handler->PSendSysMessage(
                "[Karazhan] Character is offline or not ready: {}",
                normalizedName);
            return false;
        }

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
        {
            handler->PSendSysMessage("[Karazhan] Quest not found: {}", questId);
            return false;
        }

        // 수락하지 않았으면 강제로 퀘스트 추가
        if (player->GetQuestStatus(questId) == QUEST_STATUS_NONE)
        {
            if (player->CanAddQuest(quest, false))
                player->AddQuestAndCheckCompletion(quest, nullptr);
        }

        if (player->GetQuestStatus(questId) == QUEST_STATUS_NONE)
        {
            handler->PSendSysMessage(
                "[Karazhan] Cannot add quest {} to {} (log full or requirements).",
                questId, normalizedName);
            return false;
        }

        // 목표 충족 — 코어 .quest complete 미러링
        for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
        {
            uint32 id    = quest->RequiredItemId[x];
            uint32 count = quest->RequiredItemCount[x];
            if (!id || !count)
                continue;

            uint32 curItemCount = player->GetItemCount(id, true);
            if (curItemCount >= count)
                continue;

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, id, true);
                player->SendNewItem(item, count - curItemCount, true, false);
            }
        }

        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            int32  creature      = quest->RequiredNpcOrGo[i];
            uint32 creatureCount  = quest->RequiredNpcOrGoCount[i];

            if (creature > 0)
            {
                if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creature))
                    for (uint16 z = 0; z < creatureCount; ++z)
                        player->KilledMonster(creatureInfo, ObjectGuid::Empty);
            }
            else if (creature < 0)
            {
                for (uint16 z = 0; z < creatureCount; ++z)
                    player->KillCreditGO(creature);
            }
        }

        if (quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_PLAYER_KILL))
            if (uint32 reqPlayers = quest->GetPlayersSlain())
                player->KilledPlayerCreditForQuest(reqPlayers, quest);

        if (uint32 repFaction = quest->GetRepObjectiveFaction())
        {
            uint32 repValue = quest->GetRepObjectiveValue();
            uint32 curRep   = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, static_cast<float>(repValue));
        }

        if (uint32 repFaction = quest->GetRepObjectiveFaction2())
        {
            uint32 repValue2 = quest->GetRepObjectiveValue2();
            uint32 curRep    = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue2)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, static_cast<float>(repValue2));
        }

        int32 reqOrRewMoney = quest->GetRewOrReqMoney(player->GetLevel());
        if (reqOrRewMoney < 0)
            player->ModifyMoney(-reqOrRewMoney);

        player->CompleteQuest(questId);

        SendKarazhanMonsterWhisper(
            player, "프리미엄 퀘스트가 완료되었습니다. 완료 지점에서 보상을 수령하세요.");
        handler->PSendSysMessage(
            "[Karazhan] Quest {} completed for {}.", questId, normalizedName);
        return true;
    }
};

void AddKarazhanCommandScripts()
{
    new KarazhanCommandScript();
}
