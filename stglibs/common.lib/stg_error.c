/*
 *****************************************************************************
 *
 * File:        stg_error.c
 *
 * Description: Коды Ошибок проекта StarGazer
 *
 * $Id: stg_error.c,v 1.1.1.1 2005/09/29 11:33:18 boris Exp $
 *
 *****************************************************************************
 */

#include "stg_error.h"
//#include "debug.h"


/*
 *****************************************************************************
 * -= Поиск сообщения об ошибке по коду ошибки =-
 *****************************************************************************
 */
char * GetErrorString(RESULT_DATA res)
{
    char * errorString;

    switch (res)
    {
        case SUCCESS:
        {
            errorString = "OK:   Work finished successfully";
            break;
        }
        /* astat.cgi */
        case ERROR_CONFIG_READ:
        {
            errorString = "FAIL: Read config file";
            break;
        }
        case ERROR_PORT_NUM:
        {
            errorString = "FAIL: Port value incorrect";
            break;
        }
        case ERROR_CLEAR_SID_DIR:
        {
            errorString = "FAIL: ClearSidDir() return fail";
            break;
        }
        case ERROR_UNKNOWN_HTTP_METHOD:
        {
            errorString = "FAIL: Umknown HTTP method";
            break;
        }
        case ERROR_NULL_HTTP_METHOD:
        {
            errorString = "FAIL: NULL HTTP method";
            break;
        }
        case ERROR_UNKNOWN_QUERY:
        {
            errorString = "FAIL: Unknown query";
            break;
        }
        case ERROR_LOGIN:
        {
            errorString = "FAIL: Login Error";
            break;
        }
        case ERROR_PREPARE_USER_SELECTION_PAGE_0:
        {
            errorString = "FAIL: Prepare user selection page [0]";
            break;
        }
        case ERROR_ADD_IFACE:
        {
            errorString = "FAIL: Add iface";
            break;
        }
        case ERROR_ADD_TARIFF:
        {
            errorString = "FAIL: Add tariff";
            break;
        }
        case ERROR_ADD_GROUP:
        {
            errorString = "FAIL: Add group";
            break;
        }
        case ERROR_ADD_USER:
        {
            errorString = "FAIL: Add user";
            break;
        }
        case ERROR_CREATE_SID:
        {
            errorString = "FAIL: Create sid";
            break;
        }
        case ERROR_SET_SID:
        {
            errorString = "FAIL: Set sid";
            break;
        }
        case ERROR_UPDATE_SID:
        {
            errorString = "FAIL: Update sid";
            break;
        }
        case ERROR_READ_SID_DATA:
        {
            errorString = "FAIL: Read sid data";
            break;
        }
        case ERROR_WRITE_SID_DATA:
        {
            errorString = "FAIL: Write sid data";
            break;
        }
        case ERROR_REMOVE_EXPIRED_SID:
        {
            errorString = "FAIL: Remove expired sids";
            break;
        }
        /* qParam.lib */
        case ERROR_MEMORY_ALLOCATE:
        {
            errorString = "FAIL: Error memory allocation";
            break;
        }
        case ERROR_MEMORY_DESPOSE:
        {
            errorString = "FAIL: Error memory depose";
            break;
        }
        case ERROR_NULL_QUERY:
        {
            errorString = "FAIL: Query is NULL";
            break;
        }
        case ERROR_QUERY:
        {
            errorString = "FAIL: Error query";
            break;
        }
        /* diagram.lib */
        case ERROR_ARC_DATA_FULL:
        {
            errorString = "FAIL: Arc data is full";
            break;
        }
        case ERROR_ARC_PERCENT:
        {
            errorString = "FAIL: Arc percent != 100%";
            break;
        }
        default:
        {
            errorString = "FAIL: Unknown error";
        }
    } /* switch (res) */

    return (errorString);
}/* GetErrorString() */

/* EOF */

