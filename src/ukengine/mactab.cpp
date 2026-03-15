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

/**
 * @file mactab.cpp
 * @brief Implementation of CMacroTable, managing macro storage and lookup.
 *
 * The previous fixed-size limit of 1024 macros has been removed by using
 * dynamic arrays and geometric growth.  Macro files are read/written in a
 * plain text format with optional UTF‑8 header.  Internally the table keeps
 * keys and texts in a single heap‑allocated buffer for compactness and uses
 * binary search for fast lookup at runtime.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "mactab.h"
#include "vnconv.h"

using namespace std;
#define UKMACRO_VERSION_UTF8 1

#define INITIAL_MACRO_ITEMS_CAPACITY 128
#define INITIAL_MACRO_MEM_SIZE (1024*128)

/**
 * @brief Construct an empty CMacroTable.
 *
 * All pointers and counters are initialized to zero/null.  Storage arrays are
 * allocated lazily by the first call to `init` or `addItem`.
 */
CMacroTable::CMacroTable()
{
  m_table = NULL;
  m_macroMem = NULL;
  m_count = 0;
  m_itemCapacity = 0;
  m_memSize = 0;
  m_occupied = 0;
}

/**
 * @brief Free all allocated memory.
 *
 * Called automatically when the table object is destroyed.
 */
CMacroTable::~CMacroTable()
{
  if (m_table) {
    free(m_table);
    m_table = NULL;
  }
  if (m_macroMem) {
    free(m_macroMem);
    m_macroMem = NULL;
  }
}

/**
 * @brief Grow the macro entry array to accommodate at least @p minCapacity items.
 *
 * Capacities double each time (starting from INITIAL_MACRO_ITEMS_CAPACITY) so
 * amortized allocation cost remains low.  The function returns `false` if
 * memory allocation fails or requested capacity is absurdly large.
 *
 * @param minCapacity Minimum number of entries required.
 * @return `true` on success.
 */
bool CMacroTable::ensureItemCapacity(int minCapacity)
{
  if (minCapacity <= m_itemCapacity)
    return true;

  int newCapacity = (m_itemCapacity > 0) ? m_itemCapacity : INITIAL_MACRO_ITEMS_CAPACITY;
  while (newCapacity < minCapacity) {
    if (newCapacity > (1 << 29))
      return false;
    newCapacity *= 2;
  }

  MacroDef *newTable = (MacroDef *)realloc(m_table, sizeof(MacroDef) * newCapacity);
  if (!newTable)
    return false;

  m_table = newTable;
  m_itemCapacity = newCapacity;
  return true;
}

/**
 * @brief Grow the raw memory buffer to hold at least @p minCapacity bytes.
 *
 * Similar doubling growth strategy as `ensureItemCapacity`. The buffer holds
 * all key and text strings concatenated back-to-back.
 *
 * @param minCapacity Minimum number of bytes required.
 * @return `true` on success.
 */
bool CMacroTable::ensureMemCapacity(int minCapacity)
{
  if (minCapacity <= m_memSize)
    return true;

  int newSize = (m_memSize > 0) ? m_memSize : INITIAL_MACRO_MEM_SIZE;
  while (newSize < minCapacity) {
    if (newSize > (1 << 29))
      return false;
    newSize *= 2;
  }

  char *newMem = (char *)realloc(m_macroMem, newSize);
  if (!newMem)
    return false;

  m_macroMem = newMem;
  m_memSize = newSize;
  return true;
}

//---------------------------------------------------------------
/**
 * @brief Prepare the table for use by ensuring initial allocations exist.
 *
 * Existing contents (if any) are discarded by resetting counters; this method
 * is also invoked by `loadFromFile` and acts as a lightweight reinitialization
 * mechanism.
 */
void CMacroTable::init()
{
  if (!m_table) {
    ensureItemCapacity(INITIAL_MACRO_ITEMS_CAPACITY);
  }
  if (!m_macroMem) {
    ensureMemCapacity(INITIAL_MACRO_MEM_SIZE);
  }
  m_count = 0;
  m_occupied = 0;
}

//---------------------------------------------------------------
char *MacCompareStartMem;

#define STD_TO_LOWER(x) (((x) >= VnStdCharOffset && \
                          (x) < (VnStdCharOffset + TOTAL_ALPHA_VNCHARS) && \
                          !((x) & 1)) ? \
                          (x+1) : (x))

/**
 * @brief Comparison function used for sorting the macro table.
 *
 * Performs case-insensitive comparison of keys stored in the memory buffer
 * pointed to by `MacCompareStartMem`. This ordering is used by `qsort`.
 *
 * @param p1 Pointer to the first `MacroDef` entry.
 * @param p2 Pointer to the second `MacroDef` entry.
 * @return Negative, zero, or positive according to sort order.
 */
