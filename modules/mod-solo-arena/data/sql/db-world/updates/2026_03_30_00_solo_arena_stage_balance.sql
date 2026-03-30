REPLACE INTO `solo_arena_stage` (
    `stage_id`, `name`, `arena_map_id`,
    `player_x`, `player_y`, `player_z`, `player_o`,
    `bot_x`, `bot_y`, `bot_z`, `bot_o`,
    `health_multiplier`, `damage_multiplier`,
    `attack_time_ms`, `spell_interval_ms`,
    `move_speed_rate`, `preparation_ms`, `enabled`
) VALUES
(1, '그림자 시련 1단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.00, 1.00, 1900, 4200, 1.00, 6000, 1),
(2, '그림자 시련 2단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.10, 1.10, 1900, 4200, 1.00, 6000, 1),
(3, '그림자 시련 3단계', 572, 1294.74, 1584.50, 31.62, 1.66,
    1277.50, 1751.07, 31.61, 4.70, 1.20, 1.20, 1900, 4200, 1.00, 6000, 1);
