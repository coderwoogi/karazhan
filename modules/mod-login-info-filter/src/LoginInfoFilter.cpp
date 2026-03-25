#include "ByteBuffer.h"
#include "Config.h"
#include "Opcodes.h"
#include "ScriptMgr.h"
#include "ServerScript.h"
#include "SharedDefines.h"
#include "WorldPacket.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace
{
    class LoginInfoFilterConfig
    {
    public:
        static LoginInfoFilterConfig& Instance()
        {
            static LoginInfoFilterConfig instance;
            return instance;
        }

        void Load()
        {
            _enabled = sConfigMgr->GetOption<bool>(
                "LoginInfoFilter.Enable", true);
        }

        bool IsEnabled() const
        {
            return _enabled;
        }

    private:
        bool _enabled = true;
    };

    std::string ToLowerAscii(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char ch) -> char { return std::tolower(ch); });
        return value;
    }

    bool ShouldBlockMessage(std::string const& message)
    {
        if (message.empty())
            return false;

        static std::array<std::string, 4> const patterns =
        {
            "this server runs on azerothcore",
            "this server runs on azerothccore",
            "azerothcore rev.",
            "azerothcore revision"
        };

        std::string lowered = ToLowerAscii(message);
        for (std::string const& pattern : patterns)
            if (lowered.find(pattern) != std::string::npos)
                return true;

        return false;
    }

    bool ShouldBlockSystemChatPacket(WorldPacket& packet)
    {
        try
        {
            uint8 chatType = 0;
            int32 language = 0;
            ObjectGuid senderGuid;
            uint32 flags = 0;
            ObjectGuid receiverGuid;
            std::string message;
            uint8 chatTag = 0;

            packet >> chatType;
            packet >> language;
            packet >> senderGuid;
            packet >> flags;

            if (chatType != CHAT_MSG_SYSTEM)
                return false;

            packet >> receiverGuid;
            packet >> message;
            packet >> chatTag;

            return ShouldBlockMessage(message);
        }
        catch (ByteBufferException const&)
        {
            return false;
        }
    }

    bool ShouldBlockNotificationPacket(WorldPacket& packet)
    {
        try
        {
            std::string message;
            packet >> message;
            return ShouldBlockMessage(message);
        }
        catch (ByteBufferException const&)
        {
            return false;
        }
    }
}

class LoginInfoFilterWorldScript : public WorldScript
{
public:
    LoginInfoFilterWorldScript() :
        WorldScript("LoginInfoFilterWorldScript")
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        LoginInfoFilterConfig::Instance().Load();
    }
};

class LoginInfoFilterServerScript : public ServerScript
{
public:
    LoginInfoFilterServerScript() :
        ServerScript("LoginInfoFilterServerScript",
            { SERVERHOOK_CAN_PACKET_SEND })
    {
    }

    bool CanPacketSend(WorldSession* /*session*/, WorldPacket& packet) override
    {
        if (!LoginInfoFilterConfig::Instance().IsEnabled())
            return true;

        switch (packet.GetOpcode())
        {
            case SMSG_MESSAGECHAT:
            case SMSG_GM_MESSAGECHAT:
                return !ShouldBlockSystemChatPacket(packet);
            case SMSG_NOTIFICATION:
                return !ShouldBlockNotificationPacket(packet);
            default:
                return true;
        }
    }
};

void AddLoginInfoFilterScripts()
{
    new LoginInfoFilterWorldScript();
    new LoginInfoFilterServerScript();
}
