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

/**
 * @file ukengine.cpp
 * @brief Core Vietnamese input engine implementation.
 *
 * This file contains the internal lookup tables, validation helpers, and key
 * processing routines used to transform raw keystrokes into Vietnamese text,
 * including tone placement, macro expansion, spell-check-aware restoration,
 * and charset-specific output generation.
 */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "keycons.h"

/*
#if defined(_WIN32)
#include "keyhook.h"
#endif
*/

#include "vnlexi.h"
#include "ukengine.h"

#include "charset.h"

using namespace std;

#include "ukengine_tables.h"

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

bool IsVnVowel[vnl_lastChar];

extern VnLexiName AZLexiUpper[]; //defined in inputproc.cpp
extern VnLexiName AZLexiLower[];

//see vnconv/data.cpp for explanation of these characters
unsigned char SpecialWesternChars[] = {
  0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
  0x89, 0x8A, 0x8B, 0x8C, 0x8E, 0x91, 0x92, 0x93,
  0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
  0x9C, 0x9E, 0x9F, 0x00};

StdVnChar IsoStdVnCharMap[256];

VowelSeqInfo VSeqList[] = {
    {1, 1, 1, {vnl_a, vnl_nonVnChar, vnl_nonVnChar}, {vs_a, vs_nil, vs_nil}, -1, vs_ar, -1, vs_ab},
    {1, 1, 1, {vnl_ar, vnl_nonVnChar, vnl_nonVnChar}, {vs_ar, vs_nil, vs_nil}, 0, vs_nil, -1, vs_ab},
    {1, 1, 1, {vnl_ab, vnl_nonVnChar, vnl_nonVnChar}, {vs_ab, vs_nil, vs_nil}, -1, vs_ar, 0, vs_nil},
    {1, 1, 1, {vnl_e, vnl_nonVnChar, vnl_nonVnChar}, {vs_e, vs_nil, vs_nil}, -1, vs_er, -1, vs_nil},
    {1, 1, 1, {vnl_er, vnl_nonVnChar, vnl_nonVnChar}, {vs_er, vs_nil, vs_nil}, 0, vs_nil, -1, vs_nil},
    {1, 1, 1, {vnl_i, vnl_nonVnChar, vnl_nonVnChar}, {vs_i, vs_nil, vs_nil}, -1, vs_nil, -1, vs_nil},
    {1, 1, 1, {vnl_o, vnl_nonVnChar, vnl_nonVnChar}, {vs_o, vs_nil, vs_nil}, -1, vs_or, -1, vs_oh},
    {1, 1, 1, {vnl_or, vnl_nonVnChar, vnl_nonVnChar}, {vs_or, vs_nil, vs_nil}, 0, vs_nil, -1, vs_oh},
    {1, 1, 1, {vnl_oh, vnl_nonVnChar, vnl_nonVnChar}, {vs_oh, vs_nil, vs_nil}, -1, vs_or, 0, vs_nil},
    {1, 1, 1, {vnl_u, vnl_nonVnChar, vnl_nonVnChar}, {vs_u, vs_nil, vs_nil}, -1, vs_nil, -1, vs_uh},
    {1, 1, 1, {vnl_uh, vnl_nonVnChar, vnl_nonVnChar}, {vs_uh, vs_nil, vs_nil}, -1, vs_nil, 0, vs_nil},
    {1, 1, 1, {vnl_y, vnl_nonVnChar, vnl_nonVnChar}, {vs_y, vs_nil, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_a, vnl_i, vnl_nonVnChar}, {vs_a, vs_ai, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_a, vnl_o, vnl_nonVnChar}, {vs_a, vs_ao, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_a, vnl_u, vnl_nonVnChar}, {vs_a, vs_au, vs_nil}, -1, vs_aru, -1, vs_nil},
    {2, 1, 0, {vnl_a, vnl_y, vnl_nonVnChar}, {vs_a, vs_ay, vs_nil}, -1, vs_ary, -1, vs_nil},
    {2, 1, 0, {vnl_ar, vnl_u, vnl_nonVnChar}, {vs_ar, vs_aru, vs_nil}, 0, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_ar, vnl_y, vnl_nonVnChar}, {vs_ar, vs_ary, vs_nil}, 0, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_e, vnl_o, vnl_nonVnChar}, {vs_e, vs_eo, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 0, 0, {vnl_e, vnl_u, vnl_nonVnChar}, {vs_e, vs_eu, vs_nil}, -1, vs_eru, -1, vs_nil},
    {2, 1, 0, {vnl_er, vnl_u, vnl_nonVnChar}, {vs_er, vs_eru, vs_nil}, 0, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_i, vnl_a, vnl_nonVnChar}, {vs_i, vs_ia, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 0, 1, {vnl_i, vnl_e, vnl_nonVnChar}, {vs_i, vs_ie, vs_nil}, -1, vs_ier, -1, vs_nil},
    {2, 1, 1, {vnl_i, vnl_er, vnl_nonVnChar}, {vs_i, vs_ier, vs_nil}, 1, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_i, vnl_u, vnl_nonVnChar}, {vs_i, vs_iu, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 1, 1, {vnl_o, vnl_a, vnl_nonVnChar}, {vs_o, vs_oa, vs_nil}, -1, vs_nil, -1, vs_oab},
    {2, 1, 1, {vnl_o, vnl_ab, vnl_nonVnChar}, {vs_o, vs_oab, vs_nil}, -1, vs_nil, 1, vs_nil},
    {2, 1, 1, {vnl_o, vnl_e, vnl_nonVnChar}, {vs_o, vs_oe, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_o, vnl_i, vnl_nonVnChar}, {vs_o, vs_oi, vs_nil}, -1, vs_ori, -1, vs_ohi},
    {2, 1, 0, {vnl_or, vnl_i, vnl_nonVnChar}, {vs_or, vs_ori, vs_nil}, 0, vs_nil, -1, vs_ohi},
    {2, 1, 0, {vnl_oh, vnl_i, vnl_nonVnChar}, {vs_oh, vs_ohi, vs_nil}, -1, vs_ori, 0, vs_nil},
    {2, 1, 1, {vnl_u, vnl_a, vnl_nonVnChar}, {vs_u, vs_ua, vs_nil}, -1, vs_uar, -1, vs_uha},
    {2, 1, 1, {vnl_u, vnl_ar, vnl_nonVnChar}, {vs_u, vs_uar, vs_nil}, 1, vs_nil, -1, vs_nil},
    {2, 0, 1, {vnl_u, vnl_e, vnl_nonVnChar}, {vs_u, vs_ue, vs_nil}, -1, vs_uer, -1, vs_nil},
    {2, 1, 1, {vnl_u, vnl_er, vnl_nonVnChar}, {vs_u, vs_uer, vs_nil}, 1, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_u, vnl_i, vnl_nonVnChar}, {vs_u, vs_ui, vs_nil}, -1, vs_nil, -1, vs_uhi},
    {2, 0, 1, {vnl_u, vnl_o, vnl_nonVnChar}, {vs_u, vs_uo, vs_nil}, -1, vs_uor, -1, vs_uho},
    {2, 1, 1, {vnl_u, vnl_or, vnl_nonVnChar}, {vs_u, vs_uor, vs_nil}, 1, vs_nil, -1, vs_uoh},
    {2, 1, 1, {vnl_u, vnl_oh, vnl_nonVnChar}, {vs_u, vs_uoh, vs_nil}, -1, vs_uor, 1, vs_uhoh},
    {2, 0, 0, {vnl_u, vnl_u, vnl_nonVnChar}, {vs_u, vs_uu, vs_nil}, -1, vs_nil, -1, vs_uhu},
    {2, 1, 1, {vnl_u, vnl_y, vnl_nonVnChar}, {vs_u, vs_uy, vs_nil}, -1, vs_nil, -1, vs_nil},
    {2, 1, 0, {vnl_uh, vnl_a, vnl_nonVnChar}, {vs_uh, vs_uha, vs_nil}, -1, vs_nil, 0, vs_nil},
    {2, 1, 0, {vnl_uh, vnl_i, vnl_nonVnChar}, {vs_uh, vs_uhi, vs_nil}, -1, vs_nil, 0, vs_nil},
    {2, 0, 1, {vnl_uh, vnl_o, vnl_nonVnChar}, {vs_uh, vs_uho, vs_nil}, -1, vs_nil, 0, vs_uhoh},
    {2, 1, 1, {vnl_uh, vnl_oh, vnl_nonVnChar}, {vs_uh, vs_uhoh, vs_nil}, -1, vs_nil, 0, vs_nil},
    {2, 1, 0, {vnl_uh, vnl_u, vnl_nonVnChar}, {vs_uh, vs_uhu, vs_nil}, -1, vs_nil, 0, vs_nil},
    {2, 0, 1, {vnl_y, vnl_e, vnl_nonVnChar}, {vs_y, vs_ye, vs_nil}, -1, vs_yer, -1, vs_nil},
    {2, 1, 1, {vnl_y, vnl_er, vnl_nonVnChar}, {vs_y, vs_yer, vs_nil}, 1, vs_nil, -1, vs_nil},
    {3, 0, 0, {vnl_i, vnl_e, vnl_u}, {vs_i, vs_ie, vs_ieu}, -1, vs_ieru, -1, vs_nil},
    {3, 1, 0, {vnl_i, vnl_er, vnl_u}, {vs_i, vs_ier, vs_ieru}, 1, vs_nil, -1, vs_nil},
    {3, 1, 0, {vnl_o, vnl_a, vnl_i}, {vs_o, vs_oa, vs_oai}, -1, vs_nil, -1, vs_nil},
    {3, 1, 0, {vnl_o, vnl_a, vnl_y}, {vs_o, vs_oa, vs_oay}, -1, vs_nil, -1, vs_nil},  
    {3, 1, 0, {vnl_o, vnl_e, vnl_o}, {vs_o, vs_oe, vs_oeo}, -1, vs_nil, -1, vs_nil},
    {3, 0, 0, {vnl_u, vnl_a, vnl_y}, {vs_u, vs_ua, vs_uay}, -1, vs_uary, -1, vs_nil},
    {3, 1, 0, {vnl_u, vnl_ar, vnl_y}, {vs_u, vs_uar, vs_uary}, 1, vs_nil, -1, vs_nil},
    {3, 0, 0, {vnl_u, vnl_o, vnl_i}, {vs_u, vs_uo, vs_uoi}, -1, vs_uori, -1, vs_uhoi},
    {3, 0, 0, {vnl_u, vnl_o, vnl_u}, {vs_u, vs_uo, vs_uou}, -1, vs_nil, -1, vs_uhou},
    {3, 1, 0, {vnl_u, vnl_or, vnl_i}, {vs_u, vs_uor, vs_uori}, 1, vs_nil, -1, vs_uohi},
    {3, 0, 0, {vnl_u, vnl_oh, vnl_i}, {vs_u, vs_uoh, vs_uohi}, -1, vs_uori, 1, vs_uhohi},
    {3, 0, 0, {vnl_u, vnl_oh, vnl_u}, {vs_u, vs_uoh, vs_uohu}, -1, vs_nil, 1, vs_uhohu},
    {3, 1, 0, {vnl_u, vnl_y, vnl_a}, {vs_u, vs_uy, vs_uya}, -1, vs_nil, -1, vs_nil},
    {3, 0, 1, {vnl_u, vnl_y, vnl_e}, {vs_u, vs_uy, vs_uye}, -1, vs_uyer, -1, vs_nil},
    {3, 1, 1, {vnl_u, vnl_y, vnl_er}, {vs_u, vs_uy, vs_uyer}, 2, vs_nil, -1, vs_nil},
    {3, 1, 0, {vnl_u, vnl_y, vnl_u}, {vs_u, vs_uy, vs_uyu}, -1, vs_nil, -1, vs_nil},
    {3, 0, 0, {vnl_uh, vnl_o, vnl_i}, {vs_uh, vs_uho, vs_uhoi}, -1, vs_nil, 0, vs_uhohi},
    {3, 0, 0, {vnl_uh, vnl_o, vnl_u}, {vs_uh, vs_uho, vs_uhou}, -1, vs_nil, 0, vs_uhohu},
    {3, 1, 0, {vnl_uh, vnl_oh, vnl_i}, {vs_uh, vs_uhoh, vs_uhohi}, -1, vs_nil, 0, vs_nil},
    {3, 1, 0, {vnl_uh, vnl_oh, vnl_u}, {vs_uh, vs_uhoh, vs_uhohu}, -1, vs_nil, 0, vs_nil},
    {3, 0, 0, {vnl_y, vnl_e, vnl_u}, {vs_y, vs_ye, vs_yeu}, -1, vs_yeru, -1, vs_nil},
    {3, 1, 0, {vnl_y, vnl_er, vnl_u}, {vs_y, vs_yer, vs_yeru}, 1, vs_nil, -1, vs_nil}
};

