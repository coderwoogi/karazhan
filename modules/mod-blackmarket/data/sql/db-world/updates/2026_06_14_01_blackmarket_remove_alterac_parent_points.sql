-- Remove broad Alterac Mountains parent-area points that collapse into child subareas such as Dalaran Crater.
SET NAMES utf8mb4;

DELETE FROM `blackmarket_spawn_points`
WHERE `id` IN (12068, 12069, 12070)
   OR (`zone_id` = 36 AND `comment` LIKE '[AUTO-Y10]%');
