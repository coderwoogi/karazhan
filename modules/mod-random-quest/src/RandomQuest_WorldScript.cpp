#include "RandomQuestSystem.h"
#include "ScriptMgr.h"
#include "Log.h"

class RandomQuest_WorldScript : public WorldScript
{
public:
    RandomQuest_WorldScript() : WorldScript("RandomQuest_WorldScript") {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sRandomQuestSystem->Initialize();
    }

    void OnShutdown() override
    {
        sRandomQuestSystem->Shutdown();
    }
};

void AddSC_RandomQuestWorldScript()
{
    new RandomQuest_WorldScript();
}
