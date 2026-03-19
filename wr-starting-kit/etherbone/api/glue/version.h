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

#define EB_VERSION_FULL "a0e0abe5155af0466ca4666a4cff60ce12493a59"
#define EB_DATE_FULL    "2026-03-18 16:53:57 -0500"

#define EB_VERSION_SHORT (uint32_t)UINT32_C(0xa0e0abe5)
#define EB_DATE_SHORT    (uint32_t)UINT32_C(0x20260318)

#endif
