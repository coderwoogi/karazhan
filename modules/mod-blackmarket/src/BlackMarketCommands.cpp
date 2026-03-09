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

        ObjectGuid merchantGuid = sBlackMarket->GetCurrentMerchantGUID();
        if (merchantGuid.IsEmpty())
        {
            handler->SendErrorMessage("현재 암상인 GUID를 찾을 수 없습니다.");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        Creature* merchant = ObjectAccessor::GetCreature(*player, merchantGuid);

        if (merchant)
        {
            // ?곸씤??濡쒕뱶???곹깭?쇰㈃ ?대떦 ?꾩튂濡?諛붾줈 ?대룞
            player->TeleportTo(merchant->GetMapId(), merchant->GetPositionX(), merchant->GetPositionY(), merchant->GetPositionZ(), merchant->GetOrientation());
        }
        else
        {
            // ?곸씤??硫由??덉뼱??濡쒕뱶?섏? ?딆? 寃쎌슦, ??λ맂 ?ㅽ룿 ?ъ씤??ID濡??꾩튂 李얘린
            // (BlackMarketSystem??GetPosition 愿???⑥닔媛 ?놁쑝誘濡?吏곸젒 DB?먯꽌 議고쉶?섍굅??援ъ“泥??묎렐 ?꾩슂)
            // ?꾩옱 援ъ“??吏곸젒 ?붾젅?ы듃???대젮?곕?濡? Map/Zone ?뺣낫?쇰룄 異쒕젰
             handler->SendSysMessage("|cFFFFFF00암상인이 현재 월드에 로드되어 있지 않습니다.|r");
             // 媛쒖꽑: BlackMarketSystem???꾩옱 ?꾩튂 ?뺣낫瑜?諛섑솚?섎뒗 ?⑥닔瑜?異붽??섍굅?? 
             // SpawnPoint ?뺣낫瑜??쒗쉶?댁꽌 李얜뒗 濡쒖쭅??異붽??????덉쓬.
             // ?ш린?쒕뒗 媛꾨떒???곸씤??濡쒕뱶??寃쎌슦?먮쭔 ?대룞 吏??
             // ?먮뒗 BlackMarketSystem::GetCurrentSpawnPoint() 媛숈? ?⑥닔媛 ?덈떎硫??ъ슜 媛??
        }
        
        // *媛쒖꽑??援ы쁽*: 紐⑤뱺 寃쎌슦???대룞 媛?ν븯?꾨줉 寃??濡쒖쭅 異붽?
        // private 硫ㅻ쾭 ?묎렐???대젮?곕?濡? BlackMarketSystem???몄쓽 ?⑥닔 異붽?媛 ?댁긽?곸씠??
        // ?꾩옱??System ?섏젙 ?놁씠 媛?ν븳 踰붿쐞 ?댁뿉??援ы쁽.
        // ?섏?留??ъ슜??寃쏀뿕???꾪빐 System??GetCurrentSpawnPoint()瑜?異붽??섎뒗 寃껋씠 醫뗭쓬.
        // ?쇰떒? ?곸씤 濡쒕뱶 ?щ?? 愿怨꾩뾾???대룞?????덈룄濡?System ?섏젙???ы븿?섏뿬 吏꾪뻾.
        
        return true;
    }
};

void AddBlackMarketCommandScripts()
{
    new blackmarket_commandscript();
}
