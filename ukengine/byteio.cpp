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
 * @file byteio.cpp
 * @brief Implementation for byteio.cpp.
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

/**
 * @file byteio.cpp
 * @brief Implementation of byte stream abstractions for memory and file I/O.
 *
 * This file provides concrete implementations for in-memory and file-backed
 * stream classes declared in byteio.h. These classes are used by charset
 * conversion code to read/write bytes, words and dwords uniformly.
 */
#include <string.h>
#include "byteio.h"

//------------------------------------------------
/**
 * @brief Construct a memory-backed input stream.
 *
 * @param data Pointer to input memory.
 * @param len Length of input in bytes, or -1 for zero-terminated stream.
 * @param elementSize Element width for zero-terminated mode (1, 2, or 4).
 */
StringBIStream::StringBIStream(UKBYTE *data, int len, int elementSize)
{
	m_data = m_current = data;
	m_len = m_left = len;
    if (len == -1) {
        if (elementSize == 2)
            m_eos = (*(UKWORD *)data == 0);
        else if (elementSize == 4)
            m_eos = (*(UKDWORD *)data == 4);
        else
            m_eos = (*data == 0);
    }
	else
		m_eos = (len <= 0);
	m_didBookmark = 0;
}

//------------------------------------------------
/**
 * @brief Check end-of-stream state.
 * @return Non-zero when end of stream has been reached.
 */
int StringBIStream::eos()
{
	return m_eos;
}

//------------------------------------------------
/**
 * @brief Read next byte from memory stream.
 * @param[out] b Output byte.
 * @return 1 on success, 0 on end of stream.
 */
int StringBIStream::getNext(UKBYTE & b)
{
	if (m_eos)
		return 0;
	b = *m_current++;
	if (m_len == -1) {
		m_eos = (b == 0);
	}
	else {
		m_left--;
		m_eos = (m_left <= 0);
	}
	return 1;
}

//------------------------------------------------
/**
 * @brief Push one byte back to the stream.
 * @param b Byte to push back.
 * @return Always returns 1.
 */
int StringBIStream::unget(UKBYTE b)
{
	if (m_current != m_data) {
		*--m_current = b;
		m_eos = 0;
		if (m_len != -1)
			m_left++;
	}
	return 1;
}

//------------------------------------------------
/**
 * @brief Read next 16-bit word from memory stream.
 * @param[out] w Output word.
 * @return 1 on success, 0 on end of stream.
 */
int StringBIStream::getNextW(UKWORD & w)
{
	if (m_eos) return 0;
	w = *((UKWORD *)m_current);
	m_current += 2;
	if (m_len == -1)
		m_eos = (w == 0);
	else {
		m_left -= 2;
		m_eos = (m_left <= 0);
	}
	return 1;
}

//------------------------------------------------
/**
 * @brief Read next 32-bit dword from memory stream.
 * @param[out] dw Output dword.
 * @return 1 on success, 0 on end of stream.
 */
int StringBIStream::getNextDW(UKDWORD & dw)
{
	if (m_eos) return 0;

	dw = *((UKDWORD *)m_current);
	m_current += 4;
	if (m_len == -1)
		m_eos = (dw == 0);
	else {
		m_left -= 4;
		m_eos = (m_left <= 0);
	}
	return 1;
}

//------------------------------------------------
/**
 * @brief Peek next byte without consuming it.
 * @param[out] b Output byte.
 * @return 1 if available, 0 if end of stream.
 */
int StringBIStream::peekNext(UKBYTE & b)
{
	if (m_eos)
		return 0;
	b = *m_current;
	return 1;
}

//------------------------------------------------
/**
 * @brief Peek next 16-bit word without consuming it.
 * @param[out] w Output word.
 * @return 1 if available, 0 if end of stream.
 */
int StringBIStream::peekNextW(UKWORD & w)
{
	if (m_eos)
		return 0;
	w = *((UKWORD *)m_current);
	return 1;
}

/*
//------------------------------------------------
int StringBIStream::peekNextDW(UKDWORD & dw)
{
	if (m_eos)
		return 0;
	dw = *((UKDWORD *)m_current);
	return 1;
}
*/

//------------------------------------------------
/**
 * @brief Rewind the memory stream to its beginning.
 */
