// modules/mod-blackmarket/src/BlackMarketLoader.cpp

/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Released under GNU AGPL v3 license
 */

#include "ScriptMgr.h"
#include "Config.h"
#include "BlackMarketSystem.h"

void AddBlackMarketNPCScripts();
void AddBlackMarketCommandScripts(); // ✅ 추가

class BlackMarketWorldScript : public WorldScript
{
public:
    BlackMarketWorldScript() : WorldScript("BlackMarketWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sBlackMarket->Initialize();
    }

    void OnUpdate(uint32 diff) override
    {
        sBlackMarket->Update(diff);
    }

    void OnShutdown() override
    {
        sBlackMarket->Shutdown();
    }
};

void Addmod_blackmarketScripts()
{
    new BlackMarketWorldScript();
    AddBlackMarketNPCScripts();
    AddBlackMarketCommandScripts(); // ✅追加
}