int macCompare(const void *p1, const void *p2)
{
    StdVnChar *s1 = (StdVnChar *) ((char *)MacCompareStartMem + ((MacroDef *)p1)->keyOffset);
    StdVnChar *s2 = (StdVnChar *) ((char *)MacCompareStartMem + ((MacroDef *)p2)->keyOffset);

    int i;
    StdVnChar ls1, ls2;

    for (i=0; s1[i] != 0 && s2[i] != 0; i++) {
        ls1 = STD_TO_LOWER(s1[i]);
        ls2 = STD_TO_LOWER(s2[i]);
        if (ls1 > ls2)
            return 1;
        if (ls1 < ls2)
            return -1;
        /*
        if (s1[i] > s2[i])
            return 1;
        if (s1[i] < s2[i])
            return -1;
        */
    }
    if (s1[i] == 0)
        return (s2[i] == 0)? 0 : -1;
    return 1;
}

//---------------------------------------------------------------
/**
 * @brief Comparator used by `bsearch` when looking up a key.
 *
 * Compares a standalone key string (first argument) with the key at a table
 * entry (`ele`). Uses the same case‑insensitive logic as `macCompare`.
 *
 * @param key Pointer to the lookup key string.
 * @param ele Pointer to a `MacroDef` table entry.
 * @return Negative, zero, or positive according to comparison result.
 */
int macKeyCompare(const void *key, const void *ele)
{
    StdVnChar *s1 = (StdVnChar *)key;
    StdVnChar *s2 = (StdVnChar *) ((char *)MacCompareStartMem + ((MacroDef *)ele)->keyOffset);

    StdVnChar ls1, ls2;
    int i;
    for (i=0; s1[i] != 0 && s2[i] != 0; i++) {
        ls1 = STD_TO_LOWER(s1[i]);
        ls2 = STD_TO_LOWER(s2[i]);
        if (ls1 > ls2)
            return 1;
        if (ls1 < ls2)
            return -1;
        /*
        if (s1[i] > s2[i])
            return 1;
        if (s1[i] < s2[i])
            return -1;
        */
    }
    if (s1[i] == 0)
        return (s2[i] == 0)? 0 : -1;
    return 1;
}

//---------------------------------------------------------------
/**
 * @brief Retrieve the expansion text associated with @p key.
 *
 * Uses binary search on the sorted table; callers must ensure the table has
 * been sorted and deduplicated (this is done automatically in `loadFromFile`).
 *
 * @param key Key string in the engine's internal charset.
 * @return Pointer to the associated text or `nullptr` if key not found.
 */
const StdVnChar *CMacroTable::lookup(StdVnChar *key)
{
  // Lookup relies on sorted `m_table`.
  // `loadFromFile` enforces ordering + dedupe before runtime use.
  MacCompareStartMem = m_macroMem;
  MacroDef *p = (MacroDef *)bsearch(key, m_table, m_count, sizeof(MacroDef), macKeyCompare);
  if (p)
    return (StdVnChar *)(m_macroMem + p->textOffset);
  return 0;
}

//----------------------------------------------------------------------------
// Read header, if it's present in the file. Get the version of the file
// If header is absent, go back to the beginning of file and set version to 0
// Return false if reading failed.
//
// Header format: ;[DO NOT DELETE THIS LINE]***version=n
//----------------------------------------------------------------------------
/**
 * @brief Attempt to read optional version header from macro file.
 *
 * If a header line matching the pattern `***version=n` is found the value is
 * stored in @p version.  Absence of a header sets version to 0 and rewinds the
 * file pointer.  A BOM at the start of the first line is skipped.
 *
 * @param f      Open file handle positioned at start.
 * @param version Output parameter for the parsed version.
 * @return `true` if the operation succeeded (even if no header was present);
 *         `false` on read error.
 */
bool CMacroTable::readHeader(FILE *f, int & version)
{
    char line[MAX_MACRO_LINE];
    if (!fgets(line, sizeof(line), f)) {
        if (feof(f)) {
            fseek(f, 0, SEEK_SET);
            version = 0;
            return true;
        }
        return false;
    }

    //if BOM is available, skip it
    char *p = line;
    size_t len = strlen(line);
    if (len >= 3 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && 
                    (unsigned char)line[2] == 0xBF) 
    {
        p += 3;
    }

    //read version number
    p = strstr(p, "***");
    if (p) {
        p += 3;
        //skip possible spaces
        while (*p == ' ') p++;
        if (sscanf(p, "version=%d", &version) == 1)
            return true;
    }

    fseek(f, 0, SEEK_SET);
    version = 0;
    return true;
}

