-- ============================================================================
--  서버 초기화 (계정 로그인만 보존 / 캐릭터·유저 경제·진행 데이터 전부 삭제)
--  v2 · 2026-06-19 · ※ 검토 후 수동 실행 (자동 실행 금지)
--
--  [보존]
--   - 계정: acore_auth.account / account_access / account_banned / account_muted /
--           secret_digest / rbac_account_permissions  (계정·권한·제재만)
--   - 게임 콘텐츠: acore_world 전부
--   - 기능/상품(캐릭터 무관):
--       update.web_carddraw_items   ← 카드뽑기 "뽑기 목록"
--       update.point_shop_items     ← 포인트상점 상품
--       tokreg.store / store_category / packages  ← 선술집 판매 상품
--   - 웹 페이지/게시판/글/댓글/메뉴/설정, 1:1 문의(web_inquiry_messages),
--     런처/패치, 마이그레이션(updates*), 서버메일 템플릿, 월드/이벤트 상태 등
--
--  [삭제]
--   - 모든 캐릭터 데이터 + 계정 단위 게임데이터(account_data/tutorial/instance_times)
--   - 웹 포인트/주문/코인마켓/카드뽑기·패스 결과/구독/부계정구매/프로모션수령/알림/웹로그
--   - 선술집 결제·구매내역, 멤버십 구독(acore_cms_subscriptions)
--   - acore_auth.realmcharacters (계정별 캐릭터 목록 → 재생성)
--   - acore_auth 인증·IP 로그(logs / logs_ip_actions / ip_banned) ※ IP 차단도 해제됨
--
--  [user_profiles] 진행 필드만 0/'' 초기화(대표·선택 캐릭터, 뽑기 횟수).
--                  web_rank(웹 등급)·web_theme 는 보존.
--
--  ※ 실행 전: ① worldserver 종료  ② 전체 백업
--     mysqldump -uroot -p --single-transaction --routines --events
--       --databases acore_auth acore_characters acore_world tokreg update
--       > reset_backup_20260619.sql
--     실행:  mysql -uroot -p < reset_characters_keep_accounts.sql
-- ============================================================================

SET FOREIGN_KEY_CHECKS = 0;

-- ===== acore_auth (계정·권한·제재는 보존 / 캐릭터목록·구독·인증로그는 삭제) =====
DELETE FROM acore_auth.realmcharacters;          -- 계정별 캐릭터 목록(재생성)
DELETE FROM acore_auth.acore_cms_subscriptions;  -- 멤버십 구독(결제 기반)
DELETE FROM acore_auth.logs;                     -- 인증 로그
DELETE FROM acore_auth.logs_ip_actions;          -- IP 행동 로그
DELETE FROM acore_auth.ip_banned;                -- IP 밴(주의: IP 차단도 함께 해제됨)