ConSeqInfo CSeqList[] = {
    {1, {vnl_b, vnl_nonVnChar, vnl_nonVnChar}, false},
    {1, {vnl_c, vnl_nonVnChar, vnl_nonVnChar}, true},
    {2, {vnl_c, vnl_h, vnl_nonVnChar}, true},
    {1, {vnl_d, vnl_nonVnChar, vnl_nonVnChar}, false},
    {1, {vnl_dd, vnl_nonVnChar, vnl_nonVnChar}, false},
    {2, {vnl_d, vnl_z, vnl_nonVnChar}, false},
    {1, {vnl_g, vnl_nonVnChar, vnl_nonVnChar}, false},
    {2, {vnl_g, vnl_h, vnl_nonVnChar}, false},
    {2, {vnl_g, vnl_i, vnl_nonVnChar}, false},
    {3, {vnl_g, vnl_i, vnl_n}, false},
    {1, {vnl_h, vnl_nonVnChar, vnl_nonVnChar}, false},
    {1, {vnl_k, vnl_nonVnChar, vnl_nonVnChar}, false},
    {2, {vnl_k, vnl_h, vnl_nonVnChar}, false},
    {1, {vnl_l, vnl_nonVnChar, vnl_nonVnChar}, false},
    {1, {vnl_m, vnl_nonVnChar, vnl_nonVnChar}, true},
    {1, {vnl_n, vnl_nonVnChar, vnl_nonVnChar}, true},
    {2, {vnl_n, vnl_g, vnl_nonVnChar}, true},
    {3, {vnl_n, vnl_g, vnl_h}, false},
    {2, {vnl_n, vnl_h, vnl_nonVnChar}, true},
    {1, {vnl_p, vnl_nonVnChar, vnl_nonVnChar}, true},
    {2, {vnl_p, vnl_h, vnl_nonVnChar}, false},
    {1, {vnl_q, vnl_nonVnChar, vnl_nonVnChar}, false},
    {2, {vnl_q, vnl_u, vnl_nonVnChar}, false},
    {1, {vnl_r, vnl_nonVnChar, vnl_nonVnChar}, false},
    {1, {vnl_s, vnl_nonVnChar, vnl_nonVnChar}, false},
    {1, {vnl_t, vnl_nonVnChar, vnl_nonVnChar}, true},
    {2, {vnl_t, vnl_h, vnl_nonVnChar}, false},
    {2, {vnl_t, vnl_r, vnl_nonVnChar}, false},
    {1, {vnl_v, vnl_nonVnChar, vnl_nonVnChar}, false},
    {1, {vnl_x, vnl_nonVnChar, vnl_nonVnChar}, false}
};

