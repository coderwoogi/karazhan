/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "ItemKarazhan.h"
#include "Log.h"

void Add_SC_npc_item_karazhan();
void Add_SC_item_karazhan_mgr();
void Add_SC_ItemKarazhan_WorldScript();  // ★ 추가

void Addmod_item_karazhanScripts()
{
    try
    {
        // ★ WorldScript 먼저 등록!
        Add_SC_ItemKarazhan_WorldScript();
        
        Add_SC_npc_item_karazhan();
        Add_SC_item_karazhan_mgr();
        
        LOG_INFO("module", "Karazhan: All scripts loaded successfully");
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("module", "Karazhan: Failed to load scripts - {}", e.what());
    }
}
