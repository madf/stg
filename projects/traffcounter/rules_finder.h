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
 $Revision: 1.2 $
 $Date: 2009/02/26 18:32:59 $
 $Author: faust $
 */


#ifndef __RULES_FINDER_H__
#define __RULES_FINDER_H__

#include <pthread.h>

#include "rules.h"
#include "tc_packets.h"

namespace STG
{

    class RULES_FINDER
    {
    public:
        RULES_FINDER();
        ~RULES_FINDER();

        void SetRules(const RULES & r);

        int GetDir(const PENDING_PACKET & packet) const;

    private:
        RULES rules;
        mutable pthread_mutex_t mutex;
    };

}

#endif
