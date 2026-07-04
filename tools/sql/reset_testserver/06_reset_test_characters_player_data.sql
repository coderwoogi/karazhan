-- 선택 실행: test_characters 캐릭터 DB 초기화
-- 대상: test_characters에 남아 있는 캐릭터/계정/아이템/우편/길드/로그/커스텀 상태
-- 보존: updates, updates_include, addons, banned_addons, active_arena_season, reserved_name, profanity_name, mail_server_template*
--
-- 현재 worldserver.conf는 acore_characters를 사용합니다.
-- 이 파일은 DB에 남아 있는 test_characters 데이터까지 비워야 할 때만 실행하세요.

SET FOREIGN_KEY_CHECKS = 0;

-- 계정/캐릭터 기본 상태
DELETE FROM `test_characters`.`account_data`; -- 계정 단위 UI/클라이언트 저장 데이터
DELETE FROM `test_characters`.`account_instance_times`; -- 계정 단위 인스턴스 귀속 시간
DELETE FROM `test_characters`.`account_tutorial`; -- 계정 튜토리얼 상태
DELETE FROM `test_characters`.`character_account_data`; -- 캐릭터별 계정 데이터
DELETE FROM `test_characters`.`character_achievement`; -- 캐릭터 업적
DELETE FROM `test_characters`.`character_achievement_offline_updates`; -- 오프라인 업적 갱신 대기
DELETE FROM `test_characters`.`character_achievement_progress`; -- 업적 진행도
DELETE FROM `test_characters`.`character_action`; -- 액션바
DELETE FROM `test_characters`.`character_arena_stats`; -- 투기장 통계
DELETE FROM `test_characters`.`character_aura`; -- 버프/디버프
DELETE FROM `test_characters`.`character_banned`; -- 캐릭터 제재
DELETE FROM `test_characters`.`character_battleground_random`; -- 무작위 전장 상태
DELETE FROM `test_characters`.`character_brew_of_the_month`; -- 이달의 맥주 상태
DELETE FROM `test_characters`.`character_declinedname`; -- 캐릭터 이름 변형
DELETE FROM `test_characters`.`character_entry_point`; -- 인스턴스/전장 진입 전 위치
DELETE FROM `test_characters`.`character_equipmentsets`; -- 장비 세트
DELETE FROM `test_characters`.`character_gifts`; -- 선물/보상 대기
DELETE FROM `test_characters`.`character_glyphs`; -- 문양
DELETE FROM `test_characters`.`character_homebind`; -- 귀환 위치
DELETE FROM `test_characters`.`character_instance`; -- 캐릭터 인스턴스 귀속
DELETE FROM `test_characters`.`character_inventory`; -- 인벤토리/장착 연결
DELETE FROM `test_characters`.`character_pet`; -- 펫 본체
DELETE FROM `test_characters`.`character_pet_declinedname`; -- 펫 이름 변형
DELETE FROM `test_characters`.`character_queststatus`; -- 퀘스트 진행
DELETE FROM `test_characters`.`character_queststatus_daily`; -- 일일 퀘스트
DELETE FROM `test_characters`.`character_queststatus_monthly`; -- 월간 퀘스트
DELETE FROM `test_characters`.`character_queststatus_rewarded`; -- 보상 수령 퀘스트
DELETE FROM `test_characters`.`character_queststatus_seasonal`; -- 시즌 퀘스트
DELETE FROM `test_characters`.`character_queststatus_weekly`; -- 주간 퀘스트
DELETE FROM `test_characters`.`character_reputation`; -- 평판
DELETE FROM `test_characters`.`character_settings`; -- 캐릭터 설정
DELETE FROM `test_characters`.`character_skills`; -- 스킬/전문기술
DELETE FROM `test_characters`.`character_social`; -- 친구/차단
DELETE FROM `test_characters`.`character_spell`; -- 배운 주문
DELETE FROM `test_characters`.`character_spell_cooldown`; -- 주문 쿨다운
DELETE FROM `test_characters`.`character_stats`; -- 능력치 캐시
DELETE FROM `test_characters`.`character_talent`; -- 특성
DELETE FROM `test_characters`.`characters`; -- 캐릭터 본체
DELETE FROM `test_characters`.`corpse`; -- 시체/부활 정보

-- 아이템, 우편, 경매
DELETE FROM `test_characters`.`auctionhouse`; -- 경매장
DELETE FROM `test_characters`.`item_instance`; -- 개별 아이템
DELETE FROM `test_characters`.`item_loot_storage`; -- 아이템 내부 루팅
DELETE FROM `test_characters`.`item_refund_instance`; -- 환불 아이템
DELETE FROM `test_characters`.`item_soulbound_trade_data`; -- 귀속 거래 상태
DELETE FROM `test_characters`.`mail`; -- 우편
DELETE FROM `test_characters`.`mail_items`; -- 우편 첨부
DELETE FROM `test_characters`.`mail_server_character`; -- 서버 우편 발송 기록
DELETE FROM `test_characters`.`recovery_item`; -- 복구 아이템

