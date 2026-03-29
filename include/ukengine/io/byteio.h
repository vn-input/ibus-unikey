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
 * @brief Virtual base destructor for byte streams.
 */
  virtual ~ByteStream(){};
};

//----------------------------------------------------
class ByteInStream: public ByteStream
{
public:
/**
 * @brief Read the next byte from the stream.
 *
 * Consumes one byte of input and stores it in the provided reference.
 *
 * @param b Output parameter receiving the byte value.
 * @return nonzero on success, zero on end-of-stream or error.
 */
	virtual int getNext(UKBYTE &b) = 0;
/**
 * @brief Peek at the next byte without advancing the stream position.
 *
 * The byte is copied into @p b but remains available for subsequent reads.
 *
 * @param b Output parameter receiving the peeked byte.
 * @return nonzero if a byte was available, zero on end-of-stream.
 */
	virtual int peekNext(UKBYTE &b) = 0;
/**
 * @brief Push a byte back onto the stream.
 *
 * Allows one byte to be returned to the input sequence so that a subsequent
 * `getNext` will return it again.
 *
 * @param b Byte value to un-read.
 * @return nonzero on success, zero if the stream does not support unget.
 */
	virtual int unget(UKBYTE b) = 0;

/**
 * @brief Read the next 16-bit word from the stream.
 *
 * Interprets two bytes from the stream as a `UKWORD` and advances position
 * accordingly.
 *
 * @param w Output parameter for the word value.
 * @return nonzero on success.
 */
	virtual int getNextW(UKWORD &w) = 0;
/**
 * @brief Peek at the next 16-bit word without consuming it.
 *
 * Similar to `peekNext` but operates on two-byte words.
 *
 * @param w Receives the word value.
 * @return nonzero if available.
 */
	virtual int peekNextW(UKWORD &w) = 0;

/**
 * @brief Read the next 32-bit double-word from the stream.
 *
 * Consumes four bytes and assembles them into a `UKDWORD`.
 *
 * @param dw Output parameter receiving the value.
 * @return nonzero on success.
 */
	virtual int getNextDW(UKDWORD &dw) = 0;

/**
 * @brief Save current stream position for later rewind.
 *
 * @return Non-zero on success, zero if bookmarking is unsupported.
 */
	virtual int bookmark() //no support for bookmark by default
	{
		return 0;
	}

/**
 * @brief Restore the stream to the last bookmarked position.
 *
 * @return Non-zero on success, zero if no bookmark exists.
 */
	virtual int gotoBookmark()
	{
		return 0;
	}

/**
 * @brief Check for end-of-stream condition.
 *
 * @return nonzero if the stream has reached EOF, zero otherwise.
 */
	virtual int eos() = 0; //end of stream
/**
 * @brief Close the stream and release any associated resources.
 *
 * After calling this the stream object should not be used again.
 *
 * @return nonzero on successful close.
 */
	virtual int close() = 0;
};

//----------------------------------------------------
class ByteOutStream: public ByteStream
{
public:
/**
 * @brief Write a single byte to the output stream.
 *
 * @param b Byte value to write.
 * @return nonzero on success.
 */
	virtual int putB(UKBYTE b) = 0;
/**
 * @brief Write a 16-bit word to the output stream.
 *
 * @param w Word value to write.
 * @return nonzero on success.
 */
	virtual int putW(UKWORD w) = 0;
/**
 * @brief Write a sequence of bytes (string) to the stream.
 *
 * If `size` is -1 the string is assumed NUL-terminated; otherwise exactly
 * `size` bytes are written.
 *
 * @param s    Pointer to data.
 * @param size Number of bytes, or -1 for auto-length.
 * @return nonzero on success.
 */
	virtual int puts(const char *s, int size = -1) = 0; // write an 8-bit string
/**
 * @brief Query the health/state of the stream.
 *
 * @return nonzero if the stream is currently in a good state for I/O.
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
	 * @brief Read the next byte from the in-memory input buffer.
	 *
	 * @param b Output parameter receiving the byte value.
	 * @return Non-zero on success, zero on end-of-stream.
 */
	virtual int getNext(UKBYTE &b);
/**
	 * @brief Peek at the next byte without consuming it.
	 *
	 * @param b Output parameter receiving the peeked byte.
	 * @return Non-zero if data is available.
 */
	virtual int peekNext(UKBYTE &b);
/**
	 * @brief Push a byte back into the in-memory stream.
	 *
	 * @param b Byte value to return to the stream.
	 * @return Non-zero on success.
 */
	virtual int unget(UKBYTE b);

/**
	 * @brief Read the next 16-bit word from the buffer.
	 *
	 * @param w Output parameter for the word value.
	 * @return Non-zero on success.
 */
	virtual int getNextW(UKWORD &w);
/**
	 * @brief Peek at the next 16-bit word without consuming it.
	 *
	 * @param w Output parameter for the word value.
	 * @return Non-zero if data is available.
 */
	virtual int peekNextW(UKWORD &w);

/**
	 * @brief Read the next 32-bit value from the buffer.
	 *
	 * @param dw Output parameter for the double-word value.
	 * @return Non-zero on success.
 */
	virtual int getNextDW(UKDWORD &dw);

/**
	 * @brief Report whether the in-memory stream has reached end-of-input.
	 *
	 * @return Non-zero when no unread bytes remain.
 */
    virtual int eos(); //end of stream
/**
	 * @brief Close the in-memory stream.
	 *
	 * Resets the stream state; no external resource release is required.
	 *
	 * @return Non-zero on success.
 */
	virtual int close();

/**
	 * @brief Save the current cursor position for later restoration.
	 *
	 * @return Non-zero on success.
 */
	virtual int bookmark();
/**
	 * @brief Restore the stream cursor to the last bookmarked position.
	 *
	 * @return Non-zero on success.
 */
	virtual int gotoBookmark();

/**
	 * @brief Reset the stream cursor to the beginning of the buffer.
 */
	void reopen();
/**
 * @brief Get number of bytes remaining in the stream.
 *
 * @return Number of bytes still available for reading.
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
	 * @brief Open a file for buffered input.
	 *
	 * @param fileName Path to the file to open.
	 * @return Non-zero on success.
	 */
	int open(const char *fileName);
	/**
	 * @brief Attach the stream to an already-open input file.
	 *
	 * @param f Pointer to an existing `FILE` object.
 */
	void attach(FILE *f);
/**
	 * @brief Close the file-backed input stream.
	 *
	 * @return Non-zero on success.
 */
	virtual int close();

