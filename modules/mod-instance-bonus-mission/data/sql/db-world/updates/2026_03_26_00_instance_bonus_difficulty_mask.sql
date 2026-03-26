ALTER TABLE `instance_bonus_mission_pool`
    ADD COLUMN IF NOT EXISTS `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0
    AFTER `mission_type`;

ALTER TABLE `instance_bonus_theme_pool`
    ADD COLUMN IF NOT EXISTS `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0
    AFTER `description`;

ALTER TABLE `instance_bonus_mission`
    ADD COLUMN IF NOT EXISTS `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0
    AFTER `objective_type`;

ALTER TABLE `instance_bonus_theme`
    ADD COLUMN IF NOT EXISTS `difficulty_mask` INT UNSIGNED NOT NULL DEFAULT 0
    AFTER `briefing_style`;
