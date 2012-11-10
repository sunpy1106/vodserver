-- sql:
--     mysql source file
-- History:
-- 2012/04/21  sunpy

DROP DATABASE IF EXISTS P2PVOD_DB;
CREATE DATABASE IF NOT EXISTS P2PVOD_DB;
USE P2PVOD_DB;

-- set global configuration
SET GLOBAL max_connections = 1000;
SET GLOBAL interactive_timeout = 288000;
SET GLOBAL wait_timeout = 288000;

-- 源站点上存储的视频信息
CREATE TABLE IF NOT EXISTS ivod_video_t(
-- 视频ID
	id INT UNSIGNED NOT NULL , 
-- 视频长度
	fileLength BIGINT UNSIGNED NOT NULL,
-- 视频在源站点上的存储路径
	path VARCHAR(512) ,
-- 视频的码率
	bitRate FLOAT NOT NULL,
	PRIMARY KEY(id)
);

-- server上存储的client信息
CREATE TABLE IF NOT EXISTS client_info(
-- client  ID
	client_id INT UNSIGNED NOT NULL, 
-- client ip address 
	ipaddress VARCHAR(32) ,
-- 最大并发数
	maxBand DOUBLE NOT NULL,
-- 当前并发数
	availableBand DOUBLE NOT NULL,
	PRIMARY KEY(client_id)
);


CREATE TABLE IF NOT EXISTS ivod_seg_t(
-- 视频ID
	id INT UNSIGNED NOT NULL , 
-- 视频分段号
	segNumber INT UNSIGNED NOT NULL,
-- client  ID
	client_id INT UNSIGNED NOT NULL, 
	segSize INT UNSIGNED ,
	PRIMARY KEY(id,segNumber,client_id)
);
	


