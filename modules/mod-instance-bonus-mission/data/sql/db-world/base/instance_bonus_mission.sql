DROP TABLE IF EXISTS `instance_bonus_mission_pool`;
CREATE TABLE `instance_bonus_mission_pool` (
    `map_id` INT UNSIGNED NOT NULL,
    `mission_id` INT UNSIGNED NOT NULL,
    `mission_type` TINYINT UNSIGNED NOT NULL DEFAULT 1,
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


