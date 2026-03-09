/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "boss_volgrass.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "SpellScript.h"
#include "Chat.h"
#include "World.h"
#include "Log.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "ObjectAccessor.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <vector>

// VolgrassConfig ?몃? ?좎뼵 (loader.cpp?먯꽌 ?뺤쓽??
namespace VolgrassConfig
{
    extern uint32 ADD_ENTRY_ID;
    extern uint8 PHASE2_ADD_COUNT;
    extern uint8 PHASE3_ADD_COUNT;
    extern uint32 SUMMON_INTERVAL;
    extern uint32 BERSERK_TIMER;
    extern uint32 ENCOUNTER_TIMEOUT;
}

class boss_volgrass : public CreatureScript
{
public:
    boss_volgrass() : CreatureScript("boss_volgrass") {}

    struct boss_volgrassAI : public ScriptedAI
    {
        boss_volgrassAI(Creature* creature) : ScriptedAI(creature)
        {
            _phaseOneTriggered = false;
            _phaseTwoTriggered = false;
            _phaseThreeTriggered = false;
            _currentPhase = PHASE_ONE;
            _explosionVisualGuid = ObjectGuid::Empty;
        }

        EventMap events;
        uint8 _currentPhase;
        bool _phaseOneTriggered;
        bool _phaseTwoTriggered;
        bool _phaseThreeTriggered;
        ObjectGuid _explosionVisualGuid;
        std::vector<ObjectGuid> _activeAddGuids;

        void Reset() override
        {
            events.Reset();
            DespawnBigExplosionVisuals();
            _activeAddGuids.clear();
            _currentPhase = PHASE_ONE;
            _phaseOneTriggered = false;
            _phaseTwoTriggered = false;
            _phaseThreeTriggered = false;

            LOG_INFO("module", "Volgrass: Boss reset");
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            LOG_INFO("module", "Volgrass: Combat started");

            // ?쒕쾭 怨듭? - 移④났 ?쒖옉
            ChatHandler(nullptr).SendWorldText(VolgrassAnnounce::INVASION_START);

            // 蹂댁뒪 ???- creature_text GroupID 0 (?쒕뜡)
            Talk(0);

            // Phase 1 ?대깽???쒖옉
            events.ScheduleEvent(EVENT_SUPER_GALE, 12s);
            events.ScheduleEvent(EVENT_CHARGED_LEAP, 20s);
            events.ScheduleEvent(EVENT_WHIRLWIND, 28s);
            events.ScheduleEvent(EVENT_CLEAVE, 16s);
            events.ScheduleEvent(EVENT_CHECK_PHASE, 2s);
            events.ScheduleEvent(EVENT_BERSERK, Milliseconds(VolgrassConfig::BERSERK_TIMER));
            events.ScheduleEvent(EVENT_ENCOUNTER_TIMEOUT, Milliseconds(VolgrassConfig::ENCOUNTER_TIMEOUT));

            _phaseOneTriggered = true;
        }

        void JustDied(Unit* /*killer*/) override
        {
            LOG_INFO("module", "Volgrass: Boss defeated");
            DespawnBigExplosionVisuals();
            _activeAddGuids.clear();

            // ?쒕쾭 怨듭? - 移④났 ?깃났
            ChatHandler(nullptr).SendWorldText(VolgrassAnnounce::INVASION_SUCCESS);

            // ?щ쭩 ???- creature_text GroupID 10 (?쒕뜡)
            Talk(10);

            events.Reset();
        }

