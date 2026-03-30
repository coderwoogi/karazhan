SET @solo_arena_has_preparation_ms := (
    SELECT COUNT(*)
    FROM `INFORMATION_SCHEMA`.`COLUMNS`
    WHERE `TABLE_SCHEMA` = DATABASE()
      AND `TABLE_NAME` = 'solo_arena_stage'
      AND `COLUMN_NAME` = 'preparation_ms'
);

SET @solo_arena_stage_alter_sql := IF(
    @solo_arena_has_preparation_ms = 0,
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `preparation_ms` INT UNSIGNED NOT NULL DEFAULT 6000 AFTER `move_speed_rate`',
    'SELECT 1'
);

PREPARE solo_arena_stage_stmt FROM @solo_arena_stage_alter_sql;
EXECUTE solo_arena_stage_stmt;
DEALLOCATE PREPARE solo_arena_stage_stmt;

REPLACE INTO `solo_arena_stage` (
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
(4, '그림자 시련 4단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.30, 1.30, 1900, 4200, 1.00, 6000, 1),
(5, '그림자 시련 5단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.40, 1.40, 1900, 4200, 1.00, 6000, 1),
(6, '그림자 시련 6단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.50, 1.50, 1900, 4200, 1.00, 6000, 1),
(7, '그림자 시련 7단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.60, 1.60, 1900, 4200, 1.00, 6000, 1),
(8, '그림자 시련 8단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.70, 1.70, 1900, 4200, 1.00, 6000, 1),
(9, '그림자 시련 9단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.80, 1.80, 1900, 4200, 1.00, 6000, 1),
(10, '그림자 시련 10단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.90, 1.90, 1900, 4200, 1.00, 6000, 1);

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
    0, 'npc_solo_arena_shadow', 12340);

DELETE FROM `creature_template_model`
WHERE `CreatureID` IN (190021, 190022);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(190021, 0, 3167, 1, 1, 12340),
(190022, 0, 3167, 1, 1, 12340);
