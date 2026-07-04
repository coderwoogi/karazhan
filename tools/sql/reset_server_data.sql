-- ============================================================================
--  서버 데이터 초기화 스크립트 (캐릭터 · 계정 · 로그 · 인게임 커스텀 데이터 삭제)
--  작성: 2026-06-17  /  ※ 검토 후 수동 실행할 것 (자동 실행 금지)
--
--  [삭제]  acore_auth  : 모든 계정(GM 포함) + 계정연결 + 인증 로그
--          acore_characters : 모든 캐릭터/길드/메일/아이템/로그/커스텀 per-캐릭터
--          tokreg      : 결제/구매로그/장바구니
--  [보존]  acore_world : 전부(게임 콘텐츠/상품/설정)
--          update      : 전부(웹 데이터) — 결정 #4
--          + acore_characters/acore_auth 의 마이그레이션·콘텐츠·설정·서버상태 테이블
--          + tokreg 의 상품 카탈로그(store/store_category/packages)
--
--  [충돌 방지 #4]
--    - acore_auth.account.id 는 AUTO_INCREMENT(현재 19). DELETE(=TRUNCATE 아님)로
--      카운터가 유지되어 신규 계정은 19부터 발급 → 웹 데이터 최대 user_id(17)와 안 겹침.
--    - characters/item_instance 의 guid 는 AUTO_INCREMENT가 아니며 AzerothCore가
--      MAX(guid)+1 로 재발급하므로 비우면 1부터 다시 시작한다. 그러나 웹 기록
--      (carddraw/wowpass/point_shop 등)은 user_id(계정) + 캐릭터 "스냅샷"으로
--      저장되므로, 캐릭터 guid 재사용이 있어도 과거 웹 기록을 새 유저가 물려받지 않는다.
--
--  ※※※ 실행 전 반드시 전체 백업 ※※※
--    mysqldump -uroot -p --single-transaction --routines --events
--      --databases acore_auth acore_characters acore_world tokreg update
--      > reset_backup_20260617.sql
--    (update 는 예약어지만 mysqldump CLI 인자로는 그대로 써도 됩니다)
--
--  실행:  mysql -uroot -p < reset_server_data.sql
-- ============================================================================

SET FOREIGN_KEY_CHECKS = 0;

-- ============================================================================
-- 1) acore_auth : 모든 계정 + 계정연결 + 인증 로그 삭제
--    (보존: realmlist, build_info, motd*, autobroadcast*, rbac_permissions/
--     linked/default, updates, updates_include, uptime, web_menu_permissions,
--     acore_cms_subscriptions)
-- ============================================================================
DELETE FROM acore_auth.account;                      -- 모든 계정(GM 포함)
DELETE FROM acore_auth.account_access;               -- GM 권한
DELETE FROM acore_auth.account_banned;
DELETE FROM acore_auth.account_muted;
DELETE FROM acore_auth.secret_digest;                -- 2FA 비밀
DELETE FROM acore_auth.rbac_account_permissions;     -- 계정별 권한
DELETE FROM acore_auth.realmcharacters;              -- 계정별 캐릭터 수
DELETE FROM acore_auth.logs;                         -- 인증 로그
DELETE FROM acore_auth.logs_ip_actions;              -- IP 행동 로그
DELETE FROM acore_auth.ip_banned;                    -- IP 밴

-- ============================================================================
-- 2) acore_characters : 캐릭터/계정/길드/메일/아이템/인스턴스/로그/커스텀 삭제
--    (보존 목록은 파일 하단 주석 참조 — 아래에 없는 테이블은 손대지 않음)
-- ============================================================================
-- 계정/캐릭터 본체
DELETE FROM acore_characters.account_data;
DELETE FROM acore_characters.account_instance_times;
DELETE FROM acore_characters.account_tutorial;
DELETE FROM acore_characters.character_account_data;
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