        void KilledUnit(Unit* victim) override
        {
            if (!victim || !victim->IsPlayer())
                return;

            // ?뚮젅?댁뼱 泥섏튂 硫섑듃 - creature_text GroupID 9 (?쒕뜡)
            Talk(9);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            // ?섏씠利?泥댄겕
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_PHASE:
                    {
                        uint8 healthPct = me->GetHealthPct();

                        // Phase 2 吏꾩엯 (70% ?댄븯, 1?뚮쭔)
                        if (!_phaseTwoTriggered && healthPct <= 70)
                        {
                            EnterPhaseTwo();
                            _phaseTwoTriggered = true;
                        }
                        // Phase 3 吏꾩엯 (40% ?댄븯, 1?뚮쭔)
                        else if (!_phaseThreeTriggered && healthPct <= 40)
                        {
                            EnterPhaseThree();
                            _phaseThreeTriggered = true;
                        }

                        events.ScheduleEvent(EVENT_CHECK_PHASE, 2s);
                        break;
                    }

                    case EVENT_SUPER_GALE:
                    {
                        // 珥덇컯???댄뭾 硫섑듃 - creature_text GroupID 7 (?쒕뜡)
                        Talk(7);
                        DoCastVictim(SPELL_SUPER_GALE);
                        events.ScheduleEvent(EVENT_SUPER_GALE, randtime(24s, 32s));
                        break;
                    }

                    case EVENT_CHARGED_LEAP:
                    {
                        constexpr float leapMinDistance = 12.0f;
                        std::vector<Player*> leapTargets;
                        std::list<Player*> playerList;
                        Acore::AnyPlayerInObjectRangeCheck check(me, 40.0f);
                        Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(me, playerList, check);
                        Cell::VisitObjects(me, searcher, 40.0f);

                        for (Player* player : playerList)
                        {
                            if (!player || !player->IsAlive() || player->IsGameMaster())
                                continue;

                            if (me->GetDistance(player) >= leapMinDistance)
                                leapTargets.push_back(player);
                        }

                        if (!leapTargets.empty())
                        {
                            Player* target = leapTargets[urand(0, leapTargets.size() - 1)];
                            // 異⑹쟾???꾩빟 硫섑듃 - creature_text GroupID 6 (?쒕뜡)
                            Talk(6);
                            DoCast(target, SPELL_CHARGED_LEAP);
                        }
                        events.ScheduleEvent(EVENT_CHARGED_LEAP, randtime(30s, 40s));
                        break;
                    }

                    case EVENT_DEATH_GRIP:
                    {
                        // ?뚯뼱?밴? ???- creature_text GroupID 3 (?쒕뜡)
                        Talk(3);

                        // 二쇰? ?뚮젅?댁뼱 ?뚯뼱?밴?
                        std::list<Player*> playerList;
                        Acore::AnyPlayerInObjectRangeCheck check(me, 50.0f);
                        Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(me, playerList, check);
                        Cell::VisitObjects(me, searcher, 50.0f);

                        for (Player* player : playerList)
                        {
                            if (player && player->IsAlive() && !player->IsGameMaster())
                            {
                                me->CastSpell(player, SPELL_DEATH_GRIP, true);
                            }
                        }

                        // 1.5~2珥?????컻 ?덉빟
                        events.ScheduleEvent(EVENT_BIG_EXPLOSION, randtime(2500ms, 3500ms));

                        // ?ㅼ쓬 ?뚯뼱?밴? ?ㅼ?以?
                        if (_currentPhase == PHASE_TWO)
                        {
                            events.ScheduleEvent(EVENT_DEATH_GRIP, randtime(38s, 48s));
                        }
                        else if (_currentPhase == PHASE_THREE)
                        {
                            events.ScheduleEvent(EVENT_DEATH_GRIP, randtime(28s, 36s));
                        }
                        break;
                    }

                    case EVENT_BIG_EXPLOSION:
                    {
                        // ??컻 ???- Phase???곕씪 ?ㅻⅨ GroupID ?ъ슜
                        if (_currentPhase == PHASE_THREE)
                        {
                            // creature_text GroupID 5 (Phase 3 ?꾩슜 ?꾪삊??硫섑듃)
                            Talk(5);
                        }
                        else
                        {
                            // creature_text GroupID 4 (?쇰컲 ??컻 硫섑듃)
                            Talk(4);
                        }

                        SummonBigExplosionVisuals();
                        DoCastAOE(SPELL_BIG_EXPLOSION);
                        events.ScheduleEvent(EVENT_BIG_EXPLOSION_OBJECTS_DESPAWN, Milliseconds(GetBigExplosionVisualDuration()));
                        break;
                    }
                    case EVENT_WHIRLWIND:
                    {
                        DoCastSelf(SPELL_WHIRLWIND);
                        if (_currentPhase == PHASE_THREE)
                            events.ScheduleEvent(EVENT_WHIRLWIND, randtime(20s, 26s));
                        else
                            events.ScheduleEvent(EVENT_WHIRLWIND, randtime(26s, 34s));
                        break;
                    }
                    case EVENT_CLEAVE:
                    {
                        DoCastVictim(SPELL_CLEAVE);
                        if (_currentPhase == PHASE_THREE)
                            events.ScheduleEvent(EVENT_CLEAVE, randtime(12s, 16s));
                        else
                            events.ScheduleEvent(EVENT_CLEAVE, randtime(16s, 22s));
                        break;
                    }

                    case EVENT_SUMMON_ADDS:
                    {
                        if (VolgrassConfig::ADD_ENTRY_ID == 0)
                        {
                            break;
                        }
                        CleanupActiveAdds();
                        if (!_activeAddGuids.empty())
                        {
                            // Existing wave must be cleared before summoning a new one.
                            events.ScheduleEvent(EVENT_SUMMON_ADDS, 6s);
                            break;
                        }

                        uint8 summonCount = (_currentPhase == PHASE_THREE)
                            ? VolgrassConfig::PHASE3_ADD_COUNT
                            : VolgrassConfig::PHASE2_ADD_COUNT;
                        if (summonCount == 0)
                        {
                            LOG_WARN("module", "Volgrass: Add summon skipped because summon count is 0 (phase: {})", _currentPhase);
                            events.ScheduleEvent(EVENT_SUMMON_ADDS, Milliseconds(VolgrassConfig::SUMMON_INTERVAL));
                            break;
                        }

                        Talk(8);
                        uint8 successCount = 0;
                        for (uint8 i = 0; i < summonCount; ++i)
                        {
                            float angle = (2.0f * M_PI / summonCount) * i;
                            float radius = (_currentPhase == PHASE_THREE) ? frand(10.0f, 14.0f) : frand(9.0f, 12.0f);
                            float x = me->GetPositionX() + radius * cos(angle);
                            float y = me->GetPositionY() + radius * sin(angle);
                            float z = me->GetPositionZ();

                            if (Creature* add = me->SummonCreature(VolgrassConfig::ADD_ENTRY_ID, x, y, z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
                            {
                                ++successCount;
                                _activeAddGuids.push_back(add->GetGUID());
                                if (Unit* victim = me->GetVictim())
                                    add->AI()->AttackStart(victim);
                            }
                        }

                        if (successCount == 0)
                            LOG_ERROR("module", "Volgrass: Failed to summon adds. Entry {} may be invalid or blocked at location.", VolgrassConfig::ADD_ENTRY_ID);
                        else
                            LOG_INFO("module", "Volgrass: Summoned {}/{} adds (entry: {})", successCount, summonCount, VolgrassConfig::ADD_ENTRY_ID);

                        uint32 nextInterval = (_currentPhase == PHASE_THREE)
                            ? std::max<uint32>(24000, VolgrassConfig::SUMMON_INTERVAL)
                            : VolgrassConfig::SUMMON_INTERVAL;
                        events.ScheduleEvent(EVENT_SUMMON_ADDS, Milliseconds(nextInterval));
                        break;
                    }
                    case EVENT_BERSERK:
                    {
                        if (!me->HasAura(SPELL_BERSERK))
                        {
                            Talk(2);
                            DoCastSelf(SPELL_BERSERK, true);
                            LOG_INFO("module", "Volgrass: Berserk activated");
                        }
                        break;
                    }
                    case EVENT_BIG_EXPLOSION_OBJECTS_DESPAWN:
                    {
                        DespawnBigExplosionVisuals();
                        break;
                    }

                    case EVENT_ENCOUNTER_TIMEOUT:
                    {
                        ChatHandler(nullptr).SendWorldText(VolgrassAnnounce::INVASION_FAILED_TIMEOUT);
                        LOG_INFO("module", "Volgrass: Encounter timeout reached, despawning");

                        DespawnBigExplosionVisuals();
                        _activeAddGuids.clear();
                        events.Reset();
                        me->CombatStop(true);
                        me->DespawnOrUnsummon(1s);
                        return;
                    }

                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

        void EnterPhaseTwo()
        {
            LOG_INFO("module", "Volgrass: Entering Phase 2");

            _currentPhase = PHASE_TWO;

            // ?쒕쾭 怨듭?
            ChatHandler(nullptr).SendWorldText(VolgrassAnnounce::PHASE_TWO_START);

            // 蹂댁뒪 ???- creature_text GroupID 1 (Phase 2)
            Talk(1);

            // Phase 2 ?듭떖 湲곕? ?쒖꽦??
            events.ScheduleEvent(EVENT_DEATH_GRIP, 6s);  // 利됱떆 ?뚯뼱?밴? ?쒖옉
            if (VolgrassConfig::ADD_ENTRY_ID > 0)
                events.ScheduleEvent(EVENT_SUMMON_ADDS, 12s);  // ?뚰솚 ?쒖옉
        }

        void EnterPhaseThree()
        {
            LOG_INFO("module", "Volgrass: Entering Phase 3");

            _currentPhase = PHASE_THREE;

            // ?쒕쾭 怨듭?
            ChatHandler(nullptr).SendWorldText(VolgrassAnnounce::PHASE_THREE_START);

            // 蹂댁뒪 ???- creature_text GroupID 2 (Phase 3 ??＜)
            Talk(2);

            // Phase 3: ??＜ - 紐⑤뱺 ?ㅽ궗 鍮덈룄 利앷?
            events.CancelEvent(EVENT_DEATH_GRIP);
            events.ScheduleEvent(EVENT_DEATH_GRIP, 5s);  // 利됱떆 ?뚯뼱?밴?
            if (VolgrassConfig::ADD_ENTRY_ID > 0)
                events.ScheduleEvent(EVENT_SUMMON_ADDS, 8s);
        }

        uint32 GetBigExplosionVisualDuration() const
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_BIG_EXPLOSION))
            {
                uint32 castTime = spellInfo->CalcCastTime();
                if (castTime > 0)
                    return castTime + 200;
            }

            return 1500;
        }

