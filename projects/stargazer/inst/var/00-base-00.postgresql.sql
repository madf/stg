/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *****************************************************************************
 *
 * Скрипт генерации структуры базы для хранения данных Stargazer-a
 *
 * Примечание.
 *      * dm_permission_flag. Представляет собой битовую маску - rw.
 *          r - чтение, w - изменение параметра.
 *          0 - дествие запрещено, 1 - действие разрешено
 *
 *      * dm_traff_type. Число определяющее тип подсчета трафика:
 *          0 - up - считается по upload
 *          1 - down - считается по download
 *          2 - max - считается по максимальному среди upload/download
 *          3 - up+down - считается по сумме upload и download
 *
 *      * dm_session_event_type. Указывает тип записи в логе о сессии.
 *        'c' - connect, 'd' - disconnect.
 *
 *****************************************************************************
 */

/*
 *  $Revision: 1.12 $
 *  $Date: 2009/08/20 14:58:43 $
 */


/*
 *****************************************************************************
 * -= Создание типов и доменов =-
 *****************************************************************************
 */

CREATE DOMAIN dm_name AS VARCHAR(32) NOT NULL;
CREATE DOMAIN dm_password AS VARCHAR(64) NOT NULL;
CREATE DOMAIN dm_permission_flag AS SMALLINT NOT NULL
    CHECK ( value BETWEEN 0 AND 3 );
CREATE DOMAIN dm_money AS NUMERIC(12, 4) NOT NULL DEFAULT 0;
CREATE DOMAIN dm_traff_type AS SMALLINT NOT NULL
    CHECK ( value BETWEEN 0 AND 3 );
CREATE DOMAIN dm_day AS SMALLINT NOT NULL
    CHECK ( value BETWEEN 0 AND 31 )
    DEFAULT 0;
CREATE DOMAIN dm_session_event_type AS CHAR(1) NOT NULL
    CHECK ( value = 'c' OR value = 'd' );

/*
 *****************************************************************************
 * -= Создание таблиц =-
 *****************************************************************************
 */

CREATE TABLE tb_info
(
    version INTEGER NOT NULL
);

CREATE TABLE tb_admins
(
    pk_admin SERIAL PRIMARY KEY,
    login dm_name UNIQUE,
    passwd dm_password NOT NULL,
    chg_conf dm_permission_flag,
    chg_password dm_permission_flag,
    chg_stat dm_permission_flag,
    chg_cash dm_permission_flag,
    usr_add_del dm_permission_flag,
    chg_tariff dm_permission_flag,
    chg_admin dm_permission_flag,
    chg_service dm_permission_flag,
    chg_corporation dm_permission_flag
);

CREATE TABLE tb_tariffs
(
    pk_tariff SERIAL PRIMARY KEY,
    name dm_name UNIQUE,
    fee dm_money,
    free dm_money,
    passive_cost dm_money,
    traff_type dm_traff_type
);

CREATE TABLE tb_tariffs_params
(
    pk_tariff_param SERIAL PRIMARY KEY,
    fk_tariff INTEGER NOT NULL,
    dir_num SMALLINT NOT NULL,
    price_day_a dm_money,
    price_day_b dm_money,
    price_night_a dm_money,
    price_night_b dm_money,
    threshold INTEGER NOT NULL,
    time_day_begins TIME NOT NULL,
    time_day_ends TIME NOT NULL,

    FOREIGN KEY (fk_tariff)
        REFERENCES tb_tariffs (pk_tariff)
        ON DELETE CASCADE
);

CREATE TABLE tb_corporations
(
    pk_corporation SERIAL PRIMARY KEY,
    name dm_name UNIQUE,
    cash dm_money
);

