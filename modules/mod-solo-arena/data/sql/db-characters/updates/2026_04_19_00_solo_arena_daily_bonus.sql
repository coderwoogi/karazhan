CREATE TABLE IF NOT EXISTS `solo_arena_daily_bonus` (
    `guid` BIGINT UNSIGNED NOT NULL,
    `use_date` DATE NOT NULL,
    `bonus_entries` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`, `use_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
