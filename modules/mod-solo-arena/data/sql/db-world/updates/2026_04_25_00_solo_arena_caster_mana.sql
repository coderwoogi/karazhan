SET @dbname := DATABASE();

SET @sql := IF(
    EXISTS(
        SELECT 1
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
          AND TABLE_NAME = 'solo_arena_stage'
          AND COLUMN_NAME = 'caster_mana'
    ),
    'SELECT 1',
    'ALTER TABLE `solo_arena_stage` ADD COLUMN `caster_mana` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `caster_health`'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE `solo_arena_stage`
SET `caster_mana` = 26000
WHERE `stage_id` = 1 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 29000
WHERE `stage_id` = 2 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 32000
WHERE `stage_id` = 3 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 34000
WHERE `stage_id` = 4 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 36000
WHERE `stage_id` = 5 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 39000
WHERE `stage_id` = 6 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 42000
WHERE `stage_id` = 7 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 45000
WHERE `stage_id` = 8 AND `caster_mana` = 0;

UPDATE `solo_arena_stage`
SET `caster_mana` = 48000
WHERE `stage_id` IN (9, 10) AND `caster_mana` = 0;
