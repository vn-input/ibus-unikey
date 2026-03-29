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

#ifndef __UNIKEY_USER_KEY_MAP_H
#define __UNIKEY_USER_KEY_MAP_H

/**
 * @file usrkeymap.h
 * @brief Load/store API for user-defined key mapping files.
 */

#include "inputproc.h"
struct UkKeyMapPair {
    unsigned char   key;
    int             action;
};

/**
 * @brief Load a user-defined key mapping file into a flat array.
 *
 * The file is first parsed to obtain an ordered list of key-action pairs; then
 * the provided `keyMap` array is populated so that each index corresponds to a
 * key code and contains the associated action.  Lowercase equivalents are
 * also filled when appropriate.
 *
 * @param fileName Path to the key mapping file to read.
 * @param keyMap   Caller-allocated 256-element array that receives mapping
 *                 values.

 */
DllInterface int UkLoadKeyMap(const char *fileName, int keyMap[256]);
/**
 * @brief Read an ordered key map from disk.
 *
 * Parses the file specified by `fileName` and fills the supplied `pMap`
 * buffer with `UkKeyMapPair` entries in the same order they appear in the
 * file.  The total number of pairs read is written to `pMapCount`.
 *
 * @param fileName   Path to the mapping file.
 * @param pMap        Caller-allocated buffer large enough for 256 pairs.
 * @param pMapCount   Pointer to an int that receives the number of entries
 *                    actually read.

 */
DllInterface int UkLoadKeyOrderMap(const char *fileName, UkKeyMapPair *pMap, int *pMapCount);
/**
 * @brief Write a sequence of key-action pairs to a file.
 *
 * Creates or truncates the file at `fileName` and emits a human-readable header
 * followed by each mapping in `pMap` up to `mapCount` entries.  Only valid
 * actions (those with a label) are written.
 *
 * @param fileName  Destination filename for the key map.
 * @param pMap      Array of `UkKeyMapPair` to serialize.
 * @param mapCount  Number of entries in `pMap` to write.

 */
DllInterface int UkStoreKeyOrderMap(const char *fileName, UkKeyMapPair *pMap, int mapCount);

#endif
