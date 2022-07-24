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

#include "xml.h"
#include "admins.h"
#include "tariffs.h"
#include "users.h"
#include "services.h"
#include "corps.h"
#include "info.h"

#include "api_action.h"
#include "options.h"
#include "actions.h"
#include "config.h"

#include <string>
#include <iostream>

#include <cstdlib> // getenv
#include <cstring> // str*

#include <unistd.h> // access
#include <libgen.h> // basename

namespace
{

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

template <typename A, typename R>
class FUNC1_ADAPTER : public std::unary_function<A, R>
{
    public:
        explicit FUNC1_ADAPTER(R (*func)(A)) : m_func(func) {}
        const R operator()(A arg) const { return (m_func)(arg); }
    private:
        R (*m_func)(A);
};

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

template <typename A, typename R>
FUNC1_ADAPTER<A, R> Func1Adapt(R (func)(A))
{
return FUNC1_ADAPTER<A, R>(func);
}

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

void Version(const std::string & self)
{
std::cout << self << ", version: 2.0.0.\n";
}

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

        std::string ParamDescription() const override { return m_description; }
        std::string DefaultDescription() const override { return ""; }
        OPTION_BLOCK & Suboptions() override { return m_suboptions; }
        PARSER_STATE Parse(int argc, char ** argv, void * /*data*/) override;

    private:
        SGCONF::CONFIG & m_config;
        std::string m_description;
        OPTION_BLOCK m_suboptions;

        void ParseCredentials(const std::string & credentials);
        void ParseHostAndPort(const std::string & hostAndPort);
};


PARSER_STATE CONFIG_ACTION::Parse(int argc, char ** argv, void * /*data*/)
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

std::unique_ptr<SGCONF::ACTION> MakeParamAction(SGCONF::CONFIG & config,
                                                const std::string & paramDescription)
{
return std::make_unique<CONFIG_ACTION>(config, paramDescription);
}

} // namespace SGCONF

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    std::string self(basename(argv[0]));
    SGCONF::CONFIG config;
    SGCONF::COMMANDS commands;

    SGCONF::OPTION_BLOCKS blocks;
    blocks.Add("General options")
          .Add("c", "config", SGCONF::MakeParamAction(config.configFile, std::string("~/.config/stg/sgconf.conf"), "<config file>"), "override default config file")
          .Add("h", "help", SGCONF::MakeFunc0Action(bind0(Method1Adapt(&SGCONF::OPTION_BLOCKS::Help, blocks), 0)), "\t\tshow this help and exit")
          //.Add("help-all", SGCONF::MakeFunc0Action(UsageAll), "\t\tshow full help and exit")
          .Add("v", "version", SGCONF::MakeFunc0Action(bind0(Func1Adapt(Version), self)), "\t\tshow version information and exit");
    SGCONF::OPTION_BLOCK & block = blocks.Add("Connection options")
          .Add("s", "server", SGCONF::MakeParamAction(config.server, std::string("localhost"), "<address>"), "\t\thost to connect")
          .Add("p", "port", SGCONF::MakeParamAction(config.port, uint16_t(5555), "<port>"), "\t\tport to connect")
          .Add("local-address", SGCONF::MakeParamAction(config.localAddress, std::string(""), "<address>"), "\tlocal address to bind")
          .Add("local-port", SGCONF::MakeParamAction(config.localPort, uint16_t(0), "<port>"), "\t\tlocal port to bind")
          .Add("u", "username", SGCONF::MakeParamAction(config.userName, std::string("admin"), "<username>"), "\tadministrative login")
          .Add("w", "userpass", SGCONF::MakeParamAction(config.userPass, "<password>"), "\tpassword for the administrative login")
          .Add("a", "address", SGCONF::MakeParamAction(config, "<connection string>"), "connection params as a single string in format: <login>:<password>@<host>:<port>");
    blocks.Add("Debug options")
          .Add("show-config", SGCONF::MakeParamAction(config.showConfig), "\tshow config and exit");
    SGCONF::AppendXMLOptionBlock(commands, blocks);
    SGCONF::AppendServerInfoBlock(commands, blocks);
    SGCONF::AppendAdminsOptionBlock(commands, blocks);
    SGCONF::AppendTariffsOptionBlock(commands, blocks);
    SGCONF::AppendUsersOptionBlock(commands, blocks);
    SGCONF::AppendServicesOptionBlock(commands, blocks);
    SGCONF::AppendCorpsOptionBlock(commands, blocks);

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
        // Preserve config values parsed from the command line
        SGCONF::CONFIG configOverride(config);

        if (!config.configFile)
        {
            // Read main config file.
            const char * mainConfigFile = "/etc/sgconf/sgconf.conf";
            if (access(mainConfigFile, R_OK) == 0)
                block.ParseFile(mainConfigFile);
            // Read XDG-stuff.
            ReadUserConfigFile(block);
        }
        else
        {
            // Read user-supplied file.
            block.ParseFile(config.configFile.value());
        }

        // Apply overrides from the command line
        config.splice(configOverride);

        if (config.showConfig && config.showConfig.value())
        {
            std::cout << config.Serialize() << std::endl;
            return 0;
        }
        return commands.Execute(config) ? 0 : -1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
        return -1;
    }
}
