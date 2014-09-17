/****************************************************************************
** project:  zserver
** author:   giovanniangeli@tbe.it

** license: GPL
    
    Copyright (C) 2007  giovanni angeli www.tbe.it

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************************/

#ifndef GLOBALS_H
#define GLOBALS_H

#define TRUE 1
#define FALSE 0

#define ON 1
#define OFF 0

#define MAX( a, b) (a)>(b)?(a):(b)
#define MIN( a, b) (a)<(b)?(a):(b)

#define MY_ASSERT(a) \
if(!(a))\
{ \
    qWarning("%s:%d:%s: ASSERT FAILED.\n",__FILE__, __LINE__, __FUNCTION__); \
    perror("");\
    exit(-1); \
}

#define MY_WARNING(mask) if(this->debugLevel & mask) qWarning

#endif // GLOBALS_H
