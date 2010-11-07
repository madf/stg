/*
 *  DB migration from v00 to v01 (mysql)
 */

ALTER TABLE users ADD DisabledDetailStat INT(3) DEFAULT 0;
