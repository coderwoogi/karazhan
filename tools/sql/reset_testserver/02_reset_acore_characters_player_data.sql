-- 테스트 서버 초기화: acore_characters 캐릭터 DB
-- 대상: 캐릭터/계정 진행도, 인벤토리, 우편, 경매, 길드, 파티, 펫, 로그, 월드 런타임 상태, 커스텀 모듈 상태
-- 보존: updates, updates_include, addons, banned_addons, active_arena_season, reserved_name, profanity_name, mail_server_template*
--
-- 주의:
--   1. worldserver/authserver를 먼저 종료해야 합니다.
--   2. 이 파일은 프로시저/DELIMITER를 쓰지 않으므로 SQL 클라이언트에서 직접 실행해도 됩니다.

SET FOREIGN_KEY_CHECKS = 0;

-- 계정 공통 캐릭터 상태
DELETE FROM `acore_characters`.`account_data`; -- 계정 단위 UI/클라이언트 저장 데이터
ALTER TABLE `acore_characters`.`account_data` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`account_instance_times`; -- 계정 단위 인스턴스 귀속 시간
ALTER TABLE `acore_characters`.`account_instance_times` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`account_tutorial`; -- 계정 튜토리얼 완료 상태
ALTER TABLE `acore_characters`.`account_tutorial` AUTO_INCREMENT = 1;

-- 캐릭터 본체 및 진행 상태
DELETE FROM `acore_characters`.`character_account_data`; -- 캐릭터별 계정 데이터
ALTER TABLE `acore_characters`.`character_account_data` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_achievement`; -- 캐릭터 업적 완료
ALTER TABLE `acore_characters`.`character_achievement` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_achievement_offline_updates`; -- 오프라인 업적 갱신 대기
ALTER TABLE `acore_characters`.`character_achievement_offline_updates` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_achievement_progress`; -- 업적 진행도
ALTER TABLE `acore_characters`.`character_achievement_progress` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_action`; -- 액션바 배치
ALTER TABLE `acore_characters`.`character_action` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_arena_stats`; -- 캐릭터 투기장 통계
ALTER TABLE `acore_characters`.`character_arena_stats` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_aura`; -- 캐릭터 버프/디버프 저장
ALTER TABLE `acore_characters`.`character_aura` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_banned`; -- 캐릭터 차단 기록
ALTER TABLE `acore_characters`.`character_banned` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_battleground_random`; -- 무작위 전장 보상 상태
ALTER TABLE `acore_characters`.`character_battleground_random` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_brew_of_the_month`; -- 이달의 맥주 클럽 상태
ALTER TABLE `acore_characters`.`character_brew_of_the_month` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_declinedname`; -- 캐릭터 이름 변형 데이터
ALTER TABLE `acore_characters`.`character_declinedname` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_entry_point`; -- 인스턴스/전장 진입 전 위치
ALTER TABLE `acore_characters`.`character_entry_point` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_equipmentsets`; -- 장비 세트 저장
ALTER TABLE `acore_characters`.`character_equipmentsets` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_gifts`; -- 캐릭터 선물/보상 대기
ALTER TABLE `acore_characters`.`character_gifts` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_glyphs`; -- 문양 장착 상태
ALTER TABLE `acore_characters`.`character_glyphs` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_homebind`; -- 귀환석 위치
ALTER TABLE `acore_characters`.`character_homebind` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_instance`; -- 캐릭터 인스턴스 귀속
ALTER TABLE `acore_characters`.`character_instance` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_inventory`; -- 캐릭터 인벤토리/장착 아이템 연결
ALTER TABLE `acore_characters`.`character_inventory` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_pet`; -- 소환수/펫 본체
ALTER TABLE `acore_characters`.`character_pet` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_pet_declinedname`; -- 펫 이름 변형 데이터
ALTER TABLE `acore_characters`.`character_pet_declinedname` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_queststatus`; -- 퀘스트 진행 상태
ALTER TABLE `acore_characters`.`character_queststatus` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_queststatus_daily`; -- 일일 퀘스트 완료 상태
ALTER TABLE `acore_characters`.`character_queststatus_daily` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_queststatus_monthly`; -- 월간 퀘스트 완료 상태
ALTER TABLE `acore_characters`.`character_queststatus_monthly` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_queststatus_rewarded`; -- 보상 수령 완료 퀘스트
ALTER TABLE `acore_characters`.`character_queststatus_rewarded` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_queststatus_seasonal`; -- 시즌 퀘스트 완료 상태
ALTER TABLE `acore_characters`.`character_queststatus_seasonal` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_queststatus_weekly`; -- 주간 퀘스트 완료 상태
ALTER TABLE `acore_characters`.`character_queststatus_weekly` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_random_quest_log`; -- 커스텀 랜덤 퀘스트 지급/거절 로그
ALTER TABLE `acore_characters`.`character_random_quest_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_random_quest_state`; -- 커스텀 랜덤 퀘스트 캐릭터별 일일 상태
ALTER TABLE `acore_characters`.`character_random_quest_state` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_reputation`; -- 평판 상태
ALTER TABLE `acore_characters`.`character_reputation` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_settings`; -- 캐릭터별 커스텀 설정
ALTER TABLE `acore_characters`.`character_settings` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_skills`; -- 전문기술/무기숙련 등 스킬 상태
ALTER TABLE `acore_characters`.`character_skills` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_social`; -- 친구/차단 목록
ALTER TABLE `acore_characters`.`character_social` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_spell`; -- 배운 주문 목록
ALTER TABLE `acore_characters`.`character_spell` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_spell_cooldown`; -- 주문 쿨다운 저장
ALTER TABLE `acore_characters`.`character_spell_cooldown` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_stats`; -- 캐릭터 능력치 캐시
ALTER TABLE `acore_characters`.`character_stats` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_talent`; -- 특성 투자 상태
ALTER TABLE `acore_characters`.`character_talent` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`characters`; -- 캐릭터 본체
ALTER TABLE `acore_characters`.`characters` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`corpse`; -- 시체 위치/부활 정보
ALTER TABLE `acore_characters`.`corpse` AUTO_INCREMENT = 1;