-- ===== acore_characters : 캐릭터 + 계정단위 게임데이터 전부 =====
-- 계정 단위 게임데이터
DELETE FROM acore_characters.account_data;
DELETE FROM acore_characters.account_instance_times;
DELETE FROM acore_characters.account_tutorial;
DELETE FROM acore_characters.character_account_data;
-- 캐릭터 본체/상태
DELETE FROM acore_characters.characters;
DELETE FROM acore_characters.character_achievement;
DELETE FROM acore_characters.character_achievement_offline_updates;
DELETE FROM acore_characters.character_achievement_progress;
DELETE FROM acore_characters.character_action;
DELETE FROM acore_characters.character_arena_stats;
DELETE FROM acore_characters.character_aura;
DELETE FROM acore_characters.character_banned;
DELETE FROM acore_characters.character_battleground_random;
DELETE FROM acore_characters.character_brew_of_the_month;
DELETE FROM acore_characters.character_declinedname;
DELETE FROM acore_characters.character_entry_point;
DELETE FROM acore_characters.character_equipmentsets;
DELETE FROM acore_characters.character_gifts;
DELETE FROM acore_characters.character_glyphs;
DELETE FROM acore_characters.character_homebind;
DELETE FROM acore_characters.character_instance;
DELETE FROM acore_characters.character_inventory;
DELETE FROM acore_characters.character_pet;
DELETE FROM acore_characters.character_pet_declinedname;
DELETE FROM acore_characters.character_queststatus;
DELETE FROM acore_characters.character_queststatus_daily;
DELETE FROM acore_characters.character_queststatus_monthly;
DELETE FROM acore_characters.character_queststatus_rewarded;
DELETE FROM acore_characters.character_queststatus_seasonal;
DELETE FROM acore_characters.character_queststatus_weekly;
DELETE FROM acore_characters.character_reputation;
DELETE FROM acore_characters.character_settings;
DELETE FROM acore_characters.character_skills;
DELETE FROM acore_characters.character_social;
DELETE FROM acore_characters.character_spell;
DELETE FROM acore_characters.character_spell_cooldown;
DELETE FROM acore_characters.character_stats;
DELETE FROM acore_characters.character_talent;
-- 아이템
DELETE FROM acore_characters.item_instance;
DELETE FROM acore_characters.item_loot_storage;
DELETE FROM acore_characters.item_refund_instance;
DELETE FROM acore_characters.item_soulbound_trade_data;
-- 메일 (서버메일 템플릿 mail_server_template* 은 보존)
DELETE FROM acore_characters.mail;
DELETE FROM acore_characters.mail_items;
DELETE FROM acore_characters.mail_server_character;
-- 길드
DELETE FROM acore_characters.guild;
DELETE FROM acore_characters.guild_bank_eventlog;
DELETE FROM acore_characters.guild_bank_item;
DELETE FROM acore_characters.guild_bank_right;
DELETE FROM acore_characters.guild_bank_tab;
DELETE FROM acore_characters.guild_eventlog;
DELETE FROM acore_characters.guild_member;
DELETE FROM acore_characters.guild_member_withdraw;
DELETE FROM acore_characters.guild_rank;
-- 아레나/그룹/청원/펫
DELETE FROM acore_characters.arena_team;
DELETE FROM acore_characters.arena_team_member;
DELETE FROM acore_characters.groups;
DELETE FROM acore_characters.group_member;
DELETE FROM acore_characters.petition;
DELETE FROM acore_characters.petition_sign;
DELETE FROM acore_characters.pet_aura;
DELETE FROM acore_characters.pet_spell;
DELETE FROM acore_characters.pet_spell_cooldown;
-- 경매/인스턴스/캘린더/채널/기타 런타임
DELETE FROM acore_characters.auctionhouse;
DELETE FROM acore_characters.instance;
DELETE FROM acore_characters.instance_saved_go_state_data;
DELETE FROM acore_characters.calendar_events;
DELETE FROM acore_characters.calendar_invites;
DELETE FROM acore_characters.channels;
DELETE FROM acore_characters.channels_bans;
DELETE FROM acore_characters.channels_rights;
DELETE FROM acore_characters.battleground_deserters;
DELETE FROM acore_characters.corpse;
DELETE FROM acore_characters.lfg_data;
-- 로그/리포트/제재
DELETE FROM acore_characters.log_arena_fights;
DELETE FROM acore_characters.log_arena_memberstats;
DELETE FROM acore_characters.log_encounter;
DELETE FROM acore_characters.log_money;
DELETE FROM acore_characters.pvpstats_battlegrounds;
DELETE FROM acore_characters.pvpstats_players;
DELETE FROM acore_characters.bugreport;
DELETE FROM acore_characters.gm_ticket;
DELETE FROM acore_characters.gm_survey;
DELETE FROM acore_characters.gm_subsurvey;
DELETE FROM acore_characters.lag_reports;
DELETE FROM acore_characters.spam_reports;
DELETE FROM acore_characters.quest_tracker;
DELETE FROM acore_characters.warden_action;
DELETE FROM acore_characters.web_mail_log;
-- 커스텀 per-캐릭터
DELETE FROM acore_characters.blackmarket_purchase_log;
DELETE FROM acore_characters.custom_item_enhancement;
DELETE FROM acore_characters.custom_transmogrification;
DELETE FROM acore_characters.custom_transmogrification_sets;
DELETE FROM acore_characters.custom_unlocked_appearances;
DELETE FROM acore_characters.karazhan_item_enhance;
DELETE FROM acore_characters.karazhan_enhance_log;
DELETE FROM acore_characters.item_enhancement_log;
DELETE FROM acore_characters.mod_item_grade;
DELETE FROM acore_characters.character_hero_stone_rune_box_log;
DELETE FROM acore_characters.character_hero_stone_teleport_runes;
DELETE FROM acore_characters.stone;
DELETE FROM acore_characters.character_random_quest_log;
DELETE FROM acore_characters.character_random_quest_state;
DELETE FROM acore_characters.recovery_item;
DELETE FROM acore_characters.solo_arena_daily_bonus;
DELETE FROM acore_characters.solo_arena_daily_entry;
DELETE FROM acore_characters.solo_arena_daily_purchase;
DELETE FROM acore_characters.solo_arena_event_log;
DELETE FROM acore_characters.solo_arena_progress;
DELETE FROM acore_characters.solo_arena_reward_log;
DELETE FROM acore_characters.solo_arena_run_log;
DELETE FROM acore_characters.solo_arena_stage_record;

