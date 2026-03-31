DELETE FROM `solo_arena_stage_mechanic`
WHERE `stage_id` IN (1, 2, 3, 4, 5, 6);

REPLACE INTO `solo_arena_stage_mechanic` (
    `stage_id`, `slot_id`, `mechanic_type`, `object_entry`,
    `spawn_x`, `spawn_y`, `spawn_z`, `spawn_o`,
    `spawn_interval_ms`, `duration_ms`,
    `effect_value_1`, `effect_value_2`, `summon_entry`,
    `enabled`, `name`
) VALUES
(1, 1, 1, 184663, 1281.90, 1667.30, 39.96, 0.00, 20000, 15000, 0.20, 0.25, 0, 1, '시련의 숨결'),
(2, 1, 2, 184664, 1289.60, 1668.10, 39.96, 0.00, 22000, 15000, 0.10, 0.08, 190024, 1, '뒤틀린 파편'),
(3, 1, 3, 184663, 1285.80, 1662.80, 39.96, 0.00, 25000, 15000, 18.00, 0.00, 190023, 1, '균열의 제단');
