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
 * $Id: 00-base-00.sql,v 1.7 2010/01/06 14:41:13 faust Exp $
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
 *        Как альтернативу этому полю можно сделать еще одну таблицу - типов
 *        подсчета трафика. И в этом поле хранить ссылку на эту таблицу.
 *        Вопрос только "А надо ли это?"
 *
 *      * dm_ip. IP адресс в виде четырех байтового целого числа со знаком.
 *        Выполнять приведение к знаковуму целому при занесении IP в БД!!!
 *
 *      * dm_period. Задает периодичность показа сообщения пользователю.
 *        Период задается целым числом (int16). Если значение равно 0 то
 *        сообщение показывается только при подключении пользователя.
 *        Также этот домен определяет промежуток времени в течении которого
 *        сообщение показывается пользователю.
 *
 *      * dm_session_event_type. Указывает тип записи в логе о сессии.
 *        'c' - connect, 'd' - disconnect.
 *
 *****************************************************************************
 */

/*
 * CONNECT 'localhost:/var/stg/stargazer.fdb' USER 'stg' PASSWORD '123456';
 * DROP DATABASE;
 *
 * CREATE DATABASE 'localhost:/var/stg/stargazer.fdb' USER 'stg' PASSWORD '123456' DEFAULT CHARACTER SET UTF8;
 */

 

/*
 *****************************************************************************
 * -= Создание ДОМЕНОВ =-
 *****************************************************************************
 */

CREATE DOMAIN dm_id AS INTEGER NOT NULL;
CREATE DOMAIN dm_null_id AS INTEGER;
CREATE DOMAIN dm_login AS VARCHAR(32) NOT NULL;
CREATE DOMAIN dm_tariff_name AS VARCHAR(32) NOT NULL;
CREATE DOMAIN dm_group_name AS VARCHAR(32);
CREATE DOMAIN dm_corporation_name AS VARCHAR(32);
CREATE DOMAIN dm_parameter_name AS VARCHAR(32);

CREATE DOMAIN dm_password AS VARCHAR(64) NOT NULL;
/* bitmask - rw => Read, Write */
CREATE DOMAIN dm_permission_flag AS SMALLINT NOT NULL
    CHECK ( VALUE BETWEEN 0 AND 3 );
CREATE DOMAIN dm_money AS NUMERIC(10,6) NOT NULL;
/* (0, 1, 2, 3) => (up, down, max, up+down) */
CREATE DOMAIN dm_traff_type AS SMALLINT NOT NULL
    CHECK ( VALUE BETWEEN 0 AND 3 );
CREATE DOMAIN dm_dir_num AS SMALLINT NOT NULL;
CREATE DOMAIN dm_num AS SMALLINT NOT NULL;
CREATE DOMAIN dm_traffic_mb AS INTEGER NOT NULL;
CREATE DOMAIN dm_traffic_byte AS BIGINT NOT NULL;
CREATE DOMAIN dm_time AS TIME NOT NULL;
CREATE DOMAIN dm_moment AS TIMESTAMP NOT NULL;
CREATE DOMAIN dm_credit_moment AS TIMESTAMP;
CREATE DOMAIN dm_ip AS INTEGER NOT NULL;
CREATE DOMAIN dm_mask AS INTEGER NOT NULL;
CREATE DOMAIN dm_user_address AS VARCHAR(256) DEFAULT '';
CREATE DOMAIN dm_bool AS CHAR(1) NOT NULL
    CHECK ( VALUE IN ('0', '1', 't', 'f', 'T', 'F') );
CREATE DOMAIN dm_email AS VARCHAR(256) DEFAULT '';
CREATE DOMAIN dm_note AS VARCHAR(256) DEFAULT '';
CREATE DOMAIN dm_phone AS VARCHAR(256) DEFAULT '';
CREATE DOMAIN dm_user_name AS VARCHAR(256) DEFAULT '';
CREATE DOMAIN dm_service_comment AS VARCHAR(256) DEFAULT '';
CREATE DOMAIN dm_service_name AS VARCHAR(32) DEFAULT '';
/* TODO: why 0-31? Which is default? */
CREATE DOMAIN dm_pay_day AS SMALLINT NOT NULL
    CHECK ( VALUE BETWEEN 0 AND 31 );
