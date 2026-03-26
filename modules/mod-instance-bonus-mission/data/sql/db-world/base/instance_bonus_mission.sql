DROP TABLE IF EXISTS `instance_bonus_mission_pool`;
CREATE TABLE `instance_bonus_mission_pool` (
    `map_id` INT UNSIGNED NOT NULL,
    `mission_id` INT UNSIGNED NOT NULL,
    `mission_type` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0,
    `target_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `target_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `time_limit_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `title` VARCHAR(120) NOT NULL DEFAULT '',
    `target_label` VARCHAR(120) NOT NULL DEFAULT '',
    `fallback_announcement` VARCHAR(255) NOT NULL DEFAULT '',
    `reward_item` INT UNSIGNED NOT NULL DEFAULT 0,
    `reward_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`map_id`, `mission_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_theme_pool`;
CREATE TABLE `instance_bonus_theme_pool` (
    `map_id` INT UNSIGNED NOT NULL,
    `theme_id` INT UNSIGNED NOT NULL,
    `theme_key` VARCHAR(40) NOT NULL,
    `name` VARCHAR(80) NOT NULL DEFAULT '',
    `description` VARCHAR(255) NOT NULL DEFAULT '',
    `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0,
    `min_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `max_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 5,
    `min_avg_item_level` INT UNSIGNED NOT NULL DEFAULT 0,
    `max_avg_item_level` INT UNSIGNED NOT NULL DEFAULT 9999,
    `required_tank` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `required_healer` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `weight` INT UNSIGNED NOT NULL DEFAULT 100,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`map_id`, `theme_id`),
    UNIQUE KEY `idx_instance_bonus_theme_key` (`map_id`, `theme_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_theme_mission`;
CREATE TABLE `instance_bonus_theme_mission` (
    `map_id` INT UNSIGNED NOT NULL,
    `theme_id` INT UNSIGNED NOT NULL,
    `mission_id` INT UNSIGNED NOT NULL,
    `slot` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `required` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`map_id`, `theme_id`, `mission_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_reward_tier`;
CREATE TABLE `instance_bonus_reward_tier` (
    `map_id` INT UNSIGNED NOT NULL,
    `theme_id` INT UNSIGNED NOT NULL,
    `grade` CHAR(1) NOT NULL,
    `min_score` INT NOT NULL DEFAULT 0,
    `max_score` INT NOT NULL DEFAULT 100,
    `reward_item_1` INT UNSIGNED NOT NULL DEFAULT 0,
    `reward_count_1` INT UNSIGNED NOT NULL DEFAULT 0,
    `reward_item_2` INT UNSIGNED NOT NULL DEFAULT 0,
    `reward_count_2` INT UNSIGNED NOT NULL DEFAULT 0,
    `comment` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`map_id`, `theme_id`, `grade`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_mission_live`;
CREATE TABLE `instance_bonus_mission_live` (
    `instance_id` INT UNSIGNED NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `theme_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `theme_key` VARCHAR(40) NOT NULL DEFAULT '',
    `theme_name` VARCHAR(80) NOT NULL DEFAULT '',
    `mission_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `mission_type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `title` VARCHAR(120) NOT NULL DEFAULT '',
    `target_label` VARCHAR(120) NOT NULL DEFAULT '',
    `target_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `target_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `current_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `time_limit_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `start_time` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `expire_time` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `briefing` VARCHAR(255) NOT NULL DEFAULT '',
    `announcement` VARCHAR(255) NOT NULL DEFAULT '',
    `source` VARCHAR(40) NOT NULL DEFAULT '',
    `completed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `failed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`instance_id`),
    KEY `idx_instance_bonus_live_map` (`map_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
INSERT INTO `instance_bonus_mission_pool` (
    `map_id`, `mission_id`, `mission_type`, `target_entry`, `target_count`,
    `time_limit_sec`, `title`, `target_label`, `fallback_announcement`,
    `reward_item`, `reward_count`, `enabled`
) VALUES
(
    557, 1, 1, 18315, 4, 600,
    '에테리얼 사술사 토벌',
    '에테리얼 사술사',
    '오늘의 추가 의뢰가 주어졌습니다. 10분 안에 에테리얼 사술사 4마리를 처치하십시오.',
    49426, 1, 1
),
(
    557, 2, 1, 18313, 8, 720,
    '에테리얼 마술사 제압',
    '에테리얼 마술사',
    '오늘의 추가 의뢰가 주어졌습니다. 12분 안에 에테리얼 마술사 8마리를 쓰러뜨리십시오.',
    49426, 1, 1
),
(
    557, 3, 1, 18317, 6, 540,
    '에테리얼 사제 축출',
    '에테리얼 사제',
    '오늘의 추가 의뢰가 주어졌습니다. 9분 안에 에테리얼 사제 6마리를 처치하십시오.',
    49426, 1, 1
),
(
    557, 4, 1, 18341, 1, 900,
    '팬더모니우스 격파',
    '팬더모니우스',
    '오늘의 추가 의뢰가 주어졌습니다. 15분 안에 팬더모니우스를 처치하고 균열을 안정화하십시오.',
    49426, 2, 1
),
(
    557, 5, 2, 18341, 1, 0,
    '무사고 돌파',
    '팬더모니우스',
    '오늘의 추가 의뢰가 주어졌습니다. 파티 사망 없이 팬더모니우스를 처치하십시오.',
    49426, 2, 1
);

INSERT INTO `instance_bonus_theme_pool` (
    `map_id`, `theme_id`, `theme_key`, `name`, `description`,
    `min_party_size`, `max_party_size`, `min_avg_item_level`,
    `max_avg_item_level`, `required_tank`, `required_healer`, `weight`,
    `enabled`
) VALUES
(
    557, 1, 'slaughter', '학살형',
    '광역 사냥과 다수 처치에 유리한 파티에 적합한 테마',
    1, 5, 0, 9999, 0, 0, 100, 1
),
(
    557, 2, 'clean_run', '무사고 클리어형',
    '생존력과 안정성이 높은 파티에 적합한 테마',
    1, 5, 0, 9999, 1, 1, 100, 1
),
(
    557, 3, 'speed_run', '속전속결형',
    '아이템 레벨과 화력이 높은 파티에 적합한 시간 압박 테마',
    1, 5, 200, 9999, 0, 0, 100, 1
),
(
    557, 4, 'boss_focus', '보스 집중형',
    '핵심 보스 처치에 초점을 맞춘 테마',
    1, 5, 0, 9999, 0, 0, 100, 1
);

INSERT INTO `instance_bonus_theme_mission` (
    `map_id`, `theme_id`, `mission_id`, `slot`, `required`
) VALUES
(557, 1, 1, 1, 1),
(557, 1, 2, 2, 0),
(557, 1, 3, 3, 0),
(557, 2, 5, 1, 1),
(557, 2, 4, 2, 0),
(557, 3, 2, 1, 1),
(557, 3, 4, 2, 0),
(557, 4, 4, 1, 1),
(557, 4, 3, 2, 0);

INSERT INTO `instance_bonus_reward_tier` (
    `map_id`, `theme_id`, `grade`, `min_score`, `max_score`,
    `reward_item_1`, `reward_count_1`, `reward_item_2`, `reward_count_2`,
    `comment`
) VALUES
(557, 1, 'S', 90, 100, 49426, 3, 43102, 1, '학살형 S 보상'),
(557, 1, 'A', 75, 89, 49426, 2, 43102, 1, '학살형 A 보상'),
(557, 1, 'B', 60, 74, 49426, 1, 0, 0, '학살형 B 보상'),
(557, 1, 'C', 40, 59, 49426, 1, 0, 0, '학살형 C 보상'),
(557, 1, 'D', 0, 39, 0, 0, 0, 0, '학살형 D 보상'),
(557, 2, 'S', 90, 100, 49426, 3, 43102, 1, '무사고 클리어형 S 보상'),
(557, 2, 'A', 75, 89, 49426, 2, 43102, 1, '무사고 클리어형 A 보상'),
(557, 2, 'B', 60, 74, 49426, 1, 0, 0, '무사고 클리어형 B 보상'),
(557, 2, 'C', 40, 59, 49426, 1, 0, 0, '무사고 클리어형 C 보상'),
(557, 2, 'D', 0, 39, 0, 0, 0, 0, '무사고 클리어형 D 보상'),
(557, 3, 'S', 90, 100, 49426, 3, 43102, 1, '속전속결형 S 보상'),
(557, 3, 'A', 75, 89, 49426, 2, 43102, 1, '속전속결형 A 보상'),
(557, 3, 'B', 60, 74, 49426, 1, 0, 0, '속전속결형 B 보상'),
(557, 3, 'C', 40, 59, 49426, 1, 0, 0, '속전속결형 C 보상'),
(557, 3, 'D', 0, 39, 0, 0, 0, 0, '속전속결형 D 보상'),
(557, 4, 'S', 90, 100, 49426, 3, 43102, 1, '보스 집중형 S 보상'),
(557, 4, 'A', 75, 89, 49426, 2, 43102, 1, '보스 집중형 A 보상'),
(557, 4, 'B', 60, 74, 49426, 1, 0, 0, '보스 집중형 B 보상'),
(557, 4, 'C', 40, 59, 49426, 1, 0, 0, '보스 집중형 C 보상'),
(557, 4, 'D', 0, 39, 0, 0, 0, 0, '보스 집중형 D 보상');

DROP TABLE IF EXISTS `instance_bonus_map_config`;
CREATE TABLE `instance_bonus_map_config` (
    `map_id` INT UNSIGNED NOT NULL,
    `map_name` VARCHAR(120) NOT NULL DEFAULT '',
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `allow_llm` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `allow_vote` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `daily_limit_per_player` INT UNSIGNED NOT NULL DEFAULT 0,
    `vote_timeout_sec` INT UNSIGNED NOT NULL DEFAULT 30,
    `default_time_limit_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `min_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `max_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 5,
    `theme_selection_mode` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `mission_selection_mode` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `max_active_missions` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `publish_status` TINYINT UNSIGNED NOT NULL DEFAULT 2,
    `created_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `created_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`map_id`),
    KEY `idx_instance_bonus_map_enabled` (`enabled`, `publish_status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_player_daily_usage`;
CREATE TABLE `instance_bonus_player_daily_usage` (
    `usage_date` DATE NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `guid` BIGINT UNSIGNED NOT NULL,
    `success_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`usage_date`, `map_id`, `guid`),
    KEY `idx_instance_bonus_daily_usage_guid` (`guid`, `usage_date`),
    KEY `idx_instance_bonus_daily_usage_map` (`map_id`, `usage_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_mission`;
CREATE TABLE `instance_bonus_mission` (
    `mission_id` INT UNSIGNED NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `mission_key` VARCHAR(64) NOT NULL DEFAULT '',
    `name` VARCHAR(120) NOT NULL DEFAULT '',
    `description` VARCHAR(255) NOT NULL DEFAULT '',
    `briefing_text` VARCHAR(255) NOT NULL DEFAULT '',
    `mission_type` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `objective_type` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0,
    `target_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `target_label` VARCHAR(120) NOT NULL DEFAULT '',
    `target_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `time_limit_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `failure_condition_type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `required_boss_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `required_before_boss_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `allowed_death_count` INT UNSIGNED NOT NULL DEFAULT 9999,
    `allowed_wipe_count` INT UNSIGNED NOT NULL DEFAULT 9999,
    `reward_profile_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `difficulty_weight` INT UNSIGNED NOT NULL DEFAULT 100,
    `min_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `max_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 5,
    `min_avg_item_level` INT UNSIGNED NOT NULL DEFAULT 0,
    `max_avg_item_level` INT UNSIGNED NOT NULL DEFAULT 9999,
    `required_tank` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `required_healer` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `publish_status` TINYINT UNSIGNED NOT NULL DEFAULT 2,
    `version` INT UNSIGNED NOT NULL DEFAULT 1,
    `created_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `created_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`mission_id`),
    UNIQUE KEY `idx_instance_bonus_mission_key`
        (`map_id`, `mission_key`),
    KEY `idx_instance_bonus_mission_map`
        (`map_id`, `enabled`, `publish_status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_theme`;
CREATE TABLE `instance_bonus_theme` (
    `theme_id` INT UNSIGNED NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `theme_key` VARCHAR(40) NOT NULL DEFAULT '',
    `name` VARCHAR(80) NOT NULL DEFAULT '',
    `description` VARCHAR(255) NOT NULL DEFAULT '',
    `briefing_style` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0,
    `min_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `max_party_size` TINYINT UNSIGNED NOT NULL DEFAULT 5,
    `min_avg_item_level` INT UNSIGNED NOT NULL DEFAULT 0,
    `max_avg_item_level` INT UNSIGNED NOT NULL DEFAULT 9999,
    `required_tank` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `required_healer` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `weight` INT UNSIGNED NOT NULL DEFAULT 100,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `publish_status` TINYINT UNSIGNED NOT NULL DEFAULT 2,
    `version` INT UNSIGNED NOT NULL DEFAULT 1,
    `created_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `created_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`theme_id`),
    UNIQUE KEY `idx_instance_bonus_theme_v2_key`
        (`map_id`, `theme_key`),
    KEY `idx_instance_bonus_theme_v2_map`
        (`map_id`, `enabled`, `publish_status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_theme_mission_link`;
CREATE TABLE `instance_bonus_theme_mission_link` (
    `theme_id` INT UNSIGNED NOT NULL,
    `mission_id` INT UNSIGNED NOT NULL,
    `slot` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `required` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `weight` INT UNSIGNED NOT NULL DEFAULT 100,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`theme_id`, `mission_id`),
    KEY `idx_instance_bonus_theme_mission_slot`
        (`theme_id`, `slot`, `enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_reward_profile`;
CREATE TABLE `instance_bonus_reward_profile` (
    `reward_profile_id` INT UNSIGNED NOT NULL,
    `name` VARCHAR(120) NOT NULL DEFAULT '',
    `description` VARCHAR(255) NOT NULL DEFAULT '',
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `publish_status` TINYINT UNSIGNED NOT NULL DEFAULT 2,
    `version` INT UNSIGNED NOT NULL DEFAULT 1,
    `created_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_by` INT UNSIGNED NOT NULL DEFAULT 0,
    `created_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`reward_profile_id`),
    KEY `idx_instance_bonus_reward_profile_enabled`
        (`enabled`, `publish_status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_reward_profile_item`;
CREATE TABLE `instance_bonus_reward_profile_item` (
    `reward_profile_id` INT UNSIGNED NOT NULL,
    `grade` CHAR(1) NOT NULL,
    `slot` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `item_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `item_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `bind_type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `chance` INT UNSIGNED NOT NULL DEFAULT 10000,
    PRIMARY KEY (`reward_profile_id`, `grade`, `slot`),
    KEY `idx_instance_bonus_reward_profile_grade`
        (`reward_profile_id`, `grade`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_run_live`;
CREATE TABLE `instance_bonus_run_live` (
    `run_id` BIGINT UNSIGNED NOT NULL,
    `instance_id` INT UNSIGNED NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `difficulty` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `status` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `theme_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `theme_key` VARCHAR(40) NOT NULL DEFAULT '',
    `theme_name` VARCHAR(80) NOT NULL DEFAULT '',
    `mission_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `mission_name` VARCHAR(120) NOT NULL DEFAULT '',
    `objective_type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `target_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `target_label` VARCHAR(120) NOT NULL DEFAULT '',
    `target_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `current_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `time_limit_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `start_time` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `expire_time` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `deaths` INT UNSIGNED NOT NULL DEFAULT 0,
    `wipes` INT UNSIGNED NOT NULL DEFAULT 0,
    `briefing` VARCHAR(255) NOT NULL DEFAULT '',
    `announcement` VARCHAR(255) NOT NULL DEFAULT '',
    `source` VARCHAR(40) NOT NULL DEFAULT '',
    `vote_required` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `vote_yes` INT UNSIGNED NOT NULL DEFAULT 0,
    `vote_no` INT UNSIGNED NOT NULL DEFAULT 0,
    `completed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `failed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `failure_reason` VARCHAR(80) NOT NULL DEFAULT '',
    `reward_profile_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`run_id`),
    UNIQUE KEY `idx_instance_bonus_run_live_instance` (`instance_id`),
    KEY `idx_instance_bonus_run_live_map` (`map_id`, `status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_run_history`;
CREATE TABLE `instance_bonus_run_history` (
    `run_id` BIGINT UNSIGNED NOT NULL,
    `instance_id` INT UNSIGNED NOT NULL,
    `map_id` INT UNSIGNED NOT NULL,
    `difficulty` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `leader_guid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `theme_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `theme_key` VARCHAR(40) NOT NULL DEFAULT '',
    `mission_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `mission_name` VARCHAR(120) NOT NULL DEFAULT '',
    `status` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `started_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `ended_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `clear_time_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `deaths` INT UNSIGNED NOT NULL DEFAULT 0,
    `wipes` INT UNSIGNED NOT NULL DEFAULT 0,
    `failure_reason` VARCHAR(80) NOT NULL DEFAULT '',
    `score` INT NOT NULL DEFAULT 0,
    `grade` CHAR(1) NOT NULL DEFAULT '',
    `vote_yes` INT UNSIGNED NOT NULL DEFAULT 0,
    `vote_no` INT UNSIGNED NOT NULL DEFAULT 0,
    `llm_used` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `llm_source` VARCHAR(40) NOT NULL DEFAULT '',
    `reward_profile_id` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`run_id`),
    KEY `idx_instance_bonus_run_history_map` (`map_id`, `started_at`),
    KEY `idx_instance_bonus_run_history_status` (`status`, `ended_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_run_member`;
CREATE TABLE `instance_bonus_run_member` (
    `run_id` BIGINT UNSIGNED NOT NULL,
    `guid` BIGINT UNSIGNED NOT NULL,
    `name` VARCHAR(12) NOT NULL DEFAULT '',
    `class` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `level` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `item_level` INT UNSIGNED NOT NULL DEFAULT 0,
    `role_guess` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `joined_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `left_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `present_on_start` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `present_on_finish` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`run_id`, `guid`),
    KEY `idx_instance_bonus_run_member_guid` (`guid`, `joined_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_vote_log`;
CREATE TABLE `instance_bonus_vote_log` (
    `run_id` BIGINT UNSIGNED NOT NULL,
    `guid` BIGINT UNSIGNED NOT NULL,
    `name` VARCHAR(12) NOT NULL DEFAULT '',
    `vote` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `voted_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `vote_round` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`run_id`, `guid`, `vote_round`),
    KEY `idx_instance_bonus_vote_log_run` (`run_id`, `vote_round`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_reward_log`;
CREATE TABLE `instance_bonus_reward_log` (
    `reward_log_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_id` BIGINT UNSIGNED NOT NULL,
    `guid` BIGINT UNSIGNED NOT NULL,
    `name` VARCHAR(12) NOT NULL DEFAULT '',
    `grade` CHAR(1) NOT NULL DEFAULT '',
    `reward_profile_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `item_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `item_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `granted_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `grant_status` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`reward_log_id`),
    KEY `idx_instance_bonus_reward_log_run` (`run_id`, `guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_event_log`;
CREATE TABLE `instance_bonus_event_log` (
    `event_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_id` BIGINT UNSIGNED NOT NULL,
    `event_type` VARCHAR(40) NOT NULL DEFAULT '',
    `actor_guid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `target_guid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `payload_json` LONGTEXT NOT NULL,
    `created_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`event_id`),
    KEY `idx_instance_bonus_event_log_run` (`run_id`, `created_at`),
    KEY `idx_instance_bonus_event_log_type` (`event_type`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `instance_bonus_llm_log`;
CREATE TABLE `instance_bonus_llm_log` (
    `llm_log_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `run_id` BIGINT UNSIGNED NOT NULL,
    `request_type` VARCHAR(32) NOT NULL DEFAULT '',
    `prompt_json` LONGTEXT NOT NULL,
    `response_json` LONGTEXT NOT NULL,
    `selected_theme_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `selected_mission_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `fallback_used` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `latency_ms` INT UNSIGNED NOT NULL DEFAULT 0,
    `created_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`llm_log_id`),
    KEY `idx_instance_bonus_llm_log_run` (`run_id`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `instance_bonus_map_config` (
    `map_id`, `map_name`, `enabled`, `allow_llm`, `allow_vote`,
    `daily_limit_per_player`, `vote_timeout_sec`, `default_time_limit_sec`, `min_party_size`,
    `max_party_size`, `theme_selection_mode`, `mission_selection_mode`,
    `max_active_missions`, `publish_status`
) VALUES
(
    557, '마나 무덤', 1, 1, 1, 0,
    30, 900, 1,
    5, 1, 1,
    1, 2
);

INSERT INTO `instance_bonus_reward_profile` (
    `reward_profile_id`, `name`, `description`, `enabled`,
    `publish_status`
) VALUES
(1001, '마나 무덤 공통 보상', '마나 무덤 기본 보상 프로파일', 1, 2);

INSERT INTO `instance_bonus_reward_profile_item` (
    `reward_profile_id`, `grade`, `slot`, `item_entry`, `item_count`,
    `bind_type`, `chance`
) VALUES
(1001, 'S', 1, 49426, 3, 0, 10000),
(1001, 'S', 2, 43102, 1, 0, 10000),
(1001, 'A', 1, 49426, 2, 0, 10000),
(1001, 'A', 2, 43102, 1, 0, 10000),
(1001, 'B', 1, 49426, 1, 0, 10000),
(1001, 'C', 1, 49426, 1, 0, 10000);

INSERT INTO `instance_bonus_mission` (
    `mission_id`, `map_id`, `mission_key`, `name`, `description`,
    `briefing_text`, `mission_type`, `objective_type`, `target_entry`,
    `target_label`, `target_count`, `time_limit_sec`,
    `failure_condition_type`, `reward_profile_id`,
    `min_party_size`, `max_party_size`, `enabled`, `publish_status`
) VALUES
(
    1, 557, 'mana_tombs_sorcerer_hunt', '에테리얼 사술사 토벌',
    '에테리얼 사술사를 제한 시간 안에 처치하는 미션',
    '오늘의 추가 의뢰가 주어졌습니다. 10분 안에 에테리얼 사술사 4마리를 처치하십시오.',
    1, 1, 18315,
    '에테리얼 사술사', 4, 600,
    1, 1001,
    1, 5, 1, 2
),
(
    2, 557, 'mana_tombs_magus_hunt', '에테리얼 마술사 제압',
    '에테리얼 마술사를 제한 시간 안에 처치하는 미션',
    '오늘의 추가 의뢰가 주어졌습니다. 12분 안에 에테리얼 마술사 8마리를 쓰러뜨리십시오.',
    1, 1, 18313,
    '에테리얼 마술사', 8, 720,
    1, 1001,
    1, 5, 1, 2
),
(
    3, 557, 'mana_tombs_priest_hunt', '에테리얼 사제 축출',
    '에테리얼 사제를 제한 시간 안에 처치하는 미션',
    '오늘의 추가 의뢰가 주어졌습니다. 9분 안에 에테리얼 사제 6마리를 처치하십시오.',
    1, 1, 18317,
    '에테리얼 사제', 6, 540,
    1, 1001,
    1, 5, 1, 2
),
(
    4, 557, 'mana_tombs_pandemonius', '팬더모니우스 격파',
    '팬더모니우스를 제한 시간 안에 처치하는 미션',
    '오늘의 추가 의뢰가 주어졌습니다. 15분 안에 팬더모니우스를 처치하고 균열을 안정화하십시오.',
    4, 2, 18341,
    '팬더모니우스', 1, 900,
    1, 1001,
    1, 5, 1, 2
),
(
    5, 557, 'mana_tombs_clean_run', '무사고 돌파',
    '파티 사망 없이 팬더모니우스를 처치하는 미션',
    '오늘의 추가 의뢰가 주어졌습니다. 파티 사망 없이 팬더모니우스를 처치하십시오.',
    2, 3, 18341,
    '팬더모니우스', 1, 0,
    2, 1001,
    1, 5, 1, 2
);

INSERT INTO `instance_bonus_theme` (
    `theme_id`, `map_id`, `theme_key`, `name`, `description`,
    `briefing_style`, `min_party_size`, `max_party_size`,
    `min_avg_item_level`, `max_avg_item_level`, `required_tank`,
    `required_healer`, `weight`, `enabled`, `publish_status`
) VALUES
(
    1, 557, 'slaughter', '학살형',
    '광역 사냥과 다수 처치에 유리한 파티에 적합한 테마',
    1, 1, 5,
    0, 9999, 0,
    0, 100, 1, 2
),
(
    2, 557, 'clean_run', '무사고 클리어형',
    '생존력과 안정성이 높은 파티에 적합한 테마',
    1, 1, 5,
    0, 9999, 1,
    1, 100, 1, 2
),
(
    3, 557, 'speed_run', '속전속결형',
    '아이템 레벨과 화력이 높은 파티에 적합한 시간 압박 테마',
    1, 1, 5,
    200, 9999, 0,
    0, 100, 1, 2
),
(
    4, 557, 'boss_focus', '보스 집중형',
    '핵심 보스 처치에 초점을 맞춘 테마',
    1, 1, 5,
    0, 9999, 0,
    0, 100, 1, 2
);

INSERT INTO `instance_bonus_theme_mission_link` (
    `theme_id`, `mission_id`, `slot`, `required`, `weight`, `enabled`
) VALUES
(1, 1, 1, 1, 100, 1),
(1, 2, 2, 0, 100, 1),
(1, 3, 3, 0, 100, 1),
(2, 5, 1, 1, 100, 1),
(2, 4, 2, 0, 100, 1),
(3, 2, 1, 1, 100, 1),
(3, 4, 2, 0, 100, 1),
(4, 4, 1, 1, 100, 1),
(4, 3, 2, 0, 100, 1);


