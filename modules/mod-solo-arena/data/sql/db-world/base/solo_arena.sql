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

DROP TABLE IF EXISTS `solo_arena_stage_reward`;
CREATE TABLE `solo_arena_stage_reward` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `item_count` INT UNSIGNED NOT NULL DEFAULT 1,
    `chance` FLOAT NOT NULL DEFAULT 100,
    `reward_rank_value` TINYINT UNSIGNED NOT NULL DEFAULT 3,
    `reward_rank_label` VARCHAR(8) NOT NULL DEFAULT 'B',
    `sort_order` INT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `comment` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`id`),
    KEY `idx_solo_arena_stage_reward_stage` (`stage_id`, `enabled`,
        `reward_rank_value`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `solo_arena_stage_mechanic`;
CREATE TABLE `solo_arena_stage_mechanic` (
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `slot_id` TINYINT UNSIGNED NOT NULL,
    `mechanic_type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `object_entry` INT UNSIGNED NOT NULL DEFAULT 184663,
    `spawn_x` FLOAT NOT NULL,
    `spawn_y` FLOAT NOT NULL,
    `spawn_z` FLOAT NOT NULL,
    `spawn_o` FLOAT NOT NULL DEFAULT 0,
    `spawn_interval_ms` INT UNSIGNED NOT NULL DEFAULT 20000,
    `duration_ms` INT UNSIGNED NOT NULL DEFAULT 15000,
    `effect_value_1` FLOAT NOT NULL DEFAULT 0,
    `effect_value_2` FLOAT NOT NULL DEFAULT 0,
    `summon_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `name` VARCHAR(80) NOT NULL DEFAULT '',
    PRIMARY KEY (`stage_id`, `slot_id`)
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
(1, '그림자 시련 1단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.00, 1.00, 1900, 4200, 1.00, 6000, 1),
(2, '그림자 시련 2단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.10, 1.10, 1900, 4200, 1.00, 6000, 1),
(3, '그림자 시련 3단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.20, 1.20, 1900, 4200, 1.00, 6000, 1),
(4, '그림자 시련 4단계', 529, 1354.05, 1275.48, -11.30, 4.77,
    977.02, 1046.62, -44.81, -2.60, 1.30, 1.30, 1900, 4200, 1.00, 6000, 1),
(5, '그림자 시련 5단계', 529, 1354.05, 1275.48, -11.30, 4.77,
    977.02, 1046.62, -44.81, -2.60, 1.40, 1.40, 1900, 4200, 1.00, 6000, 1),
(6, '그림자 시련 6단계', 529, 1354.05, 1275.48, -11.30, 4.77,
    977.02, 1046.62, -44.81, -2.60, 1.50, 1.50, 1900, 4200, 1.00, 6000, 1),
(7, '그림자 시련 7단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.60, 1.60, 1900, 4200, 1.00, 6000, 1),
(8, '그림자 시련 8단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.70, 1.70, 1900, 4200, 1.00, 6000, 1),
(9, '그림자 시련 9단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.80, 1.80, 1900, 4200, 1.00, 6000, 1),
(10, '그림자 시련 10단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.90, 1.90, 1900, 4200, 1.00, 6000, 1);

DELETE FROM `solo_arena_stage_reward`;
DELETE FROM `solo_arena_stage_mechanic`;

INSERT INTO `solo_arena_stage_mechanic` (
    `stage_id`, `slot_id`, `mechanic_type`, `object_entry`,
    `spawn_x`, `spawn_y`, `spawn_z`, `spawn_o`,
    `spawn_interval_ms`, `duration_ms`,
    `effect_value_1`, `effect_value_2`, `summon_entry`,
    `enabled`, `name`
) VALUES
(1, 1, 1, 184663, 1281.90, 1667.30, 39.96, 0.00, 20000, 15000, 0.20, 0.25, 0, 1, '시련의 숨결'),
(2, 1, 2, 184664, 1289.60, 1668.10, 39.96, 0.00, 22000, 15000, 0.10, 0.08, 190024, 1, '뒤틀린 파편'),
(3, 1, 3, 184663, 1285.80, 1662.80, 39.96, 0.00, 25000, 15000, 18.00, 0.00, 190023, 1, '균열의 제단'),
(4, 1, 4, 184663, 1185.71, 1185.24, -56.36, 2.56, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 2, 4, 184663, 990.75, 1008.18, -42.60, 2.43, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 3, 4, 184663, 817.66, 843.34, -56.54, 3.01, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 4, 4, 184663, 807.46, 1189.16, 11.92, 5.44, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 5, 4, 184663, 1146.62, 816.94, -98.49, 6.14, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 6, 4, 184663, 1095.00, 1098.00, -49.00, 2.45, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 7, 4, 184663, 902.00, 944.00, -49.50, 2.90, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 8, 4, 184663, 899.00, 1098.00, -15.00, 4.90, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 9, 4, 184663, 1068.00, 915.00, -70.00, 5.85, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 10, 4, 184663, 1006.00, 1188.00, -36.00, 3.65, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 11, 4, 184663, 988.00, 852.00, -73.00, 5.75, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 12, 4, 184663, 862.00, 1018.00, -24.00, 4.55, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(4, 13, 4, 184663, 1148.00, 996.00, -78.00, 0.75, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(5, 1, 4, 184663, 1095.00, 1098.00, -49.00, 2.45, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(5, 2, 4, 184663, 899.00, 1098.00, -15.00, 4.90, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(5, 3, 5, 184664, 902.00, 944.00, -49.50, 2.90, 30000, 30000, 0.00, 0.00, 0, 1, '복귀의 상자'),
(5, 4, 5, 184664, 988.00, 852.00, -73.00, 5.75, 30000, 30000, 0.00, 0.00, 0, 1, '복귀의 상자'),
(5, 5, 4, 184663, 1006.00, 1188.00, -36.00, 3.65, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(5, 6, 4, 184663, 1148.00, 996.00, -78.00, 0.75, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(5, 7, 5, 184664, 1068.00, 915.00, -70.00, 5.85, 30000, 30000, 0.00, 0.00, 0, 1, '복귀의 상자'),
(5, 8, 4, 184663, 862.00, 1018.00, -24.00, 4.55, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(6, 1, 4, 184663, 1095.00, 1098.00, -49.00, 2.45, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(6, 2, 5, 184664, 902.00, 944.00, -49.50, 2.90, 30000, 30000, 0.00, 0.00, 0, 1, '복귀의 상자'),
(6, 3, 6, 184664, 899.00, 1098.00, -15.00, 4.90, 35000, 30000, 18.00, 10.00, 0, 1, '광폭의 상자'),
(6, 4, 6, 184664, 1068.00, 915.00, -70.00, 5.85, 35000, 30000, 22.00, 12.00, 0, 1, '광폭의 상자'),
(6, 5, 4, 184663, 1006.00, 1188.00, -36.00, 3.65, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자'),
(6, 6, 5, 184664, 988.00, 852.00, -73.00, 5.75, 30000, 30000, 0.00, 0.00, 0, 1, '복귀의 상자'),
(6, 7, 6, 184664, 862.00, 1018.00, -24.00, 4.55, 35000, 30000, 20.00, 11.00, 0, 1, '광폭의 상자'),
(6, 8, 4, 184663, 1148.00, 996.00, -78.00, 0.75, 25000, 30000, 8.00, 0.00, 0, 1, '질주의 상자');

DELETE FROM `creature_template`
WHERE `entry` IN (190021, 190022, 190023, 190024);

INSERT INTO `creature_template` (
    `entry`, `name`, `subname`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
    `speed_walk`, `speed_run`, `scale`, `rank`, `DamageModifier`,
    `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `type`,
    `AIName`, `MovementType`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RegenHealth`,
    `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES
(190021, '시련 감독관', '솔로 아레나', 0,
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
    0, 'npc_solo_arena_shadow', 12340),
(190023, '시련 수호자', '균열의 응답', 0,
    80, 80, 2, 35, 0,
    1, 1.14286, 1, 1, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_helper', 12340),
(190024, '뒤틀린 잔영', '파편의 분노', 0,
    80, 80, 2, 14, 0,
    1, 1.14286, 1, 1, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_hazard', 12340);

DELETE FROM `creature_template_model`
WHERE `CreatureID` IN (190021, 190022, 190023, 190024);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(190021, 0, 3167, 1, 1, 12340),
(190022, 0, 3167, 1, 1, 12340),
(190023, 0, 1126, 1, 1, 12340),
(190024, 0, 1126, 1, 1, 12340);

DELETE FROM `creature`
WHERE `guid` = 190021
  AND `id1` = 190021
  AND `ScriptName` = 'npc_solo_arena_master';

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