CREATE DOMAIN dm_period AS INTEGER NOT NULL;
CREATE DOMAIN dm_counter AS SMALLINT NOT NULL;
/* Is it needded? */
CREATE DOMAIN dm_message_ver AS INTEGER NOT NULL;
CREATE DOMAIN dm_message_type AS INTEGER NOT NULL;
/*----------------*/
CREATE DOMAIN dm_message AS VARCHAR(256) NOT NULL;
CREATE DOMAIN dm_user_data AS VARCHAR(256) NOT NULL;
CREATE DOMAIN dm_session_event_type AS CHAR(1) NOT NULL
    CHECK ( VALUE IN ('c', 'd') );
CREATE DOMAIN dm_char_value AS VARCHAR(64) NOT NULL;
CREATE DOMAIN dm_date AS DATE NOT NULL;



/*
 *****************************************************************************
 * -= Создание ТАБЛИЦ =-
 *****************************************************************************
 */

CREATE TABLE tb_admins
(
    pk_admin dm_id PRIMARY KEY,
    login dm_login UNIQUE,
    passwd dm_password,
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
    pk_tariff dm_id PRIMARY KEY,
    name dm_tariff_name UNIQUE,
    fee dm_money,
    free dm_money,
    passive_cost dm_money,
    traff_type dm_traff_type
);

CREATE TABLE tb_tariffs_params
(
    pk_tariff_param dm_id PRIMARY KEY,
    fk_tariff dm_id,
    dir_num dm_dir_num,
    price_day_a dm_money,
    price_day_b dm_money,
    price_night_a dm_money,
    price_night_b dm_money,
    threshold dm_traffic_mb,
    time_day_begins dm_time,
    time_day_ends dm_time,

    FOREIGN KEY (fk_tariff) REFERENCES tb_tariffs (pk_tariff)
);

CREATE TABLE tb_corporations
(
    pk_corporation dm_id PRIMARY KEY,
    name dm_corporation_name UNIQUE,
    cash dm_money
);

CREATE TABLE tb_users
(
    pk_user dm_id PRIMARY KEY,
    fk_tariff dm_null_id,
    fk_tariff_change dm_null_id,
    fk_corporation dm_null_id,
    address dm_user_address,
    always_online dm_bool,
    credit dm_money,
    credit_expire dm_credit_moment,
    disabled dm_bool,
    disabled_detail_stat dm_bool,
    email dm_email,
    grp dm_group_name,
    note dm_note,
    passive dm_bool,
    passwd dm_password,
    phone dm_phone,
    name dm_login UNIQUE,
    real_name dm_user_name,

    FOREIGN KEY (fk_tariff) REFERENCES tb_tariffs (pk_tariff),
    FOREIGN KEY (fk_tariff_change) REFERENCES tb_tariffs (pk_tariff),
    FOREIGN KEY (fk_corporation) REFERENCES tb_corporations (pk_corporation)
);

CREATE TABLE tb_detail_stats
(
    pk_detail_stat dm_id PRIMARY KEY,
    fk_user dm_id,
    dir_num dm_dir_num,
    ip dm_ip,
    download dm_traffic_byte,
    upload dm_traffic_byte,
    cost dm_money,
    from_time dm_moment,
    till_time dm_moment,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user)
);

CREATE TABLE tb_services
(
    pk_service dm_id PRIMARY KEY,
    name dm_service_name UNIQUE,
    comment dm_service_comment,
    cost dm_money,
    pay_day dm_pay_day
);

CREATE TABLE tb_users_services
(
    pk_user_service dm_id PRIMARY KEY,
    fk_user dm_id,
    fk_service dm_id,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user),
    FOREIGN KEY (fk_service) REFERENCES tb_services (pk_service)
);

