/*
 *  DB migration from v01 to v02 (postgres)
 */
BEGIN;

CREATE DOMAIN DM_TARIFF_PERIOD AS TEXT NOT NULL
    CONSTRAINT valid_value CHECK (VALUE = 'month' OR VALUE = 'day');

ALTER TABLE tb_tariffs ADD period DM_TARIFF_PERIOD DEFAULT 'month';

UPDATE tb_info SET version = 7;

COMMIT;
