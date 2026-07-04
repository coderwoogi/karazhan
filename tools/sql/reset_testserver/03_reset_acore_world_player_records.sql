-- 테스트 서버 초기화: acore_world 기록성 DB
-- 대상: 월드 DB 안에 저장된 실행 로그/라이브 상태/투표/사용량
-- 보존: creature_template, item_template, quest_template, npc_vendor, 보상/설정/템플릿 데이터, updates
--
-- 주의:
--   1. worldserver를 먼저 종료해야 합니다.
--   2. 이 파일은 프로시저/DELIMITER를 쓰지 않으므로 SQL 클라이언트에서 직접 실행해도 됩니다.

SET FOREIGN_KEY_CHECKS = 0;

DELETE FROM `acore_world`.`blackmarket_state`; -- 암상인 현재 상태/위치 기록
ALTER TABLE `acore_world`.`blackmarket_state` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_event_log`; -- 인스턴스 보너스 이벤트 로그
ALTER TABLE `acore_world`.`instance_bonus_event_log` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_llm_log`; -- 인스턴스 보너스 LLM 로그
ALTER TABLE `acore_world`.`instance_bonus_llm_log` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_mission_live`; -- 현재 진행 중인 보너스 미션 상태
ALTER TABLE `acore_world`.`instance_bonus_mission_live` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_player_daily_usage`; -- 플레이어 일일 보너스 사용량
ALTER TABLE `acore_world`.`instance_bonus_player_daily_usage` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_reward_log`; -- 보너스 보상 지급 로그
ALTER TABLE `acore_world`.`instance_bonus_reward_log` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_run_history`; -- 보너스 던전 실행 이력
ALTER TABLE `acore_world`.`instance_bonus_run_history` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_run_live`; -- 현재 진행 중인 보너스 던전 상태
ALTER TABLE `acore_world`.`instance_bonus_run_live` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_run_member`; -- 보너스 던전 참가자 기록
ALTER TABLE `acore_world`.`instance_bonus_run_member` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`instance_bonus_vote_log`; -- 보너스 투표 기록
ALTER TABLE `acore_world`.`instance_bonus_vote_log` AUTO_INCREMENT = 1;

DELETE FROM `acore_world`.`custom_enchantment_log`; -- 커스텀 마법부여 로그
ALTER TABLE `acore_world`.`custom_enchantment_log` AUTO_INCREMENT = 1;

SET FOREIGN_KEY_CHECKS = 1;

SELECT 'acore_world 기록성 데이터 초기화 완료' AS `message`;
