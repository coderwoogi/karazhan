SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'run_uid'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `run_uid` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `id`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'account_id'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `account_id` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `guid`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'player_name'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `player_name` VARCHAR(12) NOT NULL DEFAULT '''' AFTER `account_id`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'stage_name'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `stage_name` VARCHAR(80) NOT NULL DEFAULT '''' AFTER `stage_id`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'result_label'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `result_label` VARCHAR(16) NOT NULL DEFAULT '''' AFTER `result`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'session_state'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `session_state` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `result_label`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'preparation_ends_at'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `preparation_ends_at` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `started_at`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'combat_started_at'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `combat_started_at` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `preparation_ends_at`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'combat_ended_at'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `combat_ended_at` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `combat_started_at`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'completed_at'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `completed_at` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `ended_at`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'failed_at'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `failed_at` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `completed_at`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'abandoned_at'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `abandoned_at` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `failed_at`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'duration_sec'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `duration_sec` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `abandoned_at`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND COLUMN_NAME = 'return_map_id'
);
SET @sql := IF(@col_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD COLUMN `return_map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0 AFTER `arena_instance_id`',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @idx_exists := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.STATISTICS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'solo_arena_run_log'
      AND INDEX_NAME = 'uk_solo_arena_run_uid'
);
SET @sql := IF(@idx_exists = 0,
    'ALTER TABLE `solo_arena_run_log` ADD UNIQUE KEY `uk_solo_arena_run_uid` (`run_uid`)',
    'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

CREATE TABLE IF NOT EXISTS `solo_arena_event_log` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_uid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `guid` BIGINT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `player_name` VARCHAR(12) NOT NULL DEFAULT '',
    `stage_id` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `event_type` VARCHAR(32) NOT NULL DEFAULT '',
    `event_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `arena_instance_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `note` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`id`),
    KEY `idx_solo_arena_event_run` (`run_uid`, `event_at`),
    KEY `idx_solo_arena_event_guid` (`guid`, `event_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `solo_arena_reward_log` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_uid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `guid` BIGINT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `player_name` VARCHAR(12) NOT NULL DEFAULT '',
    `stage_id` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `item_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `item_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `chance` FLOAT NOT NULL DEFAULT 0,
    `grant_status` VARCHAR(16) NOT NULL DEFAULT '',
    `granted_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_solo_arena_reward_run` (`run_uid`, `granted_at`),
    KEY `idx_solo_arena_reward_guid` (`guid`, `granted_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
