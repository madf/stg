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

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <limits>
#include <cerrno>
#include <locale>

#include <arpa/inet.h>
#include <netdb.h>

#include "rules.h"
#include "utils.h"

using namespace std;

STG::RULES_PARSER::RULES_PARSER()
    : rules(),
      error(false),
      errorStream(""),
      protocols()
{
error = InitProtocols();
}

STG::RULES_PARSER::RULES_PARSER(const string & fileName)
    : rules(),
      error(false),
      errorStream(""),
      protocols()
{
error = InitProtocols();
SetFile(fileName);
}

void STG::RULES_PARSER::SetFile(const string & fileName)
{
errorStream.str("");

ifstream rulesFile(fileName.c_str());

int lineNumber = 0;

if (!rulesFile)
    {
    error = true;
    errorStream << "RULES_PARSER::SetFile - Error opening file '" << fileName << "'\n";
    return;
    }

string line;

rules.erase(rules.begin(), rules.end());

while (getline(rulesFile, line))
    {
    lineNumber++;
    if (ParseLine(line))
        {
        error = true;
        errorStream << "RULES_PARSER::SetFile - Error parsing line at '" << fileName << ":" << lineNumber << "'\n";
        return;
        }
    }

STG::RULE rule;

// Adding lastest rule: ALL 0.0.0.0/0 NULL
rule.dir = -1; //NULL
rule.ip = 0;  //0.0.0.0
rule.mask = 0;
rule.port1 = 0;
rule.port2 = 65535;
rule.proto = -1;

rules.push_back(rule);

errorStream.str("");

return;
}

bool STG::RULES_PARSER::ParseLine(string line)
{
size_t pos;

pos = line.find('#');
if (pos != string::npos)
    {
    line = line.substr(0, pos);
    }

if (line.empty())
    {
    return false;
    }

size_t lpos = line.find_first_not_of("\t ", 0, 2);

if (lpos == string::npos)
    {
    return false;
    }

size_t rpos = line.find_first_of("\t ", lpos, 2);

if (rpos == string::npos)
    {
    return false;
    }

string proto(line.begin() + lpos, line.begin() + rpos);

lpos = line.find_first_not_of("\t ", rpos, 2);

if (lpos == string::npos)
    {
    return false;
    }

rpos = line.find_first_of("\t ", lpos, 2);

if (rpos == string::npos)
    {
    return false;
    }

string address(line.begin() + lpos, line.begin() + rpos);

lpos = line.find_first_not_of("\t ", rpos, 2);

if (lpos == string::npos)
    {
    return false;
    }
string direction(line.begin() + lpos, line.end());

if (proto.empty() ||
    address.empty() ||
    direction.empty())
    {
    return false;
    }

map<string, int>::const_iterator it(protocols.find(proto));

if (it == protocols.end())
    {
    errorStream << "RULES_PARSER::ParseLine - Invalid protocol\n";
    return true;
    }

STG::RULE rule;

rule.proto = it->second;

if (direction.length() < 4)
    {
    errorStream << "RULES_PARSER::ParseLine - Invalid direction\n";
    return true;
    }

if (direction == "NULL")
    {
    rule.dir = -1;
    }
else
    {
    string prefix(direction.begin(), direction.begin() + 3);
    direction = direction.substr(3, direction.length() - 3);
    if (prefix != "DIR")
        {
        errorStream << "RULES_PARSER::ParseLine - Invalid direction prefix\n";
        return true;
        }
    char * endptr;
    /* 
     * 'cause strtol don't change errno on success
     * according to: http://www.opengroup.org/onlinepubs/000095399/functions/strtol.html
     */
    errno = 0;
    rule.dir = strtol(direction.c_str(), &endptr, 10);

    // Code from strtol(3) release 3.10
    if ((errno == ERANGE && (rule.dir == numeric_limits<int>::max() ||
                             rule.dir == numeric_limits<int>::min()))
        || (errno != 0 && rule.dir == 0))
        {
            errorStream << "RULES_PARSER::ParseLine - Direction out of range\n";
            return true;
        }
    if (endptr == direction.c_str())
        {
            errorStream << "RULES_PARSER::ParseLine - Invalid direction\n";
            return true;
        }
    }

if (ParseAddress(address, &rule))
    {
    errorStream << "RULES_PARSER::ParseLine - Invalid address\n";
    return true;
    }

rules.push_back(rule);

return false;
}