CREATE TABLE tb_users
(
    pk_user SERIAL PRIMARY KEY,
    fk_tariff INTEGER,
    fk_tariff_change INTEGER,
    fk_corporation INTEGER,
    address VARCHAR(256) NOT NULL,
    always_online BOOLEAN NOT NULL,
    credit dm_money,
    credit_expire TIMESTAMP NOT NULL,
    disabled BOOLEAN NOT NULL,
    disabled_detail_stat BOOLEAN NOT NULL,
    email VARCHAR(256) NOT NULL,
    grp dm_name,
    note TEXT NOT NULL,
    passive BOOLEAN NOT NULL,
    passwd dm_password,
    phone VARCHAR(256) NOT NULL,
    name dm_name UNIQUE,
    real_name VARCHAR(256) NOT NULL,
    cash dm_money,
    free_mb dm_money,
    last_activity_time TIMESTAMP NOT NULL,
    last_cash_add dm_money,
    last_cash_add_time TIMESTAMP NOT NULL,
    passive_time INTEGER NOT NULL,

    FOREIGN KEY (fk_tariff)
        REFERENCES tb_tariffs (pk_tariff)
        ON DELETE CASCADE,
    FOREIGN KEY (fk_tariff_change)
        REFERENCES tb_tariffs (pk_tariff)
        ON DELETE CASCADE,
    FOREIGN KEY (fk_corporation)
        REFERENCES tb_corporations (pk_corporation)
        ON DELETE CASCADE
);

CREATE TABLE tb_detail_stats
(
    pk_detail_stat BIGSERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    dir_num SMALLINT NOT NULL,
    ip INET NOT NULL,
    download BIGINT NOT NULL,
    upload BIGINT NOT NULL,
    cost dm_money,
    from_time TIMESTAMP NOT NULL,
    till_time TIMESTAMP NOT NULL,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE
);

CREATE TABLE tb_services
(
    pk_service SERIAL PRIMARY KEY,
    name dm_name UNIQUE,
    comment TEXT NOT NULL,
    cost dm_money,
    pay_day dm_day
);

CREATE TABLE tb_users_services
(
    pk_user_service SERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    fk_service INTEGER NOT NULL,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE,
    FOREIGN KEY (fk_service)
        REFERENCES tb_services (pk_service)
);

CREATE TABLE tb_messages
(
    pk_message SERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    ver SMALLINT NOT NULL,
    msg_type SMALLINT NOT NULL,
    last_send_time TIMESTAMP NOT NULL,
    creation_time TIMESTAMP NOT NULL,
    show_time INTEGER NOT NULL,
    repeat SMALLINT NOT NULL,
    repeat_period INTEGER NOT NULL,
    msg_text TEXT NOT NULL,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE
);

CREATE TABLE tb_stats_traffic
(
    pk_stat_traffic BIGSERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    stats_date DATE NOT NULL,
    dir_num SMALLINT NOT NULL,
    download BIGINT NOT NULL,
    upload BIGINT NOT NULL,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE,
    UNIQUE (fk_user, stats_date, dir_num)
);

CREATE TABLE tb_users_data
(
    pk_user_data SERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    num SMALLINT NOT NULL,
    data VARCHAR(256) NOT NULL,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE
);

CREATE TABLE tb_allowed_ip
(
    pk_allowed_ip SERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    ip INET NOT NULL,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE
);

CREATE TABLE tb_sessions_log
(
    pk_session_log SERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    event_time TIMESTAMP NOT NULL,
    event_type dm_session_event_type,
    ip INET NOT NULL,
    cash dm_money,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE
);

CREATE TABLE tb_sessions_data
(
    pk_session_data SERIAL PRIMARY KEY,
    fk_session_log INTEGER NOT NULL,
    dir_num SMALLINT NOT NULL,
    session_upload BIGINT NOT NULL,
    session_download BIGINT NOT NULL,
    month_upload BIGINT NOT NULL,
    month_download BIGINT NOT NULL,

    FOREIGN KEY (fk_session_log)
        REFERENCES tb_sessions_log (pk_session_log)
        ON DELETE CASCADE
);

CREATE TABLE tb_parameters
(
    pk_parameter SERIAL PRIMARY KEY,
    name dm_name UNIQUE
);

