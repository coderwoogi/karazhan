#include "Chat.h"
#include "CommandScript.h"
#include "DBCStores.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SpellMgr.h"
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
            { "whisper", HandleKarazhanWhisperCommand, SEC_ADMINISTRATOR, Console::Yes },
            { "reward",  HandleKarazhanRewardCommand,  SEC_ADMINISTRATOR, Console::Yes }
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

    // .karazhan reward "캐릭터명" <spellId> <titleId>
    // 선술집 프리미엄 퀘스트 아이템 구매 시 호출. 대상(온라인) 캐릭터에게
    // 보상 스펠을 영구 학습시키고/또는 칭호를 영구 부여한다. 값이 0이면 해당 항목은 생략.
    static bool HandleKarazhanRewardCommand(
        ChatHandler* handler, QuotedString characterName, uint32 spellId, uint32 titleId)
    {
        if (characterName.empty() || (!spellId && !titleId))
        {
            handler->PSendSysMessage(
                "Usage: .karazhan reward \"캐릭터명\" <spellId> <titleId>");
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

        // 보상 스펠 지급: 퀘스트 완료 시와 동일하게 캐스트(triggered)하여 효과를 발동시킨다.
        // (RewardDisplaySpell 은 learnSpell 이 아니라 CastSpell 로 처리되는 스펠 —
        //  SPELL_EFFECT_LEARN_SPELL 이면 실제 능력을 영구 학습, 그 외엔 해당 보상 효과 적용.
        //  참고: Player::RewardQuest 의 GetRewSpellCast 처리와 동일한 메커니즘)
        if (spellId)
        {
            if (!sSpellMgr->GetSpellInfo(spellId))
            {
                handler->PSendSysMessage("[Karazhan] Spell not found: {}", spellId);
                return false;
            }
            player->CastSpell(player, spellId, true);
        }

        // 칭호 영구 부여
        if (titleId)
        {
            CharTitlesEntry const* title = sCharTitlesStore.LookupEntry(titleId);
            if (!title)
            {
                handler->PSendSysMessage("[Karazhan] Title not found: {}", titleId);
                return false;
            }
            player->SetTitle(title);
        }

        SendKarazhanMonsterWhisper(player, "프리미엄 퀘스트 보상이 지급되었습니다.");
        handler->PSendSysMessage(
            "[Karazhan] Reward given to {} (spell={}, title={}).",
            normalizedName, spellId, titleId);
        return true;
    }
};

void AddKarazhanCommandScripts()
{
    new KarazhanCommandScript();
}
