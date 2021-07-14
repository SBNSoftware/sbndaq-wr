/** @file version.h
 *  @brief Report the version and date of the source code.
 *
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  We reserved the low 8K memory region for this device.
 *
 *  @author Wesley W. Terpstra <w.terpstra@gsi.de>
 *
 *  @bug None!
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#ifndef VERSION_H
#define VERSION_H

#include <inttypes.h>

#define EB_VERSION_FULL "fb2777d4c4799600504fd7f58375b5d0c96a032d"
#define EB_DATE_FULL    "2017-03-03 12:23:22 -0600"

#define EB_VERSION_SHORT (uint32_t)UINT32_C(0xfb2777d4)
#define EB_DATE_SHORT    (uint32_t)UINT32_C(0x20170303)

#endif
