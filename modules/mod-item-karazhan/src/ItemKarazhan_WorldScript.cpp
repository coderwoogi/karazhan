/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "ScriptMgr.h"
#include "ItemKarazhan.h"

class ItemKarazhan_WorldScript : public WorldScript
{
public:
    ItemKarazhan_WorldScript() : WorldScript("ItemKarazhan_WorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sItemKarazhanMgr->Initialize();
    }

    void OnUpdate(uint32 diff) override
    {
        sItemKarazhanMgr->Update(diff);
    }
};

void Add_SC_ItemKarazhan_WorldScript()
{
    new ItemKarazhan_WorldScript();
}
