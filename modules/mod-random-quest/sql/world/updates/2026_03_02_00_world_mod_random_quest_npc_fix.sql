-- mod-random-quest NPC alignment for code constants
-- Date: 2026-03-02
-- Target DB: acore_world
-- Code expects:
--   NPC_RANDOM_QUEST_GIVER     = 190017 (ScriptName: npc_random_quest_giver)
--   NPC_RANDOM_QUEST_COMPLETER = 190018 (ScriptName: npc_random_quest_completer)

SET NAMES utf8mb4;

-- Ensure templates exist
INSERT IGNORE INTO `creature_template`
(`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `unit_class`, `type`, `ScriptName`)
VALUES
(190017, '무작위 퀘스트 전달자', 'RandomQuest NPC', 63, 63, 35, 3, 1, 7, 'npc_random_quest_giver'),
(190018, '일일 퀘스트 완료', 'The Karazhan', 60, 60, 35, 2, 1, 7, 'npc_random_quest_completer');

-- Force required fields for giver/completer
UPDATE `creature_template`
SET
  `name`       = CONVERT(UNHEX('EBACB4EC9E91EC9C8420ED8098EC8AA4ED8AB820ECA084EB8BACEC9E90') USING utf8mb4),
  `subname`    = 'RandomQuest NPC',
  `faction`    = 35,
  `npcflag`    = 3,
  `minlevel`   = 63,
  `maxlevel`   = 63,
  `unit_class` = 1,
  `type`       = 7,
  `ScriptName` = 'npc_random_quest_giver'
WHERE `entry` = 190017;

UPDATE `creature_template`
SET
  `faction`    = 35,
  `npcflag`    = 2,
  `ScriptName` = 'npc_random_quest_completer'
WHERE `entry` = 190018;

-- If existing placed NPC uses 190019, migrate spawn entry to 190017
UPDATE `creature`
SET `id1` = 190017
WHERE `id1` = 190019;
