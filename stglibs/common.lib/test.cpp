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
 *    Author : Maxim Mamontov
 */

 /*
 $Revision: 1.6 $
 $Date: 2009/06/10 10:31:15 $
 $Author: faust $
 */

#include <iostream>
#include <limits.h>
#include <arpa/inet.h>

using namespace std;

#include "common.h"
#include "test.h"

time_t stgTime;

int main(void)
{
char buf1[256];
BLOWFISH_CTX ctx;
int functions = 0, ok = 0;

cout << "Testing common.lib" << endl << "---------------\
--------------------" << endl;

if (!TestIntToKMG())
    ok++;
functions++;
if (!Teststrtodouble2())
    ok++;
functions++;
if (!TestIsDigit())
    ok++;
functions++;
if (!TestIsAlpha())
    ok++;
functions++;
if (!TestEncodeDecode())
    ok++;
functions++;
if (!TestParseIPString())
    ok++;
functions++;
if (!TestKOIToWIN())
    ok++;
functions++;
if (!TestDaysInMonth())
    ok++;
functions++;
if (!TestBlowfish())
    ok++;
functions++;
if (!TestMin8())
    ok++;
functions++;
if (!Testinet_ntostr())
    ok++;
functions++;
if (!TestParseTariffTimeStr())
    ok++;
functions++;
if (!TestStr2XX2Str())
    ok++;
functions++;

cout << "------------------------------------" << endl;
cout << "Functions: \t\t\t" << functions << endl;
cout << "OK's: \t\t\t\t" << ok << endl;
cout << "Fails: \t\t\t\t" << functions - ok << endl;

return (functions != ok);
}

