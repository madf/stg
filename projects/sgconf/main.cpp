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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "request.h"
#include "common_sg.h"
#include "sg_error_codes.h"

#include "xml.h"
#include "admins.h"
#include "tariffs.h"
#include "users.h"
#include "options.h"
#include "actions.h"
#include "config.h"

#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/common.h"

#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

#include <unistd.h>
#include <getopt.h>
#include <iconv.h>
#include <langinfo.h>

namespace
{

template <typename T>
struct ARRAY_TYPE
{
typedef typename T::value_type type;
};

template <typename T>
struct ARRAY_TYPE<T[]>
{
typedef T type;
};

template <typename T, size_t N>
struct ARRAY_TYPE<T[N]>
{
typedef T type;
};

template <typename T>
struct nullary_function
{
typedef T result_type;
};

template <typename F>
class binder0 : public nullary_function<typename F::result_type>
{
    public:
        binder0(const F & func, const typename F::argument_type & arg)
            : m_func(func), m_arg(arg) {}
        typename F::result_type operator()() const { return m_func(m_arg); }
    private:
        F m_func;
        typename F::argument_type m_arg;
};

template <typename F>
inline
binder0<F> bind0(const F & func, const typename F::argument_type & arg)
{
return binder0<F>(func, arg);
}

template <typename C, typename A, typename R>
class METHOD1_ADAPTER : public std::unary_function<A, R>
{
    public:
        METHOD1_ADAPTER(R (C::* func)(A), C & obj) : m_func(func), m_obj(obj) {}
        R operator()(A arg) { return (m_obj.*m_func)(arg); }
    private:
        R (C::* m_func)(A);
        C & m_obj;
};

template <typename C, typename A, typename R>
class CONST_METHOD1_ADAPTER : public std::unary_function<A, R>
{
    public:
        CONST_METHOD1_ADAPTER(R (C::* func)(A) const, C & obj) : m_func(func), m_obj(obj) {}
        R operator()(A arg) const { return (m_obj.*m_func)(arg); }
    private:
        R (C::* m_func)(A) const;
        C & m_obj;
};

template <typename C, typename A, typename R>
METHOD1_ADAPTER<C, A, R> Method1Adapt(R (C::* func)(A), C & obj)
{
return METHOD1_ADAPTER<C, A, R>(func, obj);
}

template <typename C, typename A, typename R>
CONST_METHOD1_ADAPTER<C, A, R> Method1Adapt(R (C::* func)(A) const, C & obj)
{
return CONST_METHOD1_ADAPTER<C, A, R>(func, obj);
}

template <typename T>
bool SetArrayItem(T & array, const char * index, const typename ARRAY_TYPE<T>::type & value)
{
size_t pos = 0;
if (str2x(index, pos))
    return false;
array[pos] = value;
return true;
}

void RawXMLCallback(bool result, const std::string & reason, const std::string & response, void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get raw XML response. Reason: '" << reason << "'." << std::endl;
    return;
    }
SGCONF::PrintXML(response);
}

void Usage();
void UsageAll();
void UsageImpl(bool full);
void UsageConnection();
void UsageAdmins(bool full);
void UsageTariffs(bool full);
void UsageUsers(bool full);
void UsageServices(bool full);
void UsageCorporations(bool full);

void Version();

void ReadUserConfigFile(SGCONF::OPTION_BLOCK & block)
{
std::vector<std::string> paths;
const char * configHome = getenv("XDG_CONFIG_HOME");
if (configHome == NULL)
    {
    const char * home = getenv("HOME");
    if (home == NULL)
        return;
    paths.push_back(std::string(home) + "/.config/sgconf/sgconf.conf");
    paths.push_back(std::string(home) + "/.sgconf/sgconf.conf");
    }
else
    paths.push_back(std::string(configHome) + "/sgconf/sgconf.conf");
for (std::vector<std::string>::const_iterator it = paths.begin(); it != paths.end(); ++it)
    if (access(it->c_str(), R_OK) == 0)
        {
        block.ParseFile(*it);
        return;
        }
}

} // namespace anonymous

