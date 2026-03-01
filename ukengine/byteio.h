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
 * @file byteio.h
 * @brief Public declarations for byteio.h.
 */
// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#ifndef BYTE_IO_STREAM_H
#define BYTE_IO_STREAM_H

/**
 * @file byteio.h
 * @brief Byte stream abstractions and memory/file-backed implementations.
 */


//#include "vnconv.h"
#include <stdio.h>

typedef unsigned char UKBYTE;
typedef unsigned short UKWORD;
typedef unsigned int UKDWORD;

//----------------------------------------------------
class ByteStream {
 public:
/**
 * @brief ~ByteStream.
 *
 * @return Return value.
 */
  virtual ~ByteStream(){};
};

//----------------------------------------------------
class ByteInStream: public ByteStream
{
public:
/**
 * @brief getNext.
 *
 * @param b Description of b.
 */
	virtual int getNext(UKBYTE &b) = 0;
/**
 * @brief peekNext.
 *
 * @param b Description of b.
 */
	virtual int peekNext(UKBYTE &b) = 0;
/**
 * @brief unget.
 *
 * @param b Description of b.
 */
	virtual int unget(UKBYTE b) = 0;

/**
 * @brief getNextW.
 *
 * @param w Description of w.
 */
	virtual int getNextW(UKWORD &w) = 0;
/**
 * @brief peekNextW.
 *
 * @param w Description of w.
 */
	virtual int peekNextW(UKWORD &w) = 0;

/**
 * @brief getNextDW.
 *
 * @param dw Description of dw.
 */
	virtual int getNextDW(UKDWORD &dw) = 0;

/**
 * @brief bookmark.
 *
 * @return Return value.
 */
	virtual int bookmark() //no support for bookmark by default
	{
		return 0;
	}

/**
 * @brief gotoBookmark.
 *
 * @return Return value.
 */
	virtual int gotoBookmark()
	{
		return 0;
	}

/**
 * @brief eos.
 *
 */
	virtual int eos() = 0; //end of stream
/**
 * @brief close.
 *
 */
	virtual int close() = 0;
};

//----------------------------------------------------
class ByteOutStream: public ByteStream
{
public:
/**
 * @brief putB.
 *
 * @param b Description of b.
 */
	virtual int putB(UKBYTE b) = 0;
/**
 * @brief putW.
 *
 * @param w Description of w.
 */
	virtual int putW(UKWORD w) = 0;
/**
 * @brief puts.
 *
 * @param s Description of s.
 * @param size Description of size.
 */
	virtual int puts(const char *s, int size = -1) = 0; // write an 8-bit string
/**
 * @brief isOK.
 *
 */
	virtual int isOK() = 0;// get current stream state
};

//----------------------------------------------------
class StringBIStream : public ByteInStream
{
protected:
	int m_eos;
	UKBYTE *m_data, *m_current;
	int m_len, m_left;

	struct {
		int eos;
		UKBYTE *data, *current;
		int len, left;
	} m_bookmark;

	int m_didBookmark;
		
public:
	StringBIStream(UKBYTE *data, int len, int elementSize = 1);
/**
 * @brief getNext.
 *
 * @param b Description of b.
 */
	virtual int getNext(UKBYTE &b);
/**
 * @brief peekNext.
 *
 * @param b Description of b.
 */
	virtual int peekNext(UKBYTE &b);
/**
 * @brief unget.
 *
 * @param b Description of b.
 */
	virtual int unget(UKBYTE b);

/**
 * @brief getNextW.
 *
 * @param w Description of w.
 */
	virtual int getNextW(UKWORD &w);
/**
 * @brief peekNextW.
 *
 * @param w Description of w.
 */
	virtual int peekNextW(UKWORD &w);

/**
 * @brief getNextDW.
 *
 * @param dw Description of dw.
 */
	virtual int getNextDW(UKDWORD &dw);

/**
 * @brief eos.
 *
 */
    virtual int eos(); //end of stream
/**
 * @brief close.
 *
 */
	virtual int close();

/**
 * @brief bookmark.
 *
 */
	virtual int bookmark();
/**
 * @brief gotoBookmark.
 *
 */
	virtual int gotoBookmark();

/**
 * @brief reopen.
 *
 */
	void reopen();
/**
 * @brief left.
 *
 * @return Return value.
 */
	int left() {
		return m_left;
	}
};

