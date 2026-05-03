-- 암상인 추적기: 사용 스크립트 연결 및 소비 아이템 분류

SET NAMES utf8mb4;

UPDATE `item_template`
SET
    `class` = 0,
    `subclass` = 0,
    `Quality` = 1,
    `Flags` = 64,
    `name` = '암상인 추적기',
    `description` = '사용 시 현재 암상인의 위치 정보를 우편으로 받습니다.',
    `ItemLevel` = 0,
    `InventoryType` = 0,
    `spellid_1` = 46797,
    `spelltrigger_1` = 0,
    `spellcharges_1` = 0,
    `spellcooldown_1` = 5000,
    `spellcategory_1` = 0,
    `spellcategorycooldown_1` = 5000,
    `ScriptName` = 'item_blackmarket_tracker'
WHERE `entry` = 600025;
