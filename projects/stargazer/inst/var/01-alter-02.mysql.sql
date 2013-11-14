/*
 *  DB migration from v01 to v02 (mysql)
 */

ALTER TABLE users ADD DisabledDetailStat INT(3) DEFAULT 0;
