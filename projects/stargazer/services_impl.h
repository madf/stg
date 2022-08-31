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

#pragma once

#include "stg/services.h"
#include "stg/service_conf.h"
#include "stg/noncopyable.h"
#include "stg/logger.h"

#include <vector>
#include <map>
#include <string>
#include <mutex>

namespace STG
{

class Admin;
struct Store;

class ServicesImpl : public Services {
    public:
        explicit ServicesImpl(Store* st);

        int Add(const ServiceConf& service, const Admin* admin) override;
        int Del(const std::string& name, const Admin* admin) override;
        int Change(const ServiceConf& service, const Admin* admin) override;
        bool Find(const std::string& name, ServiceConf* service) const override;
        bool Find(const std::string& name, ServiceConfOpt* service) const override;
        bool Exists(const std::string& name) const override;
        const std::string& GetStrError() const override { return strError; }

        size_t Count() const override { return data.size(); }

        int OpenSearch() const override;
        int SearchNext(int, ServiceConf* service) const override;
        int CloseSearch(int) const override;

    private:
        typedef std::vector<ServiceConf>::iterator       iterator;
        typedef std::vector<ServiceConf>::const_iterator const_iterator;

        bool Read();

        std::vector<ServiceConf> data;
        Store*                 store;
        Logger&            WriteServLog;
        mutable std::map<int, const_iterator> searchDescriptors;
        mutable unsigned int    handle;
        mutable std::mutex mutex;
        std::string             strError;
};

}
