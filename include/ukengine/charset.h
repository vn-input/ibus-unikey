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
	/**
	 * @brief Report the encoded width, in bytes, of one charset element.
	 *
	 * Variable-width encodings may still use this as their natural unit size for
	 * stream buffering decisions.
	 *
	 * @return Number of bytes used by one encoded element.
	 */
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
	/**
	 * @brief Construct a single-byte charset from a standard-to-encoded mapping table.
	 *
	 * @param vnChars Table mapping each standard Vietnamese character to its encoded byte.
	 */
	SingleByteCharset(unsigned char * vnChars);
	/**
	 * @brief Decode one character from a single-byte input stream.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character as a single output byte.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class VnInternalCharset
 * @brief Identity charset used internally by the engine (no conversion).
 */
class VnInternalCharset: public VnCharset {
public:
	/**
	 * @brief Construct the identity charset used by internal engine buffers.
	 */
  VnInternalCharset() {};
	/**
	 * @brief Read one internal standard character from the input stream.
	 */
  virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Write one internal standard character to the output stream unchanged.
	 */
  virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
	/**
	 * @brief Return the byte width of one internal standard character.
	 */
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
	/**
	 * @brief Construct a fixed-width Unicode charset from a standard mapping table.
	 *
	 * @param vnChars Table mapping each standard Vietnamese character to Unicode.
	 */
	UnicodeCharset(UnicodeChar *vnChars);
	/**
	 * @brief Decode one Unicode code unit sequence from the input stream.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character as a Unicode code unit.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
	/**
	 * @brief Return the encoded width of one Unicode code unit.
	 */
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
	UKWORD * m_toDoubleChar;          /**< reverse mapping from standard code to encoded word */
public:
	/**
	 * @brief Construct a double-byte charset from an encoding table.
	 *
	 * @param vnChars Table mapping standard Vietnamese characters to 16-bit encoded values.
	 */
	DoubleByteCharset(UKWORD *vnChars);
	/**
	 * @brief Decode one character from a potentially double-byte input stream.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character using the target double-byte charset.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class UnicodeUTF8Charset
 * @brief Unicode charset that outputs UTF-8 encoded bytes.
 *
 * This adapter reuses the Unicode lookup tables from `UnicodeCharset` but
 * serializes characters as UTF-8 instead of UTF-16 code units.
 */
class UnicodeUTF8Charset: public UnicodeCharset
{
public:
	/**
	 * @brief Construct a UTF-8 charset wrapper over the shared Unicode table.
	 *
	 * @param vnChars Table mapping standard Vietnamese characters to Unicode.
	 */
	UnicodeUTF8Charset(UnicodeChar *vnChars) : UnicodeCharset(vnChars)	{}

	/**
	 * @brief Decode one UTF-8 sequence from the input stream.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character as UTF-8.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class UnicodeRefCharset
 * @brief Unicode charset variant that references external storage for data.
 *
 * This format reads and writes numeric character references such as `&#1234;`.
 */
class UnicodeRefCharset: public UnicodeCharset
{
public:
	/**
	 * @brief Construct a numeric-reference charset wrapper over the Unicode table.
	 *
	 * @param vnChars Table mapping standard Vietnamese characters to Unicode.
	 */
	UnicodeRefCharset(UnicodeChar *vnChars) : UnicodeCharset(vnChars)	{}

