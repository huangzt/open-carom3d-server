-- Carom3D 账号表（由原若依后端 carom3d.sql 抽取，仅表结构）
SET NAMES utf8mb4;

DROP TABLE IF EXISTS `sys_account`;
CREATE TABLE `sys_account` (
  `account_id` bigint NOT NULL AUTO_INCREMENT COMMENT '角色ID',
  `account` varchar(255) NOT NULL COMMENT '账号',
  `password` varchar(255) NOT NULL COMMENT '密码',
  `maxpower` tinyint unsigned DEFAULT '150' COMMENT '最大力度',
  `minpower` tinyint unsigned DEFAULT '1' COMMENT '最小力度',
  `cueId` int unsigned DEFAULT '3000' COMMENT '球杆编号',
  `del_flag` char(1) DEFAULT '0' COMMENT '删除标志（0代表存在 2代表删除）',
  `create_by` varchar(64) DEFAULT '' COMMENT '创建者',
  `create_time` datetime DEFAULT NULL COMMENT '创建时间',
  `update_by` varchar(64) DEFAULT '' COMMENT '更新者',
  `update_time` datetime DEFAULT NULL COMMENT '更新时间',
  `remark` varchar(500) DEFAULT NULL COMMENT '备注',
  PRIMARY KEY (`account_id`) USING BTREE,
  KEY `account` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Carom3D账号';
