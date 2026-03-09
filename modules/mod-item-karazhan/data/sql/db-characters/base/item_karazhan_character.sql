/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

-- =====================================================
-- Karazhan Item Enhancement System - Character Database
-- =====================================================

-- ⚠️ 이 파일은 CHARACTER 데이터베이스에 실행하세요!

-- =====================================================
-- Table: karazhan_item_enhance
-- Description: 아이템별 강화 정보 저장
-- =====================================================

DROP TABLE IF EXISTS `karazhan_item_enhance`;
CREATE TABLE `karazhan_item_enhance` (
    `item_guid` INT UNSIGNED NOT NULL COMMENT '아이템 GUID (고유 식별자)',
    `owner_guid` INT UNSIGNED NOT NULL COMMENT '소유자 캐릭터 GUID',
    `item_entry` INT UNSIGNED NOT NULL COMMENT '아이템 Entry ID',
    `enhance_level` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '현재 강화 레벨 (0~10)',
    `total_success` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '총 성공 횟수',
    `total_fail` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '총 실패 횟수',
    `total_gold_spent` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '총 소모 골드 (copper 단위)',
    `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '생성 시간',
    `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '수정 시간',
    PRIMARY KEY (`item_guid`),
    INDEX `idx_owner` (`owner_guid`),
    INDEX `idx_item_entry` (`item_entry`),
    INDEX `idx_enhance_level` (`enhance_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='카라잔 아이템 강화 정보';

-- =====================================================
-- 설치 완료 메시지
-- =====================================================

SELECT 'Karazhan Item Enhancement System - Character DB Table Created Successfully!' AS Message;
SELECT 'You can now use the Item Enhancement system!' AS Status;
