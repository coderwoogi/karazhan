DELETE FROM `karazhan_enchant_slots`
WHERE `slot_id` = 2;

INSERT INTO `karazhan_enchant_slots`
(`slot_id`, `slot_name`, `slot_name_ko`, `can_enhance`,
 `max_enhance_level`, `enabled`, `comment`)
VALUES
(2, 'Neck', '목걸이', 1, 10, 1, '목걸이');
