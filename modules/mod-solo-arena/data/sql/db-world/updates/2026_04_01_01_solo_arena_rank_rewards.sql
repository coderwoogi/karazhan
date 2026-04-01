SET @db_name = DATABASE();

SET @has_rank_value := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = @db_name
      AND TABLE_NAME = 'solo_arena_stage_reward'
      AND COLUMN_NAME = 'reward_rank_value'
);

SET @sql := IF(
    @has_rank_value = 0,
    'ALTER TABLE `solo_arena_stage_reward` ADD COLUMN `reward_rank_value` TINYINT UNSIGNED NOT NULL DEFAULT 3 AFTER `chance`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_rank_label := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = @db_name
      AND TABLE_NAME = 'solo_arena_stage_reward'
      AND COLUMN_NAME = 'reward_rank_label'
);

SET @sql := IF(
    @has_rank_label = 0,
    'ALTER TABLE `solo_arena_stage_reward` ADD COLUMN `reward_rank_label` VARCHAR(8) NOT NULL DEFAULT ''B'' AFTER `reward_rank_value`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE `solo_arena_stage_reward`
SET `reward_rank_value` = 3
WHERE `reward_rank_value` = 0;

UPDATE `solo_arena_stage_reward`
SET `reward_rank_label` = 'B'
WHERE `reward_rank_label` = '';
