/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

-- =====================================================
-- Karazhan Item Enhancement System - Database Tables
-- =====================================================

-- =====================================================
-- Table: karazhan_enchant_slots
-- Description: 강화 가능한 슬롯 정의
-- =====================================================

DROP TABLE IF EXISTS `karazhan_enchant_slots`;
CREATE TABLE `karazhan_enchant_slots` (
    `slot_id` TINYINT UNSIGNED NOT NULL COMMENT '인벤토리 타입 (InventoryType)',
    `slot_name` VARCHAR(50) NOT NULL COMMENT '슬롯 이름 (영문)',
    `slot_name_ko` VARCHAR(50) NOT NULL COMMENT '슬롯 이름 (한글)',
    `can_enhance` TINYINT(1) NOT NULL DEFAULT 0 COMMENT '강화 가능 여부',
    `max_enhance_level` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '최대 강화 레벨',
    `enabled` TINYINT(1) NOT NULL DEFAULT 1 COMMENT '활성화 여부',
    `comment` VARCHAR(255) DEFAULT NULL COMMENT '설명',
    PRIMARY KEY (`slot_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='카라잔 강화 슬롯 설정';

-- =====================================================
-- Table: karazhan_enchant_config
-- Description: 강화 레벨별 설정 (확률, 비용, 재료)
-- =====================================================

DROP TABLE IF EXISTS `karazhan_enchant_config`;
CREATE TABLE `karazhan_enchant_config` (
    `enchant_level` TINYINT UNSIGNED NOT NULL COMMENT '강화 레벨',
    `spell_id` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '적용할 인챈트 스펠 ID',
    `random_property_id` INT NOT NULL DEFAULT 0 COMMENT '랜덤 옵션 ID (양수=Property, 음수=Suffix)',
    `success_rate` FLOAT NOT NULL DEFAULT 100.0 COMMENT '성공 확률 (%)',
    `fail_rate` FLOAT NOT NULL DEFAULT 0.0 COMMENT '실패 확률 (%) - 레벨 유지',
    `gold_cost` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '골드 비용 (골드 단위)',
    `material_1` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '재료 1 아이템 ID',
    `material_1_count` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '재료 1 개수',
    `material_2` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '재료 2 아이템 ID',
    `material_2_count` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '재료 2 개수',
    `material_3` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '재료 3 아이템 ID',
    `material_3_count` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '재료 3 개수',
    PRIMARY KEY (`enchant_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='카라잔 강화 레벨별 설정';

-- =====================================================
-- Table: karazhan_item_enhance (Character DB)
-- Description: 아이템별 강화 정보 저장
-- Note: 이 테이블은 Character DB에 생성해야 합니다!
-- =====================================================

-- 이 테이블은 Character DB에 수동으로 생성하세요:
/*
DROP TABLE IF EXISTS `karazhan_item_enhance`;
CREATE TABLE `karazhan_item_enhance` (
    `item_guid` INT UNSIGNED NOT NULL COMMENT '아이템 GUID',
    `owner_guid` INT UNSIGNED NOT NULL COMMENT '소유자 캐릭터 GUID',
    `item_entry` INT UNSIGNED NOT NULL COMMENT '아이템 Entry ID',
    `enhance_level` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '현재 강화 레벨',
    `total_success` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '총 성공 횟수',
    `total_fail` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '총 실패 횟수',
    `total_gold_spent` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '총 소모 골드 (copper)',
    `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '생성 시간',
    `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '수정 시간',
    PRIMARY KEY (`item_guid`),
    INDEX `idx_owner` (`owner_guid`),
    INDEX `idx_item_entry` (`item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='카라잔 아이템 강화 정보';
*/

-- =====================================================
-- Default Data: karazhan_enchant_slots
-- =====================================================

DELETE FROM `karazhan_enchant_slots`;
INSERT INTO `karazhan_enchant_slots` (`slot_id`, `slot_name`, `slot_name_ko`, `can_enhance`, `max_enhance_level`, `enabled`, `comment`) VALUES
-- 강화 가능 슬롯
(1, 'Head', '머리', 1, 10, 1, '투구/모자'),
(3, 'Shoulder', '어깨', 1, 10, 1, '어깨 방어구'),
(4, 'Shirt', '셔츠', 0, 0, 0, '장식용 셔츠 (강화 불가)'),
(5, 'Chest', '가슴', 1, 10, 1, '갑옷/로브'),
(6, 'Waist', '허리', 1, 10, 1, '벨트'),
(7, 'Legs', '다리', 1, 10, 1, '각반/바지'),
(8, 'Feet', '발', 1, 10, 1, '신발/장화'),
(9, 'Wrist', '손목', 1, 10, 1, '팔찌'),
(10, 'Hands', '손', 1, 10, 1, '장갑'),
(11, 'Finger', '손가락', 1, 10, 1, '반지'),
(12, 'Trinket', '장신구', 1, 10, 1, '장신구'),
(13, 'One-Hand', '한손 무기', 1, 10, 1, '한손 무기'),
(14, 'Shield', '방패', 1, 10, 1, '방패'),
(15, 'Ranged', '원거리', 1, 10, 1, '활/총/투척'),
(16, 'Back', '등', 1, 10, 1, '망토'),
(17, 'Two-Hand', '양손 무기', 1, 10, 1, '양손 무기'),
(19, 'Tabard', '휘장', 0, 0, 0, '길드 휘장 (강화 불가)'),
(20, 'Robe', '로브', 1, 10, 1, '로브 (가슴과 동일)'),
(21, 'Main Hand', '주무기', 1, 10, 1, '주 손 무기'),
(22, 'Off Hand', '보조무기', 1, 10, 1, '보조 손 무기'),
(23, 'Holdable', '보조 도구', 1, 10, 1, '오프핸드 아이템'),
(26, 'Relic', '성물', 1, 10, 1, '성물/토템/우상/인장');

-- =====================================================
-- Default Data: karazhan_enchant_config
-- =====================================================

DELETE FROM `karazhan_enchant_config`;
INSERT INTO `karazhan_enchant_config` 
(`enchant_level`, `spell_id`, `random_property_id`, `success_rate`, `fail_rate`, `gold_cost`, 
 `material_1`, `material_1_count`, `material_2`, `material_2_count`, `material_3`, `material_3_count`) 
VALUES
-- +1 강화 (100% 성공)
(1, 0, 0, 100.0, 0.0, 10, 0, 0, 0, 0, 0, 0),
-- +2 강화 (95% 성공)
(2, 0, 0, 95.0, 5.0, 20, 0, 0, 0, 0, 0, 0),
-- +3 강화 (90% 성공)
(3, 0, 0, 90.0, 10.0, 50, 0, 0, 0, 0, 0, 0),
-- +4 강화 (85% 성공)
(4, 0, 0, 85.0, 15.0, 100, 0, 0, 0, 0, 0, 0),
-- +5 강화 (80% 성공)
(5, 0, 0, 80.0, 20.0, 200, 0, 0, 0, 0, 0, 0),
-- +6 강화 (70% 성공, 파괴 가능)
(6, 0, 0, 70.0, 25.0, 500, 0, 0, 0, 0, 0, 0),
-- +7 강화 (60% 성공, 파괴 확률 증가)
(7, 0, 0, 60.0, 30.0, 1000, 0, 0, 0, 0, 0, 0),
-- +8 강화 (50% 성공)
(8, 0, 0, 50.0, 35.0, 2000, 0, 0, 0, 0, 0, 0),
-- +9 강화 (40% 성공)
(9, 0, 0, 40.0, 40.0, 5000, 0, 0, 0, 0, 0, 0),
-- +10 강화 (30% 성공, 최대 레벨)
(10, 0, 0, 30.0, 50.0, 10000, 0, 0, 0, 0, 0, 0);

-- =====================================================
-- 설치 완료 메시지
-- =====================================================

SELECT 'Karazhan Item Enhancement System - Tables Created Successfully!' AS Message;
SELECT CONCAT('Enchant Slots: ', COUNT(*), ' rows') AS Slots FROM karazhan_enchant_slots;
SELECT CONCAT('Enchant Config: ', COUNT(*), ' rows') AS Config FROM karazhan_enchant_config;
SELECT '' AS Reminder;
SELECT '⚠️ IMPORTANT: Create karazhan_item_enhance table in CHARACTER database!' AS Reminder;
