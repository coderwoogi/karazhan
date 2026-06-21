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

// 혈족(실버문)·드레나이(엑소다르) 시작 대륙 지역들. 맵 530(아웃랜드)에 얹혀 있지만
// 아웃랜드 비행 플래그가 없어 기본적으로 비행 불가 → 별도로 포함시킨다.
bool IsBloodElfDraeneiZoneId(uint32 id)
{
    return id == AREA_EVERSONG_WOODS || id == AREA_GHOSTLANDS
        || id == AREA_SILVERMOON_CITY || id == AREA_AZUREMYST_ISLE
        || id == AREA_BLOODMYST_ISLE || id == AREA_THE_EXODAR;
}

bool IsBloodElfDraeneiArea(AreaTableEntry const* area)
{
    return area->mapid == MAP_OUTLAND
        && (IsBloodElfDraeneiZoneId(area->ID)
            || IsBloodElfDraeneiZoneId(area->zone));
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
            if (!IsAzerothMap(areaEntry->mapid)
                && !IsBloodElfDraeneiArea(areaEntry))
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