-- 길드, 파티, 전장, 인스턴스
DELETE FROM `test_characters`.`guild`; -- 길드
DELETE FROM `test_characters`.`guild_bank_eventlog`; -- 길드 은행 로그
DELETE FROM `test_characters`.`guild_bank_item`; -- 길드 은행 아이템
DELETE FROM `test_characters`.`guild_bank_right`; -- 길드 은행 권한
DELETE FROM `test_characters`.`guild_bank_tab`; -- 길드 은행 탭
DELETE FROM `test_characters`.`guild_eventlog`; -- 길드 이벤트 로그
DELETE FROM `test_characters`.`guild_member`; -- 길드원
DELETE FROM `test_characters`.`guild_member_withdraw`; -- 길드원 출금 기록
DELETE FROM `test_characters`.`guild_rank`; -- 길드 등급
DELETE FROM `test_characters`.`arena_team`; -- 투기장 팀
DELETE FROM `test_characters`.`arena_team_member`; -- 투기장 팀원
DELETE FROM `test_characters`.`battleground_deserters`; -- 전장 탈영
DELETE FROM `test_characters`.`group_member`; -- 파티원
DELETE FROM `test_characters`.`groups`; -- 파티
DELETE FROM `test_characters`.`instance`; -- 인스턴스 저장
DELETE FROM `test_characters`.`instance_reset`; -- 인스턴스 리셋 예약
DELETE FROM `test_characters`.`instance_saved_go_state_data`; -- 인스턴스 오브젝트 상태
DELETE FROM `test_characters`.`lfg_data`; -- LFG 상태
DELETE FROM `test_characters`.`pvpstats_battlegrounds`; -- 전장 통계
DELETE FROM `test_characters`.`pvpstats_players`; -- 전장 플레이어 통계

-- 달력, 채널, 펫, 신고/로그
DELETE FROM `test_characters`.`calendar_events`; -- 달력 이벤트
DELETE FROM `test_characters`.`calendar_invites`; -- 달력 초대
DELETE FROM `test_characters`.`channels`; -- 채널
DELETE FROM `test_characters`.`channels_bans`; -- 채널 차단
DELETE FROM `test_characters`.`channels_rights`; -- 채널 권한
DELETE FROM `test_characters`.`petition`; -- 청원서
DELETE FROM `test_characters`.`petition_sign`; -- 청원서 서명
DELETE FROM `test_characters`.`pet_aura`; -- 펫 버프
DELETE FROM `test_characters`.`pet_spell`; -- 펫 주문
DELETE FROM `test_characters`.`pet_spell_cooldown`; -- 펫 쿨다운
DELETE FROM `test_characters`.`bugreport`; -- 버그 신고
DELETE FROM `test_characters`.`gm_subsurvey`; -- GM 설문 상세
DELETE FROM `test_characters`.`gm_survey`; -- GM 설문
DELETE FROM `test_characters`.`gm_ticket`; -- GM 티켓
DELETE FROM `test_characters`.`lag_reports`; -- 렉 신고
DELETE FROM `test_characters`.`log_arena_fights`; -- 투기장 경기 로그
DELETE FROM `test_characters`.`log_arena_memberstats`; -- 투기장 멤버 로그
DELETE FROM `test_characters`.`log_encounter`; -- 인카운터 로그
DELETE FROM `test_characters`.`log_money`; -- 골드 로그
DELETE FROM `test_characters`.`quest_tracker`; -- 퀘스트 추적 로그
-- 월드 런타임 저장 상태
DELETE FROM `test_characters`.`creature_respawn`; -- 크리처 리스폰 저장 상태
DELETE FROM `test_characters`.`gameobject_respawn`; -- 오브젝트 리스폰 저장 상태
DELETE FROM `test_characters`.`game_event_condition_save`; -- 월드 이벤트 조건 저장
DELETE FROM `test_characters`.`game_event_save`; -- 월드 이벤트 저장
DELETE FROM `test_characters`.`pool_quest_save`; -- 퀘스트 풀 저장
DELETE FROM `test_characters`.`warden_action`; -- 와든 기록
DELETE FROM `test_characters`.`world_state`; -- 월드 상태
DELETE FROM `test_characters`.`worldstates`; -- 월드 상태

-- 커스텀/웹 연동 기록
DELETE FROM `test_characters`.`blackmarket_purchase_log`; -- 암상인 구매 기록
DELETE FROM `test_characters`.`custom_item_enchant`; -- 구버전 강화 상태
DELETE FROM `test_characters`.`custom_item_enhancement`; -- 강화 상태
DELETE FROM `test_characters`.`custom_transmogrification`; -- 형상변환 적용
DELETE FROM `test_characters`.`custom_transmogrification_sets`; -- 형상변환 세트
DELETE FROM `test_characters`.`custom_unlocked_appearances`; -- 형상 해금
DELETE FROM `test_characters`.`item_enhancement_log`; -- 강화 로그
DELETE FROM `test_characters`.`karazhan_enhance_log`; -- 카라잔 강화 로그
DELETE FROM `test_characters`.`karazhan_item_enhance`; -- 카라잔 강화 상태
DELETE FROM `test_characters`.`playtime_reward_log`; -- 플레이타임 보상 로그
DELETE FROM `test_characters`.`web_mail_log`; -- 웹 우편 발송 로그

-- AUTO_INCREMENT 초기화
ALTER TABLE `test_characters`.`characters` AUTO_INCREMENT = 1;
ALTER TABLE `test_characters`.`item_instance` AUTO_INCREMENT = 1;
ALTER TABLE `test_characters`.`mail` AUTO_INCREMENT = 1;
ALTER TABLE `test_characters`.`guild` AUTO_INCREMENT = 1;
ALTER TABLE `test_characters`.`groups` AUTO_INCREMENT = 1;
ALTER TABLE `test_characters`.`instance` AUTO_INCREMENT = 1;

SET FOREIGN_KEY_CHECKS = 1;

SELECT 'test_characters 캐릭터/계정 상태 초기화 완료' AS `message`;
