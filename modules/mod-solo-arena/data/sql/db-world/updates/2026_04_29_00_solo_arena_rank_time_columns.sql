ALTER TABLE `solo_arena_stage`
    ADD COLUMN `rank_s_seconds` INT UNSIGNED NOT NULL DEFAULT 45
        AFTER `preparation_ms`,
    ADD COLUMN `rank_a_seconds` INT UNSIGNED NOT NULL DEFAULT 75
        AFTER `rank_s_seconds`,
    ADD COLUMN `rank_b_seconds` INT UNSIGNED NOT NULL DEFAULT 105
        AFTER `rank_a_seconds`,
    ADD COLUMN `rank_c_seconds` INT UNSIGNED NOT NULL DEFAULT 135
        AFTER `rank_b_seconds`;

UPDATE `solo_arena_stage`
SET
    `rank_s_seconds` = CASE
        WHEN `stage_id` BETWEEN 4 AND 6 THEN 360
        ELSE 45
    END,
    `rank_a_seconds` = CASE
        WHEN `stage_id` BETWEEN 4 AND 6 THEN 480
        ELSE 75
    END,
    `rank_b_seconds` = CASE
        WHEN `stage_id` BETWEEN 4 AND 6 THEN 600
        ELSE 105
    END,
    `rank_c_seconds` = CASE
        WHEN `stage_id` BETWEEN 4 AND 6 THEN 720
        ELSE 135
    END;
