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
  virtual ~ByteStream(){};
};

//----------------------------------------------------
class ByteInStream: public ByteStream
{
public:
	virtual int getNext(UKBYTE &b) = 0;
	virtual int peekNext(UKBYTE &b) = 0;
	virtual int unget(UKBYTE b) = 0;

	virtual int getNextW(UKWORD &w) = 0;
	virtual int peekNextW(UKWORD &w) = 0;

	virtual int getNextDW(UKDWORD &dw) = 0;

	virtual int bookmark() //no support for bookmark by default
	{
		return 0;
	}

	virtual int gotoBookmark()
	{
		return 0;
	}

	virtual int eos() = 0; //end of stream
	virtual int close() = 0;
};

//----------------------------------------------------
class ByteOutStream: public ByteStream
{
public:
	virtual int putB(UKBYTE b) = 0;
	virtual int putW(UKWORD w) = 0;
	virtual int puts(const char *s, int size = -1) = 0; // write an 8-bit string
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
	virtual int getNext(UKBYTE &b);
	virtual int peekNext(UKBYTE &b);
	virtual int unget(UKBYTE b);

	virtual int getNextW(UKWORD &w);
	virtual int peekNextW(UKWORD &w);

	virtual int getNextDW(UKDWORD &dw);

    virtual int eos(); //end of stream
	virtual int close();

	virtual int bookmark();
	virtual int gotoBookmark();

	void reopen();
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

	int open(const char *fileName);
	void attach(FILE *f);
	virtual int close();

	virtual int getNext(UKBYTE &b);
	virtual int peekNext(UKBYTE &b);
	virtual int unget(UKBYTE b);

	virtual int getNextW(UKWORD &w);
	virtual int peekNextW(UKWORD &w);

    virtual int getNextDW(UKDWORD &dw);

    virtual int eos(); //end of stream

	virtual int bookmark();
	virtual int gotoBookmark();

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
	virtual int putB(UKBYTE b);
	virtual int putW(UKWORD w);
	virtual int puts(const char *s, int size = -1);
	virtual int isOK(); // get current stream state

	virtual int close()	
	{
		return 1;
	};

	void reopen();
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

	int open(const char *fileName);
	void attach(FILE *);
	virtual int close();

	virtual int putB(UKBYTE b);
	virtual int putW(UKWORD w);
	virtual int puts(const char *s, int size = -1);
	virtual int isOK(); // get current stream state
	virtual ~FileBOStream();
};


#endif