CREATE TABLE tb_messages
(
    pk_message dm_id PRIMARY KEY,
    fk_user dm_id,
    ver dm_message_ver,
    msg_type dm_message_type,
    last_send_time dm_period,
    creation_time dm_period,
    show_time dm_period,
    repeat dm_counter,
    repeat_period dm_period,
    msg_text dm_message,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user)
);

CREATE TABLE tb_stats
(
    pk_stat dm_id PRIMARY KEY,
    fk_user dm_id,
    cash dm_money,
    free_mb dm_money,
    last_activity_time dm_moment,
    last_cash_add dm_money,
    last_cash_add_time dm_moment,
    passive_time dm_period,
    stats_date dm_date,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user)
);

CREATE TABLE tb_stats_traffic
(
    pk_stat_traffic dm_id PRIMARY KEY,
    fk_stat dm_id,
    dir_num dm_dir_num,
    download dm_traffic_byte,
    upload dm_traffic_byte,

    FOREIGN KEY (fk_stat) REFERENCES tb_stats (pk_stat)
);

CREATE TABLE tb_users_data
(
    pk_user_data dm_id PRIMARY KEY,
    fk_user dm_id,
    num dm_num, /* data_id dm_id renamed */
    data dm_user_data,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user)
);

CREATE TABLE tb_allowed_ip
(
    pk_allowed_ip dm_id PRIMARY KEY,
    fk_user dm_id,
    ip dm_ip,
    mask dm_mask,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user)
);

CREATE TABLE tb_sessions_log
(
    pk_session_log dm_id PRIMARY KEY,
    fk_user dm_id,
    event_time dm_moment,
    event_type dm_session_event_type,
    ip dm_ip,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user)
);

CREATE TABLE tb_sessions_data
(
    pk_session_data dm_id PRIMARY KEY,
    fk_session_log dm_id,
    dir_num dm_dir_num,
    session_upload dm_traffic_byte,
    session_download dm_traffic_byte,
    month_upload dm_traffic_byte,
    month_download dm_traffic_byte,

    FOREIGN KEY (fk_session_log) REFERENCES tb_sessions_log (pk_session_log)
);

CREATE TABLE tb_parameters
(
    pk_parameter dm_id PRIMARY KEY,
    name dm_parameter_name UNIQUE
);

CREATE TABLE tb_params_log
(
    pk_param_log dm_id PRIMARY KEY,
    fk_user dm_id,
    fk_parameter dm_id,
    event_time dm_moment,
    from_val dm_char_value,
    to_val dm_char_value,
    comment dm_service_comment,

    FOREIGN KEY (fk_user) REFERENCES tb_users (pk_user),
    FOREIGN KEY (fk_parameter) REFERENCES tb_parameters (pk_parameter)
);


/*
 *****************************************************************************
 * -= Создание ИНДЕКСОВ =-
 *****************************************************************************
 */



/*
 *****************************************************************************
 * -= Создание ГЕНЕРАТОРОВ =-
 *****************************************************************************
 */

