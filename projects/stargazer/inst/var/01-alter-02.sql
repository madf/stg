/*
 *  DB migration from v01 to v02 (firebird)
 */

CREATE DOMAIN DM_TARIFF_PERIOD AS VARCHAR(32) NOT NULL
    CHECK (VALUE = 'month' OR VALUE = 'day');

ALTER TABLE tb_tariffs ADD period DM_TARIFF_PERIOD DEFAULT 'month';

CREATE TABLE tb_info
(
    version INTEGER NOT NULL
);

INSERT INTO tb_info VALUES (1);
