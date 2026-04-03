UPDATE `solo_arena_stage_mechanic`
SET `object_entry` = 178187
WHERE `stage_id` IN (1, 3);

UPDATE `solo_arena_stage_mechanic`
SET `object_entry` = 20352
WHERE `stage_id` = 2;

DELETE FROM `solo_arena_stage_mechanic`
WHERE `stage_id` IN (4, 5, 6);
