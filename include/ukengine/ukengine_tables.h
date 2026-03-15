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

#pragma once

#include "vnlexi.h"
#include "ukengine.h"

/** @brief ASCII code for the Enter key. */
#ifndef ENTER_CHAR
#define ENTER_CHAR 13
#endif

/** @brief True if integer x is odd. */
#ifndef IS_ODD
#define IS_ODD(x) (x & 1)
#endif

/** @brief True if integer x is even. */
#ifndef IS_EVEN
#define IS_EVEN(x) (!(x & 1))
#endif

#ifndef IS_STD_VN_LOWER
#define IS_STD_VN_LOWER(x) ((x) >= VnStdCharOffset && (x) < (VnStdCharOffset + TOTAL_ALPHA_VNCHARS) && IS_ODD(x))
#endif
#ifndef IS_STD_VN_UPPER
#define IS_STD_VN_UPPER(x) ((x) >= VnStdCharOffset && (x) < (VnStdCharOffset + TOTAL_ALPHA_VNCHARS) && IS_EVEN(x))
#endif

extern bool IsVnVowel[vnl_lastChar];
extern unsigned char SpecialWesternChars[];
extern StdVnChar IsoStdVnCharMap[256];

inline StdVnChar IsoToStdVnChar(int keyCode)
{
    return (keyCode < 256) ? IsoStdVnCharMap[keyCode] : keyCode;
}

struct VowelSeqInfo {
    int len;
    int complete;
    int conSuffix;
    VnLexiName v[3];
    VowelSeq sub[3];

    int roofPos;
    VowelSeq withRoof;

    int hookPos;
    VowelSeq withHook;
};

extern VowelSeqInfo VSeqList[];

struct ConSeqInfo {
    int len;
    VnLexiName c[3];
    bool suffix;
};

extern ConSeqInfo CSeqList[];

void engineClassInit();
bool isValidCV(ConSeq c, VowelSeq v);
bool isValidVC(VowelSeq v, ConSeq c);
bool isValidCVC(ConSeq c1, VowelSeq v, ConSeq c2);

VowelSeq lookupVSeq(VnLexiName v1, VnLexiName v2 = vnl_nonVnChar, VnLexiName v3 = vnl_nonVnChar);
ConSeq lookupCSeq(VnLexiName c1, VnLexiName c2 = vnl_nonVnChar, VnLexiName c3 = vnl_nonVnChar);
