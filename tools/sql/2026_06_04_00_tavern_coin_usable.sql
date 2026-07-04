-- 600005 선술집 코인을 Lua on-use 이벤트가 동작하는 사용 가능 아이템으로 변경합니다.
UPDATE `acore_world`.`item_template`
SET
    `Flags` = `Flags` | 64,
    `spellid_1` = 51150,
    `spelltrigger_1` = 0,
    `spellcharges_1` = 0,
    `spellcooldown_1` = 5000,
    `spellcategory_1` = 0,
    `spellcategorycooldown_1` = 5000
WHERE `entry` = 600005;