	/**
	 * @brief Read the next byte from the buffered file stream.
	 *
	 * @param b Output parameter receiving the byte value.
	 * @return Non-zero on success, zero on end-of-file or error.
 */
	virtual int getNext(UKBYTE &b);
	/**
	 * @brief Peek at the next byte without consuming it.
	 *
	 * @param b Output parameter receiving the peeked byte.
	 * @return Non-zero if a byte is available.
 */
	virtual int peekNext(UKBYTE &b);
	/**
	 * @brief Push one byte back so it can be read again.
	 *
	 * @param b Byte to return to the stream.
	 * @return Non-zero on success.
 */
	virtual int unget(UKBYTE b);

	/**
	 * @brief Read the next 16-bit word from the file stream.
	 *
	 * @param w Output parameter receiving the word value.
	 * @return Non-zero on success.
 */
	virtual int getNextW(UKWORD &w);
	/**
	 * @brief Peek at the next 16-bit word without advancing the cursor.
	 *
	 * @param w Output parameter receiving the word value.
	 * @return Non-zero if a word is available.
 */
	virtual int peekNextW(UKWORD &w);

	/**
	 * @brief Read the next 32-bit value from the file stream.
	 *
	 * @param dw Output parameter receiving the double-word value.
	 * @return Non-zero on success.
 */
    virtual int getNextDW(UKDWORD &dw);

	/**
	 * @brief Report whether the input stream is at end-of-file.
	 *
	 * @return Non-zero when no more input remains.
 */
    virtual int eos(); //end of stream

	/**
	 * @brief Save the current file position for later restoration.
	 *
	 * @return Non-zero on success.
 */
	virtual int bookmark();
	/**
	 * @brief Restore the file position recorded by `bookmark()`.
	 *
	 * @return Non-zero on success.
 */
	virtual int gotoBookmark();

	/**
	 * @brief Destroy the file-backed input stream.
	 *
	 * Releases any owned buffers and file handles.
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
	 * @brief Write one byte into the in-memory output buffer.
	 *
	 * @param b Byte value to write.
	 * @return Non-zero on success.
 */
	virtual int putB(UKBYTE b);
/**
	 * @brief Write one 16-bit word into the in-memory output buffer.
	 *
	 * @param w Word value to write.
	 * @return Non-zero on success.
 */
	virtual int putW(UKWORD w);
/**
	 * @brief Write a byte sequence into the in-memory output buffer.
	 *
	 * @param s Pointer to the source bytes.
	 * @param size Number of bytes to write, or `-1` for NUL-terminated input.
	 * @return Non-zero on success.
 */
	virtual int puts(const char *s, int size = -1);
/**
	 * @brief Report whether the output buffer remains in a valid state.
	 *
	 * @return Non-zero if no write overflow or error has occurred.
 */
	virtual int isOK(); // get current stream state

/**
 * @brief Close the output stream, flushing buffers if necessary.
 *
 * @return 1 on success, zero on failure.
 */
	virtual int close()	
	{
		return 1;
	};

/**
	 * @brief Reset the in-memory output stream to its initial empty state.
 */
	void reopen();
/**
 * @brief Return number of bytes written so far to the output buffer.
 *
 * @return Count of bytes produced by the stream.
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
	 * @brief Open a file for buffered output.
	 *
	 * @param fileName Path to the file to open.
	 * @return Non-zero on success.
	 */
	int open(const char *fileName);
	/**
	 * @brief Attach the stream to an already-open output file.
	 *
	 * @param f Pointer to a `FILE` object to attach to the stream.
 */
	void attach(FILE *);
/**
	 * @brief Close the file-backed output stream.
	 *
	 * @return Non-zero on success.
 */
	virtual int close();

	/**
	 * @brief Write one byte to the file-backed output stream.
	 *
	 * @param b Byte value to write.
	 * @return Non-zero on success.
	 */
	virtual int putB(UKBYTE b);
/**
	 * @brief Write one 16-bit word to the file-backed output stream.
	 *
	 * @param w Word value to write.
	 * @return Non-zero on success.
	 */
	virtual int putW(UKWORD w);
/**
	 * @brief Write a byte sequence to the file-backed output stream.
	 *
	 * @param s Pointer to the source bytes.
	 * @param size Number of bytes to write, or `-1` for NUL-terminated input.
	 * @return Non-zero on success.
	 */
	virtual int puts(const char *s, int size = -1);
/**
	 * @brief Report whether the output stream is still usable.
	 *
	 * @return Non-zero if no stream error has been recorded.
	 */
	virtual int isOK(); // get current stream state
/**
	 * @brief Destroy the file-backed output stream.
	 *
	 * Releases owned buffers and closes the file if owned by the stream.
	 */
	virtual ~FileBOStream();
};


#endif