const int VSeqCount = sizeof(VSeqList)/sizeof(VowelSeqInfo);
struct VSeqPair {
    VnLexiName v[3];
    VowelSeq vs;
};
VSeqPair SortedVSeqList[VSeqCount];

const int CSeqCount = sizeof(CSeqList)/sizeof(ConSeqInfo);
struct CSeqPair {
    VnLexiName c[3];
    ConSeq cs;
};
CSeqPair SortedCSeqList[CSeqCount];

struct VCPair {
    VowelSeq v;
    ConSeq c;
};

VCPair VCPairList [] = {
  {vs_a, cs_c}, {vs_a, cs_ch}, {vs_a, cs_m}, {vs_a, cs_n}, {vs_a, cs_ng},
                {vs_a, cs_nh}, {vs_a, cs_p}, {vs_a, cs_t},
  {vs_ar, cs_c}, {vs_ar, cs_m}, {vs_ar, cs_n}, {vs_ar, cs_ng}, {vs_ar, cs_p}, {vs_ar, cs_t},
  {vs_ab, cs_c}, {vs_ab, cs_m}, {vs_ab, cs_n}, {vs_ab, cs_ng}, {vs_ab, cs_p}, {vs_ab, cs_t},

  {vs_e, cs_c}, {vs_e, cs_ch}, {vs_e, cs_m}, {vs_e, cs_n}, {vs_e, cs_ng},
                {vs_e, cs_nh}, {vs_e, cs_p}, {vs_e, cs_t},
  {vs_er, cs_c}, {vs_er, cs_ch}, {vs_er, cs_m}, {vs_er, cs_n}, {vs_er, cs_nh},
                {vs_er, cs_p}, {vs_er, cs_t},

  {vs_i, cs_c}, {vs_i, cs_ch}, {vs_i, cs_m}, {vs_i, cs_n}, {vs_i, cs_nh}, {vs_i, cs_p}, {vs_i, cs_t},

  {vs_o, cs_c}, {vs_o, cs_m}, {vs_o, cs_n}, {vs_o, cs_ng}, {vs_o, cs_p}, {vs_o, cs_t},
  {vs_or, cs_c}, {vs_or, cs_m}, {vs_or, cs_n}, {vs_or, cs_ng}, {vs_or, cs_p}, {vs_or, cs_t},
  {vs_oh, cs_m}, {vs_oh, cs_n}, {vs_oh, cs_p}, {vs_oh, cs_t},

  {vs_u, cs_c}, {vs_u, cs_m}, {vs_u, cs_n}, {vs_u, cs_ng}, {vs_u, cs_p}, {vs_u, cs_t},
  {vs_uh, cs_c}, {vs_uh, cs_m}, {vs_uh, cs_n}, {vs_uh, cs_ng}, {vs_uh, cs_t},

  {vs_y, cs_t},
  {vs_ie, cs_c}, {vs_ie, cs_m}, {vs_ie, cs_n}, {vs_ie, cs_ng}, {vs_ie, cs_p}, {vs_ie, cs_t},
  {vs_ier, cs_c}, {vs_ier, cs_m}, {vs_ier, cs_n}, {vs_ier, cs_ng}, {vs_ier, cs_p}, {vs_ier, cs_t},

  {vs_oa, cs_c}, {vs_oa, cs_ch}, {vs_oa, cs_m}, {vs_oa, cs_n}, {vs_oa, cs_ng},
                 {vs_oa, cs_nh}, {vs_oa, cs_p}, {vs_oa, cs_t},
  {vs_oab, cs_c}, {vs_oab, cs_m}, {vs_oab, cs_n}, {vs_oab, cs_ng}, {vs_oab, cs_t},

  {vs_oe, cs_n}, {vs_oe, cs_t},

  {vs_ua, cs_n}, {vs_ua, cs_ng}, {vs_ua, cs_t},
  {vs_uar, cs_n}, {vs_uar, cs_ng}, {vs_uar, cs_t},

  {vs_ue, cs_c}, {vs_ue, cs_ch}, {vs_ue, cs_n}, {vs_ue, cs_nh},
  {vs_uer, cs_c}, {vs_uer, cs_ch}, {vs_uer, cs_n}, {vs_uer, cs_nh},

  {vs_uo, cs_c}, {vs_uo, cs_m}, {vs_uo, cs_n}, {vs_uo, cs_ng}, {vs_uo, cs_p}, {vs_uo, cs_t},
  {vs_uor, cs_c}, {vs_uor, cs_m}, {vs_uor, cs_n}, {vs_uor, cs_ng}, {vs_uor, cs_t},
  {vs_uho, cs_c}, {vs_uho, cs_m}, {vs_uho, cs_n}, {vs_uho, cs_ng}, {vs_uho, cs_p}, {vs_uho, cs_t},
  {vs_uhoh, cs_c}, {vs_uhoh, cs_m}, {vs_uhoh, cs_n}, {vs_uhoh, cs_ng}, {vs_uhoh, cs_p}, {vs_uhoh, cs_t},

  {vs_uy, cs_c}, {vs_uy, cs_ch}, {vs_uy, cs_n}, {vs_uy, cs_nh}, {vs_uy, cs_p}, {vs_uy, cs_t},

  {vs_ye, cs_m}, {vs_ye, cs_n}, {vs_ye, cs_ng}, {vs_ye, cs_p}, {vs_ye, cs_t},
  {vs_yer, cs_m}, {vs_yer, cs_n}, {vs_yer, cs_ng}, {vs_yer, cs_t},

  {vs_uye, cs_n}, {vs_uye, cs_t},
  {vs_uyer, cs_n}, {vs_uyer, cs_t}

};

