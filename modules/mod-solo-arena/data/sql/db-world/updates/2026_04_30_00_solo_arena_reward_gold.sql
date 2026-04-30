SET @db_name = DATABASE();

SET @has_reward_gold := (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = @db_name
      AND TABLE_NAME = 'solo_arena_stage_reward'
      AND COLUMN_NAME = 'reward_gold'
);

SET @sql := IF(
    @has_reward_gold = 0,
    'ALTER TABLE `solo_arena_stage_reward` ADD COLUMN `reward_gold` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `item_count`',
    'SELECT 1'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
