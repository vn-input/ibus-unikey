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

#ifndef __PATTERN_H
#define __PATTERN_H

#if defined(_WIN32)
    #if defined(UNIKEYHOOK)
        #define DllInterface   __declspec( dllexport )
    #else
        #define DllInterface   __declspec( dllimport )
    #endif
#else
    #define DllInterface //not used
#endif

#define MAX_PATTERN_LEN 40

class DllInterface PatternState
{
public:
	char *m_pattern;
	int m_border[MAX_PATTERN_LEN+1];
	int m_pos;
	int m_found;
	void init(char *pattern);
	void reset();
	int foundAtNextChar(char ch); //get next input char, returns 1 if pattern is found.
};

class DllInterface PatternList
{
public:
	PatternState *m_patterns;
	int m_count;
	void init(char **patterns, int count);
	int foundAtNextChar(char ch); 
	void reset();

	PatternList() {
		m_count = 0;
		m_patterns = 0;
	}

	~PatternList()
	{
		if (m_patterns)
			delete [] m_patterns;
	}
};


#endif