const int VCPairCount = sizeof(VCPairList)/sizeof(VCPair);

//TODO: auto-complete: e.g. luan -> lua^n

/**
 * @brief Look up a vowel-sequence identifier from up to three lexicon symbols.
 *
 * @param v1 First vowel symbol.
 * @param v2 Optional second vowel symbol.
 * @param v3 Optional third vowel symbol.
 */
VowelSeq lookupVSeq(VnLexiName v1, VnLexiName v2, VnLexiName v3);
/**
 * @brief Look up a consonant-sequence identifier from up to three symbols.
 *
 * @param c1 First consonant symbol.
 * @param c2 Optional second consonant symbol.
 * @param c3 Optional third consonant symbol.
 */
ConSeq lookupCSeq(VnLexiName c1, VnLexiName c2, VnLexiName c3);

bool UkEngine::m_classInit = false;

//------------------------------------------------
/**
 * @brief Compare two vowel-sequence keys for sorting and lookup.
 *
 * @param p1 Pointer to the first `VSeqPair`.
 * @param p2 Pointer to the second `VSeqPair`.
 * @return Negative, zero, or positive according to lexicographic order.
 */
int tripleVowelCompare(const void *p1, const void *p2)
{
    VSeqPair *t1 = (VSeqPair *)p1;
    VSeqPair *t2 = (VSeqPair *)p2;

    for (int i=0; i<3; i++) {
        if (t1->v[i] < t2->v[i])
            return -1;
        if (t1->v[i] > t2->v[i])
            return 1;
    }
    return 0;
}

