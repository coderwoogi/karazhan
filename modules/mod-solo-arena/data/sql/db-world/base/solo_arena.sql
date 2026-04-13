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
    `melee_target_gs` INT UNSIGNED NOT NULL DEFAULT 0,
    `melee_health` INT UNSIGNED NOT NULL DEFAULT 1,
    `melee_attack_power` INT NOT NULL DEFAULT 0,
    `melee_crit_pct` FLOAT NOT NULL DEFAULT 0,
    `melee_armor_pen_rating` INT UNSIGNED NOT NULL DEFAULT 0,
    `caster_target_gs` INT UNSIGNED NOT NULL DEFAULT 0,
    `caster_health` INT UNSIGNED NOT NULL DEFAULT 1,
    `caster_spell_power` INT NOT NULL DEFAULT 0,
    `caster_crit_pct` FLOAT NOT NULL DEFAULT 0,
    `caster_haste_rating` INT UNSIGNED NOT NULL DEFAULT 0,
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
    `object_entry` INT UNSIGNED NOT NULL DEFAULT 178187,
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
    `move_speed_rate`, `preparation_ms`,
    `melee_target_gs`, `melee_health`, `melee_attack_power`,
    `melee_crit_pct`, `melee_armor_pen_rating`,
    `caster_target_gs`, `caster_health`, `caster_spell_power`,
    `caster_crit_pct`, `caster_haste_rating`, `enabled`
) VALUES
(1, '그림자 시련 1단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.00, 1.00, 1900, 4200, 1.00, 6000,
    4300, 22000, 3000, 25.0, 200, 4300, 20000, 1800, 20.0, 300, 1),
(2, '그림자 시련 2단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.10, 1.10, 1900, 4200, 1.00, 6000,
    4600, 24000, 3500, 28.0, 300, 4600, 22000, 2100, 22.0, 400, 1),
(3, '그림자 시련 3단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.20, 1.20, 1900, 4200, 1.00, 6000,
    4900, 26000, 4000, 30.0, 400, 4900, 24000, 2400, 25.0, 500, 1),
(4, '그림자 시련 4단계', 529, 1354.05, 1275.48, -11.30, 4.77,
    977.02, 1046.62, -44.81, -2.60, 1.00, 1.00, 1900, 4200, 1.00, 6000,
    5200, 28000, 4600, 32.0, 500, 5200, 26000, 2800, 27.0, 600, 1),
(5, '그림자 시련 5단계', 529, 1354.05, 1275.48, -11.30, 4.77,
    977.02, 1046.62, -44.81, -2.60, 1.20, 1.20, 1900, 4200, 1.20, 6000,
    5500, 30000, 5200, 35.0, 600, 5500, 28000, 3200, 30.0, 700, 1),
(6, '그림자 시련 6단계', 529, 1354.05, 1275.48, -11.30, 4.77,
    977.02, 1046.62, -44.81, -2.60, 1.50, 1.50, 1900, 4200, 1.50, 6000,
    5800, 32000, 5800, 38.0, 700, 5800, 30000, 3500, 32.0, 800, 1),
(7, '그림자 시련 7단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.60, 1.60, 1900, 4200, 1.00, 6000,
    6000, 34000, 6300, 40.0, 800, 6000, 32000, 3800, 35.0, 900, 1),
(8, '그림자 시련 8단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.70, 1.70, 1900, 4200, 1.00, 6000,
    6300, 36000, 6800, 42.0, 900, 6300, 34000, 4200, 37.0, 1000, 1),
(9, '그림자 시련 9단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.80, 1.80, 1900, 4200, 1.00, 6000,
    6500, 38000, 7200, 45.0, 1000, 6500, 36000, 4500, 40.0, 1100, 1),
(10, '그림자 시련 10단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.90, 1.90, 1900, 4200, 1.00, 6000,
    6500, 38000, 7200, 45.0, 1000, 6500, 36000, 4500, 40.0, 1100, 1);

DELETE FROM `solo_arena_stage_reward`;
DELETE FROM `solo_arena_stage_mechanic`;

INSERT INTO `solo_arena_stage_mechanic` (
    `stage_id`, `slot_id`, `mechanic_type`, `object_entry`,
    `spawn_x`, `spawn_y`, `spawn_z`, `spawn_o`,
    `spawn_interval_ms`, `duration_ms`,
    `effect_value_1`, `effect_value_2`, `summon_entry`,
    `enabled`, `name`
) VALUES
(1, 1, 1, 178187, 1281.90, 1667.30, 39.96, 0.00, 20000, 15000, 0.20, 0.25, 0, 1, '시련의 숨결'),
(2, 1, 2, 20352, 1289.60, 1668.10, 39.96, 0.00, 22000, 15000, 0.10, 0.08, 190024, 1, '뒤틀린 파편'),
(3, 1, 3, 178187, 1285.80, 1662.80, 39.96, 0.00, 25000, 15000, 18.00, 0.00, 190023, 1, '균열의 제단');

DELETE FROM `creature_template`
WHERE `entry` IN (190021, 190022, 190023, 190024, 190025);

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
    0, 'npc_solo_arena_hazard', 12340),
(190025, '시련 경로 마커', 'GM 경로 배치용', 0,
    1, 1, 0, 35, 0,
    1, 1, 0.35, 0, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    128, '', 12340);

DELETE FROM `creature_template_model`
WHERE `CreatureID` IN (190021, 190022, 190023, 190024, 190025);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(190021, 0, 3167, 1, 1, 12340),
(190022, 0, 3167, 1, 1, 12340),
(190023, 0, 1126, 1, 1, 12340),
(190024, 0, 1126, 1, 1, 12340),
(190025, 0, 1126, 0.35, 1, 12340);

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
