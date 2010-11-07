/*
 *  DB migration from v00 to v01 (postgres)
 */

ALTER TABLE tb_sessions_log ADD free_mb dm_money;
ALTER TABLE tb_sessions_log ADD reason TEXT;

DROP FUNCTION sp_add_session_log_entry ( dm_name, timestamp without time zone, dm_session_event_type, inet, dm_money);

CREATE FUNCTION sp_add_session_log_entry(_login dm_name,
                                         _event_time TIMESTAMP,
                                         _event_type dm_session_event_type,
                                         _ip INET,
                                         _cash dm_money,
                                         _free_mb dm_money,
                                         _reason TEXT)
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
         cash,
         free_mb,
         reason)
    VALUES
        (_pk_user,
         _event_time,
         _event_type,
         _ip,
         _cash,
         _free_mb,
         _reason);

    SELECT CURRVAL('tb_sessions_log_pk_session_log_seq') INTO _pk_session_log;

    RETURN _pk_session_log;
END;
$$ LANGUAGE plpgsql;

UPDATE tb_info SET version = 6;