//------------------------------------------------
/**
 * @brief Compare two consonant-sequence keys for sorting and lookup.
 *
 * @param p1 Pointer to the first `CSeqPair`.
 * @param p2 Pointer to the second `CSeqPair`.
 * @return Negative, zero, or positive according to lexicographic order.
 */
int tripleConCompare(const void *p1, const void *p2)
{
    CSeqPair *t1 = (CSeqPair *)p1;
    CSeqPair *t2 = (CSeqPair *)p2;

    for (int i=0; i<3; i++) {
        if (t1->c[i] < t2->c[i])
            return -1;
        if (t1->c[i] > t2->c[i])
            return 1;
    }
    return 0;
}

//------------------------------------------------
/**
 * @brief Compare vowel/consonant suffix pairs used by syllable validation.
 *
 * @param p1 Pointer to the first `VCPair`.
 * @param p2 Pointer to the second `VCPair`.
 * @return Negative, zero, or positive according to sort order.
 */
int VCPairCompare(const void *p1, const void *p2)
{
    VCPair *t1 = (VCPair *)p1;
    VCPair *t2 = (VCPair *)p2;

    if (t1->v < t2->v)
        return -1;
    if (t1->v > t2->v)
      return 1;
  
    if (t1->c < t2->c)
        return -1;
    if (t1->c > t2->c)
        return 1;
    return 0;
}

