DELETE FROM `creature`
WHERE `guid` = 190021
  AND `id1` = 190021
  AND `ScriptName` = 'npc_solo_arena_master';

DELETE FROM `creature_template_model`
WHERE `CreatureID` IN (190021, 190022);

DELETE FROM `creature_template`
WHERE `entry` IN (190021, 190022)
  AND `ScriptName` IN ('npc_solo_arena_master', 'npc_solo_arena_shadow');

INSERT INTO `creature_template` (
    `entry`, `name`, `subname`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
    `speed_walk`, `speed_run`, `scale`, `rank`, `DamageModifier`,
    `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `type`,
    `AIName`, `MovementType`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RegenHealth`,
    `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES
(190021, '투기장 감독관', '솔로 아레나', 0,
    80, 80, 2, 35, 1,
    1, 1.14286, 1, 0, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_master', 12340),
(190022, '그림자 전사', '도전자와 닮은 어둠', 0,
    80, 80, 2, 14, 0,
    1, 1.14286, 1, 1, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_shadow', 12340);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(190021, 0, 3167, 1, 1, 12340),
(190022, 0, 3167, 1, 1, 12340);

INSERT INTO `creature` (
    `guid`, `id1`, `id2`, `id3`, `map`, `zoneId`, `areaId`,
    `spawnMask`, `phaseMask`, `equipment_id`,
    `position_x`, `position_y`, `position_z`, `orientation`,
    `spawntimesecs`, `wander_distance`, `currentwaypoint`,
    `curhealth`, `curmana`, `MovementType`, `npcflag`,
    `unit_flags`, `dynamicflags`, `ScriptName`,
    `VerifiedBuild`, `CreateObject`, `Comment`
) VALUES
(190021, 190021, 0, 0, 571, 0, 0,
    1, 1, 0,
    5803.51, 494.77, 657.21, 5.54,
    120, 0, 0,
    50000, 10000, 0, 1,
    0, 0, 'npc_solo_arena_master',
    12340, 0, 'mod-solo-arena starter NPC');

DELETE FROM `creature`
WHERE `guid` = 910000
  AND `id1` = 910000
  AND `ScriptName` = 'npc_solo_arena_master';

DELETE FROM `creature_template`
WHERE `entry` IN (910000, 910001)
  AND `ScriptName` IN ('npc_solo_arena_master', 'npc_solo_arena_shadow');
