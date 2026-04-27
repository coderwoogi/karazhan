DROP TABLE IF EXISTS `solo_arena_daily_bonus_account_tmp`;
CREATE TABLE `solo_arena_daily_bonus_account_tmp` (
    `account_id` INT UNSIGNED NOT NULL,
    `use_date` DATE NOT NULL,
    `bonus_entries` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`account_id`, `use_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `solo_arena_daily_bonus_account_tmp`
    (`account_id`, `use_date`, `bonus_entries`, `updated_at`)
SELECT
    c.`account`,
    b.`use_date`,
    SUM(b.`bonus_entries`) AS `bonus_entries`,
    MAX(b.`updated_at`) AS `updated_at`
FROM `solo_arena_daily_bonus` b
INNER JOIN `characters` c
    ON c.`guid` = b.`guid`
GROUP BY c.`account`, b.`use_date`;

DROP TABLE `solo_arena_daily_bonus`;
RENAME TABLE `solo_arena_daily_bonus_account_tmp`
    TO `solo_arena_daily_bonus`;

DROP TABLE IF EXISTS `solo_arena_daily_purchase_account_tmp`;
CREATE TABLE `solo_arena_daily_purchase_account_tmp` (
    `account_id` INT UNSIGNED NOT NULL,
    `purchase_date` DATE NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `purchase_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`account_id`, `purchase_date`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `solo_arena_daily_purchase_account_tmp`
    (`account_id`, `purchase_date`, `item_entry`,
     `purchase_count`, `updated_at`)
SELECT
    c.`account`,
    p.`purchase_date`,
    p.`item_entry`,
    SUM(p.`purchase_count`) AS `purchase_count`,
    MAX(p.`updated_at`) AS `updated_at`
FROM `solo_arena_daily_purchase` p
INNER JOIN `characters` c
    ON c.`guid` = p.`guid`
GROUP BY c.`account`, p.`purchase_date`, p.`item_entry`;

DROP TABLE `solo_arena_daily_purchase`;
RENAME TABLE `solo_arena_daily_purchase_account_tmp`
    TO `solo_arena_daily_purchase`;
