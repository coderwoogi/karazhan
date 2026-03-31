UPDATE `solo_arena_stage`
SET `name` = CASE `stage_id`
    WHEN 1 THEN '그림자 시련 1단계'
    WHEN 2 THEN '그림자 시련 2단계'
    WHEN 3 THEN '그림자 시련 3단계'
    WHEN 4 THEN '그림자 시련 4단계'
    WHEN 5 THEN '그림자 시련 5단계'
    WHEN 6 THEN '그림자 시련 6단계'
    WHEN 7 THEN '그림자 시련 7단계'
    WHEN 8 THEN '그림자 시련 8단계'
    WHEN 9 THEN '그림자 시련 9단계'
    WHEN 10 THEN '그림자 시련 10단계'
    ELSE `name`
END
WHERE `stage_id` BETWEEN 1 AND 10;

UPDATE `solo_arena_stage_mechanic`
SET `name` = CASE `stage_id`
    WHEN 1 THEN '시련의 숨결'
    WHEN 2 THEN '뒤틀린 파편'
    WHEN 3 THEN '균열의 제단'
    ELSE `name`
END
WHERE `stage_id` IN (1, 2, 3);