CREATE TABLE tb_params_log
(
    pk_param_log SERIAL PRIMARY KEY,
    fk_user INTEGER NOT NULL,
    fk_parameter INTEGER NOT NULL,
    fk_admin INTEGER NOT NULL,
    ip INET NOT NULL,
    event_time TIMESTAMP NOT NULL,
    from_val VARCHAR(256),
    to_val VARCHAR(256),
    comment TEXT,

    FOREIGN KEY (fk_user)
        REFERENCES tb_users (pk_user)
        ON DELETE CASCADE,
    FOREIGN KEY (fk_parameter)
        REFERENCES tb_parameters (pk_parameter),
    FOREIGN KEY (fk_admin)
        REFERENCES tb_admins (pk_admin)
        ON DELETE CASCADE
);

/*
 *****************************************************************************
 * -= Создание хранимых процедур =-
 *****************************************************************************
 */

CREATE FUNCTION sp_add_message(_login dm_name,
                               _ver SMALLINT,
                               _msg_type SMALLINT,
                               _last_send_time TIMESTAMP,
                               _creation_time TIMESTAMP,
                               _show_time INTEGER,
                               _repeat SMALLINT,
                               _repeat_period INTEGER,
                               _msg_text TEXT)
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
    INSERT INTO tb_messages
        (fk_user,
         ver,
         msg_type,
         last_send_time,
         creation_time,
         show_time,
         repeat,
         repeat_period,
         msg_text)
    VALUES
        (_pk_user,
         _ver,
         _msg_type,
         _last_send_time,
         _creation_time,
         _show_time,
         _repeat,
         _repeat_period,
         _msg_text);
    RETURN CURRVAL('tb_messages_pk_message_seq');
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION sp_add_tariff(_name dm_name, _dirs INTEGER)
RETURNS INTEGER
AS $$
DECLARE
    pk_tariff INTEGER;
BEGIN
    INSERT INTO tb_tariffs
        (name,
         fee,
         free,
         passive_cost,
         traff_type)
    VALUES
        (_name,
         0, 0, 0, 0);
    SELECT CURRVAL('tb_tariffs_pk_tariff_seq') INTO pk_tariff;
    FOR i IN 1.._dirs LOOP
        INSERT INTO tb_tariffs_params
            (fk_tariff,
             dir_num,
             price_day_a,
             price_day_b,
             price_night_a,
             price_night_b,
             threshold,
             time_day_begins,
             time_day_ends)
        VALUES
            (pk_tariff,
             i - 1,
             0, 0, 0, 0, 0,
             CAST('1970-01-01 00:00:00+00' AS TIMESTAMP),
             CAST('1970-01-01 00:00:00+00' AS TIMESTAMP));
    END LOOP;
    RETURN pk_tariff;
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION sp_add_user(_name dm_name)
RETURNS INTEGER
AS $$
DECLARE
    pk_user INTEGER;
