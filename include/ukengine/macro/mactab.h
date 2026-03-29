// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Copyright (C) 2026 Hoang Duy Tran & AI Agent
 * Contact:
 *   unikey@gmail.com
 *   hoangduytran1960@googlemail.com
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

/**
 * @file mactab.h
 * @brief In-memory representation of user-defined macros for the engine.
 *
 * The `CMacroTable` class provides fast lookup and storage for macro
 * definitions.  Entries are stored in a compact contiguous memory buffer with
 * accompanying offset table.  The class supports loading/writing engine‑format
 * files, dynamic resizing, and duplicate elimination.  There is no fixed limit
 * on the number of macros (unlike the original 1024‑entry restriction).
 */

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

/**
 * @brief Single macro entry metadata stored in the table.
 *
 * Offsets point into the contiguous memory buffer `m_macroMem` where the
 * actual key and text strings are stored.
 */
struct MacroDef
{
  int keyOffset;  /**< byte offset of the key string */
  int textOffset; /**< byte offset of the expansion text */
};

#if !defined(WIN32)
typedef char TCHAR;
#endif

/**
 * @brief Container for macro definitions.
 *
 * Public methods allow file I/O, lookup by key, and modifying the table.
 * Internally allocations grow on demand and duplicate keys are handled such
 * that later definitions override earlier ones.
 */
class DllInterface CMacroTable
{
public:
    /**
     * @brief Construct an empty table.
     */
    CMacroTable();

    /**
     * @brief Destructor frees all allocated memory.
     */
    ~CMacroTable();

    /**
     * @brief Prepare internal members for use (called by constructor as well).
     */
    void init();

    /**
     * @brief Load macros from a file produced by `writeToFile`.
     *
     * @param fname Path to the macro file.
     * @return 0 on success, non‑zero error code on failure.
     */
    int loadFromFile(const char *fname);

    /**
     * @brief Write current macros to disk in engine format.
     *
     * @param fname Output file path.
     * @return 0 on success.
     */
    int writeToFile(const char *fname);

    /**
     * @brief Lookup the expansion for a given key.
     *
     * @param key Null‑terminated key string in engine charset.
     * @return Pointer to the text if found, or nullptr if missing.
     */
    const StdVnChar *lookup(StdVnChar *key);

    /**
     * @brief Retrieve the raw key string at index @p idx.
     */
    const StdVnChar *getKey(int idx);

    /**
     * @brief Retrieve the expansion text at index @p idx.
     */
    const StdVnChar *getText(int idx);

    /**
     * @brief Get number of entries currently stored.
     */
    int getCount() { return m_count; }

    /**
     * @brief Remove all macros from the table.
     */
    void resetContent();

    /**
     * @brief Add a macro using NUL‑terminated strings.
     *
     * @param item Key and text are provided in the same buffer separated by
     *             whitespace or tab; this mirrors the format of the on‑disk file.
     * @param charset Character set of the provided strings.
     */
    int addItem(const char *item, int charset);

    /**
     * @brief Add a macro providing separate key and text pointers.
     *
     * Both buffer contents are converted from `charset` to the internal storage
     * charset.
     */
    int addItem(const void *key, const void *text, int charset);

protected:
    /* header I/O helpers */
    bool readHeader(FILE *f, int & version);
    void writeHeader(FILE *f);

    /* ensure storage capacity */
    bool ensureItemCapacity(int minCapacity);
    bool ensureMemCapacity(int minCapacity);

    /**
     * @brief Sort entries and drop duplicates, keeping last occurrence.
     */
    void removeDuplicates();

    /* storage buffers */
    MacroDef *m_table; /**< dynamic array of offsets */
    char *m_macroMem;  /**< contiguous buffer holding all key/text strings */

    int m_count;        /**< number of entries currently stored */
    int m_itemCapacity; /**< allocated size of m_table array */
    int m_memSize, m_occupied; /**< total and used bytes of m_macroMem */
};

#endif
