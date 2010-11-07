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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

 /*
 $Revision: 1.1.1.1 $
 $Date: 2009/02/24 08:13:03 $
 $Author: faust $
 */


#ifndef __RULES_H__
#define __RULES_H__

#include <list>
#include <string>
#include <sstream>
#include <map>

#ifdef HAVE_STDINT
    #include <stdint.h>
#else
    #ifdef HAVE_INTTYPES
	#include <inttypes.h>
    #else
	#error "You need either stdint.h or inttypes.h to compile this!"
    #endif
#endif

namespace STG
{

    //-----------------------------------------------------------------------------
    struct RULE
    {
    uint32_t    ip;             // IP
    uint32_t    mask;           // Netmask
    uint16_t    port1;          // Port 1
    uint16_t    port2;          // Port 2
    int         proto;          // Protocol
    int         dir;            // Direction
    };
    //-----------------------------------------------------------------------------
    typedef std::list<RULE> RULES;
    //-----------------------------------------------------------------------------
    class RULES_PARSER
    {
    public:
        RULES_PARSER();

        RULES_PARSER(const std::string & fileName);

        ~RULES_PARSER() {};

        void SetFile(const std::string & fileName);

        const RULES & GetRules() const { return rules; };
        bool IsError() const { return error; };
        const std::string ErrorMsg() const { return errorStream.str(); };

    private:
        RULES rules;
        bool error;
        mutable std::stringstream errorStream;
        std::map<std::string, int> protocols;

        bool InitProtocols();
        bool ParseLine(std::string line);
        bool ParseAddress(const std::string & address, RULE * rule) const;
        bool ParseMask(const std::string & mask, RULE * rule) const;
        bool ParsePorts(const std::string & port1,
                        const std::string & port2,
                        RULE * rule) const;
    };

}

#endif // __RULES_H__