BEGIN
    INSERT INTO tb_users
        (fk_tariff,
         fk_tariff_change,
         fk_corporation,
         address,
         always_online,
         credit,
         credit_expire,
         disabled,
         disabled_detail_stat,
         email,
         grp,
         note,
         passive,
         passwd,
         phone,
         name,
         real_name,
         cash,
         free_mb,
         last_activity_time,
         last_cash_add,
         last_cash_add_time,
         passive_time)
    VALUES
        (NULL, NULL, NULL, '', FALSE, 0, CAST('now' AS TIMESTAMP),
         FALSE, FALSE, '', '', '', FALSE, '', '', _name, '', 0, 0,
         CAST('now' AS TIMESTAMP), 0, CAST('now' AS TIMESTAMP), 0);
    SELECT CURRVAL('tb_users_pk_user_seq') INTO pk_user;
    RETURN pk_user;
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION sp_add_stats_traffic (_login dm_name,
                                      _stats_date DATE,
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
          dir_num = _dir_num AND
          stats_date = _stats_date;

    IF NOT FOUND THEN
        INSERT INTO tb_stats_traffic
            (fk_user,
             dir_num,
             stats_date,
             upload,
             download)
        VALUES
            (_pk_user,
             _dir_num,
             _stats_date,
             _upload,
             _download);
    END IF;

    RETURN 1;
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION sp_set_user_data (_pk_user INTEGER,
                                  _num SMALLINT,
                                  _data VARCHAR(256))
RETURNS INTEGER
AS $$
BEGIN
    UPDATE tb_users_data SET
        data = _data
    WHERE fk_user = _pk_user AND num = _num;

    IF NOT FOUND THEN
        INSERT INTO tb_users_data
            (fk_user,
             num,
             data)
        VALUES
            (_pk_user,
             _num,
             _data);
    END IF;

    RETURN 1;
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION sp_add_param_log_entry(_login dm_name,
                                       _admin_login dm_name,
                                       _ip INET,
                                       _param_name dm_name,
                                       _event_time TIMESTAMP,
                                       _from VARCHAR(256),
                                       _to VARCHAR(256),
                                       _comment TEXT)
RETURNS INTEGER
AS $$
DECLARE
    _pk_user INTEGER;
    _pk_admin INTEGER;
    _pk_param INTEGER;
BEGIN
    SELECT pk_user INTO _pk_user
        FROM tb_users
        WHERE name = _login;
    IF _pk_user IS NULL THEN
        RAISE EXCEPTION 'User % not found', _login;
        RETURN -1;
    END IF;

    SELECT pk_admin INTO _pk_admin
        FROM tb_admins
        WHERE login = _admin_login;
    IF _pk_admin IS NULL THEN
        RAISE EXCEPTION 'Admin % not found', _admin_login;
        RETURN -1;
    END IF;

    SELECT pk_parameter INTO _pk_param
        FROM tb_parameters
        WHERE name = _param_name;

    IF NOT FOUND THEN
        INSERT INTO tb_parameters (name) VALUES (_param_name);
        SELECT CURRVAL('tb_parameters_pk_parameter_seq') INTO _pk_param;
    END IF;

    INSERT INTO tb_params_log
        (fk_user,
         fk_parameter,
         fk_admin,
         ip,
         event_time,
         from_val,
         to_val,
         comment)
    VALUES
        (_pk_user,
         _pk_param,
         _pk_admin,
         _ip,
         _event_time,
         _from,
         _to,
         _comment);

    RETURN 1;
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION sp_add_session_log_entry(_login dm_name,
                                         _event_time TIMESTAMP,
                                         _event_type dm_session_event_type,
                                         _ip INET,
                                         _cash dm_money)
RETURNS INTEGER
AS $$
DECLARE
    _pk_user INTEGER;
    _pk_session_log INTEGER;
BEGIN
    SELECT pk_user INTO _pk_user
        FROM tb_users
        WHERE name = _login;
    IF _pk_user IS NULL THEN
        RAISE EXCEPTION 'User % not found', _login;
        RETURN -1;
    END IF;
    
    INSERT INTO tb_sessions_log
        (fk_user,
         event_time,
         event_type,
         ip,
         cash)
    VALUES
        (_pk_user,
         _event_time,
         _event_type,
         _ip,
         _cash);

    SELECT CURRVAL('tb_sessions_log_pk_session_log_seq') INTO _pk_session_log;

    RETURN _pk_session_log;
END;
$$ LANGUAGE plpgsql;

/*
 *****************************************************************************
 * -= Создание администратора =-
 *
 * Двоичные права доступа пока не поддерживаются, по этому используются флаги
 *****************************************************************************
 */
INSERT INTO tb_admins
    (login, passwd,
     chg_conf, chg_password, chg_stat,
     chg_cash, usr_add_del, chg_tariff,
     chg_admin, chg_service, chg_corporation)
VALUES
    ('admin',
     'geahonjehjfofnhammefahbbbfbmpkmkmmefahbbbfbmpkmkmmefahbbbfbmpkmk',
     1, 1, 1, 1, 1, 1, 1, 1, 1);
INSERT INTO tb_admins
    (login, passwd,
     chg_conf, chg_password, chg_stat,
     chg_cash, usr_add_del, chg_tariff,
     chg_admin, chg_service, chg_corporation)
VALUES
    ('@stargazer',
     '',
     0, 0, 0, 0, 0, 0, 0, 0, 0);

INSERT INTO tb_info
    (version)
VALUES
    (5);
