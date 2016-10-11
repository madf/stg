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
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Revision: 1.42 $
 $Date: 2010/11/08 10:11:19 $
 $Author: faust $
 */

// For old and dub systems
// Like FreeBSD4
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <sys/select.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#endif

#include <iconv.h>

#include <algorithm>

#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cassert>

#include "stg/common.h"

#ifndef INET_ADDRSTRLEN
#   define INET_ADDRSTRLEN 16
#endif

namespace
{
//---------------------------------------------------------------------------
unsigned char koi2win[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
        0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
        0xA0, 0xA1, 0xA2, 0xB8, 0xBA, 0xA5, 0xB3, 0xBF,
        0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xB4, 0xAE, 0xAF,
        0xB0, 0xB1, 0xB2, 0xA8, 0xAA, 0xB5, 0xB2, 0xAF,
        0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xA5, 0xBE, 0xBF,
        0xFE, 0xE0, 0xE1, 0xF6, 0xE4, 0xE5, 0xF4, 0xE3,
        0xF5, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE,
        0xEF, 0xFF, 0xF0, 0xF1, 0xF2, 0xF3, 0xE6, 0xE2,
        0xFC, 0xFB, 0xE7, 0xF8, 0xFD, 0xF9, 0xF7, 0xFA,
        0xDE, 0xC0, 0xC1, 0xD6, 0xC4, 0xC5, 0xD4, 0xC3,
        0xD5, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE,
        0xCF, 0xDF, 0xD0, 0xD1, 0xD2, 0xD3, 0xC6, 0xC2,
        0xDC, 0xDB, 0xC7, 0xD8, 0xDD, 0xD9, 0xD7, 0xDA};


unsigned char win2koi[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
        0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xBD, 0xA6, 0xA7,
        0xB3, 0xA9, 0xB4, 0xAB, 0xAC, 0xAD, 0xAE, 0xB7,
        0xB0, 0xB1, 0xB6, 0xA6, 0xAD, 0xB5, 0xB6, 0xB7,
        0xA3, 0xB9, 0xA4, 0xBB, 0xBC, 0xBD, 0xBE, 0xA7,
        0xE1, 0xE2, 0xF7, 0xE7, 0xE4, 0xE5, 0xF6, 0xFA,
        0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0,
        0xF2, 0xF3, 0xF4, 0xF5, 0xE6, 0xE8, 0xE3, 0xFE,
        0xFB, 0xFD, 0xFF, 0xF9, 0xF8, 0xFC, 0xE0, 0xF1,
        0xC1, 0xC2, 0xD7, 0xC7, 0xC4, 0xC5, 0xD6, 0xDA,
        0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
        0xD2, 0xD3, 0xD4, 0xD5, 0xC6, 0xC8, 0xC3, 0xDE,
        0xDB, 0xDD, 0xDF, 0xD9, 0xD8, 0xDC, 0xC0, 0xD1};
}

#ifdef WIN32
//-----------------------------------------------------------------------------
const char * inet_ntop(int af, const void * src, char * dst, unsigned long length)
{
struct sockaddr_in addr;
addr.sin_family = af;
addr.sin_port = 0;
memcpy(&addr.sin_addr.s_addr, src, sizeof(addr.sin_addr.s_addr));
if (WSAAddressToStringA(reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr), 0, dst, &length))
    {
    return NULL;
    }