namespace SGCONF
{

class CONFIG_ACTION : public ACTION
{
    public:
        CONFIG_ACTION(SGCONF::CONFIG & config,
                      const std::string & paramDescription)
            : m_config(config),
              m_description(paramDescription)
        {}

        virtual ACTION * Clone() const { return new CONFIG_ACTION(*this); }

        virtual std::string ParamDescription() const { return m_description; }
        virtual std::string DefaultDescription() const { return ""; }
        virtual OPTION_BLOCK & Suboptions() { return m_suboptions; }
        virtual PARSER_STATE Parse(int argc, char ** argv);

    private:
        SGCONF::CONFIG & m_config;
        std::string m_description;
        OPTION_BLOCK m_suboptions;

        void ParseCredentials(const std::string & credentials);
        void ParseHostAndPort(const std::string & hostAndPort);
};

typedef bool (* API_FUNCTION) (const SGCONF::CONFIG &,
                               const std::string &,
                               const std::map<std::string, std::string> &);

class COMMAND
{
    public:
        COMMAND(API_FUNCTION funPtr,
                const std::string & arg,
                const std::map<std::string, std::string> & options)
            : m_funPtr(funPtr),
              m_arg(arg),
              m_options(options)
        {}
        bool Execute(const SGCONF::CONFIG & config) const
        {
            return m_funPtr(config, m_arg, m_options);
        }

    private:
        API_FUNCTION m_funPtr;
        std::string m_arg;
        std::map<std::string, std::string> m_options;
};

class COMMANDS
{
    public:
        void Add(API_FUNCTION funPtr,
                 const std::string & arg,
                 const std::map<std::string, std::string> & options) { m_commands.push_back(COMMAND(funPtr, arg, options)); }
        bool Execute(const SGCONF::CONFIG & config) const
        {
            std::list<COMMAND>::const_iterator it(m_commands.begin());
            bool res = true;
            while (it != m_commands.end() && res)
            {
                res = res && it->Execute(config);
                ++it;
            }
            return res;
        }
    private:
        std::list<COMMAND> m_commands;
};

class API_ACTION : public ACTION
{
    public:
        API_ACTION(COMMANDS & commands,
                   const std::string & paramDescription,
                   bool needArgument,
                   const OPTION_BLOCK& suboptions,
                   API_FUNCTION funPtr)
            : m_commands(commands),
              m_description(paramDescription),
              m_argument(needArgument ? "1" : ""), // Hack
              m_suboptions(suboptions),
              m_funPtr(funPtr)
        {}
        API_ACTION(COMMANDS & commands,
                   const std::string & paramDescription,
                   bool needArgument,
                   API_FUNCTION funPtr)
            : m_commands(commands),
              m_description(paramDescription),
              m_argument(needArgument ? "1" : ""), // Hack
              m_funPtr(funPtr)
        {}

        virtual ACTION * Clone() const { return new API_ACTION(*this); }

        virtual std::string ParamDescription() const { return m_description; }
        virtual std::string DefaultDescription() const { return ""; }
        virtual OPTION_BLOCK & Suboptions() { return m_suboptions; }
        virtual PARSER_STATE Parse(int argc, char ** argv)
        {
        PARSER_STATE state(false, argc, argv);
        if (!m_argument.empty())
            {
            if (argc == 0 ||
                argv == NULL ||
                *argv == NULL)
                throw ERROR("Missing argument.");
            m_argument = *argv;
            --state.argc;
            ++state.argv;
            }
        m_suboptions.Parse(state.argc, state.argv);
        m_commands.Add(m_funPtr, m_argument, m_params);
        return state;
        }

