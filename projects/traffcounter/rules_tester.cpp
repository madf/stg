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

#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <algorithm>
#include <functional>

#include "rules.h"
#include "logger.h"

using namespace STG;

STGLogger log;

typedef std::pair<std::string, bool> TEST_ENTRY;
struct TEST_INFO {
    bool wantedError;
    bool actualError; // Parser error status
    bool stdException; // Parser throws an std execption
    bool otherException; // Parser throws another exception
    std::string message; // Parser error message
};

class RULES_PARSER_TESTER : public std::unary_function<std::pair<std::string, bool>, void> {
public:
    RULES_PARSER_TESTER(RULES_PARSER & p) : parser(p),
                                            testLog(),
                                            testResult(),
                                            result(true)
        {
        };
    ~RULES_PARSER_TESTER()
        {
        PrintLog();
        if (result)
            exit(EXIT_SUCCESS);
        exit(EXIT_FAILURE);
        }
    void operator()(const std::pair<std::string, bool> & entry)
        {
        testLog[entry.first].wantedError = entry.second;
        testLog[entry.first].actualError = false;
        testLog[entry.first].stdException = false;
        testLog[entry.first].otherException = false;
        testLog[entry.first].message = "";
        testResult[entry.first] = true;
        try
            {
            parser.SetFile(entry.first);
            }
        catch (std::exception & ex)
            {
            testLog[entry.first].stdException = true;
            testResult[entry.first] &= false;
            }
        catch (...)
            {
            testLog[entry.first].otherException = true;
            testResult[entry.first] &= false;
            }
        testLog[entry.first].actualError = parser.IsError();
        testLog[entry.first].message = parser.ErrorMsg();
        testResult[entry.first] &= (parser.IsError() == entry.second);
        result &= testResult[entry.first];
        };

    void PrintLog()
        {
        std::cout << "RULES_PARSER_TESTER results:\n";
        std::cout << "-----------------------------------------------------------------\n";
        std::map<std::string, bool>::const_iterator it;
        for (it = testResult.begin(); it != testResult.end(); ++it)
            {
            std::cout << "File: '" << it->first << "'\t"
                      << "Correct: " << testLog[it->first].wantedError << "\t"
                      << "Actual:" << testLog[it->first].actualError << "\t"
                      << "STD exceptions: " << testLog[it->first].stdException << "\t"
                      << "Other exceptions: " << testLog[it->first].otherException << "\t"
                      << "Result: " << it->second << "\n";
            if (!testLog[it->first].message.empty())
                {
                    std::cout << "Messages: \n" << testLog[it->first].message << "\n";
                }
            }
        std::cout << "-----------------------------------------------------------------\n";
        std::cout << "Final result: " << (result ? "passed" : "failed") << std::endl;
        }

    bool Result() const { return result; };
private:
    RULES_PARSER & parser;
    std::map<std::string, TEST_INFO> testLog;
    std::map<std::string, bool> testResult;
    bool result;
};

int main(int argc, char ** argv)
{
RULES_PARSER parser;
std::map<std::string, bool> tests;

tests["./test_rules"] = false;
tests["./rules"] = false;
tests["./test_rules_bad_address"] = true;
tests["./test_rules_bad_port"] = true;
tests["./test_rules_bad_mask"] = true;
tests["./test_rules_bad_proto"] = true;
tests["./test_rules_bad_dir_prefix"] = true;
tests["./test_rules_bad_dir_range"] = true;
tests["./test_rules_bad_dir"] = true;

/*parser.SetFile("./rules");
std::cout << parser.ErrorMsg() << std::endl;*/

// TODO: find errors and write checks for regression

std::for_each(tests.begin(),
             tests.end(),
             RULES_PARSER_TESTER(parser));

return EXIT_FAILURE;
}
