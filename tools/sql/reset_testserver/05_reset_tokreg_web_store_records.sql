-- 테스트 서버 초기화: tokreg 웹 상점 DB
-- 대상: 장바구니, 상점 지급 로그, 결제 기록
-- 보존: packages, store, store_category 등 판매 상품/카테고리 정의
--
-- 주의:
--   1. 실서버 결제 기록이면 삭제하면 안 됩니다. 테스트 서버에서만 실행하세요.
--   2. 웹 서버/API 서버를 먼저 종료하거나 점검 모드로 전환해야 합니다.
--   3. 이 파일은 프로시저/DELIMITER를 쓰지 않으므로 SQL 클라이언트에서 직접 실행해도 됩니다.

SET FOREIGN_KEY_CHECKS = 0;

DELETE FROM `tokreg`.`shopping_cart`; -- 상점 장바구니
ALTER TABLE `tokreg`.`shopping_cart` AUTO_INCREMENT = 1;

DELETE FROM `tokreg`.`store_log`; -- 상점 구매/지급 로그
ALTER TABLE `tokreg`.`store_log` AUTO_INCREMENT = 1;

DELETE FROM `tokreg`.`payments`; -- 결제 기록
ALTER TABLE `tokreg`.`payments` AUTO_INCREMENT = 1;

SET FOREIGN_KEY_CHECKS = 1;

SELECT 'tokreg 웹 상점 기록 초기화 완료' AS `message`;
