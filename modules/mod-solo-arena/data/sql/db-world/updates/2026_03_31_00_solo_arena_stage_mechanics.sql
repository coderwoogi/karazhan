CREATE TABLE IF NOT EXISTS `solo_arena_stage_mechanic` (
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

REPLACE INTO `solo_arena_stage_mechanic` (
    `stage_id`, `slot_id`, `mechanic_type`, `object_entry`,
    `spawn_x`, `spawn_y`, `spawn_z`, `spawn_o`,
    `spawn_interval_ms`, `duration_ms`,
    `effect_value_1`, `effect_value_2`, `summon_entry`,
    `enabled`, `name`
) VALUES
(4, 1, 1, 184663, 1328.72, 1632.72, 36.73, 0.00, 20000, 15000, 0.20, 0.25, 0, 1, '시련의 숨결'),
(5, 1, 2, 184664, 1243.30, 1699.17, 34.87, 0.00, 22000, 15000, 0.10, 0.08, 190024, 1, '뒤틀린 파편'),
(6, 1, 3, 184663, 1285.81, 1667.90, 39.96, 0.00, 25000, 15000, 18.00, 0.00, 190023, 1, '균열의 제단');

DELETE FROM `creature_template`
WHERE `entry` IN (190023, 190024)
  AND `ScriptName` IN ('npc_solo_arena_helper', 'npc_solo_arena_hazard');

INSERT INTO `creature_template` (
    `entry`, `name`, `subname`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
    `speed_walk`, `speed_run`, `scale`, `rank`, `DamageModifier`,
    `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `type`,
    `AIName`, `MovementType`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RegenHealth`,
    `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES
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
WHERE `CreatureID` IN (190023, 190024);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(190023, 0, 1126, 1, 1, 12340),
(190024, 0, 1126, 1, 1, 12340);
