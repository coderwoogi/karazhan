CREATE TABLE IF NOT EXISTS `solo_arena_stage_reward` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `item_entry` INT UNSIGNED NOT NULL,
    `item_count` INT UNSIGNED NOT NULL DEFAULT 1,
    `chance` FLOAT NOT NULL DEFAULT 100,
    `sort_order` INT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `comment` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`id`),
    KEY `idx_solo_arena_stage_reward_stage` (`stage_id`, `enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