CREATE  GENERATOR gn_pk_admin;
SET     GENERATOR gn_pk_admin           TO 0;
CREATE  GENERATOR gn_pk_tariff;
SET     GENERATOR gn_pk_tariff          TO 0;
CREATE  GENERATOR gn_pk_tariff_param;
SET     GENERATOR gn_pk_tariff_param    TO 0;
CREATE  GENERATOR gn_pk_corporation;
SET     GENERATOR gn_pk_corporation     TO 0;
CREATE  GENERATOR gn_pk_user;
SET     GENERATOR gn_pk_user            TO 0;
CREATE  GENERATOR gn_pk_detail_stat;
SET     GENERATOR gn_pk_detail_stat     TO 0;
CREATE  GENERATOR gn_pk_service;
SET     GENERATOR gn_pk_service         TO 0;
CREATE  GENERATOR gn_pk_user_service;
SET     GENERATOR gn_pk_user_service    TO 0;
CREATE  GENERATOR gn_pk_message;
SET     GENERATOR gn_pk_message         TO 0;
CREATE  GENERATOR gn_pk_stat;
SET     GENERATOR gn_pk_stat            TO 0;
CREATE  GENERATOR gn_pk_stat_traffic;
SET     GENERATOR gn_pk_stat_traffic    TO 0;
CREATE  GENERATOR gn_pk_user_data;
SET     GENERATOR gn_pk_user_data       TO 0;
CREATE  GENERATOR gn_pk_allowed_ip;
SET     GENERATOR gn_pk_allowed_ip      TO 0;
CREATE  GENERATOR gn_pk_session;
SET     GENERATOR gn_pk_session         TO 0;
CREATE  GENERATOR gn_pk_session_log;
SET     GENERATOR gn_pk_session_log     TO 0;
CREATE  GENERATOR gn_pk_session_data;
SET     GENERATOR gn_pk_session_data    TO 0;
CREATE  GENERATOR gn_pk_parameter;
SET     GENERATOR gn_pk_parameter       TO 0;
CREATE  GENERATOR gn_pk_param_log;
SET     GENERATOR gn_pk_param_log       TO 0;


/*
 *****************************************************************************
 * -= Создание ТРИГГЕРОВ =-
 *****************************************************************************
 */

SET TERM !! ;
CREATE TRIGGER tr_admin_bi FOR tb_admins
ACTIVE BEFORE INSERT POSITION 0
AS
BEGIN
    IF (new.pk_admin IS NULL)
    THEN new.pk_admin = GEN_ID(gn_pk_admin, 1);
END !!
SET TERM ; !!

/*set term !! ;
create trigger tr_tariff_bi for tb_tariffs active
before insert position 0
as
begin
    if (new.pk_tariff is null)
    then new.pk_tariff = gen_id(gn_pk_tariff, 1);
end !!
set term ; !!*/

set term !! ;
create trigger tr_tariff_param_bi for tb_tariffs_params active
before insert position 0
as
begin
    if (new.pk_tariff_param is null)
    then new.pk_tariff_param = gen_id(gn_pk_tariff_param, 1);
end !!
set term ; !!

set term !! ;
create trigger tr_corporation_bi for tb_corporations active
before insert position 0
as
begin
    if (new.pk_corporation is null)
    then new.pk_corporation = gen_id(gn_pk_corporation, 1);
end !!
set term ; !!

/*set term !! ;
create trigger tr_user_bi for tb_users active
before insert position 0
as
begin
    if (new.pk_user is null)
    then new.pk_user = gen_id(gn_pk_user, 1);
end !!
set term ; !!*/

set term !! ;
create trigger tr_detail_stat_bi for tb_detail_stats active
before insert position 0
as
begin
    if (new.pk_detail_stat is null)
    then new.pk_detail_stat = gen_id(gn_pk_detail_stat, 1);
end !!
set term ; !!

set term !! ;
create trigger tr_service_bi for tb_services active
before insert position 0
as
begin
    if (new.pk_service is null)
    then new.pk_service = gen_id(gn_pk_service, 1);
end !!
set term ; !!

set term !! ;
create trigger tr_user_service_bi for tb_users_services active
before insert position 0
as
begin
    if (new.pk_user_service is null)
    then new.pk_user_service = gen_id(gn_pk_user_service, 1);
end !!
set term ; !!

/*set term !! ;
create trigger tr_message_bi for tb_messages active
before insert position 0
as
begin
    if (new.pk_message is null)
    then new.pk_message = gen_id(gn_pk_message, 1);
end !!
set term ; !!*/

/*set term !! ;
create trigger tr_stat_bi for tb_stats active
before insert position 0
as
begin
    if (new.pk_stat is null)
    then new.pk_stat = gen_id(gn_pk_stat, 1);
end !!
set term ; !!*/

set term !! ;
create trigger tr_stat_traffic_bi for tb_stats_traffic active
before insert position 0
as
begin
    if (new.pk_stat_traffic is null)
    then new.pk_stat_traffic = gen_id(gn_pk_stat_traffic, 1);
