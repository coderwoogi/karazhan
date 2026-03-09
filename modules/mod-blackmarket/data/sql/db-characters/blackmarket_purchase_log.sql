/*
 Navicat Premium Dump SQL

 Source Server         : server
 Source Server Type    : MySQL
 Source Server Version : 80407 (8.4.7)
 Source Host           : localhost:3306
 Source Schema         : acore_characters

 Target Server Type    : MySQL
 Target Server Version : 80407 (8.4.7)
 File Encoding         : 65001

 Date: 11/01/2026 17:35:28
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for blackmarket_purchase_log
-- ----------------------------
DROP TABLE IF EXISTS `blackmarket_purchase_log`;
CREATE TABLE `blackmarket_purchase_log`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `account_id` int NOT NULL,
  `character_guid` int NOT NULL,
  `item_entry` int NOT NULL,
  `purchase_time` int UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `idx_account`(`account_id` ASC) USING BTREE,
  INDEX `idx_char`(`character_guid` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 50 CHARACTER SET = utf8mb3 COLLATE = utf8mb3_general_ci ROW_FORMAT = DYNAMIC;

SET FOREIGN_KEY_CHECKS = 1;
