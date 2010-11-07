/*
 *****************************************************************************
 *
 * File:        mimetype.h
 *
 * Description: TODO:
 *
 * $Id: mimetype.h,v 1.1.1.1 2005/10/09 11:00:45 nobunaga Exp $
 *
 *****************************************************************************
 */

#ifndef _MIMETYPE_H
#define _MIMETYPE_H_


struct MIMETYPE
    {
    char *ext;
    char *type;
    };

const MIMETYPE mTypes[]=
{
    { ".jpg" , "image/jpeg" },
    { ".gif", "image/gif" },
    { ".jpeg", "image/jpeg" },
    { ".htm", "text/html" },
    { ".html", "text/html" },
    { ".txt", "text/plain" },
    { ".css", "text/css" }
};

#endif /* _MIMETYPE_H_ */

/* EOF */

