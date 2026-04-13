SET @dbname := DATABASE();

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'melee_target_gs'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `melee_target_gs` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `preparation_ms`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'melee_health'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `melee_health` INT UNSIGNED NOT NULL DEFAULT 1 AFTER `melee_target_gs`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'melee_attack_power'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `melee_attack_power` INT NOT NULL DEFAULT 0 AFTER `melee_health`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'melee_crit_pct'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `melee_crit_pct` FLOAT NOT NULL DEFAULT 0 AFTER `melee_attack_power`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'melee_armor_pen_rating'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `melee_armor_pen_rating` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `melee_crit_pct`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'caster_target_gs'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `caster_target_gs` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `melee_armor_pen_rating`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'caster_health'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `caster_health` INT UNSIGNED NOT NULL DEFAULT 1 AFTER `caster_target_gs`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'caster_spell_power'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `caster_spell_power` INT NOT NULL DEFAULT 0 AFTER `caster_health`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'caster_crit_pct'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `caster_crit_pct` FLOAT NOT NULL DEFAULT 0 AFTER `caster_spell_power`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'caster_haste_rating'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `caster_haste_rating` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `caster_crit_pct`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 4300,
    `melee_health` = 22000,
    `melee_attack_power` = 3000,
    `melee_crit_pct` = 25.0,
    `melee_armor_pen_rating` = 200,
    `caster_target_gs` = 4300,
    `caster_health` = 20000,
    `caster_spell_power` = 1800,
    `caster_crit_pct` = 20.0,
    `caster_haste_rating` = 300
WHERE `stage_id` = 1;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 4600,
    `melee_health` = 24000,
    `melee_attack_power` = 3500,
    `melee_crit_pct` = 28.0,
    `melee_armor_pen_rating` = 300,
    `caster_target_gs` = 4600,
    `caster_health` = 22000,
    `caster_spell_power` = 2100,
    `caster_crit_pct` = 22.0,
    `caster_haste_rating` = 400
WHERE `stage_id` = 2;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 4900,
    `melee_health` = 26000,
    `melee_attack_power` = 4000,
    `melee_crit_pct` = 30.0,
    `melee_armor_pen_rating` = 400,
    `caster_target_gs` = 4900,
    `caster_health` = 24000,
    `caster_spell_power` = 2400,
    `caster_crit_pct` = 25.0,
    `caster_haste_rating` = 500
WHERE `stage_id` = 3;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 5200,
    `melee_health` = 28000,
    `melee_attack_power` = 4600,
    `melee_crit_pct` = 32.0,
    `melee_armor_pen_rating` = 500,
    `caster_target_gs` = 5200,
    `caster_health` = 26000,
    `caster_spell_power` = 2800,
    `caster_crit_pct` = 27.0,
    `caster_haste_rating` = 600
WHERE `stage_id` = 4;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 5500,
    `melee_health` = 30000,
    `melee_attack_power` = 5200,
    `melee_crit_pct` = 35.0,
    `melee_armor_pen_rating` = 600,
    `caster_target_gs` = 5500,
    `caster_health` = 28000,
    `caster_spell_power` = 3200,
    `caster_crit_pct` = 30.0,
    `caster_haste_rating` = 700
WHERE `stage_id` = 5;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 5800,
    `melee_health` = 32000,
    `melee_attack_power` = 5800,
    `melee_crit_pct` = 38.0,
    `melee_armor_pen_rating` = 700,
    `caster_target_gs` = 5800,
    `caster_health` = 30000,
    `caster_spell_power` = 3500,
    `caster_crit_pct` = 32.0,
    `caster_haste_rating` = 800
WHERE `stage_id` = 6;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 6000,
    `melee_health` = 34000,
    `melee_attack_power` = 6300,
    `melee_crit_pct` = 40.0,
    `melee_armor_pen_rating` = 800,
    `caster_target_gs` = 6000,
    `caster_health` = 32000,
    `caster_spell_power` = 3800,
    `caster_crit_pct` = 35.0,
    `caster_haste_rating` = 900
WHERE `stage_id` = 7;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 6300,
    `melee_health` = 36000,
    `melee_attack_power` = 6800,
    `melee_crit_pct` = 42.0,
    `melee_armor_pen_rating` = 900,
    `caster_target_gs` = 6300,
    `caster_health` = 34000,
    `caster_spell_power` = 4200,
    `caster_crit_pct` = 37.0,
    `caster_haste_rating` = 1000
WHERE `stage_id` = 8;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 6500,
    `melee_health` = 38000,
    `melee_attack_power` = 7200,
    `melee_crit_pct` = 45.0,
    `melee_armor_pen_rating` = 1000,
    `caster_target_gs` = 6500,
    `caster_health` = 36000,
    `caster_spell_power` = 4500,
    `caster_crit_pct` = 40.0,
    `caster_haste_rating` = 1100
WHERE `stage_id` = 9;

UPDATE `solo_arena_stage`
SET
    `melee_target_gs` = 6500,
    `melee_health` = 38000,
    `melee_attack_power` = 7200,
    `melee_crit_pct` = 45.0,
    `melee_armor_pen_rating` = 1000,
    `caster_target_gs` = 6500,
    `caster_health` = 36000,
    `caster_spell_power` = 4500,
    `caster_crit_pct` = 40.0,
    `caster_haste_rating` = 1100
WHERE `stage_id` = 10;
