#pragma once

#include "vnlexi.h"
#include "ukengine.h"

#ifndef ENTER_CHAR
#define ENTER_CHAR 13
#endif

#ifndef IS_ODD
#define IS_ODD(x) ((x) & 1)
#endif

#ifndef IS_EVEN
#define IS_EVEN(x) (!((x) & 1))
#endif

#ifndef IS_STD_VN_LOWER
#define IS_STD_VN_LOWER(x) ((x) >= VnStdCharOffset && (x) < (VnStdCharOffset + TOTAL_ALPHA_VNCHARS) && IS_ODD(x))
#endif

#ifndef IS_STD_VN_UPPER
#define IS_STD_VN_UPPER(x) ((x) >= VnStdCharOffset && (x) < (VnStdCharOffset + TOTAL_ALPHA_VNCHARS) && IS_EVEN(x))
#endif

// Shared tables/state (defined in ukengine_tables.cpp).
extern bool IsVnVowel[vnl_lastChar];
extern unsigned char SpecialWesternChars[];
extern StdVnChar IsoStdVnCharMap[256];

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

typedef int (UkEngine::* UkKeyProc)(UkKeyEvent & ev);
extern UkKeyProc UkKeyProcList[vneCount];

// Shared helper functions (defined in ukengine_tables.cpp).
inline StdVnChar IsoToStdVnChar(int keyCode)
{
    return (keyCode < 256) ? IsoStdVnCharMap[keyCode] : keyCode;
}
bool isValidCV(ConSeq c, VowelSeq v);
bool isValidVC(VowelSeq v, ConSeq c);
bool isValidCVC(ConSeq c1, VowelSeq v, ConSeq c2);
void engineClassInit();
VowelSeq lookupVSeq(VnLexiName v1, VnLexiName v2 = vnl_nonVnChar, VnLexiName v3 = vnl_nonVnChar);
ConSeq lookupCSeq(VnLexiName c1, VnLexiName c2 = vnl_nonVnChar, VnLexiName c3 = vnl_nonVnChar);

VnLexiName changeCase(VnLexiName x);
VnLexiName vnToLower(VnLexiName x);
