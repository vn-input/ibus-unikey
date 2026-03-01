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
 * @file charset.h
 * @brief Public declarations for charset.h.
 */
// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/*------------------------------------------------------------------------------
VnConv: Vietnamese Encoding Converter Library
UniKey Project: http://unikey.sourceforge.net
Copyleft (C) 1998-2002 Pham Kim Long
Contact: longp@cslab.felk.cvut.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--------------------------------------------------------------------------------*/

#ifndef __CHARSET_CONVERT_H
#define __CHARSET_CONVERT_H

/**
 * @file charset.h
 * @brief Character set conversion classes used by vnconv and ukengine.
 */

#if !defined(_WIN32)
  #include <stdint.h>
#endif

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

#include "vnconv.h"
#include "byteio.h"
#include "pattern.h"

/**
 * @brief Total number of Vietnamese characters supported by the tables.
 */
#define TOTAL_VNCHARS 213
/**
 * @brief Number of alphabetic Vietnamese characters (excluding control symbols).
 */
#define TOTAL_ALPHA_VNCHARS 186

#if defined(_WIN32)
    typedef unsigned __int32 StdVnChar;
    typedef unsigned __int16 UnicodeChar;
    typedef unsigned __int16 UKWORD;
    typedef unsigned __int32 UKDWORD;
#else
//typedef unsigned int StdVnChar; //the size should be more specific
	typedef uint32_t StdVnChar;
    typedef uint16_t UnicodeChar;
    typedef uint16_t UKWORD;
    typedef uint32_t UKDWORD;
#endif

//typedef unsigned short UnicodeChar;
//typedef unsigned short UKWORD;

//typedef unsigned int UKDWORD; //the size should be more specific

/**
 * @brief Extract low-order word from double-word.
 */
#ifndef LOWORD
#define LOWORD(l)           ((UKWORD)(l))
#endif

/**
 * @brief Extract high-order word from double-word.
 */
#ifndef HIWORD
#define HIWORD(l)           ((UKWORD)(((UKDWORD)(l) >> 16) & 0xFFFF))
#endif

/**
 * @brief Construct a word from two bytes.
 */
#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((UKWORD)(((UKBYTE)(a)) | ((UKWORD)((UKBYTE)(b))) << 8))
#endif

/**
 * @brief Base offset used for standard Vietnamese character codes.
 */
const StdVnChar VnStdCharOffset = 0x10000;
/**
 * @brief Value representing an invalid standardized character.
 */
const StdVnChar INVALID_STD_CHAR = 0xFFFFFFFF;

/**
 * @brief Padding character inserted during VIQR conversion (currently '#').
 */
const unsigned char PadChar = '#';
/**
 * @brief Character used to start quoted text during conversion.
 */
const unsigned char PadStartQuote = '"';
/**
 * @brief Character used to end quoted text during conversion.
 */
const unsigned char PadEndQuote = '"';
/**
 * @brief Character used to mark ellipsis sequences.
 */
const unsigned char PadEllipsis = '.';

/**
 * @class VnCharset
 * @brief Abstract base class for Vietnamese character set conversion.
 *
 * Concrete subclasses implement encoding/decoding of various one- and
 * multi-byte Vietnamese character sets used by the engine.
 */
class DllInterface VnCharset {
public:
	/**
	 * @brief Prepare for a sequence of input conversions.
	 */
	virtual void startInput() {};
	/**
	 * @brief Prepare for a sequence of output conversions.
	 */
	virtual void startOutput() {};