    private:
        COMMANDS & m_commands;
        std::string m_description;
        std::string m_argument;
        OPTION_BLOCK m_suboptions;
        std::map<std::string, std::string> m_params;
        API_FUNCTION m_funPtr;
};

PARSER_STATE CONFIG_ACTION::Parse(int argc, char ** argv)
{
if (argc == 0 ||
    argv == NULL ||
    *argv == NULL)
    throw ERROR("Missing argument.");
char * pos = strchr(*argv, '@');
if (pos != NULL)
    {
    ParseCredentials(std::string(*argv, pos));
    ParseHostAndPort(std::string(pos + 1));
    }
else
    {
    ParseHostAndPort(std::string(*argv));
    }
return PARSER_STATE(false, --argc, ++argv);
}

void CONFIG_ACTION::ParseCredentials(const std::string & credentials)
{
std::string::size_type pos = credentials.find_first_of(':');
if (pos != std::string::npos)
    {
    m_config.userName = credentials.substr(0, pos);
    m_config.userPass = credentials.substr(pos + 1);
    }
else
    {
    m_config.userName = credentials;
    }
}

void CONFIG_ACTION::ParseHostAndPort(const std::string & hostAndPort)
{
std::string::size_type pos = hostAndPort.find_first_of(':');
if (pos != std::string::npos)
    {
    m_config.server = hostAndPort.substr(0, pos);
    uint16_t port = 0;
    if (str2x(hostAndPort.substr(pos + 1), port))
        throw ERROR("Invalid port value: '" + hostAndPort.substr(pos + 1) + "'");
    m_config.port = port;
    }
else
    {
    m_config.server = hostAndPort;
    }
}

inline
CONFIG_ACTION * MakeParamAction(SGCONF::CONFIG & config,
                                const std::string & paramDescription)
{
return new CONFIG_ACTION(config, paramDescription);
}

inline
ACTION * MakeAPIAction(COMMANDS & commands,
                       const std::string & paramDescription,
                       bool needArgument,
                       API_FUNCTION funPtr)
{
return new API_ACTION(commands, paramDescription, needArgument, funPtr);
}

inline
ACTION * MakeAPIAction(COMMANDS & commands,
                       API_FUNCTION funPtr)
{
return new API_ACTION(commands, "", false, funPtr);
}

bool RawXMLFunction(const SGCONF::CONFIG & config,
                    const std::string & arg,
                    const std::map<std::string, std::string> & /*options*/)
{
    STG::SERVCONF proto(config.server.data(),
                        config.port.data(),
                        config.userName.data(),
                        config.userPass.data());
    return proto.RawXML(arg, RawXMLCallback, NULL) == STG::st_ok;
}

} // namespace SGCONF

time_t stgTime;

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
SGCONF::CONFIG config;
SGCONF::COMMANDS commands;

SGCONF::OPTION_BLOCKS blocks;
blocks.Add("General options")
      .Add("c", "config", SGCONF::MakeParamAction(config.configFile, std::string("~/.config/stg/sgconf.conf"), "<config file>"), "override default config file")
      .Add("h", "help", SGCONF::MakeFunc0Action(bind0(Method1Adapt(&SGCONF::OPTION_BLOCKS::Help, blocks), 0)), "\t\tshow this help and exit")
      .Add("help-all", SGCONF::MakeFunc0Action(UsageAll), "\t\tshow full help and exit")
      .Add("v", "version", SGCONF::MakeFunc0Action(Version), "\t\tshow version information and exit");
SGCONF::OPTION_BLOCK & block = blocks.Add("Connection options")
      .Add("s", "server", SGCONF::MakeParamAction(config.server, std::string("localhost"), "<address>"), "\t\thost to connect")
      .Add("p", "port", SGCONF::MakeParamAction(config.port, uint16_t(5555), "<port>"), "\t\tport to connect")
      .Add("u", "username", SGCONF::MakeParamAction(config.userName, std::string("admin"), "<username>"), "\tadministrative login")
      .Add("w", "userpass", SGCONF::MakeParamAction(config.userPass, "<password>"), "\tpassword for the administrative login")
      .Add("a", "address", SGCONF::MakeParamAction(config, "<connection string>"), "connection params as a single string in format: <login>:<password>@<host>:<port>");
blocks.Add("Raw XML")
      .Add("r", "raw", SGCONF::MakeAPIAction(commands, "<xml>", true, SGCONF::RawXMLFunction), "\tmake raw XML request");
blocks.Add("Admin management options")
      .Add("get-admins", SGCONF::MakeAPIAction(commands, SGCONF::GetAdminsFunction), "\tget admin list")
      .Add("get-admin", SGCONF::MakeAPIAction(commands, "<login>", true, SGCONF::GetAdminFunction), "\tget admin")
      .Add("add-admin", SGCONF::MakeAPIAction(commands, "<login>", true, SGCONF::AddAdminFunction), "\tadd admin")
      .Add("del-admin", SGCONF::MakeAPIAction(commands, "<login>", true, SGCONF::DelAdminFunction), "\tdel admin")
      .Add("chg-admin", SGCONF::MakeAPIAction(commands, "<login>", true, SGCONF::ChgAdminFunction), "\tchange admin");