end !!
set term ; !!

set term !! ;
create trigger tr_user_data_bi for tb_users_data active
before insert position 0
as
begin
    if (new.pk_user_data is null)
    then new.pk_user_data = gen_id(gn_pk_user_data, 1);
end !!
set term ; !!

set term !! ;
create trigger tr_allowed_ip_bi for tb_allowed_ip active
before insert position 0
as
begin
    if (new.pk_allowed_ip is null)
    then new.pk_allowed_ip = gen_id(gn_pk_allowed_ip, 1);
end !!
set term ; !!

/*set term !! ;
create trigger tr_session_log_bi for tb_sessions_log active
before insert position 0
as
begin
    if (new.pk_session_log is null)
    then new.pk_session_log = gen_id(gn_pk_session_log, 1);
end !!
set term ; !!*/

set term !! ;
create trigger tr_session_data_bi for tb_sessions_data active
before insert position 0
as
begin
    if (new.pk_session_data is null)
    then new.pk_session_data = gen_id(gn_pk_session_data, 1);
end !!
set term ; !!

set term !! ;
create trigger tr_parameter_bi for tb_parameters active
before insert position 0
as
begin
    if (new.pk_parameter is null)
    then new.pk_parameter = gen_id(gn_pk_parameter, 1);
end !!
set term ; !!

set term !! ;
create trigger tr_param_log_bi for tb_params_log active
before insert position 0
as
begin
    if (new.pk_param_log is null)
    then new.pk_param_log = gen_id(gn_pk_param_log, 1);
end !!
set term ; !!

/*
 *****************************************************************************
 * -= Создание stored procedure =-
 *****************************************************************************
 */

/*
 * Add a message returning it's ID
 */
set term !! ;
create procedure sp_add_message(pk_message integer, login varchar(32), ver integer, msg_type integer, last_send_time integer, creation_time integer, show_time integer, repeat integer, repeat_period integer, msg_text varchar(256))
returns(res integer)
as  
begin
    if (:pk_message is null) then
    begin
	pk_message = gen_id(gn_pk_message, 1);
	insert into tb_messages values (:pk_message,
					(select pk_user from tb_users where name = :login),
					:ver,
					:msg_type,
					:last_send_time,
					:creation_time, 
					:show_time,
					:repeat,
					:repeat_period,
					:msg_text);
    end
    else
    begin
	update tb_messages set fk_user = (select pk_user from tb_users where name = :login),
			       ver = :ver,
			       msg_type = :msg_type,
			       last_send_time = :last_send_time,
			       creation_time = :creation_time,
			       show_time = :show_time,
			       repeat = :repeat_period,
			       repeat_period = :repeat_period,
			       msg_text = :msg_text
			   where pk_message = :pk_message;
    end
    res = :pk_message;
end !!
set term ; !!

set term !! ;
create procedure sp_delete_service(name varchar(32))
as
declare variable pk_service integer;
begin
    select pk_service from tb_services where name = :name into pk_service;
    if (pk_service is  not null) then
    begin
	delete from tb_users_services where fk_service = :pk_service;
	delete from tb_services where pk_service = :pk_service;
    end
end !!
set term ; !!

set term !! ;
create procedure sp_add_tariff(name varchar(32), dirs integer)
as
declare variable pk_tariff integer;
begin
    pk_tariff = gen_id(gn_pk_tariff, 1);
    insert into tb_tariffs (pk_tariff, name, fee, free, passive_cost, traff_type) values (:pk_tariff, :name, 0, 0, 0, 0);
    while (dirs > 0) do
    begin
        insert into tb_tariffs_params (fk_tariff, dir_num, price_day_a, 
					price_day_b, price_night_a, price_night_b, 
					threshold, time_day_begins, time_day_ends)
		    values (:pk_tariff, :dirs - 1, 0, 0, 0, 0, 0, '0:0', '0:0');
	dirs = dirs - 1;
    end
end !!
set term ; !!

