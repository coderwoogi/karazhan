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
    `enhance_type` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '강화 타입 (1=밀리, 2=캐스터, 3=힐러, 4=탱커)',
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
    `comment` VARCHAR(255) DEFAULT NULL COMMENT '비고',
    PRIMARY KEY (`enchant_level`, `enhance_type`)
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
(`enchant_level`, `enhance_type`, `spell_id`, `random_property_id`, `success_rate`, `fail_rate`, `gold_cost`, 
 `material_1`, `material_1_count`, `material_2`, `material_2_count`, `material_3`, `material_3_count`, `comment`) 
VALUES
-- 밀리 (1)
(1, 1, 0, 0, 100.0, 0.0, 5, 33568, 5, 0, 0, 0, 0, '밀리 +1, 힘 1 / 민첩 1 / 방관 0, 북풍가죽 x5, 5골드'),
(2, 1, 0, 0, 100.0, 0.0, 10, 33568, 8, 0, 0, 0, 0, '밀리 +2, 힘 2 / 민첩 2 / 방관 1, 북풍가죽 x8, 10골드'),
(3, 1, 0, 0, 95.0, 5.0, 15, 33568, 10, 0, 0, 0, 0, '밀리 +3, 힘 3 / 민첩 3 / 방관 2, 북풍가죽 x10, 15골드'),
(4, 1, 0, 0, 95.0, 5.0, 20, 35623, 2, 0, 0, 0, 0, '밀리 +4, 힘 4 / 민첩 4 / 방관 3, 영원의 바람 x2, 20골드'),
(5, 1, 0, 0, 90.0, 10.0, 30, 35623, 4, 0, 0, 0, 0, '밀리 +5, 힘 5 / 민첩 5 / 방관 4, 영원의 바람 x4, 30골드'),
(6, 1, 0, 0, 85.0, 15.0, 50, 35623, 6, 0, 0, 0, 0, '밀리 +6, 힘 6 / 민첩 6 / 방관 5, 영원의 바람 x6, 50골드'),
(7, 1, 0, 0, 80.0, 20.0, 80, 35623, 10, 0, 0, 0, 0, '밀리 +7, 힘 7 / 민첩 7 / 방관 6, 영원의 바람 x10, 80골드'),
(8, 1, 0, 0, 75.0, 25.0, 120, 43102, 1, 0, 0, 0, 0, '밀리 +8, 힘 8 / 민첩 8 / 방관 7, 얼어붙은 보주 x1, 120골드'),
(9, 1, 0, 0, 70.0, 30.0, 180, 43102, 2, 0, 0, 0, 0, '밀리 +9, 힘 9 / 민첩 9 / 방관 8, 얼어붙은 보주 x2, 180골드'),
(10, 1, 0, 0, 65.0, 35.0, 250, 43102, 3, 0, 0, 0, 0, '밀리 +10, 힘 11 / 민첩 11 / 방관 8, 얼어붙은 보주 x3, 250골드'),
-- 캐스터 (2)
(1, 2, 0, 0, 100.0, 0.0, 5, 33470, 10, 0, 0, 0, 0, '캐스터 +1, 주문력 0 / 지능 1 / 가속 0, 서리매듭 옷감 x10, 5골드'),
(2, 2, 0, 0, 100.0, 0.0, 10, 33470, 15, 0, 0, 0, 0, '캐스터 +2, 주문력 1 / 지능 2 / 가속 1, 서리매듭 옷감 x15, 10골드'),
(3, 2, 0, 0, 95.0, 5.0, 15, 33470, 20, 0, 0, 0, 0, '캐스터 +3, 주문력 2 / 지능 3 / 가속 2, 서리매듭 옷감 x20, 15골드'),
(4, 2, 0, 0, 95.0, 5.0, 20, 36860, 2, 0, 0, 0, 0, '캐스터 +4, 주문력 3 / 지능 4 / 가속 3, 영원의 불 x2, 20골드'),
(5, 2, 0, 0, 90.0, 10.0, 30, 36860, 4, 0, 0, 0, 0, '캐스터 +5, 주문력 4 / 지능 5 / 가속 4, 영원의 불 x4, 30골드'),
(6, 2, 0, 0, 85.0, 15.0, 50, 36860, 6, 0, 0, 0, 0, '캐스터 +6, 주문력 5 / 지능 6 / 가속 5, 영원의 불 x6, 50골드'),
(7, 2, 0, 0, 80.0, 20.0, 80, 36860, 10, 0, 0, 0, 0, '캐스터 +7, 주문력 6 / 지능 8 / 가속 6, 영원의 불 x10, 80골드'),
(8, 2, 0, 0, 75.0, 25.0, 120, 43102, 1, 0, 0, 0, 0, '캐스터 +8, 주문력 7 / 지능 10 / 가속 7, 얼어붙은 보주 x1, 120골드'),
(9, 2, 0, 0, 70.0, 30.0, 180, 43102, 2, 0, 0, 0, 0, '캐스터 +9, 주문력 8 / 지능 11 / 가속 8, 얼어붙은 보주 x2, 180골드'),
(10, 2, 0, 0, 65.0, 35.0, 250, 43102, 3, 0, 0, 0, 0, '캐스터 +10, 주문력 8 / 지능 12 / 가속 8, 얼어붙은 보주 x3, 250골드'),
-- 힐러 (3)
(1, 3, 0, 0, 100.0, 0.0, 5, 33470, 5, 0, 0, 0, 0, '힐러 +1, 지능 1 / 정신 1 / 가속 0, 서리매듭 옷감 x5, 5골드'),
(2, 3, 0, 0, 100.0, 0.0, 10, 33470, 10, 0, 0, 0, 0, '힐러 +2, 지능 2 / 정신 2 / 가속 1, 서리매듭 옷감 x10, 10골드'),
(3, 3, 0, 0, 95.0, 5.0, 15, 33470, 20, 0, 0, 0, 0, '힐러 +3, 지능 3 / 정신 3 / 가속 2, 서리매듭 옷감 x20, 15골드'),
(4, 3, 0, 0, 95.0, 5.0, 20, 35625, 1, 0, 0, 0, 0, '힐러 +4, 지능 4 / 정신 4 / 가속 3, 영원의 생명 x1, 20골드'),
(5, 3, 0, 0, 90.0, 10.0, 30, 35625, 3, 0, 0, 0, 0, '힐러 +5, 지능 6 / 정신 6 / 가속 5, 영원의 생명 x3, 30골드'),
(6, 3, 0, 0, 85.0, 15.0, 50, 35625, 5, 0, 0, 0, 0, '힐러 +6, 지능 7 / 정신 7 / 가속 6, 영원의 생명 x5, 50골드'),
(7, 3, 0, 0, 80.0, 20.0, 80, 35625, 7, 0, 0, 0, 0, '힐러 +7, 지능 9 / 정신 9 / 가속 7, 영원의 생명 x7, 80골드'),
(8, 3, 0, 0, 75.0, 25.0, 100, 43102, 1, 0, 0, 0, 0, '힐러 +8, 지능 10 / 정신 10 / 가속 8, 얼어붙은 보주 x1, 100골드'),
(9, 3, 0, 0, 70.0, 30.0, 150, 43102, 2, 0, 0, 0, 0, '힐러 +9, 지능 12 / 정신 12 / 가속 9, 얼어붙은 보주 x2, 150골드'),
(10, 3, 0, 0, 65.0, 35.0, 200, 43102, 3, 0, 0, 0, 0, '힐러 +10, 지능 15 / 정신 15 / 가속 12, 얼어붙은 보주 x3, 200골드'),
-- 탱커 (4)
(1, 4, 0, 0, 100.0, 0.0, 5, 36912, 3, 0, 0, 0, 0, '탱커 +1, 힘 1 / 체력 1 / 방숙 1, 사로나이트 광석 x3, 5골드'),
(2, 4, 0, 0, 100.0, 0.0, 10, 36912, 5, 0, 0, 0, 0, '탱커 +2, 힘 2 / 체력 2 / 방숙 1, 사로나이트 광석 x5, 10골드'),
(3, 4, 0, 0, 95.0, 5.0, 15, 36912, 8, 0, 0, 0, 0, '탱커 +3, 힘 3 / 체력 3 / 방숙 1, 사로나이트 광석 x8, 15골드'),
(4, 4, 0, 0, 95.0, 5.0, 20, 35624, 1, 0, 0, 0, 0, '탱커 +4, 힘 4 / 체력 4 / 방숙 2, 영원의 대지 x1, 20골드'),
(5, 4, 0, 0, 90.0, 10.0, 30, 35624, 3, 0, 0, 0, 0, '탱커 +5, 힘 5 / 체력 5 / 방숙 2, 영원의 대지 x3, 30골드'),
(6, 4, 0, 0, 85.0, 15.0, 50, 35624, 5, 0, 0, 0, 0, '탱커 +6, 힘 7 / 체력 7 / 방숙 2, 영원의 대지 x5, 50골드'),
(7, 4, 0, 0, 80.0, 20.0, 80, 35624, 7, 0, 0, 0, 0, '탱커 +7, 힘 8 / 체력 8 / 방숙 2, 영원의 대지 x7, 80골드'),
(8, 4, 0, 0, 75.0, 25.0, 100, 43102, 1, 0, 0, 0, 0, '탱커 +8, 힘 9 / 체력 9 / 방숙 3, 얼어붙은 보주 x1, 100골드'),
(9, 4, 0, 0, 70.0, 30.0, 150, 43102, 2, 0, 0, 0, 0, '탱커 +9, 힘 11 / 체력 11 / 방숙 4, 얼어붙은 보주 x2, 150골드'),
(10, 4, 0, 0, 65.0, 35.0, 200, 43102, 3, 0, 0, 0, 0, '탱커 +10, 힘 12 / 체력 12 / 방숙 4, 얼어붙은 보주 x3, 200골드');

-- =====================================================
-- 설치 완료 메시지
-- =====================================================

SELECT 'Karazhan Item Enhancement System - Tables Created Successfully!' AS Message;
SELECT CONCAT('Enchant Slots: ', COUNT(*), ' rows') AS Slots FROM karazhan_enchant_slots;
SELECT CONCAT('Enchant Config: ', COUNT(*), ' rows') AS Config FROM karazhan_enchant_config;
SELECT '' AS Reminder;
SELECT '⚠️ IMPORTANT: Create karazhan_item_enhance table in CHARACTER database!' AS Reminder;
