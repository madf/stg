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
 *    Date: 07.11.2007
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#pragma once

#include "stg/tariff.h"
#include "stg/tariffs.h"
#include "stg/tariff_conf.h"
#include "tariff_impl.h"

#include <string>
#include <vector>
#include <set>
#include <mutex>

namespace STG
{

struct Store;
class Logger;
struct Admin;

class TariffsImpl : public Tariffs {
    public:
        using Data = std::vector<TariffImpl>;

        explicit TariffsImpl(Store * store);

        int ReadTariffs () override;
        const Tariff * FindByName(const std::string & name) const override;
        const Tariff * GetNoTariff() const override { return &noTariff; }
        size_t Count() const override;
        int Del(const std::string & name, const Admin * admin) override;
        int Add(const std::string & name, const Admin * admin) override;
        int Chg(const TariffData & td, const Admin * admin) override;

        void AddNotifierAdd(NotifierBase<TariffData> * notifier) override;
        void DelNotifierAdd(NotifierBase<TariffData> * notifier) override;

        void AddNotifierDel(NotifierBase<TariffData> * notifier) override;
        void DelNotifierDel(NotifierBase<TariffData> * notifier) override;

        void GetTariffsData(std::vector<TariffData> * tdl) const override;

        const std::string & GetStrError() const override { return strError; }

    private:
        Data               tariffs;
        Store*             store;
        Logger&            WriteServLog;
        mutable std::mutex m_mutex;
        std::string        strError;
        TariffImpl         noTariff;

        std::set<NotifierBase<TariffData>*> onAddNotifiers;
        std::set<NotifierBase<TariffData>*> onDelNotifiers;
};

}
