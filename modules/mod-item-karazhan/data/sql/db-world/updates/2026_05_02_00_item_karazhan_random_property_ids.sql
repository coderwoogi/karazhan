-- Map item enhancement levels to ItemRandomProperties.dbc entries.
-- 2165 = +1, 2166 = +2, ..., 2174 = +10.
UPDATE `karazhan_enchant_config`
SET `random_property_id` = 2164 + `enchant_level`
WHERE `enhance_type` IN (1, 2, 3, 4)
  AND `enchant_level` BETWEEN 1 AND 10;
