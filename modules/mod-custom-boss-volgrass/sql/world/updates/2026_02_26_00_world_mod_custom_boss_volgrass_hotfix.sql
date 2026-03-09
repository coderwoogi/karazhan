-- mod-custom-boss-volgrass hotfix
-- Date: 2026-02-26
-- Purpose:
-- 1) Ensure entry 900001 is not used
-- 2) Ensure summon add template 190019 exists
-- 3) Ensure creature_text exists for live boss entry 190016

-- -----------------------------------------------------------------------------
-- 1) Ensure entry 900001 is not used
-- -----------------------------------------------------------------------------
DELETE FROM `creature_template` WHERE `entry` = 900001;

-- -----------------------------------------------------------------------------
-- 2) Ensure summon add template 190019 exists
-- -----------------------------------------------------------------------------
DELETE FROM `creature_template` WHERE `entry` = 190019;
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `unit_class`, `unit_flags`, `unit_flags2`, `type`, `type_flags`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `AIName`, `ScriptName`)
VALUES
(190019, 'Invasion Vanguard', 'Prophet Velen Reinforcement', 82, 82, 14, 1.0, 1, 0, 3.2, 2000, 1, 0, 0, 7, 0, 10, 0, 1.2, 'SmartAI', '');

-- -----------------------------------------------------------------------------
-- 3) Ensure creature_text for entry 190016
-- -----------------------------------------------------------------------------
DELETE FROM `creature_text` WHERE `CreatureID` = 190016;

-- Preferred path: clone full text set from module base entry 900000
INSERT INTO `creature_text`
(`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT
    190016,
    `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`
FROM `creature_text`
WHERE `CreatureID` = 900000;

-- Fallback: if 900000 text source does not exist, insert at least one line per group used by script
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 0, 0, '이 섬의 수호자들아, 무릎 꿇어라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Aggro fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 0);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 1, 0, '방어선이 흔들린다! 이제 본대가 밀어붙인다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Phase2 fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 1);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 2, 0, '끝까지 버티겠다면 섬째로 침몰시켜 주마!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Phase3 fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 2);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 3, 0, '모두 내 앞으로 끌려와라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - DeathGrip fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 3);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 4, 0, '폭발해라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Explosion fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 4);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 5, 0, '이 섬의 중심이 무너진다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - ExplosionP3 fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 5);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 6, 0, '도약으로 분쇄해 주마!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Leap fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 6);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 7, 0, '초강풍으로 밀어내라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Gale fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 7);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 8, 0, '침공 부대여, 전원 돌격!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Summon fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 8);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 9, 0, '하나 정리했다. 다음은 누구냐!', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Kill fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 9);

INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 190016, 10, 0, '이 섬의 저항이... 이렇게 강할 줄은...', 14, 0, 100, 0, 0, 0, 0, 0, 'Velen - Death fallback'
WHERE NOT EXISTS (SELECT 1 FROM `creature_text` WHERE `CreatureID` = 190016 AND `GroupID` = 10);

