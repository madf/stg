/*
 *****************************************************************************
 *
 * File:        debug.h
 *
 * Description: Вывод отладочной информации в log файл
 *
 * $Id: debug.h,v 1.2 2006/03/07 18:33:56 nobunaga Exp $
 *
 *****************************************************************************
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_


#include <stdio.h>


#define MAX_LOG_BUFF_LEN    (2048)


void PrintfLog(FILE * logFile, char * scriptName, char * fmt, ...);

#endif  /* _DEBUG_H_ */

/* EOF */

