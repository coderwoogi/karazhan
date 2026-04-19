CREATE TABLE IF NOT EXISTS `solo_arena_daily_purchase` (
    `guid` BIGINT UNSIGNED NOT NULL,
    `purchase_date` DATE NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `purchase_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`, `purchase_date`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
