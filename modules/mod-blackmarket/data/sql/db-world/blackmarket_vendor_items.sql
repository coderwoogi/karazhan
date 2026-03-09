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

 Date: 11/01/2026 17:34:55
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for blackmarket_vendor_items
-- ----------------------------
DROP TABLE IF EXISTS `blackmarket_vendor_items`;
CREATE TABLE `blackmarket_vendor_items`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `item_entry` int NOT NULL,
  `price_gold` int NOT NULL,
  `price_item` int NULL DEFAULT 0,
  `price_item_count` int NULL DEFAULT 0,
  `remaining_count` int NULL DEFAULT 1,
  `comment` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NULL DEFAULT NULL COMMENT '아이템 이름',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 6 CHARACTER SET = utf8mb3 COLLATE = utf8mb3_general_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
