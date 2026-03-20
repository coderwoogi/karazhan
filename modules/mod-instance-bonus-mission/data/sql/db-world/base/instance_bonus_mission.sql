DROP TABLE IF EXISTS `instance_bonus_mission_pool`;
CREATE TABLE `instance_bonus_mission_pool` (
    `map_id` INT UNSIGNED NOT NULL,
    `mission_id` INT UNSIGNED NOT NULL,
    `mission_type` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `target_entry` INT UNSIGNED NOT NULL DEFAULT 0,
    `target_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `time_limit_sec` INT UNSIGNED NOT NULL DEFAULT 0,
    `title` VARCHAR(120) NOT NULL DEFAULT '',
    `fallback_announcement` VARCHAR(255) NOT NULL DEFAULT '',
    `reward_item` INT UNSIGNED NOT NULL DEFAULT 0,
    `reward_count` INT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`map_id`, `mission_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `instance_bonus_mission_pool` (
    `map_id`, `mission_id`, `mission_type`, `target_entry`, `target_count`,
    `time_limit_sec`, `title`, `fallback_announcement`, `reward_item`,
    `reward_count`, `enabled`
) VALUES
(
    557, 1, 1, 18315, 12, 600,
    '에테리얼 정화',
    '오늘의 추가 의뢰가 주어졌습니다. 10분 안에 에테리얼 침략자 12명을 처치하십시오.',
    49426, 1, 1
);
