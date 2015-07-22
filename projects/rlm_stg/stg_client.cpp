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

#include "stg_client.h"

#include "stg/common.h"

#include <boost/bind.hpp>

#include <stdexcept>

namespace {

STG_CLIENT* stgClient = NULL;

unsigned fromType(STG_CLIENT::TYPE type)
{
    return static_cast<unsigned>(type);
}

STG::SGCP::TransportType toTransport(const std::string& value)
{
    std::string type = ToLower(value);
    if (type == "unix") return STG::SGCP::UNIX;
    else if (type == "udp") return STG::SGCP::UDP;
    else if (type == "tcp") return STG::SGCP::TCP;
    throw ChannelConfig::Error("Invalid transport type. Should be 'unix', 'udp' or 'tcp'.");
}

}

ChannelConfig::ChannelConfig(std::string addr)
    : transport(STG::SGCP::TCP)
{
    // unix:pass@/var/run/stg.sock
    // tcp:secret@192.168.0.1:12345
    // udp:key@isp.com.ua:54321

    size_t pos = addr.find_first_of(':');
    if (pos == std::string::npos)
        throw Error("Missing transport name.");
    transport = toTransport(addr.substr(0, pos));
    addr = addr.substr(pos + 1);
    if (addr.empty())
        throw Error("Missing address to connect to.");
    pos = addr.find_first_of('@');
    if (pos != std::string::npos) {
        key = addr.substr(0, pos);
        addr = addr.substr(pos + 1);
        if (addr.empty())
            throw Error("Missing address to connect to.");
    }
    if (transport == STG::SGCP::UNIX)
    {
        address = addr;
        return;
    }
    pos = addr.find_first_of(':');
    if (pos == std::string::npos)
        throw Error("Missing port.");
    address = addr.substr(0, pos);
    if (str2x(addr.substr(pos + 1), port))
        throw Error("Invalid port value.");
}

STG_CLIENT::STG_CLIENT(const std::string& address)
    : m_config(address),
      m_proto(m_config.transport, m_config.key),
      m_thread(boost::bind(&STG_CLIENT::m_run, this))
{
}

STG_CLIENT::~STG_CLIENT()
{
    stop();
}

bool STG_CLIENT::stop()
{
    return m_proto.stop();
}

RESULT STG_CLIENT::request(TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs)
{
    m_writeHeader(type, userName, password);
    m_writePairBlock(pairs);
    RESULT result;
    result.modify = m_readPairBlock();
    result.reply = m_readPairBlock();
    return result;
}

STG_CLIENT* STG_CLIENT::get()
{
    return stgClient;
}

bool STG_CLIENT::configure(const std::string& address)
{
    if ( stgClient != NULL && stgClient->stop() )
        delete stgClient;
    try {
        stgClient = new STG_CLIENT(address);
        return true;
    } catch (const ChannelConfig::Error& ex) {
        // TODO: Log it
    }
    return false;
}

void STG_CLIENT::m_writeHeader(TYPE type, const std::string& userName, const std::string& password)
{
    try {
        m_proto.writeAll<uint64_t>(fromType(type));
        m_proto.writeAll(userName);
        m_proto.writeAll(password);
    } catch (const STG::SGCP::Proto::Error& ex) {
        throw Error(ex.what());
    }
}

void STG_CLIENT::m_writePairBlock(const PAIRS& pairs)
{
    try {
        m_proto.writeAll<uint64_t>(pairs.size());
        for (size_t i = 0; i < pairs.size(); ++i) {
            m_proto.writeAll(pairs[i].first);
            m_proto.writeAll(pairs[i].second);
        }
    } catch (const STG::SGCP::Proto::Error& ex) {
        throw Error(ex.what());
    }
}

PAIRS STG_CLIENT::m_readPairBlock()
{
    try {
        size_t count = m_proto.readAll<uint64_t>();
        if (count == 0)
            return PAIRS();
        PAIRS res(count);
        for (size_t i = 0; i < count; ++i) {
            res[i].first = m_proto.readAll<std::string>();
            res[i].second = m_proto.readAll<std::string>();
        }
        return res;
    } catch (const STG::SGCP::Proto::Error& ex) {
        throw Error(ex.what());
    }
}

void STG_CLIENT::m_run()
{
    m_proto.connect(m_config.address, m_config.port);
    m_proto.run();
}
