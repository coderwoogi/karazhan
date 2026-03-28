DROP TABLE IF EXISTS `solo_arena_stage`;
CREATE TABLE `solo_arena_stage` (
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `name` VARCHAR(80) NOT NULL,
    `arena_map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 572,
    `player_x` FLOAT NOT NULL,
    `player_y` FLOAT NOT NULL,
    `player_z` FLOAT NOT NULL,
    `player_o` FLOAT NOT NULL,
    `bot_x` FLOAT NOT NULL,
    `bot_y` FLOAT NOT NULL,
    `bot_z` FLOAT NOT NULL,
    `bot_o` FLOAT NOT NULL,
    `health_multiplier` FLOAT NOT NULL DEFAULT 1,
    `damage_multiplier` FLOAT NOT NULL DEFAULT 1,
    `attack_time_ms` INT UNSIGNED NOT NULL DEFAULT 2000,
    `spell_interval_ms` INT UNSIGNED NOT NULL DEFAULT 4000,
    `move_speed_rate` FLOAT NOT NULL DEFAULT 1,
    `preparation_ms` INT UNSIGNED NOT NULL DEFAULT 6000,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`stage_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DELETE FROM `solo_arena_stage`;
INSERT INTO `solo_arena_stage` (
    `stage_id`, `name`, `arena_map_id`,
    `player_x`, `player_y`, `player_z`, `player_o`,
    `bot_x`, `bot_y`, `bot_z`, `bot_o`,
    `health_multiplier`, `damage_multiplier`,
    `attack_time_ms`, `spell_interval_ms`,
    `move_speed_rate`, `preparation_ms`, `enabled`
) VALUES
(1, '그림자 시련 1단계', 572, 1298.61, 1598.59, 31.62, 1.57,
    1273.71, 1734.05, 31.61, 4.71, 1.00, 1.00, 1900, 4200, 1.00, 6000, 1),
(2, '그림자 시련 2단계', 572, 1298.61, 1598.59, 31.62, 1.57,
    1273.71, 1734.05, 31.61, 4.71, 1.35, 1.25, 1600, 3000, 1.10, 6000, 1),
(3, '그림자 시련 3단계', 572, 1298.61, 1598.59, 31.62, 1.57,
    1273.71, 1734.05, 31.61, 4.71, 1.75, 1.55, 1300, 2100, 1.20, 6000, 1);

DELETE FROM `creature_template` WHERE `entry` IN (910000, 910001);
INSERT INTO `creature_template` (
    `entry`, `name`, `subname`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
    `speed_walk`, `speed_run`, `scale`, `rank`, `DamageModifier`,
    `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `type`,
    `AIName`, `MovementType`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RegenHealth`,
    `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES
(910000, '투기장 감독관', '솔로 아레나', 0,
    80, 80, 2, 35, 1,
    1, 1.14286, 1, 0, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_master', 12340),
(910001, '그림자 전사', '도전자와 닮은 어둠', 0,
    80, 80, 2, 14, 0,
    1, 1.14286, 1, 1, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_shadow', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (910000, 910001);
INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(910000, 0, 3167, 1, 1, 12340),
(910001, 0, 3167, 1, 1, 12340);

DELETE FROM `creature` WHERE `guid` = 910000;
INSERT INTO `creature` (
    `guid`, `id1`, `id2`, `id3`, `map`, `zoneId`, `areaId`,
    `spawnMask`, `phaseMask`, `equipment_id`,
    `position_x`, `position_y`, `position_z`, `orientation`,
    `spawntimesecs`, `wander_distance`, `currentwaypoint`,
    `curhealth`, `curmana`, `MovementType`, `npcflag`,
    `unit_flags`, `dynamicflags`, `ScriptName`,
    `VerifiedBuild`, `CreateObject`, `Comment`
) VALUES
(910000, 910000, 0, 0, 571, 0, 0,
    1, 1, 0,
    5803.51, 494.77, 657.21, 5.54,
    120, 0, 0,
    50000, 10000, 0, 1,
    0, 0, 'npc_solo_arena_master',
    12340, 0, 'mod-solo-arena starter NPC');