int TestIntToKMG()
{
int res = 1;
cout << "Testing IntToKMG: \t\t";
res = res && (strcmp(IntToKMG(LONG_LONG_MAX), TEST1_LLMAX) == 0);
//cout << IntToKMG(LONG_LONG_MAX) << " " << TEST1_LLMAX << endl;

res = res && (strcmp(IntToKMG(1024 * 1024 + 1), TEST1_1) == 0);
//cout << IntToKMG(1024 * 1024 + 1) << " " << TEST1_1 << endl;

res = res && (strcmp(IntToKMG(0), TEST1_0) == 0);
//cout << IntToKMG(0) << " " << TEST1_0 << endl;

res = res && (strcmp(IntToKMG(LONG_LONG_MIN), TEST1_LLMIN) == 0);
//cout << IntToKMG(LONG_LONG_MIN) << " " << TEST1_LLMIN << endl;

if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int Teststrtodouble2()
{
double a;
int res = 1;
cout << "Testing strtodouble2: \t\t";
res = res && !strtodouble2("0.0", a);
res = res && (a == 0.0);
res = res && !strtodouble2("0.123456", a);
res = res && (a == 0.123456);
res = res && !strtodouble2("123456.0", a);
res = res && (a == 123456.0);
res = res && !strtodouble2("123456.123456", a);
res = res && (a == 123456.123456);
res = res && !strtodouble2("-0.123456", a);
res = res && (a == -0.123456);
res = res && !strtodouble2("-123456.0", a);
res = res && (a == -123456.0);
res = res && !strtodouble2("-123456.123456", a);
res = res && (a == -123456.123456);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestIsDigit()
{
char a;
int res = 1;
cout << "Testing IsDigit: \t\t";
for(a = '0'; a < '9'; a++)
    res = res && IsDigit(a);
for(a = 'a'; a < 'z'; a++)
    res = res && !IsDigit(a);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestIsAlpha()
{
char a;
int res = 1;
cout << "Testing IsAlpha: \t\t";
for(a = '0'; a < '9'; a++)
    res = res && !IsAlpha(a);
for(a = 'a'; a < 'z'; a++)
    res = res && IsAlpha(a);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestEncodeDecode()
{
char enc[256], dec[512];
int res = 1;
cout << "Testing EncodeDecode: \t\t";
Encode12(enc, TEST2_STRING, strlen(TEST2_STRING));
Decode21(dec, enc);
res = res && !strcmp(dec, TEST2_STRING);
Encode12(enc, TEST2_STRING, 256); // Overflow
Decode21(dec, enc);
res = res && !strcmp(dec, TEST2_STRING);
Encode12(enc, TEST2_STRING, 5); // Underflow
Decode21(dec, enc);
res = res && !strcmp(dec, TEST2_PART);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestParseIPString()
{
unsigned int ips[4];
int res = 1;
cout << "Testing ParseIPString: \t\t";
res = res && (ParseIPString("127.0.0.1, 192.168.58.1, 10.0.0.1", ips, 4) == 0);
res = res && ips[0] == 0x0100007F;
res = res && ips[1] == 0x013AA8C0;
res = res && ips[2] == 0x0100000A;
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestKOIToWIN()
{
char enc[256], dec[256];
int res = 1;
cout << "Testing KOIToWin: \t\t";
KOIToWin(TEST3_STRING, enc, 256);
WinToKOI(enc, dec, 256);
res = res && !strcmp(dec, TEST3_STRING);
KOIToWin(TEST3_STRING, enc, strlen(TEST3_STRING) - 5);
WinToKOI(enc, dec, strlen(TEST3_STRING) - 5);
res = res && !strcmp(dec, TEST3_STRING);
KOIToWin(TEST3_STRING, enc, strlen(TEST3_STRING) + 5);
WinToKOI(enc, dec, strlen(TEST3_STRING) + 5);
res = res && !strcmp(dec, TEST3_STRING);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestDaysInMonth()
{
int res = 1;
cout << "Testing DaysInMonth: \t\t";
res = res && (DaysInMonth(2000, 0) == 31);
res = res && (DaysInMonth(2000, 1) == 29);
res = res && (DaysInMonth(2001, 1) == 28);
res = res && (DaysInMonth(2100, 1) == 28);
res = res && (DaysInMonth(2400, 1) == 29);
res = res && (DaysInMonth(2000, 2) == 31);
res = res && (DaysInMonth(2000, 3) == 30);
res = res && (DaysInMonth(2000, 4) == 31);
res = res && (DaysInMonth(2000, 5) == 30);
res = res && (DaysInMonth(2000, 6) == 31);
res = res && (DaysInMonth(2000, 7) == 31);
res = res && (DaysInMonth(2000, 8) == 30);
res = res && (DaysInMonth(2000, 9) == 31);
res = res && (DaysInMonth(2000, 10) == 30);
res = res && (DaysInMonth(2000, 11) == 31);
res = res && (DaysInMonth(2000, 20) == 33);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestBlowfish()
{
BLOWFISH_CTX ctx;
char enc[256], dec[256];
int res = 1, i, len = strlen(TEST4_STRING);
cout << "Testing Blowfish: \t\t";
EnDecodeInit(TEST4_PASSWORD, strlen(TEST4_PASSWORD), &ctx);
strcpy(dec, TEST4_STRING);
for(i = 0; i < len; i += 8)
    EncodeString(&enc[i], &dec[i], &ctx);
for(i = 0; i < len; i += 8)
    DecodeString(&dec[i], &enc[i], &ctx);
res = res && !strcmp(dec, TEST4_STRING);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestMin8()
{
int res = 1;
cout << "Testing Min8: \t\t\t";
res = res && (Min8(INT_MAX) == INT_MAX + 1);
res = res && (Min8(INT_MIN) == INT_MIN);
res = res && (Min8(0) == 0);
res = res && (Min8(7) == 8);
res = res && (Min8(8) == 8);
res = res && (Min8(9) == 16);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;;
}

int Testinet_ntostr()
{
unsigned long ip;
char buf[32];
int res = 1;
cout << "Testing inet_ntostr: \t\t";
res = res && (strcmp(inet_ntostr(inet_addr("127.0.0.1")), "127.0.0.1") == 0);
res = res && (strcmp(inet_ntostr(inet_addr("255.255.255.255")), "255.255.255.255") == 0);
res = res && (strcmp(inet_ntostr(inet_addr("0.0.0.0")), "0.0.0.0") == 0);
res = res && (strcmp(inet_ntostr(inet_addr("10.0.0.1")), "10.0.0.1") == 0);
res = res && (strcmp(inet_ntostr(inet_addr("192.168.58.240")), "192.168.58.240") == 0);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}

int TestParseTariffTimeStr()
{
int h1, m1, h2, m2;
int res = 1;
cout << "Testing ParseTariffTimeStr: \t";
res = res && !ParseTariffTimeStr("00:00-00:00", h1, m1, h2, m2);
res = res && (h1 == 0 && m1 == 0 && h2 == 0 && m2 == 0);
res = res && !ParseTariffTimeStr("0:0-0:0", h1, m1, h2, m2);
res = res && (h1 == 0 && m1 == 0 && h2 == 0 && m2 == 0);
res = res && !ParseTariffTimeStr("99:99-99:99", h1, m1, h2, m2);
res = res && (h1 == 99 && m1 == 99 && h2 == 99 && m2 == 99);
res = res && !ParseTariffTimeStr("12:34-56:78", h1, m1, h2, m2);
res = res && (h1 == 12 && m1 == 34 && h2 == 56 && m2 == 78);
if (res)
    cout << "OK" << endl;
else
    cout << "Fail" << endl;
return !res;
}
//-----------------------------------------------------------------------------
int TestStr2XX2Str()
{
cout << "Testing Str2XX2Str: \t\t";

# define INT8_MIN       (-128)
# define INT16_MIN      (-32767-1)
# define INT32_MIN      (-2147483647-1)
# define INT64_MIN      (-__INT64_C(9223372036854775807)-1)
# define INT8_MAX       (127)
# define INT16_MAX      (32767)
# define INT32_MAX      (2147483647)
# define INT64_MAX      (__INT64_C(9223372036854775807))

int xx;
string s;
for (int i = -5000000; i < 5000000; i+=10)
{
    x2str(i, s);
    str2x(s, xx);
    if (i != xx)
    {
    cout << "Fail" << endl;
        return 1;
    }
}

x2str(INT32_MIN, s);
str2x(s, xx);
if (xx != INT32_MIN)
{
    cout << INT32_MIN << " " << s << endl;
    cout << INT32_MIN << " " << xx << endl;
cout << "Fail" << endl;
    return 1;
}

x2str(INT32_MAX, s);
str2x(s, xx);
if (xx != INT32_MAX)
{
cout << "Fail" << endl;
    return 1;
}
cout << "OK" << endl;
return 0;
}
//-----------------------------------------------------------------------------

