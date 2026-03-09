/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef BOSS_VOLGRASS_H
#define BOSS_VOLGRASS_H

#include "ScriptMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellAuras.h"

// Volgrass spell IDs (must exist in spell.dbc)
enum VolgrassSpells
{
    SPELL_SUPER_GALE         = 64746,
    SPELL_CHARGED_LEAP       = 64779,
    SPELL_DEATH_GRIP         = 47756,
    SPELL_BIG_EXPLOSION      = 64443,
    SPELL_WHIRLWIND          = 17207,
    SPELL_CLEAVE             = 15284,
    SPELL_BERSERK            = 26662
};

// Event IDs
enum VolgrassEvents
{
    EVENT_SUPER_GALE = 1,
    EVENT_CHARGED_LEAP,
    EVENT_WHIRLWIND,
    EVENT_CLEAVE,
    EVENT_DEATH_GRIP,
    EVENT_BIG_EXPLOSION,
    EVENT_SUMMON_ADDS,
    EVENT_CHECK_PHASE,
    EVENT_BERSERK,
    EVENT_ENCOUNTER_TIMEOUT,
    EVENT_BIG_EXPLOSION_OBJECTS_DESPAWN
};

// Boss phases
enum VolgrassPhases
{
    PHASE_ONE = 1,    // 100-70% HP
    PHASE_TWO = 2,    // 70-40% HP
    PHASE_THREE = 3   // 40-0% HP
};

// World announcements
namespace VolgrassAnnounce
{
    constexpr const char* INVASION_START = "[침공] 섬 중심에서 침공 보스 예언자 벨렌이 모습을 드러냈습니다!";
    constexpr const char* PHASE_TWO_START = "[침공] 예언자 벨렌이 분노하며 침공 병력을 소집합니다!";
    constexpr const char* PHASE_THREE_START = "[침공] 예언자 벨렌이 폭주했습니다! 섬의 중심이 붕괴 직전입니다!";
    constexpr const char* INVASION_SUCCESS = "[침공] 예언자 벨렌을 격퇴했습니다! 섬 방어전에 성공했습니다.";
    constexpr const char* INVASION_FAILED_TIMEOUT = "[침공] 20분 내에 격퇴하지 못해 예언자 벨렌이 철수했습니다.";
}

#endif // BOSS_VOLGRASS_H


