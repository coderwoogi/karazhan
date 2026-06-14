// modules/mod-blackmarket/src/BlackMarketCommands.cpp

#include "ScriptMgr.h"
#include "Chat.h"
#include "Player.h"
#include "DatabaseEnv.h"
#include "MapMgr.h"
#include "Map.h"
#include "StringFormat.h"
#include "BlackMarketSystem.h"
#include "ObjectAccessor.h"

using namespace Acore::ChatCommands;

class blackmarket_commandscript : public CommandScript
{
public:
    blackmarket_commandscript() : CommandScript("blackmarket_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable blackmarketTeleCommandTable =
        {
            { "add", HandleBlackMarketTeleAddCommand, SEC_GAMEMASTER, Console::No }
        };

        static ChatCommandTable blackmarketCommandTable =
        {
            { "tele", blackmarketTeleCommandTable },
            { "enable", HandleBlackMarketEnableCommand, SEC_ADMINISTRATOR, Console::No },
            { "disable", HandleBlackMarketDisableCommand, SEC_ADMINISTRATOR, Console::No },
            { "toggle", HandleBlackMarketToggleCommand, SEC_ADMINISTRATOR, Console::No },
            { "status", HandleBlackMarketStatusCommand, SEC_GAMEMASTER, Console::No },
            { "go", HandleBlackMarketGoCommand, SEC_GAMEMASTER, Console::No }
        };

        static ChatCommandTable commandTable =
        {
            { "blackmarket", blackmarketCommandTable }
        };

        return commandTable;
    }

    static bool HandleBlackMarketTeleAddCommand(ChatHandler* handler, std::string const& comment)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            handler->SendErrorMessage("Player not found");
            return false;
        }

        if (comment.empty())
        {
            handler->SendErrorMessage("Usage: .blackmarket tele add <comment>");
            return false;
        }

        if (comment.length() > 255)
        {
            handler->SendErrorMessage("Comment too long (max 255 characters)");
            return false;
        }

        uint16 mapId = player->GetMapId();
        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float o = player->GetOrientation();
        uint32 zoneId = player->GetZoneId();

        if (!sMapMgr->IsValidMapCoord(mapId, x, y, z))
        {
            handler->SendErrorMessage("Invalid map coordinates");
            return false;
        }

        WorldDatabase.Execute(
            "INSERT INTO blackmarket_spawn_points "
            "(map, zone_id, position_x, position_y, position_z, orientation, comment) "
            "VALUES ({}, {}, {}, {}, {}, {}, '{}')",
            mapId, zoneId, x, y, z, o, comment
        );

        std::string message = Acore::StringFormat(
            "|cFF00FF00BlackMarket spawn point added|r\n"
            "Map: {}, Zone: {}, Position: ({:.2f}, {:.2f}, {:.2f})\n"
            "Orientation: {:.2f}, Comment: {}",
            mapId, zoneId, x, y, z, o, comment
        );

        handler->SendSysMessage(message.c_str());

        LOG_INFO("module", "BlackMarket: GM {} added spawn point at Map:{} Zone:{} Pos:({:.2f},{:.2f},{:.2f}) Comment:'{}'",
            player->GetName(), mapId, zoneId, x, y, z, comment);

        return true;
    }

    static bool HandleBlackMarketEnableCommand(ChatHandler* handler)
    {
        if (!sBlackMarket)
        {
            handler->SendErrorMessage("BlackMarket system not found");
            return false;
        }

        if (sBlackMarket->IsEnabled())
        {
            handler->SendSysMessage("|cFFFFFF00BlackMarket system is already enabled|r");
            return true;
        }

        sBlackMarket->Enable();
        handler->SendSysMessage("|cFF00FF00BlackMarket system enabled|r");
        
        Player* player = handler->GetSession()->GetPlayer();
        if (player)
        {
            LOG_INFO("module", "BlackMarket: GM {} enabled the system", player->GetName());
        }

        return true;
    }

    static bool HandleBlackMarketDisableCommand(ChatHandler* handler)
    {
        if (!sBlackMarket)
        {
            handler->SendErrorMessage("BlackMarket system not found");
            return false;
        }

        if (!sBlackMarket->IsEnabled())
        {
            handler->SendSysMessage("|cFFFFFF00BlackMarket system is already disabled|r");
            return true;
        }

        sBlackMarket->Disable();
        handler->SendSysMessage("|cFFFF0000BlackMarket system disabled|r");
        
        Player* player = handler->GetSession()->GetPlayer();
        if (player)
        {
            LOG_INFO("module", "BlackMarket: GM {} disabled the system", player->GetName());
        }

        return true;
    }

    static bool HandleBlackMarketToggleCommand(ChatHandler* handler)
    {
        if (!sBlackMarket)
        {
            handler->SendErrorMessage("BlackMarket system not found");
            return false;
        }

        bool wasEnabled = sBlackMarket->IsEnabled();
        
        if (wasEnabled)
        {
            sBlackMarket->Disable();
            handler->SendSysMessage("|cFFFF0000BlackMarket system disabled|r");
        }
        else
        {
            sBlackMarket->Enable();
            handler->SendSysMessage("|cFF00FF00BlackMarket system enabled|r");
        }
        
        Player* player = handler->GetSession()->GetPlayer();
        if (player)
        {
            LOG_INFO("module", "BlackMarket: GM {} toggled the system to {}", 
                player->GetName(), wasEnabled ? "disabled" : "enabled");
        }

        return true;
    }

    static bool HandleBlackMarketStatusCommand(ChatHandler* handler)
    {
        if (!sBlackMarket)
        {
            handler->SendErrorMessage("BlackMarket system not found");
            return false;
        }

        bool isEnabled = sBlackMarket->IsEnabled();
        bool isActive = sBlackMarket->IsActive();

        std::string statusMessage = Acore::StringFormat(
            "|cFF00FFFF=== BlackMarket System Status ===|r\n"
            "System: {}\n"
            "NPC Spawned: {}\n"
            "Session ID: {}",
            isEnabled ? "|cFF00FF00Enabled|r" : "|cFFFF0000Disabled|r",
            isActive ? "|cFF00FF00Yes|r" : "|cFFFFFF00No|r",
            sBlackMarket->GetCurrentSessionId()
        );

        handler->SendSysMessage(statusMessage.c_str());

        return true;
    }
    static bool HandleBlackMarketGoCommand(ChatHandler* handler)
    {
        if (!sBlackMarket)
        {
            handler->SendErrorMessage("BlackMarket system not found");
            return false;
        }

        if (!sBlackMarket->IsActive())
        {
            handler->SendSysMessage("|cFFFFFF00현재 암상인이 열려 있지 않습니다.|r");
            return true;
        }

        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            handler->SendErrorMessage("플레이어를 찾을 수 없습니다.");
            return false;
        }

        uint16 mapId = 0;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float o = 0.0f;
        if (!sBlackMarket->GetCurrentSpawnLocation(mapId, x, y, z, o))
        {
            handler->SendErrorMessage("현재 암상인 위치 정보를 찾을 수 없습니다.");
            return false;
        }

        if (!sMapMgr->IsValidMapCoord(mapId, x, y, z))
        {
            handler->SendErrorMessage("암상인 좌표가 올바르지 않습니다.");
            return false;
        }

        player->TeleportTo(mapId, x, y, z, o);
        return true;
    }
};

void AddBlackMarketCommandScripts()
{
    new blackmarket_commandscript();
}
