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
 */

/*
 $Revision: 1.5 $
 $Date: 2010/03/04 11:57:11 $
 $Author: faust $
*/


#ifndef STG_LOCKER_H
#define STG_LOCKER_H

#include <pthread.h>

//-----------------------------------------------------------------------------
class STG_LOCKER
{
public:
    STG_LOCKER(pthread_mutex_t * m)
        : mutex(m)
        {
        pthread_mutex_lock(mutex);
        }

    ~STG_LOCKER()
        {
        pthread_mutex_unlock(mutex);
        }
private:
    STG_LOCKER(const STG_LOCKER & rvalue);
    STG_LOCKER & operator=(const STG_LOCKER & rvalue);

    pthread_mutex_t * mutex;
};
//-----------------------------------------------------------------------------

#endif //STG_LOCKER_H
