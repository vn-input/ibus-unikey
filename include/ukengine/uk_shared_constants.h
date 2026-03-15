// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
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

#ifndef __UK_SHARED_CONSTANTS_H
#define __UK_SHARED_CONSTANTS_H

/**
 * @file uk_shared_constants.h
 * @brief Shared constants used across ukengine and ibus-unikey components.
 *
 * This header centralizes constants that were previously duplicated across
 * multiple source files, ensuring consistency and reducing maintenance risk.
 */

/**
 * @brief Characters considered to break words (delimiters).
 *
 * Used by both the input processor (ukengine) and the IBus engine (src) to
 * decide when to end the current word.
 *
 * Note: `~`, `` ` ``, and `^` are intentionally EXCLUDED to avoid interfering
 * with Vietnamese composition:
 *   - `~` is used for tilde tone mark
 *   - `^` is used for circumflex/roof diacritic  
 *   - `` ` `` is used for grave tone mark
 */
#define UK_WORD_BREAK_SYMS \
    ',', ';', ':', '.', '\"', '\'', '!', '?', ' ', \
    '<', '>', '=', '+', '-', '*', '/', '\\', \
    '_', '@', '#', '$', '%', '&', '(', ')', '{', '}', '[', ']', '|'

/**
 * @brief Number of word break symbols.
 *
 * Computed at compile time from UK_WORD_BREAK_SYMS.
 */
#define UK_WORD_BREAK_SYMS_COUNT 31

#endif // __UK_SHARED_CONSTANTS_H