//----------------------------------------------------------------
/**
 * @brief Write the version header line into a macro output file.
 *
 * On Windows, prepend a UTF-8 BOM for compatibility with text editors.
 *
 * @param f Output stream that receives the header line.
 */
void CMacroTable::writeHeader(FILE *f)
{
#if defined(WIN32)
    fprintf(f, "\xEF\xBB\xBF;DO NOT DELETE THIS LINE*** version=%d ***\n", UKMACRO_VERSION_UTF8);
#else
    fprintf(f, "DO NOT DELETE THIS LINE*** version=%d ***\n", UKMACRO_VERSION_UTF8);
#endif
}
//---------------------------------------------------------------
/**
 * @brief Load macros from a text file into the table.
 *
 * Existing contents are cleared.  The function handles optional headers and
 * converts entries to the internal charset.  After reading, the table is
 * sorted and duplicates removed; if the file was in an old format it is
 * rewritten to the current UTF-8 version.
 *
 * @param fname Path to input file.
 * @return 1 on success, 0 if the file could not be opened.
 */
int CMacroTable::loadFromFile(const char *fname)
{
    FILE *f;
#if defined(WIN32)
    f = _tfopen(fname, _TEXT("rt"));
#else
    f = fopen(fname, "r");
#endif

    if (f == NULL) 
        return 0;
    char line[MAX_MACRO_LINE];
    size_t len;

    resetContent();

    //read possible header
    int version;
    if (!readHeader(f, version)) {
        version = 0;
    }

    while (fgets(line, sizeof(line), f)) {
        len = strlen(line);
        if (len > 0 && line[len-1] == '\n')
            line[len-1] = 0;
        if (len > 1 && line[len-2] == '\r')
            line[len-2] = 0;
        if (version == UKMACRO_VERSION_UTF8)
            addItem(line, CONV_CHARSET_UNIUTF8);
        else
            addItem(line, CONV_CHARSET_VIQR);
    }
    fclose(f);
    // Keep table sorted for binary-search lookup and normalize duplicates.
    MacCompareStartMem = m_macroMem;
    qsort(m_table, m_count, sizeof(MacroDef), macCompare);
    removeDuplicates();
    // Convert old version
    if (version != UKMACRO_VERSION_UTF8) {
        writeToFile(fname);
    }
    return 1;
}

//---------------------------------------------------------------
// Remove consecutive duplicates after sorting (keeps LAST occurrence - dictionary semantics)
// Later additions overwrite earlier ones with the same key
// Among duplicates, the one with higher keyOffset was added later in the file
//---------------------------------------------------------------
/**
 * @brief Eliminate adjacent duplicate keys in the sorted table.
 *
 * After calling `qsort`, identical keys will be consecutive.  This method
 * keeps the last occurrence found (dictionary semantics) by comparing the
 * memory offset; entries added later reside further into `m_macroMem`.
 */
void CMacroTable::removeDuplicates()
{
    if (m_count <= 1)
        return;

    MacCompareStartMem = m_macroMem;
    int writeIdx = 0;
    for (int readIdx = 1; readIdx < m_count; readIdx++) {
        // Compare current with last kept entry
        if (macCompare(&m_table[writeIdx], &m_table[readIdx]) != 0) {
            // Different key - move write position and copy
            writeIdx++;
            if (writeIdx != readIdx) {
                m_table[writeIdx] = m_table[readIdx];
            }
        } else {
            // Same key (duplicate) - keep whichever was added later (higher offset in memory)
            if (m_table[readIdx].keyOffset > m_table[writeIdx].keyOffset) {
                m_table[writeIdx] = m_table[readIdx];
            }
        }
    }
    m_count = writeIdx + 1;
}

//---------------------------------------------------------------
/**
 * @brief Persist current macros to a text file.
 *
 * Each entry is converted to UTF-8 and written as `key:value` lines.  If
 * conversion of a particular key or value fails it is silently skipped.
 *
 * @param fname Destination file path.
 * @return 1 on success, 0 if the file could not be opened.
 */
int CMacroTable::writeToFile(const char *fname)
{
  int ret;
  int inLen, maxOutLen;
  FILE *f;
#if defined(WIN32)
  f = _tfopen(fname, _TEXT("wt"));
#else
  f = fopen(fname, "w");
#endif

  if (f == NULL)
    return 0;

  char line[MAX_MACRO_LINE*3+1]; //1 VnChar may need 3 chars in UTF8
  char key[MAX_MACRO_KEY_LEN*3];
  char text[MAX_MACRO_TEXT_LEN*3];

  writeHeader(f);

  UKBYTE *p;
  for (int i=0; i < m_count; i++) {
    p = (UKBYTE *)m_macroMem + m_table[i].keyOffset;
    inLen = -1;
    maxOutLen = sizeof(key);
    ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_UNIUTF8,
		    (UKBYTE *) p, (UKBYTE *)key,
		    &inLen, &maxOutLen);
    if (ret != 0)
      continue;

    p = (UKBYTE *)m_macroMem + m_table[i].textOffset;
    inLen = -1;
    maxOutLen = sizeof(text);
    ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_UNIUTF8,
		    p, (UKBYTE *)text,
		    &inLen, &maxOutLen);
    if (ret != 0)
      continue;
    if (i < m_count-1)
      sprintf(line, "%s:%s\n", key, text);
    else
      sprintf(line, "%s:%s", key, text);
    fputs(line, f);
  }

  fclose(f);
  return 1;
}

