-- 테스트 서버 초기화: acore_auth 계정 DB
-- 대상: 계정 본체, 계정 권한/제재, 로그인/서버 런타임 기록, 계정별 렐름 캐릭터 캐시
-- 보존: realmlist, build_info, motd, ip_banned, rbac 기본 권한 정의, web_menu_permissions, updates
--
-- 주의:
--   1. authserver/worldserver를 먼저 종료해야 합니다.
--   2. 이 파일은 프로시저/DELIMITER를 쓰지 않으므로 SQL 클라이언트에서 직접 실행해도 됩니다.

SET FOREIGN_KEY_CHECKS = 0;

DELETE FROM `acore_auth`.`account_access`; -- 계정 GM 권한/접근 레벨
ALTER TABLE `acore_auth`.`account_access` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`account_banned`; -- 계정 차단 기록
ALTER TABLE `acore_auth`.`account_banned` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`account_muted`; -- 계정 채팅 제재 기록
ALTER TABLE `acore_auth`.`account_muted` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`rbac_account_permissions`; -- 계정별 RBAC 권한 부여 상태
ALTER TABLE `acore_auth`.`rbac_account_permissions` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`realmcharacters`; -- 계정별 렐름 캐릭터 수 캐시
ALTER TABLE `acore_auth`.`realmcharacters` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`logs`; -- 로그인 DB 일반 로그
ALTER TABLE `acore_auth`.`logs` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`logs_ip_actions`; -- 계정/IP 행동 로그
ALTER TABLE `acore_auth`.`logs_ip_actions` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`uptime`; -- 서버 가동 시간 기록
ALTER TABLE `acore_auth`.`uptime` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`acore_cms_subscriptions`; -- 커스텀 구독 모듈 계정 구독 상태
ALTER TABLE `acore_auth`.`acore_cms_subscriptions` AUTO_INCREMENT = 1;

DELETE FROM `acore_auth`.`account`; -- 로그인 계정 본체
ALTER TABLE `acore_auth`.`account` AUTO_INCREMENT = 1;

SET FOREIGN_KEY_CHECKS = 1;

SELECT 'acore_auth 계정 초기화 완료' AS `message`;