-- 아이템 인스턴스
DELETE FROM acore_characters.item_instance;
DELETE FROM acore_characters.item_loot_storage;
DELETE FROM acore_characters.item_refund_instance;
DELETE FROM acore_characters.item_soulbound_trade_data;

-- 메일 (서버메일 템플릿은 보존 — mail_server_template* 삭제 안 함)
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

-- 커스텀 (per-캐릭터/계정 데이터)
DELETE FROM acore_characters.blackmarket_purchase_log;          -- 선술집 구매 로그
DELETE FROM acore_characters.custom_item_enhancement;           -- 강화 레벨(per-item)
DELETE FROM acore_characters.custom_transmogrification;
DELETE FROM acore_characters.custom_transmogrification_sets;
DELETE FROM acore_characters.custom_unlocked_appearances;       -- 외형 수집
DELETE FROM acore_characters.karazhan_item_enhance;             -- 카라잔 강화
DELETE FROM acore_characters.karazhan_enhance_log;
DELETE FROM acore_characters.item_enhancement_log;
DELETE FROM acore_characters.mod_item_grade;                    -- 아이템 등급
DELETE FROM acore_characters.character_hero_stone_rune_box_log; -- 영웅석
DELETE FROM acore_characters.character_hero_stone_teleport_runes;
DELETE FROM acore_characters.stone;
DELETE FROM acore_characters.character_random_quest_log;        -- 랜덤 퀘스트
DELETE FROM acore_characters.character_random_quest_state;
DELETE FROM acore_characters.recovery_item;
DELETE FROM acore_characters.solo_arena_daily_bonus;            -- 솔로 아레나
DELETE FROM acore_characters.solo_arena_daily_entry;
DELETE FROM acore_characters.solo_arena_daily_purchase;
DELETE FROM acore_characters.solo_arena_event_log;
DELETE FROM acore_characters.solo_arena_progress;
DELETE FROM acore_characters.solo_arena_reward_log;
DELETE FROM acore_characters.solo_arena_run_log;
DELETE FROM acore_characters.solo_arena_stage_record;

-- ============================================================================
-- 3) tokreg : 결제/구매로그/장바구니 삭제 (상품 카탈로그 보존)
--    (보존: store, store_category, packages)
-- ============================================================================
DELETE FROM tokreg.payments;       -- 실결제 기록 (결정 #3)
DELETE FROM tokreg.store_log;      -- 구매/지급 로그
DELETE FROM tokreg.shopping_cart;  -- 장바구니

-- ============================================================================
-- 4) update (웹 DB) : 보존 — 삭제 없음 (결정 #4)
-- ============================================================================

-- ============================================================================
-- 5) 충돌 방지: 계정 id 연속성 보정 (안전 여유)
--    DELETE로 이미 AUTO_INCREMENT=19 유지되지만, 웹 최대 user_id(17) 위로 확실히
--    띄우기 위해 명시적으로 올린다. (원치 않으면 이 줄 삭제)
-- ============================================================================
ALTER TABLE acore_auth.account AUTO_INCREMENT = 1000;

SET FOREIGN_KEY_CHECKS = 1;

-- ============================================================================
-- [보존된 acore_characters 테이블 — 의도적으로 삭제하지 않음]
--   updates, updates_include                       (DB 마이그레이션 이력 — 절대 보존)
--   mail_server_template, mail_server_template_conditions, mail_server_template_items
--                                                  (서버메일 템플릿 = 콘텐츠)
--   worldstates, world_state, pool_quest_save, game_event_save,
--   game_event_condition_save                      (서버 월드/이벤트 상태)
--   creature_respawn, gameobject_respawn, instance_reset   (리스폰/리셋 타이머)
--   addons, banned_addons                          (애드온 레지스트리/설정)
--   profanity_name, reserved_name, chat_filter     (이름/욕설 필터)
--   active_arena_season                            (현재 시즌 설정)
-- ============================================================================
