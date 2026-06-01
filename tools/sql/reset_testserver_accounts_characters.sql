-- 테스트 서버 계정/캐릭터 전체 초기화 쿼리
-- 작성일: 2026-06-01
--
-- 목적:
--   1. 계정 정보 초기화: acore_auth.account 및 직접 연관 테이블 삭제
--   2. 캐릭터 정보 초기화: acore_characters의 캐릭터/아이템/길드/우편/경매/펫/파티/투기장/로그 삭제
--   3. 커스텀 모듈의 계정/캐릭터 이용 기록 삭제
--
-- 보존 대상:
--   - acore_world의 템플릿/설정 데이터
--   - creature_template, item_template, npc_vendor, quest_template 등 월드 데이터
--   - realmlist, build_info, motd, autobroadcast 등 서버 구동 설정
--   - updates, updates_include 등 DB 업데이트 이력
--   - mail_server_template* 등 시스템 우편 템플릿
--
-- 실행 전 권장:
--   1. worldserver/authserver 종료
--   2. acore_auth, acore_characters 백업
--   3. 필요 시 acore_world도 백업
--
-- 예시:
--   mysql -u root -p < E:\server\azerothcore-wotlk\tools\sql\reset_testserver_accounts_characters.sql

SET @AUTH_DB := 'acore_auth';
SET @CHAR_DB := 'acore_characters';
SET @WORLD_DB := 'acore_world';

DELIMITER $$

DROP PROCEDURE IF EXISTS `karazhan_reset_truncate_if_exists`$$
CREATE PROCEDURE `karazhan_reset_truncate_if_exists`(
    IN p_schema VARCHAR(64),
    IN p_table VARCHAR(64)
)
BEGIN
    IF EXISTS (
        SELECT 1
        FROM `information_schema`.`TABLES`
        WHERE `TABLE_SCHEMA` = p_schema
          AND `TABLE_NAME` = p_table
          AND `TABLE_TYPE` = 'BASE TABLE'
    ) THEN
        SET @reset_sql := CONCAT(
            'TRUNCATE TABLE `',
            REPLACE(p_schema, '`', '``'),
            '`.`',
            REPLACE(p_table, '`', '``'),
            '`'
        );
        PREPARE reset_stmt FROM @reset_sql;
        EXECUTE reset_stmt;
        DEALLOCATE PREPARE reset_stmt;
    END IF;
END$$

DELIMITER ;

SET FOREIGN_KEY_CHECKS = 0;

-- acore_auth: 계정 및 계정 직접 연관 데이터만 초기화
CALL `karazhan_reset_truncate_if_exists`(@AUTH_DB, 'account_access');
CALL `karazhan_reset_truncate_if_exists`(@AUTH_DB, 'account_banned');
CALL `karazhan_reset_truncate_if_exists`(@AUTH_DB, 'account_muted');
CALL `karazhan_reset_truncate_if_exists`(@AUTH_DB, 'realmcharacters');
CALL `karazhan_reset_truncate_if_exists`(@AUTH_DB, 'logs');
CALL `karazhan_reset_truncate_if_exists`(@AUTH_DB, 'logs_ip_actions');
CALL `karazhan_reset_truncate_if_exists`(@AUTH_DB, 'account');

-- acore_characters: 계정 공통 캐릭터 상태
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'account_data');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'account_instance_times');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'account_tutorial');

-- acore_characters: 캐릭터 본체 및 캐릭터 연동 상태
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_account_data');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_achievement');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_achievement_offline_updates');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_achievement_progress');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_action');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_arena_stats');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_aura');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_banned');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_battleground_random');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_brew_of_the_month');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_declinedname');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_entry_point');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_equipmentsets');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_gifts');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_glyphs');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_homebind');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_instance');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_inventory');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_pet');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_pet_declinedname');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_queststatus');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_queststatus_daily');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_queststatus_monthly');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_queststatus_rewarded');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_queststatus_seasonal');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_queststatus_weekly');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_reputation');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_settings');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_skills');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_social');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_spell');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_spell_cooldown');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_stats');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_talent');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'characters');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'corpse');

-- acore_characters: 아이템, 경매, 우편
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'auctionhouse');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'item_instance');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'item_loot_storage');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'item_refund_instance');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'item_soulbound_trade_data');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'mail');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'mail_items');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'mail_server_character');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'recovery_item');

-- acore_characters: 길드
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_bank_eventlog');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_bank_item');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_bank_right');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_bank_tab');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_eventlog');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_member');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_member_withdraw');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'guild_rank');

-- acore_characters: 파티, 투기장, 전장, LFG, 인스턴스 저장
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'arena_team');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'arena_team_member');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'battleground_deserters');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'group_member');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'groups');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'instance');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'instance_reset');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'instance_saved_go_state_data');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'lfg_data');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'pvpstats_battlegrounds');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'pvpstats_players');

-- acore_characters: 달력, 채널, 청원, 펫
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'calendar_events');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'calendar_invites');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'channels');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'channels_bans');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'channels_rights');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'petition');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'petition_sign');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'pet_aura');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'pet_spell');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'pet_spell_cooldown');

-- acore_characters: GM 문의/리포트/로그
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'bugreport');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'gm_subsurvey');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'gm_survey');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'gm_ticket');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'lag_reports');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'log_arena_fights');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'log_arena_memberstats');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'log_encounter');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'log_money');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'quest_tracker');

-- acore_characters: 커스텀 캐릭터/계정 데이터
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'blackmarket_purchase_log');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_hero_stone_rune_box_log');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'character_hero_stone_teleport_runes');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'custom_transmogrification');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'custom_transmogrification_sets');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'custom_unlocked_appearances');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'karazhan_item_enhance');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_daily_bonus');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_daily_bonus_account_tmp');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_daily_entry');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_daily_purchase');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_daily_purchase_account_tmp');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_event_log');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_progress');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_reward_log');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_run_log');
CALL `karazhan_reset_truncate_if_exists`(@CHAR_DB, 'solo_arena_stage_record');

-- acore_world: 커스텀 모듈이 world DB에 저장하는 계정/캐릭터 이용 기록만 초기화
-- 설정 테이블(instance_bonus_map_config, reward_profile, mission, theme 등)은 삭제하지 않는다.
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_event_log');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_llm_log');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_mission_live');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_player_daily_usage');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_reward_log');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_run_history');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_run_live');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_run_member');
CALL `karazhan_reset_truncate_if_exists`(@WORLD_DB, 'instance_bonus_vote_log');

SET FOREIGN_KEY_CHECKS = 1;

DROP PROCEDURE IF EXISTS `karazhan_reset_truncate_if_exists`;

SELECT '테스트 서버 계정/캐릭터 초기화 쿼리 실행 완료' AS `message`;