	/**
	 * @brief Decode one numeric character reference or plain byte sequence.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character as a numeric character reference when needed.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class UnicodeHexCharset
 * @brief Numeric-reference charset that writes hexadecimal references.
 *
 * Produces forms such as `&#x1EA1;` for characters that cannot be emitted as
 * plain ASCII bytes.
 */
class UnicodeHexCharset: public UnicodeRefCharset
{
public:
	/**
	 * @brief Construct a hexadecimal numeric-reference charset.
	 *
	 * @param vnChars Table mapping standard Vietnamese characters to Unicode.
	 */
	UnicodeHexCharset(UnicodeChar *vnChars) : UnicodeRefCharset(vnChars) {}
	/**
	 * @brief Encode one standard character as plain ASCII or a hexadecimal reference.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class UnicodeCStringCharset
 * @brief Unicode charset that uses C-style `\xNNNN` escape sequences.
 *
 * This encoding is useful when Vietnamese text must be embedded inside C/C++
 * string literals or similar escaped text formats.
 */
class UnicodeCStringCharset: public UnicodeCharset
{
protected:
	int m_prevIsHex;
public:
	/**
	 * @brief Construct a C-string escape charset.
	 *
	 * @param vnChars Table mapping standard Vietnamese characters to Unicode.
	 */
	UnicodeCStringCharset(UnicodeChar *vnChars) : UnicodeCharset(vnChars) {}
	/**
	 * @brief Decode one plain byte or `\x` escape sequence from the input stream.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character using plain ASCII or `\x` escapes.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
	/**
	 * @brief Reset per-stream escape state before starting a new input sequence.
	 */
	virtual void startInput();
};

//--------------------------------------------------
/**
 * @class WinCP1258Charset
 * @brief Converter for Windows code page 1258 with combining-mark handling.
 *
 * CP1258 mixes single-byte Latin characters with combining diacritics, so this
 * converter needs extra lookup state beyond the simpler single-byte classes.
 */
class WinCP1258Charset: public VnCharset {
protected:
	UKWORD m_stdMap[256];
	UKDWORD m_vnChars[TOTAL_VNCHARS*2];
	UKWORD *m_toDoubleChar;
	int m_totalChars;

public:
	/**
	 * @brief Construct a CP1258 converter from composite and precomposed tables.
	 *
	 * @param compositeChars Table describing decomposed CP1258 forms.
	 * @param precomposedChars Table describing precomposed CP1258 forms.
	 */
	WinCP1258Charset(UKWORD *compositeChars, UKWORD *precomposedChars);
	/**
	 * @brief Decode one CP1258 character sequence from the input stream.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character into CP1258 bytes.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @struct UniCompCharInfo
 * @brief Lookup record used by decomposed/precomposed Unicode converters.
 */
struct UniCompCharInfo {
	UKDWORD	compChar;
	int stdIndex;
};

/**
 * @class UnicodeCompCharset
 * @brief Unicode charset that recognizes precomposed and decomposed sequences.
 *
 * The converter can map a base code point plus combining mark back to one
 * standard Vietnamese character index.
 */
class UnicodeCompCharset: public VnCharset {
protected:
	UniCompCharInfo m_info[TOTAL_VNCHARS*2];
	UKDWORD *m_uniCompChars;
	int m_totalChars;
public:
	/**
	 * @brief Construct a composite-Unicode charset from base and composed tables.
	 *
	 * @param uniChars Base Unicode mapping table.
	 * @param uniCompChars Precomposed Unicode mapping table.
	 */
	UnicodeCompCharset(UnicodeChar *uniChars, UKDWORD *uniCompChars);
	/**
	 * @brief Decode one precomposed or decomposed Unicode character sequence.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character using composed Unicode output.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
	/**
	 * @brief Return the natural code-unit width used by this Unicode variant.
	 */
    virtual int elementSize();
};

//--------------------------------------------------
/**
 * @class VIQRCharset
 * @brief Converter for the VIQR ASCII transcription format.
 *
 * VIQR represents Vietnamese marks using punctuation, so this class tracks
 * escaping and word-boundary state while decoding and encoding.
 */
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
	/**
	 * @brief Construct a VIQR converter from the standard VIQR mapping table.
	 *
	 * @param vnChars Table describing VIQR representations for standard characters.
	 */
	VIQRCharset(UKDWORD *vnChars);
	/**
	 * @brief Reset VIQR input-side state before decoding a new stream.
	 */
	virtual void startInput();
	/**
	 * @brief Reset VIQR output-side escaping state before encoding a new stream.
	 */
	virtual void startOutput();
	/**
	 * @brief Decode one VIQR sequence from the input stream.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character as VIQR text.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
/**
 * @class UTF8VIQRCharset
 * @brief Hybrid converter that combines UTF-8 data with VIQR escaping rules.
 *
 * This is used when UTF-8 text must preserve VIQR-style escape behaviour for
 * ambiguous punctuation sequences.
 */
class UTF8VIQRCharset: public VnCharset {

protected:
	VIQRCharset *m_pViqr;
	UnicodeUTF8Charset *m_pUtf;

public:
	/**
	 * @brief Construct the hybrid UTF-8/VIQR converter.
	 *
	 * @param pUtf UTF-8 converter used for Unicode byte encoding.
	 * @param pViqr VIQR converter used for escape/state handling.
	 */
	UTF8VIQRCharset(UnicodeUTF8Charset *pUtf, VIQRCharset *pViqr);
	/**
	 * @brief Reset input-side state before decoding a new stream.
	 */
	virtual void startInput();
	/**
	 * @brief Reset output-side state before encoding a new stream.
	 */
	virtual void startOutput();
	/**
	 * @brief Decode one character from a UTF-8 stream with VIQR-aware handling.
	 */
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	/**
	 * @brief Encode one standard character using UTF-8 plus VIQR escape rules.
	 */
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};


//--------------------------------------------------
/**
 * @class CVnCharsetLib
 * @brief Registry and factory for all charset converters used by vnconv.
 *
 * Owns converter instances, shared pattern tables, and global conversion
 * options used throughout the encoding library.
 */
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
	/**
	 * @brief Construct and initialize the charset registry.
	 */
	CVnCharsetLib();
	/**
	 * @brief Destroy all registered charset converters and shared resources.
	 */
	~CVnCharsetLib();
	/**
	 * @brief Return the converter instance associated with a charset id.
	 *
	 * @param charsetIdx Charset identifier from the conversion enums.
	 * @return Pointer to the matching converter, or `nullptr` if unsupported.
	 */
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

/**
 * @brief Convert a full byte stream from one charset implementation to another.
 *
 * @param incs Input charset decoder.
 * @param outcs Output charset encoder.
 * @param input Source byte stream.
 * @param output Destination byte stream.
 * @return Zero on success, or a conversion error code.
 */
DllInterface int genConvert(VnCharset & incs, VnCharset & outcs, ByteInStream & input, ByteOutStream & output);

/**
 * @brief Convert one standard Vietnamese character to uppercase.
 */
StdVnChar StdVnToUpper(StdVnChar ch);
/**
 * @brief Convert one standard Vietnamese character to lowercase.
 */
StdVnChar StdVnToLower(StdVnChar ch);
/**
 * @brief Strip tone and decoration from one standard Vietnamese character.
 */
StdVnChar StdVnGetRoot(StdVnChar ch);

#endif
