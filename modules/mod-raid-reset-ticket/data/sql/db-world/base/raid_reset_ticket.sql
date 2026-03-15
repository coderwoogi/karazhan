DROP TABLE IF EXISTS `raid_reset_ticket`;
CREATE TABLE `raid_reset_ticket` (
    `item_entry` INT UNSIGNED NOT NULL,
    `map_id` SMALLINT UNSIGNED NOT NULL,
    `enabled` TINYINT(1) NOT NULL DEFAULT 1,
    `comment` VARCHAR(100) NOT NULL DEFAULT '',
    PRIMARY KEY (`item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `raid_reset_ticket`
WHERE `item_entry` BETWEEN 950001 AND 950009;

INSERT INTO `raid_reset_ticket` (`item_entry`, `map_id`, `enabled`, `comment`)
VALUES
(950001, 533, 1, 'Naxxramas'),
(950002, 615, 1, 'The Obsidian Sanctum'),
(950003, 616, 1, 'The Eye of Eternity'),
(950004, 624, 1, 'Vault of Archavon'),
(950005, 603, 1, 'Ulduar'),
(950006, 649, 1, 'Trial of the Crusader'),
(950007, 249, 1, 'Onyxia''s Lair'),
(950008, 631, 1, 'Icecrown Citadel'),
(950009, 724, 1, 'The Ruby Sanctum');

DELETE FROM `item_template_locale`
WHERE `ID` BETWEEN 950001 AND 950009
  AND `locale` = 'koKR';

DELETE FROM `item_template`
WHERE `entry` BETWEEN 950001 AND 950009;

CREATE TEMPORARY TABLE `tmp_raid_reset_item` LIKE `item_template`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950001, `name` = 'Naxxramas Reset Ticket',
    `description` = 'Use: Resets your Naxxramas raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950002, `name` = 'Obsidian Sanctum Reset Ticket',
    `description` = 'Use: Resets your Obsidian Sanctum raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950003, `name` = 'Eye of Eternity Reset Ticket',
    `description` = 'Use: Resets your Eye of Eternity raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950004, `name` = 'Vault of Archavon Reset Ticket',
    `description` = 'Use: Resets your Vault of Archavon raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950005, `name` = 'Ulduar Reset Ticket',
    `description` = 'Use: Resets your Ulduar raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950006, `name` = 'Trial of the Crusader Reset Ticket',
    `description` = 'Use: Resets your Trial of the Crusader raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950007, `name` = 'Onyxia Reset Ticket',
    `description` = 'Use: Resets your Onyxia''s Lair raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950008, `name` = 'Icecrown Citadel Reset Ticket',
    `description` = 'Use: Resets your Icecrown Citadel raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;
TRUNCATE TABLE `tmp_raid_reset_item`;

INSERT INTO `tmp_raid_reset_item` SELECT * FROM `item_template` WHERE `entry` = 35704;
UPDATE `tmp_raid_reset_item`
SET `entry` = 950009, `name` = 'Ruby Sanctum Reset Ticket',
    `description` = 'Use: Resets your Ruby Sanctum raid lockouts.',
    `spellid_1` = 0, `spelltrigger_1` = 0, `spellcharges_1` = 0,
    `spellcooldown_1` = -1, `spellcategory_1` = 0,
    `spellcategorycooldown_1` = -1, `RequiredLevel` = 80,
    `bonding` = 1, `ScriptName` = 'item_raid_reset_ticket';
INSERT INTO `item_template` SELECT * FROM `tmp_raid_reset_item`;

DROP TEMPORARY TABLE `tmp_raid_reset_item`;

INSERT INTO `item_template_locale` (
    `ID`, `locale`, `Name`, `Description`, `VerifiedBuild`
) VALUES
(950001, 'koKR', '낙스라마스 초기화권', '사용 시 낙스라마스 귀속이 초기화됩니다.', 0),
(950002, 'koKR', '흑요석 성소 초기화권', '사용 시 흑요석 성소 귀속이 초기화됩니다.', 0),
(950003, 'koKR', '영원의 눈 초기화권', '사용 시 영원의 눈 귀속이 초기화됩니다.', 0),
(950004, 'koKR', '아카본 석실 초기화권', '사용 시 아카본 석실 귀속이 초기화됩니다.', 0),
(950005, 'koKR', '울두아르 초기화권', '사용 시 울두아르 귀속이 초기화됩니다.', 0),
(950006, 'koKR', '십자군의 시험장 초기화권', '사용 시 십자군의 시험장 귀속이 초기화됩니다.', 0),
(950007, 'koKR', '오닉시아의 둥지 초기화권', '사용 시 오닉시아의 둥지 귀속이 초기화됩니다.', 0),
(950008, 'koKR', '얼음왕관 성채 초기화권', '사용 시 얼음왕관 성채 귀속이 초기화됩니다.', 0),
(950009, 'koKR', '루비 성소 초기화권', '사용 시 루비 성소 귀속이 초기화됩니다.', 0);
