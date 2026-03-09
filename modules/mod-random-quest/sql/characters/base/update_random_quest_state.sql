ALTER TABLE `character_random_quest_state`
ADD COLUMN `quests_completed_today` tinyint(3) unsigned DEFAULT 0 AFTER `quests_given_today`;
