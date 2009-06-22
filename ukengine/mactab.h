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

#ifndef __MACRO_TABLE_H
#define __MACRO_TABLE_H

#include "keycons.h"
#include "charset.h"

#if defined(_WIN32)
    #if defined(UNIKEYHOOK)
        #define DllInterface   __declspec( dllexport )
    #else
        #define DllInterface   __declspec( dllimport )
    #endif
#else
    #define DllInterface //not used
    #define DllExport
    #define DllImport
#endif

struct MacroDef
{
  int keyOffset;
  int textOffset;
};

#if !defined(WIN32)
typedef char TCHAR;
#endif

class DllInterface CMacroTable
{
public:
    void init();
    int loadFromFile(const char *fname);
    int writeToFile(const char *fname);

    const StdVnChar *lookup(StdVnChar *key);
    const StdVnChar *getKey(int idx);
    const StdVnChar *getText(int idx);
    int getCount() { return m_count; }
    void resetContent();
    int addItem(const char *item, int charset);
    int addItem(const void *key, const void *text, int charset);

protected:
    bool readHeader(FILE *f, int & version);
    void writeHeader(FILE *f);

    MacroDef m_table[MAX_MACRO_ITEMS];
    char m_macroMem[MACRO_MEM_SIZE];

    int m_count;
    int m_memSize, m_occupied;
};

#endif
