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

#define EB_VERSION_FULL "155e248a63d52f7b491c5ba83056e23b784f4cdf"
#define EB_DATE_FULL    "2026-04-07 15:04:38 -0500"

#define EB_VERSION_SHORT (uint32_t)UINT32_C(0x155e248a)
#define EB_DATE_SHORT    (uint32_t)UINT32_C(0x20260407)

#endif
