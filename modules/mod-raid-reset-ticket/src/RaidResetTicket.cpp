#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "InstanceSaveMgr.h"
#include "Item.h"
#include "ItemScript.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "StringFormat.h"

#include <unordered_map>
#include <vector>

namespace
{
    struct RaidResetTicketData
    {
        uint32 mapId;
        std::string label;
    };

    class RaidResetTicketStore
    {
    public:
        static RaidResetTicketStore& Instance()
        {
            static RaidResetTicketStore instance;
            return instance;
        }

        void Load()
        {
            _enabled = sConfigMgr->GetOption<bool>("RaidResetTicket.Enable", true);
            _tickets.clear();

            if (!_enabled)
                return;

            QueryResult result = WorldDatabase.Query(
                "SELECT item_entry, map_id, comment "
                "FROM raid_reset_ticket "
                "WHERE enabled = 1");

            if (!result)
                return;

            do
            {
                Field* fields = result->Fetch();

                uint32 itemEntry = fields[0].Get<uint32>();

                RaidResetTicketData data;
                data.mapId = fields[1].Get<uint32>();
                data.label = fields[2].Get<std::string>();

                _tickets[itemEntry] = data;
            } while (result->NextRow());
        }

        bool IsEnabled() const
        {
            return _enabled;
        }

        RaidResetTicketData const* Get(uint32 itemEntry) const
        {
            auto itr = _tickets.find(itemEntry);
            if (itr == _tickets.end())
                return nullptr;

            return &itr->second;
        }

    private:
        bool _enabled = true;
        std::unordered_map<uint32, RaidResetTicketData> _tickets;
    };

    bool ResetRaidForPlayer(Player* player, uint32 mapId, uint32& resetCount)
    {
        resetCount = 0;

        for (uint8 difficultyIndex = 0; difficultyIndex < MAX_DIFFICULTY; ++difficultyIndex)
        {
            BoundInstancesMap const& boundInstances =
                sInstanceSaveMgr->PlayerGetBoundInstances(
                    player->GetGUID(), Difficulty(difficultyIndex));

            auto itr = boundInstances.find(mapId);
            if (itr == boundInstances.end())
                continue;

            sInstanceSaveMgr->PlayerUnbindInstance(
                player->GetGUID(), mapId, Difficulty(difficultyIndex), true,
                player);
            ++resetCount;
        }

        return resetCount > 0;
    }
}

class RaidResetTicketWorldScript : public WorldScript
{
public:
    RaidResetTicketWorldScript() : WorldScript("RaidResetTicketWorldScript")
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        RaidResetTicketStore::Instance().Load();
    }
};

class item_raid_reset_ticket : public ItemScript
{
public:
    item_raid_reset_ticket() : ItemScript("item_raid_reset_ticket")
    {
    }

    bool OnUse(Player* player, Item* item,
        SpellCastTargets const& /*targets*/) override
    {
        RaidResetTicketStore const& store = RaidResetTicketStore::Instance();
        if (!store.IsEnabled())
            return true;

        RaidResetTicketData const* ticketData = store.Get(item->GetEntry());
        if (!ticketData)
            return true;

        ChatHandler handler(player->GetSession());

        if (player->IsInCombat())
        {
            handler.SendSysMessage(
                "You cannot use a raid reset ticket while in combat.");
            return true;
        }

        if (player->GetGroup())
        {
            handler.SendSysMessage(
                "You cannot use a raid reset ticket while in a party or raid.");
            return true;
        }

        if (player->GetMap() && player->GetMap()->IsDungeon())
        {
            handler.SendSysMessage(
                "You cannot use a raid reset ticket while inside an instance.");
            return true;
        }

        uint32 resetCount = 0;
        if (!ResetRaidForPlayer(player, ticketData->mapId, resetCount))
        {
            std::string message = Acore::StringFormat(
                "No active raid lockout was found for {}.",
                ticketData->label);
            handler.SendSysMessage(message.c_str());
            return true;
        }

        player->DestroyItemCount(item->GetEntry(), 1, true);

        std::string successMessage = Acore::StringFormat(
            "Raid lockouts for {} have been reset. Removed {} bind(s).",
            ticketData->label, resetCount);
        handler.SendSysMessage(successMessage.c_str());

        return true;
    }
};

void AddRaidResetTicketScripts()
{
    new RaidResetTicketWorldScript();
    new item_raid_reset_ticket();
}