blocks.Add("Tariff management options")
      .Add("get-tariffs", SGCONF::MakeAPIAction(commands, SGCONF::GetTariffsFunction), "\tget tariff list")
      .Add("get-tariff", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::GetTariffFunction), "\tget tariff")
      .Add("add-tariff", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::AddTariffFunction), "\tadd tariff")
      .Add("del-tariff", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::DelTariffFunction), "\tdel tariff")
      .Add("chg-tariff", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::ChgTariffFunction), "\tchange tariff");
blocks.Add("User management options")
      .Add("get-users", SGCONF::MakeAPIAction(commands, SGCONF::GetUsersFunction), "\tget user list")
      .Add("get-user", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::GetUserFunction), "\tget user")
      .Add("add-user", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::AddUserFunction), "\tadd user")
      .Add("del-user", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::DelUserFunction), "\tdel user")
      .Add("chg-user", SGCONF::MakeAPIAction(commands, "<name>", true, SGCONF::ChgUserFunction), "\tchange user");

SGCONF::PARSER_STATE state(false, argc, argv);

try
{
state = blocks.Parse(--argc, ++argv); // Skipping self name
}
catch (const SGCONF::OPTION::ERROR& ex)
{
std::cerr << ex.what() << "\n";
return -1;
}

if (state.stop)
    return 0;

if (state.argc > 0)
    {
    std::cerr << "Unknown option: '" << *state.argv << "'\n";
    return -1;
    }

try
{
SGCONF::CONFIG configOverride(config);

if (config.configFile.empty())
    {
    const char * mainConfigFile = "/etc/sgconf/sgconf.conf";
    if (access(mainConfigFile, R_OK) == 0)
        block.ParseFile(mainConfigFile);
    ReadUserConfigFile(block);
    }
else
    {
    block.ParseFile(config.configFile.data());
    }

config = configOverride;
}
catch (const std::exception& ex)
{
std::cerr << ex.what() << "\n";
return -1;
}

std::cerr << "Config: " << config.Serialize() << std::endl;
return commands.Execute(config) ? 0 : -1;

/*return 0;

if (argc < 2)
    {
    Usage();
    return 1;
    }

if (argc <= 2)
    {
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (strcmp(argv[1], "get") == 0)
    {
    //printf("get\n");
    return mainGet(argc - 1, argv + 1);
    }
else if (strcmp(argv[1], "set") == 0)
    {
    //printf("set\n");
    if (mainSet(argc - 1, argv + 1) )
        return 0;
    return -1;
    }
else
    {
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }
return UNKNOWN_ERR_CODE;*/
}
//-----------------------------------------------------------------------------