//---------------------------------------------------------------
/**
 * @brief Append a new macro given separate key and text buffers.
 *
 * The inputs are converted from @p charset to the internal Vietnamese standard
 * charset and stored consecutively in the memory buffer.  Offsets are recorded
 * in the table.  This routine ensures there is enough space for both the
 * metadata and the string data.
 *
 * @param key     Null-terminated key string.
 * @param text    Null-terminated expansion text.
 * @param charset Charset identifier of the input strings.
 * @return Index of newly added entry on success, -1 on failure.
 */
int CMacroTable::addItem(const void *key, const void *text, int charset)
{
  int ret;
  int inLen, maxOutLen;
  int offset = m_occupied;
  char *p;

  if (!ensureItemCapacity(m_count + 1))
    return -1;

  int needed = offset + MAX_MACRO_KEY_LEN * (int)sizeof(StdVnChar) +
               MAX_MACRO_TEXT_LEN * (int)sizeof(StdVnChar);
  if (!ensureMemCapacity(needed))
    return -1;

  p = m_macroMem + offset;
  
  m_table[m_count].keyOffset = offset;

  // Convert macro key to VN standard
  inLen = -1; //input is null-terminated
  maxOutLen = MAX_MACRO_KEY_LEN * sizeof(StdVnChar);
  ret = VnConvert(charset, CONV_CHARSET_VNSTANDARD, 
		          (UKBYTE *)key, (UKBYTE *)p,
		          &inLen, &maxOutLen);
  if (ret != 0)
    return -1;

  offset += maxOutLen;
  p += maxOutLen;

  //convert macro text to VN standard
  m_table[m_count].textOffset = offset;
  inLen = -1; //input is null-terminated
  maxOutLen = MAX_MACRO_TEXT_LEN * sizeof(StdVnChar);
  ret = VnConvert(charset, CONV_CHARSET_VNSTANDARD, 
		  (UKBYTE *)text, (UKBYTE *)p,
		  &inLen, &maxOutLen);
  if (ret != 0)
    return -1;

  m_occupied = offset + maxOutLen;
  m_count++;
  return (m_count-1);
}

//---------------------------------------------------------------
// add a new macro into the sorted macro table
// item format: key:text (key and text are separated by a colon)
//---------------------------------------------------------------
/**
 * @brief Parse and add a macro string of the form "key:value".
 *
 * Splits the input at the first colon and delegates to the other `addItem`
 * overload.  Returns -1 on parse failure.
 *
 * @param item Single macro definition in `key:text` form.
 * @param charset Charset identifier used to decode `item`.
 * @return Index of the new entry on success, or -1 if parsing/conversion fails.
 */
int CMacroTable::addItem(const char *item, int charset)
{
  char key[MAX_MACRO_KEY_LEN];
  
  // Parse the input item
  char * pos = (char*)strchr(item, ':');
  if (pos == NULL)
    return -1;
  int keyLen = (int)(pos - item);
  if (keyLen > MAX_MACRO_KEY_LEN-1)
    keyLen = MAX_MACRO_KEY_LEN-1;
  strncpy(key, item, keyLen);
  key[keyLen] = '\0';
  return addItem(key, ++pos, charset);
}

//---------------------------------------------------------------
/**
 * @brief Clear all entries without releasing allocated buffers.
 *
 * Useful when reloading from file to avoid repeated allocations.
 */
void CMacroTable::resetContent()
{
  m_occupied = 0;
  m_count = 0;
}

//---------------------------------------------------------------
/**
 * @brief Return pointer to the stored key at index @p idx.
 */
const StdVnChar *CMacroTable::getKey(int idx)
{
    if (idx < 0 || idx >= m_count)
        return 0;
    return (StdVnChar *)(m_macroMem + m_table[idx].keyOffset);
}

//---------------------------------------------------------------
/**
 * @brief Return pointer to the stored expansion text at index @p idx.
 */
const StdVnChar *CMacroTable::getText(int idx)
{
    if (idx < 0 || idx >= m_count)
        return 0;
    return (StdVnChar *)(m_macroMem + m_table[idx].textOffset);
}
