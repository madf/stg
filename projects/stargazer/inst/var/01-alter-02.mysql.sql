/*
 *  DB migration from v01 to v02 (mysql)
 */

ALTER TABLE tariffs ADD period VARCHAR(32) NOT NULL DEFAULT 'month';

CREATE TABLE info
(
    version INTEGER NOT NULL
);

INSERT INTO info VALUES (1);