return dst;
}
//-----------------------------------------------------------------------------
int inet_pton(int af, const char * src, void * dst)
{
// Fuck you Microsoft!
// Why the hell not to use const char *?
size_t slen = strlen(src);
char * buf = new char[slen + 1];
strncpy(buf, src, slen + 1);
buf[slen] = 0;
struct sockaddr_in addr;
addr.sin_family = af;
addr.sin_port = 0;
addr.sin_addr.s_addr = 0;
int length = sizeof(addr);
if (WSAStringToAddressA(buf, af, 0, reinterpret_cast<struct sockaddr *>(&addr), &length))
    {
    delete[] buf;
    return -1;
    }
memcpy(dst, &addr, sizeof(addr));
delete[] buf;
return 1;
}
#endif
//-----------------------------------------------------------------------------
int strtodouble2(const char * s, double &a)
{
char *res;

a = strtod(s, &res);

if (*res != 0)
    return EINVAL;

return 0;
}
//-----------------------------------------------------------------------------
#ifdef DEBUG
int printfd(const char * __file__, const char * fmt, ...)
#else
int printfd(const char *, const char *, ...)
#endif
{
#ifdef DEBUG
char buff[1024];

time_t t = time(NULL);

va_list vl;
va_start(vl, fmt);
vsnprintf(buff, sizeof(buff), fmt, vl);
va_end(vl);

printf("%18s > %s > ", __file__, LogDate(t)+11);
printf("%s", buff);

#endif
return 0;
}
//-----------------------------------------------------------------------------
int strprintf(std::string * str, const char * fmt, ...)
{
char buff[1024];

va_list vl;
va_start(vl, fmt);
int n = vsnprintf(buff, sizeof(buff), fmt, vl);
va_end(vl);
buff[1023] = 0;
*str = buff;

return n;
}
//-----------------------------------------------------------------------------
const char *IntToKMG(int64_t a, int stat)
{
static const double K = 1024;
static const double M = 1024 * 1024;
static const double G = 1024 * 1024 * 1024;
static char str[30];
double value = a;

switch (stat)
    {
    case ST_B:
        #ifdef __WIN32__
        sprintf(str, "%Ld", a);
        #else
        sprintf(str, "%lld", a);
        #endif
        break;
    case ST_KB:
        sprintf(str, "%.2f kb", value / K);
        break;
    case ST_MB:
        sprintf(str, "%.2f Mb", value / M);
        break;
    default:
        if (a > G)
            {
            sprintf(str, "%.2f Gb", value / G);
            return &str[0];
            }
        if (a < -G)
            {
            sprintf(str, "%.2f Gb", value / G);
            return &str[0];
            }
        if (a > M)
            {
            sprintf(str, "%.2f Mb", value / M);
            return &str[0];
            }
        if (a < -M)
            {
            sprintf(str, "%.2f Mb", value / M);
            return &str[0];
            }

        sprintf(str, "%.2f kb", value / K);
        break;
    }
return str;
}
//---------------------------------------------------------------------------
void KOIToWin(const char * s1, char * s2, int l)
{
for (int j = 0; j < l; j++)
    {
    unsigned char t = s1[j];
    s2[j] = koi2win[t];

    if (s1[j] == 0)
        break;
    }
}
//---------------------------------------------------------------------------
void WinToKOI(const char * s1, char * s2, int l)
{
for (int j = 0; j < l; j++)
    {
    unsigned char t = s1[j];
    s2[j] = win2koi[t];

    if (s1[j] == 0)
        break;
    }
}
//---------------------------------------------------------------------------
void KOIToWin(const std::string & s1, std::string * s2)
{
s2->erase(s2->begin(), s2->end());
s2->reserve(s1.length());
for (int j = 0; j < (int)s1.length(); j++)
    {
    unsigned char t = s1[j];
    s2->push_back(koi2win[t]);
    }
}
//---------------------------------------------------------------------------
void WinToKOI(const std::string & s1, std::string * s2)
{
s2->erase(s2->begin(), s2->end());
s2->reserve(s1.length());
for (int j = 0; j < (int)s1.length(); j++)
    {
    unsigned char t = s1[j];
    s2->push_back(win2koi[t]);
    }
}
//---------------------------------------------------------------------------
void Encode12str(std::string & dst, const std::string & src)
{
dst.erase(dst.begin(), dst.end());
for (size_t i = 0; i < src.length(); i++)
    {
    dst.push_back('a' + (src[i] & 0x0f));
    dst.push_back('a' + ((src[i] & 0xf0) >> 4));
    }
}
//---------------------------------------------------------------------------
void Decode21str(std::string & dst, const std::string & src)
{
dst.erase(dst.begin(), dst.end());
for (size_t i = 0; i < src.length() / 2; i++)
    {
    char c1 = src[i * 2];
    char c2 = src[i * 2 + 1];

    c1 -= 'a';
    c2 -= 'a';

    dst.push_back(static_cast<char>(c1 + (c2 << 4)));
    }
}
//---------------------------------------------------------------------------
void Encode12(char * dst, const char * src, size_t srcLen)
{
for (size_t i = 0; i <= srcLen; i++)
    {
    char c1 = src[i] & 0x0f;
    char c2 = (src[i] & 0xf0) >> 4;

    c1 += 'a';
    c2 += 'a';

    dst[i * 2] = c1;
    dst[i * 2 + 1] = c2;
    }
dst[srcLen * 2] = 0;
}
//---------------------------------------------------------------------------
void Decode21(char * dst, const char * src)
{
for (size_t i = 0; ; i++)
    {
    if (src[i * 2] == 0)
        break;

    char c1 = src[i * 2];
    char c2 = src[i * 2 + 1];

    c1 -= 'a';
    c2 -= 'a';

    dst[i] = static_cast<char>(c1 + (c2 << 4));
    }
dst[strlen(src) / 2] = 0;
}
//---------------------------------------------------------------------------
int ParseIPString(const char * str, uint32_t * ips, int maxIP)
{
/*
 *Function Name:ParseIPString
 *
 *Parameters:
 строка для разбора и массив куда заносить полученные адреса
 *
 *Description:
 На входе должна быть строка вида "ip1,ip2,ip3" или "*"
 В первом случае в массив заносятся разобранные адреса.
 Если их меньше MAX_IP?, то последний адрес будет 255.255.255.255
 Если строка * , то перваый адрес будет 0.0.0.0, т.е. любой
 *
 *Returns: 0 если все ОК
 *
 */

char p[255];
int n = 0;

strncpy(p, str, 254);
char * pp = p;

memset(ips, 0xFF, sizeof(unsigned long) * maxIP);

if (str[0] == '*' && strlen(str) == 1)
    {
    ips[0] = 0;
    return 0;
    }

for (int i = 0; i < maxIP; i++)
    {
    char * p1 = strtok(pp, ",\n ");
    pp = NULL;

    if (p1 == NULL && n == 0)// указатель нуль и прочитано адресов тоже ноль
        {
        return EINVAL;
        }

    if (p1 == NULL && n)
        {
        return 0;
        }

    struct in_addr in;
    if (inet_pton(AF_INET, p1, &in) != 1)
        {
        //printf("INADDR_NONE\n");
        return EINVAL;
        }

    ips[n] = in.s_addr;

    /*if (ips[n] == INADDR_NONE)
        return EINVAL;*/

    n++;

    if (n >= maxIP)
        return 0;

    }

return 0;
}
//-----------------------------------------------------------------------------
int DaysInCurrentMonth()
{
time_t t = time(NULL);

struct tm * lt = localtime(&t);

return DaysInMonth(lt->tm_year, lt->tm_mon);
}
//-----------------------------------------------------------------------------
int DaysInMonth(unsigned year, unsigned mon)
{
assert(mon < 12 && "Month number should be 0 - 11");
switch (mon)
    {
    case 0: return 31;  //jan
    case 1:
        if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
            return 29;
        return 28;      //feb
    case 2: return 31;  //mar
    case 3: return 30;  //apr
    case 4: return 31;  //may
    case 5: return 30;  //june
    case 6: return 31;  //jule
    case 7: return 31;  //aug
    case 8: return 30;  //sep
    case 9: return 31;  //oct
    case 10: return 30; //nov
    case 11: return 31; //dec
    }
return -1; // We will never reach here
}
//-----------------------------------------------------------------------------
int Min8(int a)
{
/*
Функция возвращает наименьшее число кратное 8-ми большее или равное заданному
 * */
if (a % 8 == 0)
    return a;

return a + (8 - a % 8);
}
//-----------------------------------------------------------------------------
/*char * inet_ntostr(unsigned long ip)
{
struct in_addr addr = {ip};
return inet_ntoa(addr);
}*/
//-----------------------------------------------------------------------------
std::string inet_ntostring(uint32_t ip)
{
    char buf[INET_ADDRSTRLEN + 1];
    return inet_ntop(AF_INET, &ip, buf, INET_ADDRSTRLEN);
}
//-----------------------------------------------------------------------------
uint32_t inet_strington(const std::string & value)
{
    uint32_t result;

    if (inet_pton(AF_INET, value.c_str(), &result) <= 0)
        return 0;

    return result;
}
//-----------------------------------------------------------------------------
int ParseTariffTimeStr(const char * str, int &h1, int &m1, int &h2, int &m2)
{
char hs1[10], ms1[10], hs2[10], ms2[10];
char s1[25], s2[25];
char ss[49];
char *p1, *p2;

strncpy(ss, str, 48);

p1 = strtok(ss, "-");
if (!p1)
    return -1;

strncpy(s1, p1, 24);

p2 = strtok(NULL, "-");
if (!p2)
    return -1;

strncpy(s2, p2, 24);

p1 = strtok(s1, ":");
if (!p1)
    return -1;

strncpy(hs1, p1, 9);

p2 = strtok(NULL, ":");
if (!p2)
    return -1;

strncpy(ms1, p2, 9);

p1 = strtok(s2, ":");
if (!p1)
    return -1;

strncpy(hs2, p1, 9);

p2 = strtok(NULL, ":");
if (!p2)
    return -1;

strncpy(ms2, p2, 9);

if (str2x(hs1, h1) != 0)
    return -1;

if (str2x(ms1, m1) != 0)
    return -1;

if (str2x(hs2, h2) != 0)
    return -1;

if (str2x(ms2, m2) != 0)
    return -1;

return 0;
}
/*//---------------------------------------------------------------------------
bool IsDigit(char c)
{
if (c >= '0' && c <= '9')
    return true;
return false;
}
//-----------------------------------------------------------------------------
bool IsAlpha(char c)
{
if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
    return true;
return false;
}*/
//-----------------------------------------------------------------------------
const char * LogDate(time_t t)
{
static char s[32];
struct tm * tt = localtime(&t);

snprintf(s, 20, "%d-%s%d-%s%d %s%d:%s%d:%s%d",
         tt->tm_year + 1900,
         tt->tm_mon + 1 < 10 ? "0" : "", tt->tm_mon + 1,
         tt->tm_mday    < 10 ? "0" : "", tt->tm_mday,
         tt->tm_hour    < 10 ? "0" : "", tt->tm_hour,
         tt->tm_min     < 10 ? "0" : "", tt->tm_min,
         tt->tm_sec     < 10 ? "0" : "", tt->tm_sec);

return s;
}
//-----------------------------------------------------------------------------
uint32_t CalcMask(uint32_t msk)
{
if (msk >= 32) return 0xFFffFFff;
if (msk == 0) return 0;
return htonl(0xFFffFFff << (32 - msk));
}
//---------------------------------------------------------------------------
void TouchFile(const std::string & fileName)
{
FILE * f = fopen(fileName.c_str(), "w");
if (f)
    fclose(f);
}
//---------------------------------------------------------------------------
#ifdef WIN32
void EncodeStr(char * str, unsigned long serial, int useHDD)
{
int len = strlen(str);
char stren[100];
int i, j = 0;
char c1, c2;
char serial_c[sizeof(serial)];
memcpy(serial_c, &serial, sizeof(serial));

for (i = 0; i < len; i++)
    {
    if (!useHDD)
        str[i] = str[i]^49;
    else
        {
        str[i] = str[i]^serial_c[j%sizeof(serial)];
        j++;
        }
    }

for (i = 0; i < 2*len; i++)
    {
    if (i%2)
        {
        c1 = (str[i/2] >> 4);
        c1 = c1 + 50;
        stren[i] = c1;
        }
    else
        {
        c2 = (str[i/2] & 0x0f);
        c2 += 50;
        stren[i] = c2;
        }
    }
stren[i] = 0;
strcpy(str, stren);
}
//---------------------------------------------------------------------------
void DecodeStr(char * str, unsigned long serial, int useHDD)
{
size_t len = strlen(str);
char strdc[100];
char serial_c[sizeof(serial)];
memcpy(serial_c, &serial, sizeof(serial));

for (size_t i = 0; i < len; i += 2)
    {
    char c1 = (str[i] - 50);
    char c2 = (str[i+1] - 50)<<4;
    strdc[i/2] = c1+c2;
    }
for (size_t i = 0; i < len/2; i++)
    {
    if (!useHDD)
        strdc[i] = strdc[i]^49;
    else
        {
        strdc[i] = strdc[i]^serial_c[j%sizeof(serial)];
        j++;
        }
    }
strdc[i] = 0;
strcpy(str, strdc);
}
//---------------------------------------------------------------------------
#endif //WIN32
void SwapBytes(uint16_t & value)
{
    value = static_cast<uint16_t>((value >> 8) |
                                  (value << 8));
}
//---------------------------------------------------------------------------
void SwapBytes(uint32_t & value)
{
    value = static_cast<uint32_t>((value >> 24) |
                                  ((value << 8) &  0x00FF0000L) |
                                  ((value >> 8) &  0x0000FF00L) |
                                  (value << 24));
}
//---------------------------------------------------------------------------
void SwapBytes(uint64_t & value)
{
    value = static_cast<uint64_t>((value >> 56) |
                                  ((value << 40) & 0x00FF000000000000LL) |
                                  ((value << 24) & 0x0000FF0000000000LL) |
                                  ((value << 8)  & 0x000000FF00000000LL) |
                                  ((value >> 8)  & 0x00000000FF000000LL) |
                                  ((value >> 24) & 0x0000000000FF0000LL) |
                                  ((value >> 40) & 0x000000000000FF00LL) |
                                  (value << 56));
}
//---------------------------------------------------------------------------
void SwapBytes(int16_t & value)
{
    uint16_t temp = value;
    SwapBytes(temp);
    value = temp;
}
//---------------------------------------------------------------------------
void SwapBytes(int32_t & value)
{
    uint32_t temp = value;
    SwapBytes(temp);
    value = temp;
}
//---------------------------------------------------------------------------
void SwapBytes(int64_t & value)
{
    uint64_t temp = value;
    SwapBytes(temp);
    value = temp;
}
//---------------------------------------------------------------------------
std::string formatTime(time_t ts)
{
char buf[32];
struct tm brokenTime;

brokenTime.tm_wday = 0;
brokenTime.tm_yday = 0;
brokenTime.tm_isdst = 0;

gmtime_r(&ts, &brokenTime);

strftime(buf, 32, "%Y-%m-%d %H:%M:%S", &brokenTime);

return buf;
}
//---------------------------------------------------------------------------
time_t readTime(const std::string & ts)
{
if (ts == "0000-00-00 00:00:00")
    return 0;

struct tm brokenTime;

brokenTime.tm_wday = 0;
brokenTime.tm_yday = 0;
brokenTime.tm_isdst = 0;

stg_strptime(ts.c_str(), "%Y-%m-%d %H:%M:%S", &brokenTime);

return stg_timegm(&brokenTime);
}
//---------------------------------------------------------------------------
int str2x(const std::string & str, int32_t & x)
{
x = static_cast<int32_t>(strtol(str.c_str(), NULL, 10));

if (errno == ERANGE)
    return -1;

return 0;
}
//---------------------------------------------------------------------------
int str2x(const std::string & str, uint32_t & x)
{
x = static_cast<uint32_t>(strtoul(str.c_str(), NULL, 10));

if (errno == ERANGE)
    return -1;

return 0;
}
//---------------------------------------------------------------------------
int str2x(const std::string & str, double & x)
{
return strtodouble2(str.c_str(), x);
}
#ifndef WIN32
//---------------------------------------------------------------------------
int str2x(const std::string & str, int64_t & x)
{
x = strtoll(str.c_str(), NULL, 10);

if (errno == ERANGE)
    return -1;

return 0;
}
//---------------------------------------------------------------------------
int str2x(const std::string & str, uint64_t & x)
{
x = strtoull(str.c_str(), NULL, 10);

if (errno == ERANGE)
    return -1;

return 0;
}
#endif
//---------------------------------------------------------------------------
const std::string & x2str(uint32_t x, std::string & s)
{
return unsigned2str(x, s);
}
//---------------------------------------------------------------------------
const std::string & x2str(uint64_t x, std::string & s)
{
return unsigned2str(x, s);
}
//---------------------------------------------------------------------------
const std::string & x2str(double x, std::string & s)
{
char buf[256];
snprintf(buf, sizeof(buf), "%f", x);
s = buf;
return s;
}
//---------------------------------------------------------------------------
std::string & TrimL(std::string & val)
{
size_t pos = val.find_first_not_of(" \t");
if (pos == std::string::npos)
    {
    val.erase(val.begin(), val.end());
    }
else
    {
    val.erase(0, pos);
    }
return val;
}
//---------------------------------------------------------------------------
std::string & TrimR(std::string & val)
{
size_t pos = val.find_last_not_of(" \t");
if (pos != std::string::npos)
    {
    val.erase(pos + 1);
    }
return val;
}
//---------------------------------------------------------------------------
std::string & Trim(std::string & val)
{
return TrimR(TrimL(val));
}
//---------------------------------------------------------------------------
std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value;
}
//---------------------------------------------------------------------------
std::string ToUpper(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), ::toupper);
    return value;
}
//---------------------------------------------------------------------------
#ifdef WIN32
static int is_leap(unsigned y)
{
    y += 1900;
    return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}
