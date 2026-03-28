REPLACE INTO `solo_arena_stage` (
    `stage_id`, `name`, `arena_map_id`,
    `player_x`, `player_y`, `player_z`, `player_o`,
    `bot_x`, `bot_y`, `bot_z`, `bot_o`,
    `health_multiplier`, `damage_multiplier`,
    `attack_time_ms`, `spell_interval_ms`,
    `move_speed_rate`, `enabled`
) VALUES
(1, '그림자 수련 1단계', 572, 1281.60, 1660.20, 39.96, 1.57,
    1290.10, 1676.10, 39.96, 4.71, 1.00, 1.00, 1900, 4200, 1.00, 1),
(2, '그림자 수련 2단계', 572, 1281.60, 1660.20, 39.96, 1.57,
    1290.10, 1676.10, 39.96, 4.71, 1.35, 1.25, 1600, 3000, 1.10, 1),
(3, '그림자 수련 3단계', 572, 1281.60, 1660.20, 39.96, 1.57,
    1290.10, 1676.10, 39.96, 4.71, 1.75, 1.55, 1300, 2100, 1.20, 1);

DELETE FROM `creature_template` WHERE `entry` IN (910000, 910001);
INSERT INTO `creature_template` (
    `entry`, `name`, `subname`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
    `speed_walk`, `speed_run`, `scale`, `rank`, `DamageModifier`,
    `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `type`,
    `AIName`, `MovementType`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RegenHealth`,
    `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES
(910000, '투기장 감독관', '솔로 아레나', 0,
    80, 80, 2, 35, 1,
    1, 1.14286, 1, 0, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_master', 12340),
(910001, '그림자 전사', '도전자', 0,
    80, 80, 2, 14, 0,
    1, 1.14286, 1, 1, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    0, 'npc_solo_arena_shadow', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (910000, 910001);
INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(910000, 0, 3167, 1, 1, 12340),
(910001, 0, 3167, 1, 1, 12340);
