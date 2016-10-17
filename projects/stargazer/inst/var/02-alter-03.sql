/*
 *  DB migration from v02 to v03 (firebird)
 */

CREATE DOMAIN DM_TARIFF_CHANGE_POLICY AS  VARCHAR(32) NOT NULL
    CHECK (VALUE IN ('allow', 'to_cheap', 'to_expensive', 'deny'));

ALTER TABLE tb_tariffs ADD change_policy DM_TARIFF_CHANGE_POLICY DEFAULT 'allow';
ALTER TABLE tb_tariffs ADD change_policy_timeout DM_MOMENT DEFAULT '1970-01-01 00:00:00';

ALTER TABLE tb_tariffs ALTER COLUMN change_policy DROP DEFAULT;
ALTER TABLE tb_tariffs ALTER COLUMN change_policy_timeout DROP DEFAULT;

UPDATE tb_tariffs SET change_policy = 'allow', change_policy_timeout = '1970-01-01 00:00:00';

UPDATE tb_info SET version = 2;
