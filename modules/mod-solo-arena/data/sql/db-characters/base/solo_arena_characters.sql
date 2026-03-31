CREATE TABLE IF NOT EXISTS `solo_arena_progress` (
    `guid` BIGINT UNSIGNED NOT NULL,
    `highest_stage_cleared` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `solo_arena_stage_record` (
    `guid` BIGINT UNSIGNED NOT NULL,
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `best_rank` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `best_rank_label` VARCHAR(8) NOT NULL DEFAULT '',
    `best_time_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`, `stage_id`),
    KEY `idx_solo_arena_stage_record_stage` (`stage_id`, `best_rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `solo_arena_run_log` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_uid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `guid` BIGINT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `player_name` VARCHAR(12) NOT NULL DEFAULT '',
    `stage_id` TINYINT UNSIGNED NOT NULL,
    `stage_name` VARCHAR(80) NOT NULL DEFAULT '',
    `result` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `result_label` VARCHAR(16) NOT NULL DEFAULT '',
    `session_state` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `started_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `preparation_ends_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `combat_started_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `combat_ended_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `ended_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `completed_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `failed_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `abandoned_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `duration_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `rank_value` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `rank_label` VARCHAR(8) NOT NULL DEFAULT '',
    `combat_duration_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `arena_map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 617,
    `arena_instance_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `return_map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_solo_arena_run_uid` (`run_uid`),
    KEY `idx_solo_arena_run_log_guid` (`guid`, `started_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `solo_arena_event_log` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_uid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `guid` BIGINT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `player_name` VARCHAR(12) NOT NULL DEFAULT '',
    `stage_id` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `event_type` VARCHAR(32) NOT NULL DEFAULT '',
    `event_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `map_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `arena_instance_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `note` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`id`),
    KEY `idx_solo_arena_event_run` (`run_uid`, `event_at`),
    KEY `idx_solo_arena_event_guid` (`guid`, `event_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `solo_arena_reward_log` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_uid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `guid` BIGINT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `player_name` VARCHAR(12) NOT NULL DEFAULT '',
    `stage_id` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `item_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `item_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `chance` FLOAT NOT NULL DEFAULT 0,
    `grant_status` VARCHAR(16) NOT NULL DEFAULT '',
    `granted_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_solo_arena_reward_run` (`run_uid`, `granted_at`),
    KEY `idx_solo_arena_reward_guid` (`guid`, `granted_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
