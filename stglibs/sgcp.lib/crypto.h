#ifndef __STG_SGCP_CRYPTO_H__
#define __STG_SGCP_CRYPTO_H__

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

#include "stg/sgcp_transport.h"

#include "stg/os_int.h"

#include <string>

#include <unistd.h> // ssize_t

namespace STG
{
namespace SGCP
{

class CryptoProto : public TransportProto
{
    public:
        CryptoProto(const std::string& key, TransportProto* underlying);
        virtual ~CryptoProto();

        virtual void connect(const std::string& address, uint16_t port);
        virtual ssize_t write(const void* buf, size_t size);
        virtual ssize_t read(void* buf, size_t size);

    private:
        TransportProto* m_underlying;
};

} // namespace SGCP
} // namespace STG

#endif
