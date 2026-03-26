SET @has_col := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_mission_pool'
      AND COLUMN_NAME = 'difficulty_mask'
);
SET @sql := IF(
    @has_col = 0,
    'ALTER TABLE `instance_bonus_mission_pool` ADD COLUMN `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `mission_type`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_col := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_theme_pool'
      AND COLUMN_NAME = 'difficulty_mask'
);
SET @sql := IF(
    @has_col = 0,
    'ALTER TABLE `instance_bonus_theme_pool` ADD COLUMN `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `description`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_col := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_mission'
      AND COLUMN_NAME = 'difficulty_mask'
);
SET @sql := IF(
    @has_col = 0,
    'ALTER TABLE `instance_bonus_mission` ADD COLUMN `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `objective_type`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_col := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_theme'
      AND COLUMN_NAME = 'difficulty_mask'
);
SET @sql := IF(
    @has_col = 0,
    'ALTER TABLE `instance_bonus_theme` ADD COLUMN `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `briefing_style`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