-- 아이템, 경매, 우편
DELETE FROM `acore_characters`.`auctionhouse`; -- 경매장 등록 물품
ALTER TABLE `acore_characters`.`auctionhouse` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`item_instance`; -- 개별 아이템 인스턴스
ALTER TABLE `acore_characters`.`item_instance` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`item_loot_storage`; -- 아이템 내부 루팅 저장
ALTER TABLE `acore_characters`.`item_loot_storage` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`item_refund_instance`; -- 환불 가능 아이템 상태
ALTER TABLE `acore_characters`.`item_refund_instance` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`item_soulbound_trade_data`; -- 귀속 아이템 거래 가능 상태
ALTER TABLE `acore_characters`.`item_soulbound_trade_data` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`mail`; -- 우편 본문/금액/발신 정보
ALTER TABLE `acore_characters`.`mail` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`mail_items`; -- 우편 첨부 아이템 연결
ALTER TABLE `acore_characters`.`mail_items` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`mail_server_character`; -- 서버 우편 캐릭터 발송 기록
ALTER TABLE `acore_characters`.`mail_server_character` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`recovery_item`; -- 복구 가능 아이템 기록
ALTER TABLE `acore_characters`.`recovery_item` AUTO_INCREMENT = 1;

-- 길드
DELETE FROM `acore_characters`.`guild`; -- 길드 본체
ALTER TABLE `acore_characters`.`guild` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_bank_eventlog`; -- 길드 은행 로그
ALTER TABLE `acore_characters`.`guild_bank_eventlog` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_bank_item`; -- 길드 은행 아이템
ALTER TABLE `acore_characters`.`guild_bank_item` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_bank_right`; -- 길드 은행 권한
ALTER TABLE `acore_characters`.`guild_bank_right` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_bank_tab`; -- 길드 은행 탭
ALTER TABLE `acore_characters`.`guild_bank_tab` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_eventlog`; -- 길드 이벤트 로그
ALTER TABLE `acore_characters`.`guild_eventlog` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_member`; -- 길드원 목록
ALTER TABLE `acore_characters`.`guild_member` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_member_withdraw`; -- 길드원 출금 제한/사용량
ALTER TABLE `acore_characters`.`guild_member_withdraw` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`guild_rank`; -- 길드 등급
ALTER TABLE `acore_characters`.`guild_rank` AUTO_INCREMENT = 1;

-- 파티, 투기장, 전장, LFG, 인스턴스 저장
DELETE FROM `acore_characters`.`arena_team`; -- 투기장 팀
ALTER TABLE `acore_characters`.`arena_team` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`arena_team_member`; -- 투기장 팀원
ALTER TABLE `acore_characters`.`arena_team_member` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`battleground_deserters`; -- 전장 탈영 상태
ALTER TABLE `acore_characters`.`battleground_deserters` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`group_member`; -- 파티/공격대 구성원
ALTER TABLE `acore_characters`.`group_member` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`groups`; -- 파티/공격대 본체
ALTER TABLE `acore_characters`.`groups` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`instance`; -- 인스턴스 저장 정보
ALTER TABLE `acore_characters`.`instance` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`instance_reset`; -- 인스턴스 리셋 예약
ALTER TABLE `acore_characters`.`instance_reset` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`instance_saved_go_state_data`; -- 인스턴스 오브젝트 상태 저장
ALTER TABLE `acore_characters`.`instance_saved_go_state_data` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`lfg_data`; -- 무작위 던전/LFG 상태
ALTER TABLE `acore_characters`.`lfg_data` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`pvpstats_battlegrounds`; -- 전장 통계 본체
ALTER TABLE `acore_characters`.`pvpstats_battlegrounds` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`pvpstats_players`; -- 전장 플레이어별 통계
ALTER TABLE `acore_characters`.`pvpstats_players` AUTO_INCREMENT = 1;

-- 달력, 채널, 청원, 펫 주문
DELETE FROM `acore_characters`.`calendar_events`; -- 달력 이벤트
ALTER TABLE `acore_characters`.`calendar_events` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`calendar_invites`; -- 달력 초대
ALTER TABLE `acore_characters`.`calendar_invites` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`channels`; -- 커스텀 채널
ALTER TABLE `acore_characters`.`channels` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`channels_bans`; -- 채널 차단 목록
ALTER TABLE `acore_characters`.`channels_bans` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`channels_rights`; -- 채널 권한
ALTER TABLE `acore_characters`.`channels_rights` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`petition`; -- 길드/투기장 창단 청원서
ALTER TABLE `acore_characters`.`petition` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`petition_sign`; -- 청원서 서명
ALTER TABLE `acore_characters`.`petition_sign` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`pet_aura`; -- 펫 버프/디버프
ALTER TABLE `acore_characters`.`pet_aura` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`pet_spell`; -- 펫 주문
ALTER TABLE `acore_characters`.`pet_spell` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`pet_spell_cooldown`; -- 펫 주문 쿨다운
ALTER TABLE `acore_characters`.`pet_spell_cooldown` AUTO_INCREMENT = 1;

-- GM 문의, 리포트, 로그
DELETE FROM `acore_characters`.`bugreport`; -- 버그 신고
ALTER TABLE `acore_characters`.`bugreport` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`gm_subsurvey`; -- GM 설문 상세
ALTER TABLE `acore_characters`.`gm_subsurvey` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`gm_survey`; -- GM 설문
ALTER TABLE `acore_characters`.`gm_survey` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`gm_ticket`; -- GM 티켓
ALTER TABLE `acore_characters`.`gm_ticket` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`lag_reports`; -- 렉 신고
ALTER TABLE `acore_characters`.`lag_reports` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`log_arena_fights`; -- 투기장 경기 로그
ALTER TABLE `acore_characters`.`log_arena_fights` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`log_arena_memberstats`; -- 투기장 멤버별 로그
ALTER TABLE `acore_characters`.`log_arena_memberstats` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`log_encounter`; -- 보스/전투 인카운터 로그
ALTER TABLE `acore_characters`.`log_encounter` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`log_money`; -- 골드 이동 로그
ALTER TABLE `acore_characters`.`log_money` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`quest_tracker`; -- 퀘스트 추적 로그
ALTER TABLE `acore_characters`.`quest_tracker` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`spam_reports`; -- 유저 스팸 신고 기록
ALTER TABLE `acore_characters`.`spam_reports` AUTO_INCREMENT = 1;

-- 월드 런타임 저장 상태
DELETE FROM `acore_characters`.`creature_respawn`; -- 크리처 리스폰 저장 상태
ALTER TABLE `acore_characters`.`creature_respawn` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`gameobject_respawn`; -- 오브젝트 리스폰 저장 상태
ALTER TABLE `acore_characters`.`gameobject_respawn` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`game_event_condition_save`; -- 월드 이벤트 조건 저장 상태
ALTER TABLE `acore_characters`.`game_event_condition_save` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`game_event_save`; -- 월드 이벤트 진행 저장 상태
ALTER TABLE `acore_characters`.`game_event_save` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`pool_quest_save`; -- 퀘스트 풀 저장 상태
ALTER TABLE `acore_characters`.`pool_quest_save` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`warden_action`; -- 와든 감지/조치 기록
ALTER TABLE `acore_characters`.`warden_action` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`world_state`; -- 월드 상태 저장
ALTER TABLE `acore_characters`.`world_state` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`worldstates`; -- 월드 상태 저장
ALTER TABLE `acore_characters`.`worldstates` AUTO_INCREMENT = 1;

-- 커스텀 모듈 계정/캐릭터 데이터
DELETE FROM `acore_characters`.`blackmarket_purchase_log`; -- 암상인 구매 기록
ALTER TABLE `acore_characters`.`blackmarket_purchase_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_hero_stone_rune_box_log`; -- 영웅석 룬상자 획득 로그
ALTER TABLE `acore_characters`.`character_hero_stone_rune_box_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`character_hero_stone_teleport_runes`; -- 영웅석 순간이동 룬문자 해금 상태
ALTER TABLE `acore_characters`.`character_hero_stone_teleport_runes` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`custom_transmogrification`; -- 형상변환 적용 상태
ALTER TABLE `acore_characters`.`custom_transmogrification` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`custom_transmogrification_sets`; -- 형상변환 세트 저장
ALTER TABLE `acore_characters`.`custom_transmogrification_sets` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`custom_unlocked_appearances`; -- 계정 단위 형상 해금 목록
ALTER TABLE `acore_characters`.`custom_unlocked_appearances` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`custom_item_enhancement`; -- 커스텀 아이템 강화 상태
ALTER TABLE `acore_characters`.`custom_item_enhancement` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`item_enhancement_log`; -- 아이템 강화 결과 로그
ALTER TABLE `acore_characters`.`item_enhancement_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`karazhan_enhance_log`; -- 카라잔 강화 상세 로그
ALTER TABLE `acore_characters`.`karazhan_enhance_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`karazhan_item_enhance`; -- 아이템 강화 캐릭터/아이템별 상태
ALTER TABLE `acore_characters`.`karazhan_item_enhance` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_daily_bonus`; -- 시련 일일 추가 입장권 사용 상태
ALTER TABLE `acore_characters`.`solo_arena_daily_bonus` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_daily_entry`; -- 시련 계정 단위 일일 입장 횟수
ALTER TABLE `acore_characters`.`solo_arena_daily_entry` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_daily_purchase`; -- 시련 일일 구매 제한 상태
ALTER TABLE `acore_characters`.`solo_arena_daily_purchase` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_event_log`; -- 시련 진행 이벤트 로그
ALTER TABLE `acore_characters`.`solo_arena_event_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_progress`; -- 시련 캐릭터 진행도
ALTER TABLE `acore_characters`.`solo_arena_progress` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_reward_log`; -- 시련 보상 지급 로그
ALTER TABLE `acore_characters`.`solo_arena_reward_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_run_log`; -- 시련 도전 실행/결과 로그
ALTER TABLE `acore_characters`.`solo_arena_run_log` AUTO_INCREMENT = 1;
DELETE FROM `acore_characters`.`solo_arena_stage_record`; -- 시련 단계별 최고 기록
ALTER TABLE `acore_characters`.`solo_arena_stage_record` AUTO_INCREMENT = 1;

SET FOREIGN_KEY_CHECKS = 1;

SELECT 'acore_characters 캐릭터/계정 상태 초기화 완료' AS `message`;