//----------------------------------------------------------
/**
 * @brief Check whether a leading consonant sequence can precede a vowel sequence.
 *
 * @param c Candidate leading consonant sequence.
 * @param v Candidate vowel sequence.
 * @return `true` when the combination is allowed by Vietnamese spelling rules.
 */
bool isValidCV(ConSeq c, VowelSeq v)
{
    if (c == cs_nil || v == vs_nil)
        return true;

    VowelSeqInfo & vInfo = VSeqList[v];

    if ((c == cs_gi && vInfo.v[0] == vnl_i) ||
        (c == cs_qu && vInfo.v[0] == vnl_u))
        return false; // gi doesn't go with i, qu doesn't go with u
  
    if (c == cs_k) {
        // k can only go with the following vowel sequences
        static VowelSeq kVseq[] = {vs_e, vs_i, vs_y, vs_er, vs_eo, vs_eu, 
                                   vs_eru, vs_ia, vs_ie, vs_ier, vs_ieu, vs_ieru, vs_nil};
        int i;
        for (i=0; kVseq[i] != vs_nil && kVseq[i] != v; i++);
        return (kVseq[i] != vs_nil);
    }

    //More checks
    return true;
}

//----------------------------------------------------------
/**
 * @brief Check whether a vowel sequence can take a trailing consonant sequence.
 *
 * @param v Candidate vowel sequence.
 * @param c Candidate trailing consonant sequence.
 * @return `true` when the final combination is valid.
 */
