-- Use typed ItemRandomProperties names for enhanced items.
-- Melee: 2195-2204, Healer: 2205-2214,
-- Caster: 2215-2224, Tank: 2225-2234.
UPDATE `karazhan_enchant_config`
SET `random_property_id` = CASE
    WHEN `enhance_type` = 1 THEN 2194 + `enchant_level`
    WHEN `enhance_type` = 2 THEN 2214 + `enchant_level`
    WHEN `enhance_type` = 3 THEN 2204 + `enchant_level`
    WHEN `enhance_type` = 4 THEN 2224 + `enchant_level`
    ELSE `random_property_id`
END
WHERE `enhance_type` IN (1, 2, 3, 4)
  AND `enchant_level` BETWEEN 1 AND 10;