bool STG::RULES_PARSER::ParseAddress(const string & address, RULE * rule) const
{
// Format: <address>[/<mask>[:<port1>[-<port2>]]]
size_t pos = address.find('/');
string ip;
string mask;
string ports;

if (pos != string::npos)
    {
    ip = address.substr(0, pos);
    mask = address.substr(pos + 1, address.length() - pos - 1);
    pos = mask.find(':');
    if (pos != string::npos)
        {
        ports = mask.substr(pos + 1, mask.length() - pos - 1);
        mask = mask.substr(0, pos);
        }
    else
        {
        ports = "0-65535";
        }
    }
else
    {
    mask = "32";
    pos = address.find(':');
    if (pos != string::npos)
        {
        ip = address.substr(0, pos);
        ports = address.substr(pos + 1, address.length() - pos - 1);
        }
    else
        {
        ip = address;
        ports = "0-65536";
        }
    }

struct in_addr ipaddr;

if (!inet_aton(ip.c_str(), &ipaddr))
    {
    errorStream << "RULES_PARSER::ParseAddress - Invalid IP\n";
    return true;
    }

rule->ip = ntohl(ipaddr.s_addr);

if (ParseMask(mask, rule))
    {
    errorStream << "RULES_PARSER::ParseAddress - Error parsing mask\n";
    return true;
    }

pos = ports.find('-');
string port1;
string port2;

if (pos != string::npos)
    {
    port1 = ports.substr(0, pos);
    port2 = ports.substr(pos + 1, ports.length() - pos - 1);
    }
else
    {
    port1 = port2 = ports;
    }

if (ParsePorts(port1, port2, rule))
    {
    errorStream << "RULES_PARSER::ParseAddress - Error pasing ports\n";
    return true;
    }

return false;
}

bool STG::RULES_PARSER::ParseMask(const string & mask, RULE * rule) const
{
char * endptr;

errno = 0;
/* 
 * 'cause strtol don't change errno on success
 * according to: http://www.opengroup.org/onlinepubs/000095399/functions/strtol.html
 */
rule->mask = strtol(mask.c_str(), &endptr, 10);

if ((errno == ERANGE && (rule->mask == numeric_limits<uint32_t>::max() ||
                         rule->mask == numeric_limits<uint32_t>::min()))
    || (errno != 0 && rule->mask == 0))
    {
    errorStream << "RULES_PARSER::ParseMask - Mask is out of range\n";
    return true;
    }

if (endptr == NULL)
    {
    errorStream << "RULES_PARSER::ParseMask - NULL endptr\n";
    return true;
    }

if (*endptr != '\0')
    {
    errorStream << "RULES_PARSER::ParseMask - Invalid mask\n";
    return true;
    }

if (rule->mask > 32)
    {
    errorStream << "RULES_PARSER::ParseMask - Mask is greater than 32\n";
    return true;
    }

rule->mask = 0xffFFffFF >> (32 - rule->mask);

return false;
}

bool STG::RULES_PARSER::ParsePorts(const string & port1,
                              const string & port2,
                              RULE * rule) const
{
char * endptr;

errno = 0;
/* 
 * 'cause strtol don't change errno on success
 * according to: http://www.opengroup.org/onlinepubs/000095399/functions/strtol.html
 */
rule->port1 = strtol(port1.c_str(), &endptr, 10);

if ((errno == ERANGE && (rule->port1 == numeric_limits<uint16_t>::max() ||
                         rule->port1 == numeric_limits<uint16_t>::min()))
    || (errno != 0 && rule->port1 == 0))
    {
    errorStream << "RULES_PARSER::ParsePorts - Min port is out of range\n";
    return true;
    }

if (endptr == NULL)
    {
    errorStream << "RULES_PARSER::ParsePorts - NULL endptr on min port\n";
    return true;
    }

if (*endptr != '\0')
    {
    errorStream << "RULES_PARSER::ParsePorts - Invalid min port\n";
    return true;
    }

errno = 0;
/* 
 * 'cause strtol don't change errno on success
 * according to: http://www.opengroup.org/onlinepubs/000095399/functions/strtol.html
 */
rule->port2 = strtol(port2.c_str(), &endptr, 10);

if ((errno == ERANGE && (rule->port2 == numeric_limits<uint16_t>::max() ||
                         rule->port2 == numeric_limits<uint16_t>::min()))
    || (errno != 0 && rule->port2 == 0))
    {
        errorStream << "RULES_PARSER::ParseAddress - Max port is out of range\n";
        return true;
    }

if (endptr == NULL)
    {
    errorStream << "RULES_PARSER::ParsePorts - NULL endptr on max port\n";
    return true;
    }

if (*endptr != '\0')
    {
    errorStream << "RULES_PARSER::ParsePorts - Invalid max port\n";
    return true;
    }

return false;
}

bool STG::RULES_PARSER::InitProtocols()
{
struct protoent * pe;

locale loc("");

protocols.erase(protocols.begin(), protocols.end());

setprotoent(true); // Open link to /etc/protocols

while ((pe = getprotoent()) != NULL)
    {
    string proto(pe->p_name);
    protocols.insert(make_pair(STG::ToUpper(pe->p_name, loc), pe->p_proto));
    }

endprotoent();

protocols["ALL"] = -1;
protocols["TCP_UDP"] = -2;

errorStream.str("");

return protocols.empty();
}
