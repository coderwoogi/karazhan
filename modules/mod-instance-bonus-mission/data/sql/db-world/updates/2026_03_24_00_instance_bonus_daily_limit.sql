ALTER TABLE `instance_bonus_map_config`
    ADD COLUMN `daily_limit_per_player` INT UNSIGNED NOT NULL DEFAULT 0
    AFTER `allow_vote`;

CREATE TABLE IF NOT EXISTS `instance_bonus_player_daily_usage` (
    `usage_date` DATE NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `guid` BIGINT UNSIGNED NOT NULL,
    `success_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`usage_date`, `map_id`, `guid`),
    KEY `idx_instance_bonus_daily_usage_guid` (`guid`, `usage_date`),
    KEY `idx_instance_bonus_daily_usage_map` (`map_id`, `usage_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

UPDATE `instance_bonus_map_config`
SET `daily_limit_per_player` = 0
WHERE `map_id` = 557 AND `daily_limit_per_player` IS NULL;
