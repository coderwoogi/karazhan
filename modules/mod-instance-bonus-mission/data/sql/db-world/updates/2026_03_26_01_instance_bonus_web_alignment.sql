SET @has_map_difficulty_mask := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_map_config'
      AND COLUMN_NAME = 'difficulty_mask'
);
SET @sql := IF(
    @has_map_difficulty_mask = 0,
    'ALTER TABLE `instance_bonus_map_config` ADD COLUMN `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `map_name`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_map_max_concurrent := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_map_config'
      AND COLUMN_NAME = 'max_concurrent_missions'
);
SET @sql := IF(
    @has_map_max_concurrent = 0,
    'ALTER TABLE `instance_bonus_map_config` ADD COLUMN `max_concurrent_missions` TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER `max_party_size`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_map_notes := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_map_config'
      AND COLUMN_NAME = 'notes'
);
SET @sql := IF(
    @has_map_notes = 0,
    'ALTER TABLE `instance_bonus_map_config` ADD COLUMN `notes` TEXT NULL AFTER `max_concurrent_missions`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE `instance_bonus_map_config`
SET `max_concurrent_missions` = IFNULL(NULLIF(`max_concurrent_missions`, 0), IFNULL(`max_active_missions`, 1))
WHERE 1 = 1;

SET @has_reward_profile_map := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_reward_profile'
      AND COLUMN_NAME = 'map_id'
);
SET @sql := IF(
    @has_reward_profile_map = 0,
    'ALTER TABLE `instance_bonus_reward_profile` ADD COLUMN `map_id` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `reward_profile_id`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_reward_profile_key := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_reward_profile'
      AND COLUMN_NAME = 'profile_key'
);
SET @sql := IF(
    @has_reward_profile_key = 0,
    'ALTER TABLE `instance_bonus_reward_profile` ADD COLUMN `profile_key` VARCHAR(80) NOT NULL DEFAULT '''' AFTER `map_id`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE `instance_bonus_reward_profile`
SET `profile_key` = CONCAT('legacy_reward_', `reward_profile_id`)
WHERE IFNULL(`profile_key`, '') = '';

SET @has_reward_profile_key_index := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.STATISTICS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_reward_profile'
      AND INDEX_NAME = 'idx_instance_bonus_reward_profile_key'
);
SET @sql := IF(
    @has_reward_profile_key_index = 0,
    'ALTER TABLE `instance_bonus_reward_profile` ADD UNIQUE KEY `idx_instance_bonus_reward_profile_key` (`profile_key`)',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_reward_item_id := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_reward_profile_item'
      AND COLUMN_NAME = 'item_id'
);
SET @sql := IF(
    @has_reward_item_id = 0,
    'ALTER TABLE `instance_bonus_reward_profile_item` ADD COLUMN `item_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE FIRST',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_reward_sort_order := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_reward_profile_item'
      AND COLUMN_NAME = 'sort_order'
);
SET @sql := IF(
    @has_reward_sort_order = 0,
    'ALTER TABLE `instance_bonus_reward_profile_item` ADD COLUMN `sort_order` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `chance`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_reward_item_updated_at := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'instance_bonus_reward_profile_item'
      AND COLUMN_NAME = 'updated_at'
);
SET @sql := IF(
    @has_reward_item_updated_at = 0,
    'ALTER TABLE `instance_bonus_reward_profile_item` ADD COLUMN `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `sort_order`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
