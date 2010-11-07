/*
 *  DB migration from v00 to v01 (firebird)
 */

alter table tb_users add disabled_detail_stat dm_bool;

drop procedure sp_add_user;

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