namespace
{

void Usage()
{
UsageImpl(false);
}

void UsageAll()
{
UsageImpl(true);
}

void UsageImpl(bool full)
{
std::cout << "sgconf is the Stargazer management utility.\n\n"
          << "Usage:\n"
          << "\tsgconf [options]\n\n"
          << "General options:\n"
          << "\t-c, --config <config file>\t\toverride default config file (default: \"~/.config/stg/sgconf.conf\")\n"
          << "\t-h, --help\t\t\t\tshow this help and exit\n"
          << "\t--help-all\t\t\t\tshow full help and exit\n"
          << "\t-v, --version\t\t\t\tshow version information and exit\n\n";
UsageConnection();
UsageAdmins(full);
UsageTariffs(full);
UsageUsers(full);
UsageServices(full);
UsageCorporations(full);
}
//-----------------------------------------------------------------------------
void UsageConnection()
{
std::cout << "Connection options:\n"
          << "\t-s, --server <address>\t\t\thost to connect (ip or domain name, default: \"localhost\")\n"
          << "\t-p, --port <port>\t\t\tport to connect (default: \"5555\")\n"
          << "\t-u, --username <username>\t\tadministrative login (default: \"admin\")\n"
          << "\t-w, --userpass <password>\t\tpassword for administrative login\n"
          << "\t-a, --address <connection string>\tconnection params as a single string in format: <login>:<password>@<host>:<port>\n\n";
}
//-----------------------------------------------------------------------------
void UsageAdmins(bool full)
{
std::cout << "Admins management options:\n"
          << "\t--get-admins\t\t\t\tget a list of admins (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--login\t\t\t\tshow admin's login\n"
              << "\t\t--priv\t\t\t\tshow admin's priviledges\n\n";
std::cout << "\t--get-admin\t\t\t\tget the information about admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to show\n"
              << "\t\t--priv\t\t\t\tshow admin's priviledges\n\n";
std::cout << "\t--add-admin\t\t\t\tadd a new admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to add\n"
              << "\t\t--password <password>\t\tpassword of the admin to add\n"
              << "\t\t--priv <priv number>\t\tpriviledges of the admin to add\n\n";
std::cout << "\t--del-admin\t\t\t\tdelete an existing admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to delete\n\n";
std::cout << "\t--chg-admin\t\t\t\tchange an existing admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to change\n"
              << "\t\t--priv <priv number>\t\tnew priviledges\n\n";
}
//-----------------------------------------------------------------------------
void UsageTariffs(bool full)
{
std::cout << "Tariffs management options:\n"
          << "\t--get-tariffs\t\t\t\tget a list of tariffs (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--name\t\t\t\tshow tariff's name\n"
              << "\t\t--fee\t\t\t\tshow tariff's fee\n"
              << "\t\t--free\t\t\t\tshow tariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost\t\t\tshow tariff's cost of \"freeze\"\n"
              << "\t\t--traff-type\t\t\tshow what type of traffix will be accounted by the tariff\n"
              << "\t\t--dirs\t\t\t\tshow tarification rules for directions\n\n";
std::cout << "\t--get-tariff\t\t\t\tget the information about tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to show\n"
              << "\t\t--fee\t\t\t\tshow tariff's fee\n"
              << "\t\t--free\t\t\t\tshow tariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost\t\t\tshow tariff's cost of \"freeze\"\n"
              << "\t\t--traff-type\t\t\tshow what type of traffix will be accounted by the tariff\n"
              << "\t\t--dirs\t\t\t\tshow tarification rules for directions\n\n";
std::cout << "\t--add-tariff\t\t\t\tadd a new tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to add\n"
              << "\t\t--fee <fee>\t\t\tstariff's fee\n"
              << "\t\t--free <free>\t\t\ttariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost <cost>\t\ttariff's cost of \"freeze\"\n"
              << "\t\t--traff-type <type>\t\twhat type of traffi will be accounted by the tariff\n"
              << "\t\t--times <times>\t\t\tslash-separated list of \"day\" time-spans (in form \"hh:mm-hh:mm\") for each direction\n"
              << "\t\t--prices-day-a <prices>\t\tslash-separated list of prices for \"day\" traffic before threshold for each direction\n"
              << "\t\t--prices-night-a <prices>\tslash-separated list of prices for \"night\" traffic before threshold for each direction\n"
              << "\t\t--prices-day-b <prices>\t\tslash-separated list of prices for \"day\" traffic after threshold for each direction\n"
              << "\t\t--prices-night-b <prices>\tslash-separated list of prices for \"night\" traffic after threshold for each direction\n"
              << "\t\t--single-prices <yes|no>\tslash-separated list of \"single price\" flags for each direction\n"
              << "\t\t--no-discounts <yes|no>\t\tslash-separated list of \"no discount\" flags for each direction\n"
              << "\t\t--thresholds <thresholds>\tslash-separated list of thresholds (in Mb) for each direction\n\n";
std::cout << "\t--del-tariff\t\t\t\tdelete an existing tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to delete\n\n";
std::cout << "\t--chg-tariff\t\t\t\tchange an existing tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to change\n"
              << "\t\t--fee <fee>\t\t\tstariff's fee\n"
              << "\t\t--free <free>\t\t\ttariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost <cost>\t\ttariff's cost of \"freeze\"\n"
              << "\t\t--traff-type <type>\t\twhat type of traffix will be accounted by the tariff\n"
              << "\t\t--dir <N>\t\t\tnumber of direction data to change\n"
              << "\t\t\t--time <time>\t\t\"day\" time-span (in form \"hh:mm-hh:mm\")\n"
              << "\t\t\t--price-day-a <price>\tprice for \"day\" traffic before threshold\n"
              << "\t\t\t--price-night-a <price>\tprice for \"night\" traffic before threshold\n"
              << "\t\t\t--price-day-b <price>\tprice for \"day\" traffic after threshold\n"
              << "\t\t\t--price-night-b <price>\tprice for \"night\" traffic after threshold\n"
              << "\t\t\t--single-price <yes|no>\t\"single price\" flag\n"
              << "\t\t\t--no-discount <yes|no>\t\"no discount\" flag\n"
              << "\t\t\t--threshold <threshold>\tthreshold (in Mb)\n\n";
}
//-----------------------------------------------------------------------------
void UsageUsers(bool full)
{
std::cout << "Users management options:\n"
          << "\t--get-users\t\t\t\tget a list of users (subsequent options will define what to show)\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--get-user\t\t\t\tget the information about user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--add-user\t\t\t\tadd a new user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--del-user\t\t\t\tdelete an existing user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--chg-user\t\t\t\tchange an existing user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--check-user\t\t\t\tcheck credentials is valid\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--send-message\t\t\t\tsend a message to a user\n";
if (full)
    std::cout << "\n\n";
}
//-----------------------------------------------------------------------------
void UsageServices(bool full)
{
std::cout << "Services management options:\n"
          << "\t--get-services\t\t\t\tget a list of services (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--name\t\t\t\tshow service's name\n"
              << "\t\t--comment\t\t\tshow a comment to the service\n"
              << "\t\t--cost\t\t\t\tshow service's cost\n"
              << "\t\t--pay-day\t\t\tshow service's pay day\n\n";
std::cout << "\t--get-service\t\t\t\tget the information about service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to show\n"
              << "\t\t--comment\t\t\tshow a comment to the service\n"
              << "\t\t--cost\t\t\t\tshow service's cost\n"
              << "\t\t--pay-day\t\t\tshow service's pay day\n\n";
std::cout << "\t--add-service\t\t\t\tadd a new service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to add\n"
              << "\t\t--comment <comment>\t\ta comment to the service\n"
              << "\t\t--cost <cost>\t\t\tservice's cost\n"
              << "\t\t--pay-day <day>\t\t\tservice's pay day\n\n";
std::cout << "\t--del-service\t\t\t\tdelete an existing service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to delete\n\n";
std::cout << "\t--chg-service\t\t\t\tchange an existing service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to change\n"
              << "\t\t--comment <comment>\t\ta comment to the service\n"
              << "\t\t--cost <cost>\t\t\tservice's cost\n"
              << "\t\t--pay-day <day>\t\t\tservice's pay day\n\n";
}
//-----------------------------------------------------------------------------
void UsageCorporations(bool full)
{
std::cout << "Corporations management options:\n"
          << "\t--get-corporations\t\t\tget a list of corporations (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--name\t\t\t\tshow corporation's name\n"
              << "\t\t--cash\t\t\t\tshow corporation's cash\n\n";
std::cout << "\t--get-corp\t\t\t\tget the information about corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to show\n"
              << "\t\t--cash\t\t\t\tshow corporation's cash\n\n";
std::cout << "\t--add-corp\t\t\t\tadd a new corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to add\n"
              << "\t\t--cash <cash>\t\t\tinitial corporation's cash (default: \"0\")\n\n";
std::cout << "\t--del-corp\t\t\t\tdelete an existing corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to delete\n\n";
std::cout << "\t--chg-corp\t\t\t\tchange an existing corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to change\n"
              << "\t\t--add-cash <amount>[:<message>]\tadd cash to the corporation's account and optional comment message\n"
              << "\t\t--set-cash <cash>[:<message>]\tnew corporation's cash and optional comment message\n\n";
}

void Version()
{
std::cout << "sgconf, version: 2.0.0-alpha.\n";
}

} // namespace anonymous