bool isValidVC(VowelSeq v, ConSeq c)
{
    if (v == vs_nil || c == cs_nil)
        return true;

    VowelSeqInfo & vInfo = VSeqList[v];
    if (!vInfo.conSuffix)
        return false;

    ConSeqInfo & cInfo = CSeqList[c];
    if (!cInfo.suffix)
        return false;

    VCPair p;
    p.v = v;
    p.c = c;
    if (bsearch(&p, VCPairList, VCPairCount, sizeof(VCPair), VCPairCompare))
        return true;

    return false;
}

//----------------------------------------------------------
/**
 * @brief Validate a full consonant-vowel-consonant syllable structure.
 *
 * @param c1 Leading consonant sequence.
 * @param v Middle vowel sequence.
 * @param c2 Trailing consonant sequence.
 * @return `true` when the syllable is accepted by the engine rules.
 */
bool isValidCVC(ConSeq c1, VowelSeq v, ConSeq c2)
{
    if (v == vs_nil)
        return (c1 == cs_nil || c2 != cs_nil);

    if (c1 == cs_nil)
        return isValidVC(v, c2);

    if (c2 == cs_nil)
        return isValidCV(c1, v);

    bool okCV = isValidCV(c1, v);
    bool okVC = isValidVC(v, c2);

    if (okCV && okVC)
        return true;

    if (!okVC) {
        //check some exceptions: vc fails but cvc passes

        // quyn, quynh
        if (c1 == cs_qu && v == vs_y && (c2 == cs_n || c2 == cs_nh))
            return true;

        // gieng, gie^ng
        if (c1 == cs_gi && (v == vs_e || v == vs_er) && (c2 == cs_n || c2 == cs_ng))
            return true;
    }
    return false;
}