-- ===== tokreg : 결제/구매/장바구니 (상품 카탈로그는 보존) =====
DELETE FROM tokreg.payments;
DELETE FROM tokreg.store_log;
DELETE FROM tokreg.shopping_cart;

-- ===== update (웹) : 포인트/주문/카드뽑기 결과/구독 등 유저 데이터 =====
DELETE FROM `update`.user_points;                 -- 웹 포인트 초기화
DELETE FROM `update`.user_point_logs;
DELETE FROM `update`.point_shop_orders;
DELETE FROM `update`.point_shop_order_logs;
DELETE FROM `update`.point_coin_market_listings;
DELETE FROM `update`.carddraw_draw_logs;           -- 카드뽑기 결과(목록 web_carddraw_items 는 보존)
DELETE FROM `update`.wowpass_draw_logs;
DELETE FROM `update`.web_feature_subscriptions;    -- 구독 혜택(결제 기반)
DELETE FROM `update`.web_second_account_purchases; -- 부계정 구매(결제 기반)
DELETE FROM `update`.web_promotion_reward_log;     -- 프로모션 수령 내역
DELETE FROM `update`.web_account_recovery_requests;
DELETE FROM `update`.notifications;
DELETE FROM `update`.logs;                          -- 웹 유저 행동 로그

-- user_profiles: 진행 필드만 초기화(web_rank·web_theme 보존, 대표/선택 캐릭터 제거)
UPDATE `update`.user_profiles SET
    main_char_guid              = NULL,
    main_char_name              = NULL,
    wowpass_draw_count          = 0,
    wowpass_selected_char_guid  = 0,
    wowpass_selected_char_name  = '',
    wowpass_selected_char_race  = 0,
    wowpass_selected_char_class = 0,
    wowpass_selected_char_gender= 0,
    wowpass_selected_char_level = 0,
    carddraw_draw_count          = 0,
    carddraw_selected_char_guid  = 0,
    carddraw_selected_char_name  = '',
    carddraw_selected_char_race  = 0,
    carddraw_selected_char_class = 0,
    carddraw_selected_char_gender= 0,
    carddraw_selected_char_level = 0;

SET FOREIGN_KEY_CHECKS = 1;

-- ============================================================================
-- [보존된 acore_characters 테이블 — 삭제하지 않음]
--   updates, updates_include / mail_server_template(_conditions/_items) /
--   worldstates, world_state, pool_quest_save, game_event_save,
--   game_event_condition_save / creature_respawn, gameobject_respawn,
--   instance_reset / addons, banned_addons, profanity_name, reserved_name,
--   chat_filter, active_arena_season
-- [참고] 캐릭터 guid 는 비워지면 1부터 재발급됩니다(웹 프로필의 대표/선택 캐릭터는
--        위에서 0/'' 로 초기화하여 옛 guid 참조가 남지 않게 했습니다).
-- ============================================================================
