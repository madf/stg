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
 $Revision: 1.4 $
 $Date: 2007/12/02 18:52:05 $
 $Author: nobunaga $
 */

#undef STG_TIME

#define TEST1_LLMAX "8589934592.00 Gb"
#define TEST1_LLMIN "-8589934592.00 Gb"
#define TEST1_0 "0.00 kb"
#define TEST1_1 "1.00 Mb"
#define TEST2_STRING "This is a test string! 0123456789+-*/"
#define TEST2_PART "This i"
#define TEST3_STRING "Это тестовая строка! 0123456789+-*/"
#define TEST3_PART "Это тестовая строка! 012345678"
#define TEST4_STRING "Try to encode this using blowfish"
#define TEST4_PASSWORD "Ha*yN).3zqL!"

int TestIntToKMG();
int Teststrtodouble2();
int TestIsDigit();
int TestIsAlpha();
int TestEncodeDecode();
int TestParseIPString();
int TestKOIToWIN();
int TestDaysInMonth();
int TestBlowfish();
int TestMin8();
int Testinet_ntostr();
int TestParseTariffTimeStr();
int TestStr2XX2Str();

