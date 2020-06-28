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
 *    Author : Maksym Mamontov <stg@madf.info>
 */

/*
 $Revision: 1.1 $
 $Date: 2010/02/11 12:32:25 $
 $Author: faust $
 */

/*
 *  An implementation of RAII pid-file writer
 */

#include <fstream>
#include <unistd.h>

#include "pidfile.h"

PIDFile::PIDFile(const std::string & fn)
    : fileName(fn)
{
if (fileName != "")
    {
    std::ofstream pf(fileName.c_str());
    pf << getpid() << std::endl;
    pf.close();
    }
}

PIDFile::~PIDFile()
{
if (fileName != "")
    {
    unlink(fileName.c_str());
    }
}