void StringBIStream::reopen()
{
	m_current = m_data;
	m_left = m_len;
	if (m_len == -1)
		m_eos = (m_data == 0);
	else
		m_eos = (m_len <= 0);
	m_didBookmark = 0;
}

//------------------------------------------------
/**
 * @brief Save current position/state as a bookmark.
 * @return Always returns 1.
 */
int StringBIStream::bookmark()
{
	m_didBookmark = 1;
	m_bookmark.current = m_current;
	m_bookmark.data = m_data;
	m_bookmark.eos = m_eos;
	m_bookmark.left = m_left;
	m_bookmark.len = m_len;
	return 1;
}

//------------------------------------------------
/**
 * @brief Restore the stream to the saved bookmark.
 * @return 1 on success, 0 if no bookmark exists.
 */
int StringBIStream::gotoBookmark()
{
	if (!m_didBookmark)
		return 0;
	m_current = m_bookmark.current;
	m_data = m_bookmark.data;
	m_eos = m_bookmark.eos;
	m_left = m_bookmark.left;
	m_len = m_bookmark.len;
	return 1;
}

//------------------------------------------------
/**
 * @brief Close the stream.
 *
 * Memory-backed streams do not own external resources, so this is a no-op.
 * @return Always returns 1.
 */
int StringBIStream::close()
{
	return 1;
}

//////////////////////////////////////////////////
// Class StringBOStream
//////////////////////////////////////////////////

//------------------------------------------------
/**
 * @brief Construct a memory-backed output stream.
 * @param buf Output buffer.
 * @param len Buffer capacity in bytes.
 */
StringBOStream::StringBOStream(UKBYTE *buf, int len)
{
	m_current = m_buf = buf;
	m_len = len;
	m_out = 0;
	m_bad = 0;
}

//------------------------------------------------
/**
 * @brief Write one byte to memory output stream.
 * @param b Byte to write.
 * @return 1 on success, 0 on overflow/error.
 */
int StringBOStream::putB(UKBYTE b)
{
	m_out++;
/*
	if (m_out >= 2147483647) {
		int err;
		err = 1;
	}
*/
	if (m_bad)
		return 0;
/*
	if (m_out < 0) {
		int i;
		i = 1;
	}
*/
	if (m_out <= m_len) {
		*m_current++ = b;
		return 1;
	}
	m_bad = 1;
	return 0;
}

//------------------------------------------------
/**
 * @brief Write one 16-bit word to memory output stream.
 * @param w Word to write.
 * @return 1 on success, 0 on overflow/error.
 */
int StringBOStream::putW(UKWORD w)
{
	m_out += 2;
	if (m_bad)
		return 0;
	if (m_out <= m_len) {
		*((UKWORD *)m_current) = w;
		m_current += 2;
		return 1;
	}
	m_bad = 1;
	return 0;
}

//------------------------------------------------
/**
 * @brief Write a byte sequence to memory output stream.
 * @param s Source bytes.
 * @param size Number of bytes to write, or -1 for C-string mode.
 * @return 1 on success, 0 on overflow/error.
 */
int StringBOStream::puts(const char *s, int size)
{
	if (size == -1) {
		while (*s) {
			m_out++;
			if (m_out <= m_len) 
				*m_current++ = *s;
			s++;
		}
		if (!m_bad && m_out > m_len)
			m_bad = 1;
		return (!m_bad);
	}

	int n;
	if (!m_bad && m_out <= m_len) {
		n = m_len - m_out;
		if (n>size)
			n = size;
		memcpy(m_current, s, n);
		m_current += n;
	}

	m_out += size;
	if (!m_bad && m_out > m_len)
		m_bad = 1;
	return (!m_bad);
}

//------------------------------------------------
/**
 * @brief Reset the output stream to empty state.
 */
void StringBOStream::reopen()
{
	m_current = m_buf;
	m_out = 0;
	m_bad = 0;
}


//------------------------------------------------
/**
 * @brief Check output stream health.
 * @return 1 if no overflow/error happened, 0 otherwise.
 */
int StringBOStream::isOK()
{
	return !m_bad;
}


////////////////////////////////////////////////////
// Class FileBIStream                             //
////////////////////////////////////////////////////

//----------------------------------------------------
/**
 * @brief Construct a file-backed input stream.
 * @param bufSize stdio buffer size to use with setvbuf.
 * @param buf Optional caller-provided stdio buffer.
 */