//----------------------------------------------------
class FileBIStream : public ByteInStream
{
protected:
	FILE *m_file;
	int m_bufSize;
	char *m_buf;
	int m_own;
	int m_didBookmark;

	struct {
		long pos;
	} m_bookmark;

	//some systems don't have wide char IO functions
	//we have to use this variables to implement that
	UKBYTE m_readByte; 
	int m_readAhead;
	int m_lastIsAhead;

public:

	FileBIStream(int bufsize = 8192, char *buf = NULL);
//	FileBIStream(char *fileName, int bufsize = 8192, void *buf = NULL);

/**
 * @brief open.
 *
 * @param fileName Description of fileName.
 */
	int open(const char *fileName);
/**
 * @brief attach.
 *
 * @param f Description of f.
 */
	void attach(FILE *f);
/**
 * @brief close.
 *
 */
	virtual int close();

/**
 * @brief getNext.
 *
 * @param b Description of b.
 */
	virtual int getNext(UKBYTE &b);
/**
 * @brief peekNext.
 *
 * @param b Description of b.
 */
	virtual int peekNext(UKBYTE &b);
/**
 * @brief unget.
 *
 * @param b Description of b.
 */
	virtual int unget(UKBYTE b);

/**
 * @brief getNextW.
 *
 * @param w Description of w.
 */
	virtual int getNextW(UKWORD &w);
/**
 * @brief peekNextW.
 *
 * @param w Description of w.
 */
	virtual int peekNextW(UKWORD &w);

/**
 * @brief getNextDW.
 *
 * @param dw Description of dw.
 */
    virtual int getNextDW(UKDWORD &dw);

/**
 * @brief eos.
 *
 */
    virtual int eos(); //end of stream

/**
 * @brief bookmark.
 *
 */
	virtual int bookmark();
/**
 * @brief gotoBookmark.
 *
 */
	virtual int gotoBookmark();

/**
 * @brief ~FileBIStream.
 *
 */
	virtual ~FileBIStream();
};


//----------------------------------------------------
class StringBOStream : public ByteOutStream
{
protected:
	UKBYTE *m_buf, *m_current;
	int m_out;
	int m_len;
	int m_bad;
public:
	StringBOStream(UKBYTE *buf, int len);
/**
 * @brief putB.
 *
 * @param b Description of b.
 */
	virtual int putB(UKBYTE b);
/**
 * @brief putW.
 *
 * @param w Description of w.
 */
	virtual int putW(UKWORD w);
/**
 * @brief puts.
 *
 * @param s Description of s.
 * @param size Description of size.
 */
	virtual int puts(const char *s, int size = -1);
/**
 * @brief isOK.
 *
 */
	virtual int isOK(); // get current stream state

/**
 * @brief close.
 *
 * @return Return value.
 */
	virtual int close()	
	{
		return 1;
	};

/**
 * @brief reopen.
 *
 */
	void reopen();
/**
 * @brief getOutBytes.
 *
 * @return Return value.
 */
	int getOutBytes() {
		return m_out;
	}
};

//----------------------------------------------------
class FileBOStream : public ByteOutStream
{
protected:
	FILE *m_file;
	int m_bufSize;
	char *m_buf;
	int m_own;
	int m_bad;

public:
	FileBOStream(int bufsize = 8192, char *buf = NULL);
//	FileBOStream(char *fileName, int bufsize = 8192, void *buf = NULL);

/**
 * @brief open.
 *
 * @param fileName Description of fileName.
 */
	int open(const char *fileName);
/**
 * @brief attach.
 *
 * @param FILE Description of FILE.
 */
	void attach(FILE *);
/**
 * @brief close.
 *
 */
	virtual int close();

/**
 * @brief putB.
 *
 * @param b Description of b.
 */
	virtual int putB(UKBYTE b);
/**
 * @brief putW.
 *
 * @param w Description of w.
 */
	virtual int putW(UKWORD w);
/**
 * @brief puts.
 *
 * @param s Description of s.
 * @param size Description of size.
 */
	virtual int puts(const char *s, int size = -1);
/**
 * @brief isOK.
 *
 */
	virtual int isOK(); // get current stream state
/**
 * @brief ~FileBOStream.
 *
 */
	virtual ~FileBOStream();
};


#endif
