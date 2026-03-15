// -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Contact:
 *   unikey@gmail.com
 *   UniKey project: http://unikey.org
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#ifndef VIETNAMESE_CHARSET_DATA_H
#define VIETNAMESE_CHARSET_DATA_H

/**
 * @file data.h
 * @brief Constants for special Vietnamese characters in conversion tables.
 */

// This header defines some special characters
const StdVnChar StdStartQuote = (VnStdCharOffset + 201);  // 0x93 in the Western charset
// 201 is the offset of character 0x93 (start quote) in Vn charsets
const StdVnChar StdEndQuote = (VnStdCharOffset + 202); // 0x94 in the Western charset
const StdVnChar StdEllipsis = (VnStdCharOffset + 190); // 0x85 in Western charet.

#endif
