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

INSERT INTO `instance_bonus_mission_pool` (
    `map_id`, `mission_id`, `mission_type`, `target_entry`, `target_count`,
    `time_limit_sec`, `title`, `target_label`, `fallback_announcement`,
    `reward_item`, `reward_count`, `enabled`
) VALUES
(
    557, 1, 1, 18315, 12, 600,
    '에테리얼 사술사 토벌',
    '에테리얼 사술사',
    '오늘의 추가 의뢰가 주어졌습니다. 10분 안에 에테리얼 사술사 12마리를 처치하십시오.',
    49426, 1, 1
),
(
    557, 2, 1, 18313, 14, 720,
    '에테리얼 마술사 제압',
    '에테리얼 마술사',
    '오늘의 추가 의뢰가 주어졌습니다. 12분 안에 에테리얼 마술사 14마리를 쓰러뜨리십시오.',
    49426, 1, 1
),
(
    557, 3, 1, 18317, 8, 540,
    '에테리얼 사제 축출',
    '에테리얼 사제',
    '오늘의 추가 의뢰가 주어졌습니다. 9분 안에 에테리얼 사제 8마리를 처치하십시오.',
    49426, 1, 1
),
(
    557, 4, 1, 18341, 1, 900,
    '팬더모니우스 격파',
    '팬더모니우스',
    '오늘의 추가 의뢰가 주어졌습니다. 15분 안에 팬더모니우스를 처치하고 균열을 안정화하십시오.',
    49426, 2, 1
);
