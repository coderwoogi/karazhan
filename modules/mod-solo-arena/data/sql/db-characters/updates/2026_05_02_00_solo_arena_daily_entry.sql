CREATE TABLE IF NOT EXISTS `solo_arena_daily_entry` (
  `account_id` INT UNSIGNED NOT NULL,
  `use_date` DATE NOT NULL,
  `entry_count` INT UNSIGNED NOT NULL DEFAULT 0,
  `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`, `use_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `solo_arena_daily_entry`
  (`account_id`, `use_date`, `entry_count`, `updated_at`)
SELECT
  `account_id`,
  FROM_UNIXTIME(`started_at`, '%Y-%m-%d') AS `use_date`,
  COUNT(*) AS `entry_count`,
  UNIX_TIMESTAMP() AS `updated_at`
FROM `solo_arena_run_log`
WHERE `account_id` <> 0
GROUP BY `account_id`, FROM_UNIXTIME(`started_at`, '%Y-%m-%d')
ON DUPLICATE KEY UPDATE
  `entry_count` = GREATEST(
    `solo_arena_daily_entry`.`entry_count`,
    VALUES(`entry_count`)
  ),
  `updated_at` = VALUES(`updated_at`);