#endif
//---------------------------------------------------------------------------

time_t stg_timegm(struct tm * brokenTime)
{
#ifdef WIN32
static const unsigned ndays[2][12] ={
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
time_t res = 0;
for (int i = 70; i < brokenTime->tm_year; ++i)
    res += is_leap(i) ? 366 : 365;
for (int i = 0; i < brokenTime->tm_mon; ++i)
    res += ndays[is_leap(brokenTime->tm_year)][i];
res += brokenTime->tm_mday - 1;
res *= 24;
res += brokenTime->tm_hour;
res *= 60;
res += brokenTime->tm_min;
res *= 60;
res += brokenTime->tm_sec;
return res;
#else
#ifdef HAVE_TIMEGM
return timegm(brokenTime);
#else
time_t ret;
char *tz;
tz = getenv("TZ");
setenv("TZ", "", 1);
tzset();
ret = mktime(brokenTime);
if (tz)
    setenv("TZ", tz, 1);
else
    unsetenv("TZ");
tzset();
return ret;
#endif // HAVE_TIMEGM
#endif // WIN32
}
//---------------------------------------------------------------------------
std::string IconvString(const std::string & source,
                        const std::string & from,
                        const std::string & to)
{
if (source.empty())
    return std::string();

size_t inBytesLeft = source.length() + 1;
size_t outBytesLeft = source.length() * 2 + 1;

char * inBuf = new char[inBytesLeft];
char * outBuf = new char[outBytesLeft];

strncpy(inBuf, source.c_str(), source.length());

inBuf[source.length()] = 0;

#if defined(CONST_ICONV)
const char * srcPos = inBuf;
#else
char * srcPos = inBuf;
#endif
char * dstPos = outBuf;

iconv_t handle = iconv_open(to.c_str(),
                            from.c_str());

if (handle == iconv_t(-1))
    {
    if (errno == EINVAL)
        {
        printfd(__FILE__, "IconvString(): iconv from %s to %s failed\n", from.c_str(), to.c_str());
        delete[] outBuf;
        delete[] inBuf;
        return source;
        }
    else
        printfd(__FILE__, "IconvString(): iconv_open error\n");

    delete[] outBuf;
    delete[] inBuf;
    return source;
    }

size_t res = iconv(handle,
                   &srcPos, &inBytesLeft,
                   &dstPos, &outBytesLeft);

if (res == size_t(-1))
    {
    printfd(__FILE__, "IconvString(): '%s'\n", strerror(errno));

    iconv_close(handle);
    delete[] outBuf;
    delete[] inBuf;
    return source;
    }

dstPos = 0;

std::string dst(outBuf);

iconv_close(handle);

delete[] outBuf;
delete[] inBuf;

return dst;
}

int ParseYesNo(const std::string & str, bool * val)
{
if (0 == strncasecmp(str.c_str(), "yes", 3))
    {
    *val = true;
    return 0;
    }

if (0 == strncasecmp(str.c_str(), "no", 2))
    {
    *val = false;
    return 0;
    }

return -1;
}

int ParseInt(const std::string & str, int * val)
{
if (str2x<int>(str, *val))
    return -1;
return 0;
}

int ParseUnsigned(const std::string & str, unsigned * val)
{
if (str2x<unsigned>(str, *val))
    return -1;
return 0;
}

int ParseIntInRange(const std::string & str, int min, int max, int * val)
{
if (ParseInt(str, val) != 0)
    return -1;

if (*val < min || *val > max)
    return -1;

return 0;
}

int ParseUnsignedInRange(const std::string & str, unsigned min,
                         unsigned max, unsigned * val)
{
if (ParseUnsigned(str, val) != 0)
    return -1;

if (*val < min || *val > max)
    return -1;

return 0;
}

bool WaitPackets(int sd)
{
fd_set rfds;
FD_ZERO(&rfds);
FD_SET(sd, &rfds);

struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = 500000;

int res = select(sd + 1, &rfds, NULL, NULL, &tv);
if (res == -1) // Error
    {
    if (errno != EINTR)
        printfd(__FILE__, "Error on select: '%s'\n", strerror(errno));
    return false;
    }

if (res == 0) // Timeout
    return false;

return true;
}

bool ReadAll(int sd, void * dest, size_t size)
{
size_t done = 0;
char * ptr = static_cast<char *>(dest);
while (done < size)
    {
    if (!WaitPackets(sd))
        return false;
    ssize_t res = read(sd, ptr + done, size - done);
    if (res < 0)
        return false;
    if (res == 0)
        return true;
    done += res;
    }
return true;
}

bool WriteAll(int sd, const void * source, size_t size)
{
size_t done = 0;
const char * ptr = static_cast<const char *>(source);
while (done < size)
    {
    ssize_t res = write(sd, ptr + done, size - done);
    if (res <= 0)
        return false;
    done += res;
    }
return true;
}

std::string ToPrintable(const std::string & src)
{
    std::string dest;

    for (size_t i = 0; i < src.size(); ++i)
        if (std::isprint(src[i]))
            dest += src[i];
        else
            dest += "\\" + x2str(src[i]);

    return dest;
}
