/*
Navicat MySQL Data Transfer

Source Server         : localhost_3306
Source Server Version : 50717
Source Host           : localhost:3306
Source Database       : methane-detection-system

Target Server Type    : MYSQL
Target Server Version : 50717
File Encoding         : 65001

Date: 2017-05-17 10:54:54
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for data
-- ----------------------------
DROP TABLE IF EXISTS `data`;
CREATE TABLE `data` (
  `ID` int(8) NOT NULL AUTO_INCREMENT,
  `nDevId` int(8) DEFAULT NULL,
  `nAdcData` int(8) DEFAULT NULL,
  `nAdcBits` int(8) DEFAULT NULL,
  `dRealData` double(4,2) DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=6471 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of data
-- ----------------------------
INSERT INTO `data` VALUES ('1', '1', '81', '8', '1.03');

-- ----------------------------
-- Table structure for device
-- ----------------------------
DROP TABLE IF EXISTS `device`;
CREATE TABLE `device` (
  `ID` int(8) NOT NULL AUTO_INCREMENT,
  `strSN` varchar(16) COLLATE utf8_unicode_ci DEFAULT NULL,
  `nUserId` int(8) DEFAULT NULL,
  `dThresholdValue` double(4,2) DEFAULT NULL,
  `strNickName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of device
-- ----------------------------
INSERT INTO `device` VALUES ('1', '12345678', '1', '1.04', '我的设备');

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `Id` int(8) NOT NULL AUTO_INCREMENT,
  `strUserName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `strPassWd` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=InnoDB AUTO_INCREMENT=32 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of user
-- ----------------------------
INSERT INTO `user` VALUES ('1', '123', '12345');
INSERT INTO `user` VALUES ('31', '1234567', '1234567');
