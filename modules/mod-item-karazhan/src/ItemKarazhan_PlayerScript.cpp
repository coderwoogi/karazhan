/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "ItemKarazhan.h"
#include "Player.h"
#include "ScriptMgr.h"

class ItemKarazhan_PlayerScript : public PlayerScript
{
public:
    ItemKarazhan_PlayerScript()
        : PlayerScript("ItemKarazhan_PlayerScript")
    {
    }

    void OnPlayerLogin(Player* player) override
    {
        sItemKarazhanMgr->RepairPlayerEnhancedItems(player);
    }
};

void Add_SC_ItemKarazhan_PlayerScript()
{
    new ItemKarazhan_PlayerScript();
}
