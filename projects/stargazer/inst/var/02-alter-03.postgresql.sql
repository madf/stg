/*
 *  DB migration from v02 to v03 (postgres)
 */
BEGIN;

CREATE DOMAIN DM_TARIFF_CHANGE_POLICY AS TEXT NOT NULL
    CONSTRAINT valid_value CHECK (VALUE IN ('allow', 'to_cheap', 'to_expensive', 'deny'));

ALTER TABLE tb_tariffs ADD change_policy DM_TARIFF_CHANGE_POLICY DEFAULT 'allow';
ALTER TABLE tb_tariffs ADD change_policy_timeout TIMESTAMP NOT NULL DEFAULT '1970-01-01 00:00:00';
ALTER TABLE tb_tariffs ALTER COLUMN change_policy DROP DEFAULT;
ALTER TABLE tb_tariffs ALTER COLUMN change_policy_timeout DROP DEFAULT;

UPDATE tb_info SET version = 8;

COMMIT;