//------------------------------------------------
/**
 * @brief Initialize lookup tables shared by all `UkEngine` instances.
 */
void engineClassInit()
{
    int i, j;

    for (i=0; i < VSeqCount; i++) {
        for (j=0; j<3; j++)
            SortedVSeqList[i].v[j] = VSeqList[i].v[j];
        SortedVSeqList[i].vs = (VowelSeq)i;
    }

    for (i=0; i < CSeqCount; i++) {
        for (j=0; j<3; j++)
            SortedCSeqList[i].c[j] = CSeqList[i].c[j];
        SortedCSeqList[i].cs = (ConSeq)i;
    }

    qsort(SortedVSeqList, VSeqCount, sizeof(VSeqPair), tripleVowelCompare);
    qsort(SortedCSeqList, CSeqCount, sizeof(CSeqPair), tripleConCompare);
    qsort(VCPairList, VCPairCount, sizeof(VCPair), VCPairCompare);

    for (i=0; i<vnl_lastChar; i++)
        IsVnVowel[i] = true;

    unsigned char ch;
    for (ch='a'; ch <= 'z'; ch++) {
        if (ch != 'a' && ch != 'e' && ch != 'i' &&
            ch != 'o' && ch != 'u' && ch != 'y') {
            IsVnVowel[AZLexiLower[ch-'a']] = false;
            IsVnVowel[AZLexiUpper[ch-'a']] = false;
        }
    }
    IsVnVowel[vnl_dd] = false;
    IsVnVowel[vnl_DD] = false;
}

//------------------------------------------------
/**
 * @brief Resolve a vowel sequence from its component lexicon symbols.
 *
 * @param v1 First vowel symbol.
 * @param v2 Optional second vowel symbol.
 * @param v3 Optional third vowel symbol.
 * @return Matching `VowelSeq`, or `vs_nil` when the combination is unknown.
 */
VowelSeq lookupVSeq(VnLexiName v1, VnLexiName v2, VnLexiName v3)
{
    VSeqPair key;
    key.v[0] = v1;
    key.v[1] = v2;
    key.v[2] = v3;

    VSeqPair *pInfo = (VSeqPair *)bsearch(&key, SortedVSeqList, VSeqCount, sizeof(VSeqPair), tripleVowelCompare);
    if (pInfo == 0)
        return vs_nil;
    return pInfo->vs;
}

//------------------------------------------------
/**
 * @brief Resolve a consonant sequence from its component symbols.
 *
 * @param c1 First consonant symbol.
 * @param c2 Optional second consonant symbol.
 * @param c3 Optional third consonant symbol.
 * @return Matching `ConSeq`, or `cs_nil` when the combination is unknown.
 */
ConSeq lookupCSeq(VnLexiName c1, VnLexiName c2, VnLexiName c3)
{
    CSeqPair key;
    key.c[0] = c1;
    key.c[1] = c2;
    key.c[2] = c3;

    CSeqPair *pInfo = (CSeqPair *)bsearch(&key, SortedCSeqList, CSeqCount, sizeof(CSeqPair), tripleConCompare);
    if (pInfo == 0)
        return cs_nil;
    return pInfo->cs;
}

//------------------------------------------------------------------
/**
 * @brief Apply or remove a roof diacritic on the current vowel sequence.
 *
 * @param ev Key event describing which roof key was pressed.
 * @return Non-zero when the event was handled by the composition engine.
 */
