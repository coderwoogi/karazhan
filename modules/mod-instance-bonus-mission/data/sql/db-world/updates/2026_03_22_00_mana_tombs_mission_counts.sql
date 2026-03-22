UPDATE `instance_bonus_mission_pool`
SET `target_count` = 4,
    `fallback_announcement` = '오늘의 추가 의뢰가 주어졌습니다. 10분 안에 에테리얼 사술사 4마리를 처치하십시오.'
WHERE `map_id` = 557 AND `mission_id` = 1;

UPDATE `instance_bonus_mission_pool`
SET `target_count` = 8,
    `fallback_announcement` = '오늘의 추가 의뢰가 주어졌습니다. 12분 안에 에테리얼 마술사 8마리를 쓰러뜨리십시오.'
WHERE `map_id` = 557 AND `mission_id` = 2;

UPDATE `instance_bonus_mission_pool`
SET `target_count` = 6,
    `fallback_announcement` = '오늘의 추가 의뢰가 주어졌습니다. 9분 안에 에테리얼 사제 6마리를 처치하십시오.'
WHERE `map_id` = 557 AND `mission_id` = 3;
