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

#ifndef __STG_RLM_CLIENT_H__
#define __STG_RLM_CLIENT_H__

#include "types.h"

#include "stg/os_int.h"

#include <boost/scoped_ptr.hpp>

#include <string>

namespace STG
{
namespace RLM
{

class Client
{
public:
    Client(const std::string& address);
    ~Client();

    bool stop();

    static Client* get();
    static bool configure(const std::string& address);

    RESULT request(REQUEST_TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs);

private:
    class Impl;
    boost::scoped_ptr<Impl> m_impl;
};

} // namespace RLM
} // namespace STG

#endif
