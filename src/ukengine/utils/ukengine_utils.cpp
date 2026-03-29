// -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Contact:
 *   unikey@gmail.com
 *   UniKey project: http://unikey.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
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

#include "ukengine_utils.h"

VnLexiName changeCase(VnLexiName x)
{
    if (x >= vnl_lastChar)
        return x;
    if (x == vnl_dd)
        return vnl_DD;
    if (x == vnl_DD)
        return vnl_dd;
    return (VnLexiName)(x^1);
}

VnLexiName vnToLower(VnLexiName x)
{
    if (x > vnl_lastChar || (x & 1))
        return x;
    return (VnLexiName)(x+1);
}