FileBIStream::FileBIStream(int bufSize, char *buf) 
{
	m_file = NULL;
	m_buf = buf;
	m_bufSize = bufSize;
	m_own = 1;
	m_didBookmark = 0;
	
	m_readAhead = 0;
	m_lastIsAhead = 0;
}

//----------------------------------------------------
/**
 * @brief Destroy input stream and close owned file.
 */
FileBIStream::~FileBIStream()
{
	if (m_own)
		close();
}

//----------------------------------------------------
/**
 * @brief Open file for binary input.
 * @param fileName Path to file.
 * @return 1 on success, 0 on failure.
 */
int FileBIStream::open(const char *fileName)
{
	m_file = fopen(fileName, "rb");
	if (m_file == NULL)
		return 0;
	setvbuf(m_file, m_buf, _IOFBF, m_bufSize);
	m_own = 0;
	m_readAhead = 0;
	m_lastIsAhead = 0;
	return 1;
}

//----------------------------------------------------
/**
 * @brief Close associated file handle.
 * @return Always returns 1.
 */
int FileBIStream::close()
{
	if (m_file != NULL) {
		fclose(m_file);
		m_file = NULL;
	}
	return 1;
}

//----------------------------------------------------
/**
 * @brief Attach an existing FILE* for input.
 * @param f Existing file handle; ownership is not taken.
 */
void FileBIStream::attach(FILE * f)
{
	m_file = f;
	m_own = 0;
	m_readAhead = 0;
	m_lastIsAhead = 0;
}

//----------------------------------------------------
/**
 * @brief Check end-of-stream for file input.
 * @return 1 if EOF reached and no read-ahead byte pending, 0 otherwise.
 */
int FileBIStream::eos()
{
  if (m_readAhead)
    return 0;
  return feof(m_file);
}

//----------------------------------------------------
/**
 * @brief Read next byte from file input stream.
 * @param[out] b Output byte.
 * @return 1 on success, 0 on EOF.
 */
int FileBIStream::getNext(UKBYTE &b)
{
  if (m_readAhead) {
    m_readAhead = 0;
    b = m_readByte;
    m_lastIsAhead = 1;
    return 1;
  }

  m_lastIsAhead = 0;
  b = fgetc(m_file);
  return (!feof(m_file));
}

//----------------------------------------------------
/**
 * @brief Peek next byte from file input stream.
 * @param[out] b Output byte.
 * @return 1 on success, 0 on EOF.
 */
int FileBIStream::peekNext(UKBYTE &b)
{
  if (m_readAhead) {
    b = m_readByte;
    return 1;
  }

  b = fgetc(m_file);
  if (feof(m_file))
    return 0;
  ungetc(b, m_file);
  return 1;
}

//----------------------------------------------------
/**
 * @brief Push one byte back to the file stream.
 * @param b Byte to push back.
 * @return Always returns 1.
 */
int FileBIStream::unget(UKBYTE b)
{
  if (m_lastIsAhead) {
    m_lastIsAhead = 0;
    m_readAhead = 1;
    m_readByte = b;
    return 1;
  }

  ungetc(b, m_file);
  return 1;
}

//----------------------------------------------------
/**
 * @brief Read next 16-bit word from file stream.
 * @param[out] w Output word.
 * @return 1 on success, 0 on EOF/partial read.
 */
int FileBIStream::getNextW(UKWORD &w)
{
  UKBYTE b1, b2;

  if (getNext(b1)) {
    if (getNext(b2)) {
      *((UKBYTE *)&w) = b1;
      *(((UKBYTE *)&w)+1) = b2;
      return 1;
    }
  }
  return 0;
}

//----------------------------------------------------
/**
 * @brief Read next 32-bit dword from file stream.
 * @param[out] dw Output dword.
 * @return 1 on success, 0 on EOF/partial read.
 */
int FileBIStream::getNextDW(UKDWORD &dw)
{
  UKWORD w1, w2;
  if (getNextW(w1)) {
    if (getNextW(w2)) {
      *((UKWORD *)&dw) = w1;
      *(((UKWORD *)&dw)+1) = w2;
      return 1;
    }
  }
  return 0;
      
}
//----------------------------------------------------
/**
 * @brief Peek next 16-bit word from file stream.
 * @param[out] w Output word.
 * @return 1 on success, 0 on EOF/partial read.
 */