	/**
	 * @brief Convert next byte sequence from input stream to standard code.
	 * @param is Byte input stream to read from.
	 * @param[out] stdChar Standardized character result.
	 * @param[out] bytesRead Number of bytes consumed from input.
	 * @return nonzero on success, zero on end-of-input.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead) = 0;

	/**
	 * @brief Write a standard character to output stream in the target charset.
	 * @param os Output stream to write converted bytes.
	 * @param stdChar Standard character to convert.
	 * @param[out] outLen Number of bytes written.
	 * @return updated stream position (typically ignored).
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen) = 0;
    virtual int elementSize();
	virtual ~VnCharset() {}
};

//--------------------------------------------------
/**
 * @class SingleByteCharset
 * @brief Charset converter for single-byte Vietnamese encodings.
 *
 * Uses a 256-entry map and an array of Vietnamese characters to perform
 * conversions to/from the standard codepoint space.
 */
class SingleByteCharset: public VnCharset {
protected:
	UKWORD m_stdMap[256]; /**< lookup table from byte to standard code */
	unsigned char * m_vnChars; /**< array of Vietnamese bytes */
public:
	SingleByteCharset(unsigned char * vnChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class VnInternalCharset
 * @brief Identity charset used internally by the engine (no conversion).
 */
class VnInternalCharset: public VnCharset {
public:
  VnInternalCharset() {};
  virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
  virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
  virtual int elementSize();
};

//--------------------------------------------------
/**
 * @class UnicodeCharset
 * @brief Converter for fixed-width Unicode Vietnamese charsets.
 */
class UnicodeCharset: public VnCharset {
protected:
	UKDWORD m_vnChars[TOTAL_VNCHARS]; /**< mapping table from standard to VN char */
	UnicodeChar * m_toUnicode;       /**< reverse mapping to Unicode codes */
public:
	UnicodeCharset(UnicodeChar *vnChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
    virtual int elementSize();
};

//--------------------------------------------------
/**
 * @class DoubleByteCharset
 * @brief Converter for double-byte Vietnamese charsets.
 */
class DoubleByteCharset: public VnCharset {
protected:
	UKWORD m_stdMap[256]; /**< single-byte map to standard codes */
	UKDWORD m_vnChars[TOTAL_VNCHARS]; /**< array of double-byte VN chars */
	UKWORD * m_toDoubleChar;          /**< reverse mapping
 */
public:
	DoubleByteCharset(UKWORD *vnChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class UnicodeUTF8Charset
 * @brief Unicode charset that outputs UTF-8 encoded bytes.
 */class UnicodeUTF8Charset: public UnicodeCharset
{
public:
	UnicodeUTF8Charset(UnicodeChar *vnChars) : UnicodeCharset(vnChars)	{}

	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class UnicodeRefCharset
 * @brief Unicode charset variant that references external storage for data.
 */
class UnicodeRefCharset: public UnicodeCharset
{
public:
	UnicodeRefCharset(UnicodeChar *vnChars) : UnicodeCharset(vnChars)	{}

	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UnicodeHexCharset: public UnicodeRefCharset
{
public:
	UnicodeHexCharset(UnicodeChar *vnChars) : UnicodeRefCharset(vnChars) {}
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UnicodeCStringCharset: public UnicodeCharset
{
protected:
	int m_prevIsHex;
public:
	UnicodeCStringCharset(UnicodeChar *vnChars) : UnicodeCharset(vnChars) {}
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
	virtual void startInput();
};

//--------------------------------------------------
class WinCP1258Charset: public VnCharset {
protected:
	UKWORD m_stdMap[256];
	UKDWORD m_vnChars[TOTAL_VNCHARS*2];
	UKWORD *m_toDoubleChar;
	int m_totalChars;

public:
	WinCP1258Charset(UKWORD *compositeChars, UKWORD *precomposedChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
struct UniCompCharInfo {
	UKDWORD	compChar;
	int stdIndex;
};

class UnicodeCompCharset: public VnCharset {
protected:
	UniCompCharInfo m_info[TOTAL_VNCHARS*2];
	UKDWORD *m_uniCompChars;
	int m_totalChars;
public:
	UnicodeCompCharset(UnicodeChar *uniChars, UKDWORD *uniCompChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
    virtual int elementSize();
};

//--------------------------------------------------
class VIQRCharset: public VnCharset {
protected:
	UKDWORD *m_vnChars;
	UKWORD m_stdMap[256];
	int m_atWordBeginning;
	int m_escapeBowl;
	int m_escapeRoof;
	int m_escapeHook;
	int m_escapeTone;
	int m_gotTone;
	int m_escAll;
	int m_noOutEsc;
public:
	int m_suspicious;
	VIQRCharset(UKDWORD *vnChars);
	virtual void startInput();
	virtual void startOutput();
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UTF8VIQRCharset: public VnCharset {

protected:
	VIQRCharset *m_pViqr;
	UnicodeUTF8Charset *m_pUtf;

public:
	UTF8VIQRCharset(UnicodeUTF8Charset *pUtf, VIQRCharset *pViqr);
	virtual void startInput();
	virtual void startOutput();
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};


//--------------------------------------------------
class DllInterface CVnCharsetLib {
protected:
	SingleByteCharset * m_sgCharsets[CONV_TOTAL_SINGLE_CHARSETS];
	DoubleByteCharset * m_dbCharsets[CONV_TOTAL_DOUBLE_CHARSETS];
	UnicodeCharset * m_pUniCharset;	
	UnicodeCompCharset * m_pUniCompCharset;
	UnicodeUTF8Charset * m_pUniUTF8;
	UnicodeRefCharset * m_pUniRef;
	UnicodeHexCharset * m_pUniHex;
	VIQRCharset * m_pVIQRCharObj;
	UTF8VIQRCharset * m_pUVIQRCharObj;
	WinCP1258Charset * m_pWinCP1258;
	UnicodeCStringCharset *m_pUniCString;
	VnInternalCharset *m_pVnIntCharset;

public:
	PatternList m_VIQREscPatterns, m_VIQROutEscPatterns;
	VnConvOptions m_options;
	CVnCharsetLib();
	~CVnCharsetLib();
	VnCharset * getVnCharset(int charsetIdx);
};

extern unsigned char SingleByteTables[][TOTAL_VNCHARS];
extern UKWORD DoubleByteTables[][TOTAL_VNCHARS];
extern UnicodeChar UnicodeTable[TOTAL_VNCHARS];
extern UKDWORD VIQRTable[TOTAL_VNCHARS];
extern UKDWORD UnicodeComposite[TOTAL_VNCHARS];
extern UKWORD WinCP1258[TOTAL_VNCHARS];
extern UKWORD WinCP1258Pre[TOTAL_VNCHARS];

extern DllInterface CVnCharsetLib VnCharsetLibObj;
extern VnConvOptions VnConvGlobalOptions;
extern int StdVnNoTone[TOTAL_VNCHARS];
extern int StdVnRootChar[TOTAL_VNCHARS];

DllInterface int genConvert(VnCharset & incs, VnCharset & outcs, ByteInStream & input, ByteOutStream & output);

StdVnChar StdVnToUpper(StdVnChar ch);
StdVnChar StdVnToLower(StdVnChar ch);
StdVnChar StdVnGetRoot(StdVnChar ch);

#endif
