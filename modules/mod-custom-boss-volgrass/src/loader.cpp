/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "Chat.h"
#include "StringFormat.h"
#include "Timer.h"

namespace VolgrassConfig
{
    uint32 ADD_ENTRY_ID = 190019;
    uint8 PHASE2_ADD_COUNT = 3;
    uint8 PHASE3_ADD_COUNT = 5;
    uint32 SUMMON_INTERVAL = 30000;
    uint32 BERSERK_TIMER = 1080000;
    uint32 ENCOUNTER_TIMEOUT = 1200000;
    uint32 PRE_ANNOUNCE_MINUTES = 5;
}

class VolgrassWorldScript : public WorldScript
{
public:
    VolgrassWorldScript() : WorldScript("VolgrassWorldScript"), _updateTimer(0), _lastAnnounceKey(-1) {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        VolgrassConfig::ADD_ENTRY_ID = sConfigMgr->GetOption<uint32>("Volgrass.Add.EntryID", 190019);
        if (VolgrassConfig::ADD_ENTRY_ID == 0)
        {
            VolgrassConfig::ADD_ENTRY_ID = 190019;
        }
        VolgrassConfig::PHASE2_ADD_COUNT = sConfigMgr->GetOption<uint8>("Volgrass.Phase2.AddCount", 3);
        VolgrassConfig::PHASE3_ADD_COUNT = sConfigMgr->GetOption<uint8>("Volgrass.Phase3.AddCount", 5);
        VolgrassConfig::SUMMON_INTERVAL = sConfigMgr->GetOption<uint32>("Volgrass.Summon.Interval", 30000);
        VolgrassConfig::BERSERK_TIMER = sConfigMgr->GetOption<uint32>("Volgrass.Berserk.Timer", 1080000);
        VolgrassConfig::ENCOUNTER_TIMEOUT = sConfigMgr->GetOption<uint32>("Volgrass.Encounter.Timeout", 1200000);
        VolgrassConfig::PRE_ANNOUNCE_MINUTES = sConfigMgr->GetOption<uint32>("Volgrass.Invasion.PreAnnounceMinutes", 5);

    }

    void OnUpdate(uint32 diff) override
    {
        _updateTimer += diff;
        if (_updateTimer < 10000)
            return;

        _updateTimer = 0;

        tm localTm = Acore::Time::TimeBreakdown();
        int32 announceKey = localTm.tm_yday * 24 * 60 + localTm.tm_hour * 60 + localTm.tm_min;
        if (announceKey == _lastAnnounceKey)
            return;

        _lastAnnounceKey = announceKey;

        // Pre-announcement for the next even-hour spawn (e.g. 01:55, 03:55...)
        uint32 preMinute = VolgrassConfig::PRE_ANNOUNCE_MINUTES;
        if (preMinute < 60 && localTm.tm_hour % 2 == 1 && localTm.tm_min == static_cast<int32>(60 - preMinute))
            ChatHandler(nullptr).SendWorldText(
                Acore::StringFormat(
                    "[침공] {}분 후, 섬 중심에 침공 보스 예언자 벨렌이 출현합니다. 방어 병력을 집결하십시오!",
                    preMinute
                ).c_str());

        // Spawn-time announcement on the hour (00:00, 02:00, 04:00...)
        if (localTm.tm_hour % 2 == 0 && localTm.tm_min == 0)
            ChatHandler(nullptr).SendWorldText("[침공] 예언자 벨렌의 침공이 시작되었습니다. 20분 안에 처치하지 못하면 철수합니다!");
    }

private:
    uint32 _updateTimer;
    int32 _lastAnnounceKey;
};

void AddSC_boss_volgrass();

void Addmod_custom_boss_volgrassScripts()
{
    AddSC_boss_volgrass();
    new VolgrassWorldScript();
}



