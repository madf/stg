/*
 *  DB migration from v01 to v02 (postgres)
 */

BEGIN;

CREATE TABLE tb_month_stats
(
    pk_month_stats BIGSERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    stats_date DATE NOT NULL,
    cash dm_money,
    free_mb dm_money,
    last_activity_time TIMESTAMP NOT NULL,
    last_cash_add dm_money,
    last_cash_add_time TIMESTAMP NOT NULL,
    passive_time INTEGER NOT NULL,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE,
    UNIQUE (fk_user, stats_date)
);

CREATE TABLE tb_month_stats_traffic
(
    pk_month_stat_traffic BIGSERIAL PRIMARY KEY,
    fk_month_stats BIGINT NOT NULL,
    dir_num SMALLINT NOT NULL,
    download BIGINT NOT NULL,
    upload BIGINT NOT NULL,

    FOREIGN KEY (fk_month_stats)
        REFERENCES tb_month_stats (pk_month_stats)
        ON DELETE CASCADE,
    UNIQUE (fk_month_stats, dir_num)
);

INSERT INTO tb_month_stats
    (fk_user,
     stats_date,
     cash,
     free_mb,
     last_activity_time,
     last_cash_add,
     last_cash_add_time,
     passive_time)
SELECT fk_user,
       stats_date,
       0,
       0,
       '1970-01-01 00:00:00'::TIMESTAMP WITHOUT TIME ZONE,
       0,
       '1970-01-01 00:00:00'::TIMESTAMP WITHOUT TIME ZONE,
       0
FROM tb_stats_traffic
WHERE date_trunc('month', stats_date) < date_trunc('month', 'now'::DATE)
GROUP BY fk_user,
         stats_date;

INSERT INTO tb_month_stats_traffic
    (fk_month_stats,
     dir_num,
     download,
     upload)
SELECT s.pk_month_stats,
       t.dir_num,
       t.download,
       t.upload
FROM tb_stats_traffic AS t
LEFT JOIN tb_month_stats AS s
    ON s.fk_user = t.fk_user AND
       s.stats_date = t.stats_date
WHERE date_trunc('month', t.stats_date) < date_trunc('month', 'now'::DATE);

DROP FUNCTION sp_add_stats_traffic ( dm_name, DATE, SMALLINT, BIGINT, BIGINT );

CREATE FUNCTION sp_add_stats_traffic (_login dm_name,
                                      _dir_num SMALLINT,
                                      _upload BIGINT,
                                      _download BIGINT)
RETURNS INTEGER
AS $$
DECLARE
    _pk_user INTEGER;
BEGIN
    SELECT pk_user INTO _pk_user
        FROM tb_users
        WHERE name = _login;

    IF _pk_user IS NULL THEN
        RAISE EXCEPTION 'User % not found', _login;
        RETURN -1;
    END IF;

    UPDATE tb_stats_traffic SET
        upload = _upload,
        download = _download
    WHERE fk_user = _pk_user AND
          dir_num = _dir_num;

    IF NOT FOUND THEN
        INSERT INTO tb_stats_traffic
            (fk_user,
             dir_num,
             upload,
             download)
        VALUES
            (_pk_user,
             _dir_num,
             _upload,
             _download);
    END IF;

    RETURN 1;
END;
$$ LANGUAGE plpgsql;

DELETE FROM tb_stats_traffic WHERE date_trunc('month', stats_date) < date_trunc('month', 'now'::DATE);
ALTER TABLE tb_stats_traffic DROP stats_date;
ALTER TABLE tb_stats_traffic ADD UNIQUE (fk_user, dir_num);


CREATE FUNCTION sp_add_month_stats (_login dm_name,
                                    _stats_date DATE,
                                    _cash dm_money,
                                    _free_mb dm_money,
                                    _last_activity_time TIMESTAMP,
                                    _last_cash_add dm_money,
                                    _last_cash_add_time TIMESTAMP,
                                    _passive_time INTEGER)
RETURNS BIGINT
AS $$
DECLARE
    _pk_user INTEGER;
    _pk_month_stats BIGINT;
BEGIN
    SELECT pk_user INTO _pk_user
        FROM tb_users
        WHERE name = _login;

    IF _pk_user IS NULL THEN
        RAISE EXCEPTION 'User % not found', _login;
        RETURN -1;
    END IF;

    INSERT INTO tb_month_stats
        (fk_user,
         stats_date,
         cash,
         free_mb,
         last_activity_time,
         last_cash_add,
         last_cash_add_time,
         passive_time)
    VALUES
        (_pk_user,
         _stats_date,
         _cash,
         _free_mb,
         _last_activity_time,
         _last_cash_add,
         _last_cash_add_time,
         _passive_time);

    SELECT CURRVAL('tb_month_stats_pk_month_stats_seq') INTO _pk_month_stats;

    RETURN _pk_month_stats;
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION sp_add_month_stats_traffic (_pk_month_stats BIGINT,
                                            _dir_num SMALLINT,
                                            _upload BIGINT,
                                            _download BIGINT)
RETURNS INTEGER
AS $$
BEGIN
    INSERT INTO tb_month_stats_traffic
        (fk_month_stats,
         dir_num,
         upload,
         download)
    VALUES
        (_pk_month_stats,
         _dir_num,
         _upload,
         _download);

    RETURN 1;
END;
$$ LANGUAGE plpgsql;

UPDATE tb_info SET version = 7;

COMMIT;
