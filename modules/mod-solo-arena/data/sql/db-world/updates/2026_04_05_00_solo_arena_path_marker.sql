DELETE FROM `creature_template_model`
WHERE `CreatureID` = 190025;

DELETE FROM `creature_template`
WHERE `entry` = 190025;

INSERT INTO `creature_template` (
    `entry`, `name`, `subname`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
    `speed_walk`, `speed_run`, `scale`, `rank`, `DamageModifier`,
    `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `type`,
    `AIName`, `MovementType`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RegenHealth`,
    `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES
(190025, '시련 경로 마커', 'GM 경로 배치용', 0,
    1, 1, 0, 35, 0,
    1, 1, 0.35, 0, 1,
    2000, 2000, 1, 7,
    '', 0, 1, 1,
    1, 1, 1,
    128, '', 12340);

INSERT INTO `creature_template_model` (
    `CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`,
    `Probability`, `VerifiedBuild`
) VALUES
(190025, 0, 1126, 0.35, 1, 12340);
