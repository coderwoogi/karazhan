SET @drop_map_index = (
    SELECT IF(
        EXISTS(
            SELECT 1
            FROM INFORMATION_SCHEMA.STATISTICS
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'instance_bonus_map_config'
              AND INDEX_NAME = 'idx_instance_bonus_map_enabled'
        ),
        'ALTER TABLE `instance_bonus_map_config` DROP INDEX `idx_instance_bonus_map_enabled`',
        'SELECT 1'
    )
);
PREPARE stmt FROM @drop_map_index;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @drop_map_publish = (
    SELECT IF(
        EXISTS(
            SELECT 1
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'instance_bonus_map_config'
              AND COLUMN_NAME = 'publish_status'
        ),
        'ALTER TABLE `instance_bonus_map_config` DROP COLUMN `publish_status`',
        'SELECT 1'
    )
);
PREPARE stmt FROM @drop_map_publish;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @add_map_index = (
    SELECT IF(
        EXISTS(
            SELECT 1
            FROM INFORMATION_SCHEMA.TABLES
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'instance_bonus_map_config'
        ),
        'ALTER TABLE `instance_bonus_map_config` ADD KEY `idx_instance_bonus_map_enabled` (`enabled`)',
        'SELECT 1'
    )
);
PREPARE stmt FROM @add_map_index;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @drop_mission_index = (
    SELECT IF(
        EXISTS(
            SELECT 1
            FROM INFORMATION_SCHEMA.STATISTICS
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'instance_bonus_mission'
              AND INDEX_NAME = 'idx_instance_bonus_mission_map'
        ),
        'ALTER TABLE `instance_bonus_mission` DROP INDEX `idx_instance_bonus_mission_map`',
        'SELECT 1'
    )
);
PREPARE stmt FROM @drop_mission_index;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @drop_mission_publish = (
    SELECT IF(
        EXISTS(
            SELECT 1
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'instance_bonus_mission'
              AND COLUMN_NAME = 'publish_status'
        ),
        'ALTER TABLE `instance_bonus_mission` DROP COLUMN `publish_status`',
        'SELECT 1'
    )
);
PREPARE stmt FROM @drop_mission_publish;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @add_mission_index = (
    SELECT IF(
        EXISTS(
            SELECT 1
            FROM INFORMATION_SCHEMA.TABLES
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'instance_bonus_mission'
        ),
        'ALTER TABLE `instance_bonus_mission` ADD KEY `idx_instance_bonus_mission_map` (`map_id`, `enabled`)',
        'SELECT 1'
    )
);
PREPARE stmt FROM @add_mission_index;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
