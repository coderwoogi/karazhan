CREATE TABLE IF NOT EXISTS `solo_arena_stage_record` (
    `guid` BIGINT UNSIGNED NOT NULL,
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `best_rank` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `best_rank_label` VARCHAR(8) NOT NULL DEFAULT '',
    `best_time_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`, `stage_id`),
    KEY `idx_solo_arena_stage_record_stage` (`stage_id`, `best_rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'rank_value'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `rank_value` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `duration_sec`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'rank_label'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `rank_label` VARCHAR(8) NOT NULL DEFAULT '''' AFTER `rank_value`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'combat_duration_sec'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `combat_duration_sec` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `rank_label`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
