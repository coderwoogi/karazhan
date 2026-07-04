-- 테스트 서버 초기화 검증용 조회 SQL
-- 이 파일은 데이터를 삭제하지 않습니다.
-- count_rows가 0이 아니면 해당 분류의 데이터가 아직 남아 있다는 뜻입니다.

SELECT 'acore_auth.account' AS table_name, COUNT(*) AS count_rows FROM `acore_auth`.`account`
UNION ALL SELECT 'acore_auth.account_access', COUNT(*) FROM `acore_auth`.`account_access`
UNION ALL SELECT 'acore_auth.account_banned', COUNT(*) FROM `acore_auth`.`account_banned`
UNION ALL SELECT 'acore_auth.realmcharacters', COUNT(*) FROM `acore_auth`.`realmcharacters`
UNION ALL SELECT 'acore_auth.uptime', COUNT(*) FROM `acore_auth`.`uptime`

UNION ALL SELECT 'acore_characters.characters', COUNT(*) FROM `acore_characters`.`characters`
UNION ALL SELECT 'acore_characters.character_inventory', COUNT(*) FROM `acore_characters`.`character_inventory`
UNION ALL SELECT 'acore_characters.item_instance', COUNT(*) FROM `acore_characters`.`item_instance`
UNION ALL SELECT 'acore_characters.mail', COUNT(*) FROM `acore_characters`.`mail`
UNION ALL SELECT 'acore_characters.guild', COUNT(*) FROM `acore_characters`.`guild`
UNION ALL SELECT 'acore_characters.solo_arena_daily_entry', COUNT(*) FROM `acore_characters`.`solo_arena_daily_entry`
UNION ALL SELECT 'acore_characters.character_hero_stone_teleport_runes', COUNT(*) FROM `acore_characters`.`character_hero_stone_teleport_runes`
UNION ALL SELECT 'acore_characters.creature_respawn', COUNT(*) FROM `acore_characters`.`creature_respawn`
UNION ALL SELECT 'acore_characters.worldstates', COUNT(*) FROM `acore_characters`.`worldstates`

UNION ALL SELECT 'update.user_profiles', COUNT(*) FROM `update`.`user_profiles`
UNION ALL SELECT 'update.user_points', COUNT(*) FROM `update`.`user_points`
UNION ALL SELECT 'update.user_point_logs', COUNT(*) FROM `update`.`user_point_logs`
UNION ALL SELECT 'update.carddraw_draw_logs', COUNT(*) FROM `update`.`carddraw_draw_logs`
UNION ALL SELECT 'update.point_shop_orders', COUNT(*) FROM `update`.`point_shop_orders`
UNION ALL SELECT 'update.web_feature_subscriptions', COUNT(*) FROM `update`.`web_feature_subscriptions`

UNION ALL SELECT 'tokreg.shopping_cart', COUNT(*) FROM `tokreg`.`shopping_cart`
UNION ALL SELECT 'tokreg.store_log', COUNT(*) FROM `tokreg`.`store_log`
UNION ALL SELECT 'tokreg.payments', COUNT(*) FROM `tokreg`.`payments`

UNION ALL SELECT 'test_characters.characters', COUNT(*) FROM `test_characters`.`characters`
UNION ALL SELECT 'test_characters.character_inventory', COUNT(*) FROM `test_characters`.`character_inventory`
UNION ALL SELECT 'test_characters.item_instance', COUNT(*) FROM `test_characters`.`item_instance`
UNION ALL SELECT 'test_characters.mail', COUNT(*) FROM `test_characters`.`mail`;
