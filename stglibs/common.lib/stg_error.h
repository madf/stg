/*
 *****************************************************************************
 *
 * File:        stg_error.h
 *
 * Description: Коды ошибок
 *
 * $Id: stg_error.h,v 1.1.1.1 2005/09/29 11:33:18 boris Exp $
 *
 *****************************************************************************
 */

#ifndef _STG_ERROR_H_
#define _STG_ERROR_H_


/* Начало области ошибок модуля astat.cgi */
#define ERROR_ASTAT_START   (100000)
/* Начало области ошибок библитеки qparam.lib */
#define ERROR_QPARAM_START  (102000)
/* Начало области ошибок библитеки diagram.lib */
#define ERROR_DIAGRAM_START (103000)


typedef enum
{
    SUCCESS = 0,

    ERROR_CONFIG_READ = ERROR_ASTAT_START,
    ERROR_PORT_NUM,
    ERROR_CLEAR_SID_DIR,
    ERROR_UNKNOWN_HTTP_METHOD,
    ERROR_NULL_HTTP_METHOD,
    ERROR_UNKNOWN_QUERY,
    ERROR_LOGIN,        // пока что одна ошибка на много ситуаций:
                        //   * не верное имя и пароль
                        //   * нет связи с сервером
                        //   * ....
                        // это связано с кодами ошибок модуля srvconf.lib
                        // в дальнейшем надо передалать все на один
                        // enum ошибок
    ERROR_PREPARE_USER_SELECTION_PAGE_0,
    ERROR_ADD_IFACE,
    ERROR_ADD_TARIFF,
    ERROR_ADD_GROUP,
    ERROR_ADD_USER,
    ERROR_CREATE_SID,
    ERROR_SET_SID,
    ERROR_UPDATE_SID,
    ERROR_READ_SID_DATA,
    ERROR_WRITE_SID_DATA,
    ERROR_REMOVE_EXPIRED_SID,

    ERROR_MEMORY_ALLOCATE = ERROR_QPARAM_START,
    ERROR_MEMORY_DESPOSE,
    ERROR_NULL_QUERY,   // запрос не получен
    ERROR_QUERY,        // ошибка в запросе - несоответствие стандарту

    ERROR_ARC_DATA_FULL = ERROR_DIAGRAM_START,
    ERROR_ARC_PERCENT,
    TODO
} RESULT_DATA;


char * GetErrorString(RESULT_DATA res);

#endif  /* _STG_ERROR_H_ */

/* EOF */

