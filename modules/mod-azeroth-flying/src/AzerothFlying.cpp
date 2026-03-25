#include "DBCEnums.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "Log.h"
#include "AreaDefines.h"
#include "WorldScript.h"

namespace
{
bool IsAzerothMap(uint32 mapId)
{
    return mapId == MAP_EASTERN_KINGDOMS || mapId == MAP_KALIMDOR;
}
}

class AzerothFlyingWorldScript : public WorldScript
{
public:
    AzerothFlyingWorldScript() : WorldScript("AzerothFlyingWorldScript") { }

    void OnStartup() override
    {
        uint32 updatedAreas = 0;

        for (AreaTableEntry const* areaEntryConst : sAreaTableStore)
        {
            if (!areaEntryConst)
                continue;

            AreaTableEntry* areaEntry = const_cast<AreaTableEntry*>(areaEntryConst);
            if (!IsAzerothMap(areaEntry->mapid))
                continue;

            if ((areaEntry->flags & AREA_FLAG_OUTLAND) != 0)
                continue;

            areaEntry->flags |= AREA_FLAG_OUTLAND;
            ++updatedAreas;
        }
        (void)updatedAreas;
    }
};

void AddSC_AzerothFlying()
{
    new AzerothFlyingWorldScript();
}
