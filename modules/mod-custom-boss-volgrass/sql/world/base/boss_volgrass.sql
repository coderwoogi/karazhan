/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

-- =====================================================
-- Volgrass Boss - Creature Template
-- =====================================================

DELETE FROM `creature_template` WHERE `entry` = 900000;
INSERT INTO `creature_template`
(`entry`, `modelid1`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`,
`scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `unit_class`,
`unit_flags`, `unit_flags2`, `type`, `type_flags`, `HealthModifier`, `ManaModifier`,
`ArmorModifier`, `AIName`, `ScriptName`)
VALUES
(900000, 26527, '예언자 벨렌', '섬 침공 지휘관', 83, 83, 14,
1.5, 3, 0, 50, 2000, 2, 0, 2048, 2, 72, 300, 100, 1.5,
'', 'boss_volgrass');

-- =====================================================
-- Default Add Template (used by Volgrass.Add.EntryID = 900001)
-- =====================================================

DELETE FROM `creature_template` WHERE `entry` = 900001;
INSERT INTO `creature_template`
(`entry`, `modelid1`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`,
`scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `unit_class`,
`unit_flags`, `unit_flags2`, `type`, `type_flags`, `HealthModifier`, `ManaModifier`,
`ArmorModifier`, `AIName`, `ScriptName`)
VALUES
(900001, 16925, '침공 돌격병', '예언자 벨렌의 선봉대', 80, 80, 14,
1.0, 1, 0, 3, 2000, 1, 0, 0, 7, 0, 8, 0, 1.0,
'SmartAI', '');

-- =====================================================
-- Spawn Location (example coordinates; adjust as needed)
-- =====================================================

DELETE FROM `creature` WHERE `id1` = 900000;
INSERT INTO `creature`
(`guid`, `id1`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`)
VALUES
(9900000, 900000, 0, -8913.23, 510.887, 96.6128, 0.0, 300);

-- =====================================================
-- Timed Spawn Event
-- - Spawn every 2 hours on the hour
-- - Event duration is 20 minutes
-- =====================================================

DELETE FROM `game_event_creature` WHERE `guid` = 9900000 AND `eventEntry` = 250;
DELETE FROM `game_event` WHERE `eventEntry` = 250;

INSERT INTO `game_event`
(`eventEntry`, `start_time`, `end_time`, `occurence`, `length`, `holiday`, `holidayStage`, `description`, `world_event`, `announce`)
VALUES
(250, '2026-01-01 00:00:00', '2035-12-31 23:59:59', 120, 20, 0, 0, 'Prophet Velen Island Invasion (2h)', 1, 1);

INSERT INTO `game_event_creature` (`eventEntry`, `guid`)
VALUES
(250, 9900000);

-- =====================================================
-- Creature Text (all Korean invasion narrative)
-- =====================================================

DELETE FROM `creature_text` WHERE `CreatureID` = 900000;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
VALUES
-- Aggro (GroupID 0)
(900000, 0, 0, '이 섬의 수호자들아, 무릎 꿇어라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Aggro 1'),
(900000, 0, 1, '폭풍 군단이 이 섬을 점령한다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Aggro 2'),
(900000, 0, 2, '도망칠 곳은 없다. 침공은 이미 시작됐다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Aggro 3'),
(900000, 0, 3, '섬의 심장을 꿰뚫고 깃발을 꽂아주마!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Aggro 4'),

-- Phase 2 (GroupID 1)
(900000, 1, 0, '방어선이 흔들린다! 이제 본대가 밀어붙인다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Phase 2 - 1'),
(900000, 1, 1, '섬의 해안은 이미 우리 것이다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Phase 2 - 2'),
(900000, 1, 2, '저항은 끝났다. 침공의 파도를 느껴라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Phase 2 - 3'),

-- Phase 3 (GroupID 2)
(900000, 2, 0, '끝까지 버티겠다면 섬째로 침몰시켜 주마!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Phase 3 - 1'),
(900000, 2, 1, '폭풍의 군주를 막을 수는 없다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Phase 3 - 2'),
(900000, 2, 2, '모든 진지를 파괴하고 이 땅을 불태워라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Phase 3 - 3'),
(900000, 2, 3, '광폭한 폭풍이 너희의 최후가 된다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Phase 3 - 4'),

-- Death Grip (GroupID 3)
(900000, 3, 0, '모두 내 앞으로 끌려와라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Death Grip 1'),
(900000, 3, 1, '도망치지 마라. 여기서 끝낸다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Death Grip 2'),
(900000, 3, 2, '산산이 찢어 주마!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Death Grip 3'),

-- Big Explosion (GroupID 4)
(900000, 4, 0, '폭발해라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Explosion 1'),
(900000, 4, 1, '섬의 대지를 갈라버리겠다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Explosion 2'),
(900000, 4, 2, '방벽째로 날려 주마!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Explosion 3'),

-- Big Explosion Phase 3 (GroupID 5)
(900000, 5, 0, '이 섬의 중심이 무너진다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Explosion P3 - 1'),
(900000, 5, 1, '남김없이 쓸어버려라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Explosion P3 - 2'),
(900000, 5, 2, '광폭한 폭풍 앞에 모두 사라져라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Explosion P3 - 3'),

-- Charged Leap (GroupID 6)
(900000, 6, 0, '도약으로 분쇄해 주마!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Leap 1'),
(900000, 6, 1, '표적 확인. 처형 시작!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Leap 2'),
(900000, 6, 2, '피할 수 있다고 생각했나!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Leap 3'),

-- Super Gale (GroupID 7)
(900000, 7, 0, '초강풍으로 밀어내라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Gale 1'),
(900000, 7, 1, '폭풍이 이 섬의 숨통을 끊는다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Gale 2'),
(900000, 7, 2, '해안선까지 전부 날려버려!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Gale 3'),

-- Summon Adds (GroupID 8)
(900000, 8, 0, '침공 부대여, 전원 돌격!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Summon 1'),
(900000, 8, 1, '지원 병력을 투입한다! 방어선을 찢어라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Summon 2'),
(900000, 8, 2, '이 섬의 깃발을 끌어내려라!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Summon 3'),

-- Kill Player (GroupID 9)
(900000, 9, 0, '하나 정리했다. 다음은 누구냐!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Kill 1'),
(900000, 9, 1, '방어는 무너졌다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Kill 2'),
(900000, 9, 2, '침공군을 막기엔 역부족이다!', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Kill 3'),

-- Death (GroupID 10)
(900000, 10, 0, '이 섬의 저항이... 이렇게 강할 줄은...', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Death 1'),
(900000, 10, 1, '폭풍 군단이... 물러난다...', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Death 2'),
(900000, 10, 2, '다음 침공은... 더 거셀 것이다...', 14, 0, 100, 0, 0, 0, 0, 0, 'Volgrass - Death 3');

-- =====================================================
-- Compatibility: clone creature_text to custom boss entry 190016
-- =====================================================

DELETE FROM `creature_text` WHERE `CreatureID` = 190016;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT
    190016 AS `CreatureID`,
    `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`
FROM `creature_text`
WHERE `CreatureID` = 900000;

