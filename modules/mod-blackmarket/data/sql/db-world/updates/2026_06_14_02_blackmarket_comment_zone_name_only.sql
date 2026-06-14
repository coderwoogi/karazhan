-- Normalize black market spawn point comments to zone names only.
SET NAMES utf8mb4;

UPDATE `blackmarket_spawn_points` bsp
JOIN `areatable_dbc` area ON area.`ID` = bsp.`zone_id`
SET bsp.`comment` = COALESCE(NULLIF(area.`AreaName_Lang_koKR`, ''), NULLIF(area.`AreaName_Lang_enUS`, ''), bsp.`comment`);