        void SummonBigExplosionVisuals()
        {
            DespawnBigExplosionVisuals();

            constexpr uint32 GO_BIG_EXPLOSION_VISUAL = 193613;
            constexpr float fixedRadius = 24.0f;

            float x = me->GetPositionX();
            float y = me->GetPositionY();
            float z = me->GetPositionZ();
            float angle = frand(0.0f, float(2.0f * M_PI));
            float goX = x + std::cos(angle) * fixedRadius;
            float goY = y + std::sin(angle) * fixedRadius;
            float faceBoss = std::atan2(y - goY, x - goX);

            uint32 despawnMs = GetBigExplosionVisualDuration() + 1000;
            if (GameObject* go = me->SummonGameObject(GO_BIG_EXPLOSION_VISUAL,
                goX, goY, z, faceBoss, 0.0f, 0.0f, 0.0f, 0.0f, despawnMs))
            {
                _explosionVisualGuid = go->GetGUID();
            }
        }

        void DespawnBigExplosionVisuals()
        {
            if (_explosionVisualGuid.IsEmpty())
                return;

            if (GameObject* go = ObjectAccessor::GetGameObject(*me, _explosionVisualGuid))
                go->Delete();

            _explosionVisualGuid.Clear();
        }

        void CleanupActiveAdds()
        {
            _activeAddGuids.erase(std::remove_if(_activeAddGuids.begin(), _activeAddGuids.end(),
                [this](ObjectGuid const& guid)
                {
                    Creature* add = ObjectAccessor::GetCreature(*me, guid);
                    return !add || !add->IsAlive();
                }), _activeAddGuids.end());
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_volgrassAI(creature);
    }
};

void AddSC_boss_volgrass()
{
    new boss_volgrass();
    LOG_INFO("module", "Volgrass: Boss script registered");
}


