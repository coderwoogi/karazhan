/*
 Navicat Premium Dump SQL

 Source Server         : server
 Source Server Type    : MySQL
 Source Server Version : 80407 (8.4.7)
 Source Host           : localhost:3306
 Source Schema         : acore_world

 Target Server Type    : MySQL
 Target Server Version : 80407 (8.4.7)
 File Encoding         : 65001

 Date: 11/01/2026 17:34:46
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for blackmarket_state
-- ----------------------------
DROP TABLE IF EXISTS `blackmarket_state`;
CREATE TABLE `blackmarket_state`  (
  `id` tinyint NOT NULL DEFAULT 1,
  `is_active` tinyint NULL DEFAULT 0,
  `creature_guid` int NULL DEFAULT 0,
  `spawn_point_id` int NULL DEFAULT 0,
  `spawn_time` int UNSIGNED NOT NULL DEFAULT 0,
  `despawn_time` int UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb3 COLLATE = utf8mb3_general_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
