CREATE TABLE IF NOT EXISTS `solo_arena_progress` (
    `guid` BIGINT UNSIGNED NOT NULL,
    `highest_stage_cleared` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `solo_arena_run_log` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `guid` BIGINT UNSIGNED NOT NULL,
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `result` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `started_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `ended_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `arena_map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 617,
    `arena_instance_id` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_solo_arena_run_log_guid` (`guid`, `started_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
