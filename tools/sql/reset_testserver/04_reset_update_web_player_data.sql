-- 테스트 서버 초기화: update 웹 DB
-- 대상: 웹 계정 프로필, 포인트, 카드/와우패스 뽑기 로그, 포인트 상점 주문, 코인시장, 게시글/댓글/문의/알림/로그
-- 보존: 웹 메뉴/권한/설정, 판매 상품 정의, 카드 뽑기 상품 정의, 런처/업데이트 설정, 이벤트/슬라이더 설정
--
-- 주의:
--   1. 웹 서버/API 서버를 먼저 종료하거나 점검 모드로 전환해야 합니다.
--   2. 이 파일은 프로시저/DELIMITER를 쓰지 않으므로 SQL 클라이언트에서 직접 실행해도 됩니다.

SET FOREIGN_KEY_CHECKS = 0;

DELETE FROM `update`.`user_profiles`; -- 웹 계정 프로필
ALTER TABLE `update`.`user_profiles` AUTO_INCREMENT = 1;

DELETE FROM `update`.`user_points`; -- 웹 포인트 현재 잔액
ALTER TABLE `update`.`user_points` AUTO_INCREMENT = 1;

DELETE FROM `update`.`user_point_logs`; -- 웹 포인트 증감 로그
ALTER TABLE `update`.`user_point_logs` AUTO_INCREMENT = 1;

DELETE FROM `update`.`notifications`; -- 웹 알림
ALTER TABLE `update`.`notifications` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_feature_subscriptions`; -- 웹 기능 구독 상태
ALTER TABLE `update`.`web_feature_subscriptions` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_second_account_purchases`; -- 보조 계정 구매 기록
ALTER TABLE `update`.`web_second_account_purchases` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_account_recovery_requests`; -- 계정 복구 요청
ALTER TABLE `update`.`web_account_recovery_requests` AUTO_INCREMENT = 1;

DELETE FROM `update`.`carddraw_draw_logs`; -- 카드 뽑기 로그
ALTER TABLE `update`.`carddraw_draw_logs` AUTO_INCREMENT = 1;

DELETE FROM `update`.`wowpass_draw_logs`; -- 와우패스 뽑기 로그
ALTER TABLE `update`.`wowpass_draw_logs` AUTO_INCREMENT = 1;

DELETE FROM `update`.`point_shop_order_logs`; -- 포인트 상점 주문 로그
ALTER TABLE `update`.`point_shop_order_logs` AUTO_INCREMENT = 1;

DELETE FROM `update`.`point_shop_orders`; -- 포인트 상점 주문
ALTER TABLE `update`.`point_shop_orders` AUTO_INCREMENT = 1;

DELETE FROM `update`.`point_coin_market_listings`; -- 코인시장 등록/거래 데이터
ALTER TABLE `update`.`point_coin_market_listings` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_attachments`; -- 게시판 첨부파일
ALTER TABLE `update`.`web_attachments` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_comments`; -- 게시판 댓글
ALTER TABLE `update`.`web_comments` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_inquiry_messages`; -- 문의 메시지
ALTER TABLE `update`.`web_inquiry_messages` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_post_views`; -- 게시글 조회 기록
ALTER TABLE `update`.`web_post_views` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_promotion_links`; -- 추천/홍보 링크 발급 기록
ALTER TABLE `update`.`web_promotion_links` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_promotion_reward_log`; -- 추천/홍보 보상 지급 로그
ALTER TABLE `update`.`web_promotion_reward_log` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_posts`; -- 게시판 글
ALTER TABLE `update`.`web_posts` AUTO_INCREMENT = 1;

DELETE FROM `update`.`web_board_sequences`; -- 게시판 글 번호 시퀀스
ALTER TABLE `update`.`web_board_sequences` AUTO_INCREMENT = 1;

DELETE FROM `update`.`gm_todos`; -- GM 업무/테스트 메모
ALTER TABLE `update`.`gm_todos` AUTO_INCREMENT = 1;

DELETE FROM `update`.`logs`; -- 웹 로그
ALTER TABLE `update`.`logs` AUTO_INCREMENT = 1;

DELETE FROM `update`.`launcher_announce_history`; -- 런처 공지 발송 이력
ALTER TABLE `update`.`launcher_announce_history` AUTO_INCREMENT = 1;

SET FOREIGN_KEY_CHECKS = 1;

SELECT 'update 웹 계정/기록 데이터 초기화 완료' AS `message`;
