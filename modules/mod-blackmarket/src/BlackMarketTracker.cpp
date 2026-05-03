// modules/mod-blackmarket/src/BlackMarketTracker.cpp

#include "BlackMarketSystem.h"
#include "Chat.h"
#include "Item.h"
#include "ItemScript.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"

namespace
{
    constexpr uint32 BLACKMARKET_TRACKER_ITEM = 600025;
}

class item_blackmarket_tracker : public ItemScript
{
public:
    item_blackmarket_tracker() : ItemScript("item_blackmarket_tracker")
    {
    }

    bool OnUse(Player* player, Item* item,
        SpellCastTargets const& /*targets*/) override
    {
        if (!player || !item || item->GetEntry() != BLACKMARKET_TRACKER_ITEM)
            return true;

        if (!sBlackMarket->SendCurrentLocationMail(player))
            return true;

        return false;
    }
};

void AddBlackMarketTrackerScripts()
{
    new item_blackmarket_tracker();
}
