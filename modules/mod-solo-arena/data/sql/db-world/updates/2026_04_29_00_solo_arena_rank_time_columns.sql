SET @dbname := DATABASE();

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'rank_s_seconds'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `rank_s_seconds` INT UNSIGNED NOT NULL DEFAULT 45 AFTER `preparation_ms`'
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
          AND COLUMN_NAME = 'rank_a_seconds'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `rank_a_seconds` INT UNSIGNED NOT NULL DEFAULT 75 AFTER `rank_s_seconds`'
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
          AND COLUMN_NAME = 'rank_b_seconds'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `rank_b_seconds` INT UNSIGNED NOT NULL DEFAULT 105 AFTER `rank_a_seconds`'
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
          AND COLUMN_NAME = 'rank_c_seconds'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `rank_c_seconds` INT UNSIGNED NOT NULL DEFAULT 135 AFTER `rank_b_seconds`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE `solo_arena_stage`
SET
    `rank_s_seconds` = 45,
    `rank_a_seconds` = 75,
    `rank_b_seconds` = 105,
    `rank_c_seconds` = 135
WHERE `stage_id` IN (1, 2, 3, 7, 8, 9, 10)
  AND `rank_s_seconds` = 45
  AND `rank_a_seconds` = 75
  AND `rank_b_seconds` = 105
  AND `rank_c_seconds` = 135;

UPDATE `solo_arena_stage`
SET
    `rank_s_seconds` = 360,
    `rank_a_seconds` = 480,
    `rank_b_seconds` = 600,
    `rank_c_seconds` = 720
WHERE `stage_id` IN (4, 5, 6)
  AND `rank_s_seconds` = 45
  AND `rank_a_seconds` = 75
  AND `rank_b_seconds` = 105
  AND `rank_c_seconds` = 135;