int FileBIStream::peekNextW(UKWORD &w)
{
  UKBYTE hi, low;
  if (getNext(low)) {
    if (getNext(hi)) {
      unget(hi);
      w = hi;
      w = (w << 8) + low;
      m_readAhead = 1;
      m_readByte = low;
      m_lastIsAhead = 0;
      return 1;
    }

    m_readAhead = 1;
    m_readByte = low;
    m_lastIsAhead = 0;
    return 0;
  }
  return 0;
}

//----------------------------------------------------
/**
 * @brief Save current file position bookmark.
 * @return Always returns 1.
 */
int FileBIStream::bookmark()
{
	m_didBookmark = 1;
	m_bookmark.pos = ftell(m_file);
	return 1;
}


//----------------------------------------------------
/**
 * @brief Seek back to previously saved bookmark.
 * @return 1 on success, 0 if no bookmark exists.
 */
int FileBIStream::gotoBookmark()
{
	if (!m_didBookmark)
		return 0;
	fseek(m_file, m_bookmark.pos, SEEK_SET);
	return 1;
}

////////////////////////////////////////////////////
// Class FileBOStream                             //
////////////////////////////////////////////////////
//----------------------------------------------------
/**
 * @brief Construct a file-backed output stream.
 * @param bufSize stdio buffer size for setvbuf.
 * @param buf Optional caller-provided stdio buffer.
 */
FileBOStream::FileBOStream(int bufSize, char *buf) 
{
	m_file = NULL;
	m_buf = buf;
	m_bufSize = bufSize;
	m_own = 1;
	m_bad = 1;
}

//----------------------------------------------------
/**
 * @brief Destroy output stream and close owned file.
 */
FileBOStream::~FileBOStream()
{
	if (m_own)
		close();
}

//----------------------------------------------------
/**
 * @brief Open file for binary output.
 * @param fileName Path to output file.
 * @return 1 on success, 0 on failure.
 */
int FileBOStream::open(const char *fileName)
{
	m_file = fopen(fileName, "wb");
	if (m_file == NULL)
		return 0;
	m_bad = 0;
	setvbuf(m_file, m_buf, _IOFBF, m_bufSize);
	m_own = 1;
	return 1;
}

//----------------------------------------------------
/**
 * @brief Attach existing FILE* for output.
 * @param f Existing handle; ownership is not taken.
 */
void FileBOStream::attach(FILE * f)
{
	m_file = f;
	m_own = 0;
	m_bad = 0;
}

//----------------------------------------------------
/**
 * @brief Close output file handle.
 * @return Always returns 1.
 */
int FileBOStream::close()
{
	if (m_file != NULL) {
		fclose(m_file);
		m_file = NULL;
	}
	return 1;
}

//----------------------------------------------------
/**
 * @brief Write one byte to file output stream.
 * @param b Byte to write.
 * @return 1 on success, 0 on error.
 */
int FileBOStream::putB(UKBYTE b)
{
	if (m_bad)
		return 0;
	m_bad = (fputc(b, m_file) == EOF);
	return (!m_bad);
}

//----------------------------------------------------
/**
 * @brief Write one 16-bit word to file output stream.
 * @param w Word to write.
 * @return 1 on success, 0 on error.
 */
int FileBOStream::putW(UKWORD w)
{
	if (m_bad)
		return 0;
	//	m_bad = (fputwc(w, m_file) == WEOF); 
	m_bad = (fputc((UKBYTE)w, m_file) == EOF);
	if (m_bad)
	  return 0;
	m_bad = (fputc((UKBYTE)(w >> 8), m_file) == EOF);
	return (!m_bad);
}

//----------------------------------------------------
/**
 * @brief Write byte sequence to file output stream.
 * @param s Source bytes.
 * @param size Number of bytes to write, or -1 for C-string mode.
 * @return 1 on success, 0 on error.
 */
int FileBOStream::puts(const char *s, int size)
{
	if (m_bad)
		return 0;
	if (size == -1) {
		m_bad = (fputs(s, m_file) == EOF);
		return (!m_bad);
	}
	int out = fwrite(s, 1, size, m_file);
	m_bad = (out != size);
	return (!m_bad);
}

//----------------------------------------------------
/**
 * @brief Check output stream error state.
 * @return 1 if stream is healthy, 0 if an error occurred.
 */
int FileBOStream::isOK()
{
	return !m_bad;
}