set term !! ;
create procedure sp_delete_tariff(name varchar(32))
as
declare variable pk_tariff integer;
begin
    select pk_tariff from tb_tariffs where name = :name into pk_tariff;
    if (pk_tariff is not null) then
    begin
	delete from tb_tariffs_params where fk_tariff = :pk_tariff;
	delete from tb_tariffs where pk_tariff = :pk_tariff;
    end
end !!
set term ; !!

set term !! ;
create procedure sp_add_user(name varchar(32), dirs integer)
as
declare variable pk_user integer;
declare variable pk_stat integer;
begin
    pk_user = gen_id(gn_pk_user, 1);
    insert into tb_users(pk_user, fk_tariff, fk_tariff_change, fk_corporation, address, always_online, credit, credit_expire, disabled, disabled_detail_stat, email, grp, note, passive, passwd, phone, name, real_name) values (:pk_user, NULL, NULL, NULL, '', 0, 0, 'now', 0, 0, '', '_', '', 0, '', '', :name, '');
    pk_stat = gen_id(gn_pk_stat, 1);
    insert into tb_stats values (:pk_stat, :pk_user, 0, 0, 'now', 0, 'now', 0, 'now');
    while (dirs > 0) do
    begin
	insert into tb_stats_traffic (fk_stat, dir_num, upload, download) values (:pk_stat, :dirs - 1, 0, 0);
	dirs = dirs - 1;
    end
end!!
set term ; !!

set term !! ;
create procedure sp_delete_user(name varchar(32))
as
declare variable pk_user integer;
begin
    select pk_user from tb_users where name = :name into pk_user;
    if (pk_user is not null) then
    begin
	delete from tb_users_services where fk_user = :pk_user;
	delete from tb_params_log where fk_user = :pk_user;
	delete from tb_detail_stats where fk_user = :pk_user;
	delete from tb_stats_traffic where fk_stat in (select pk_stat from tb_stats where fk_user = :pk_user);
	delete from tb_stats where fk_user = :pk_user;
	delete from tb_sessions_data where fk_session_log in (select pk_session_log from tb_sessions_log where fk_user = :pk_user);
	delete from tb_sessions_log where fk_user = :pk_user;
	delete from tb_allowed_ip where fk_user = :pk_user;
	delete from tb_users_data where fk_user = :pk_user;
	delete from tb_messages where fk_user = :pk_user;
	delete from tb_users where pk_user = :pk_user;
    end
end !!
set term ; !!

set term !! ;
create procedure sp_append_session_log(name varchar(32), event_time timestamp, event_type char(1), ip integer)
returns(pk_session_log integer)
as
begin
    pk_session_log = gen_id(gn_pk_session_log, 1);
    insert into tb_sessions_log (pk_session_log, fk_user, event_time, event_type, ip) values (:pk_session_log, (select pk_user from tb_users where name = :name), :event_time, :event_type, :ip);
end !!
set term ; !!

set term !! ;
create procedure sp_add_stat(name varchar(32), cash numeric(10,6), free_mb numeric(10,6), last_activity_time timestamp, last_cash_add numeric(10,6), last_cash_add_time timestamp, passive_time integer, stats_date date)
returns(pk_stat integer)
as
begin
    pk_stat = gen_id(gn_pk_stat, 1);
    insert into tb_stats (pk_stat, fk_user, cash, free_mb, last_activity_time, last_cash_add, last_cash_add_time, passive_time, stats_date) values (:pk_stat, (select pk_user from tb_users where name = :name), :cash, :free_mb, :last_activity_time, :last_cash_add, :last_cash_add_time, :passive_time, :stats_date);
end !!
set term ; !!

/*
 *****************************************************************************
 * -= Создание администратора =-
 *****************************************************************************
 */

insert into tb_admins values(0, 'admin', 'geahonjehjfofnhammefahbbbfbmpkmkmmefahbbbfbmpkmkmmefahbbbfbmpkmk', 1, 1, 1, 1, 1, 1, 1, 1, 1);

/* EOF */

