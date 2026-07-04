-- 초기지원 날탈 지급을 진영별로 정리합니다.
-- 얼라이언스 종족: 25472
-- 호드 종족: 25477

UPDATE `acore_world`.`playercreateinfo_item`
SET `itemid` = 25472
WHERE `itemid` IN (25472, 25477)
  AND `race` IN (1, 3, 4, 7, 11);

UPDATE `acore_world`.`playercreateinfo_item`
SET `itemid` = 25477
WHERE `itemid` IN (25472, 25477)
  AND `race` IN (2, 5, 6, 8, 10);
