#include "Chat.h"
#include "CommandScript.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
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
            { "whisper", HandleKarazhanWhisperCommand, SEC_ADMINISTRATOR, Console::Yes }
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
};

void AddKarazhanCommandScripts()
{
    new KarazhanCommandScript();
}
