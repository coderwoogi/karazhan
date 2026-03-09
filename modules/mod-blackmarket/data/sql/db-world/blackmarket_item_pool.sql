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

 Date: 11/01/2026 17:34:39
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for blackmarket_item_pool
-- ----------------------------
DROP TABLE IF EXISTS `blackmarket_item_pool`;
CREATE TABLE `blackmarket_item_pool`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `item_entry` int NOT NULL,
  `price_gold` bigint NOT NULL,
  `price_item` int NULL DEFAULT 0,
  `price_item_count` int NULL DEFAULT 0,
  `rarity` tinyint NULL DEFAULT 0,
  `weight` int NULL DEFAULT 100,
  `max_per_spawn` int NULL DEFAULT 1,
  `comment` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `idx_weight`(`weight` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 69 CHARACTER SET = utf8mb3 COLLATE = utf8mb3_general_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
