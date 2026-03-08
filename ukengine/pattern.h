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

/**
 * @file pattern.h
 * @brief Pattern matching helpers used by conversion routines.
 */

#if defined(_WIN32)
    #if defined(UNIKEYHOOK)
        #define DllInterface   __declspec( dllexport )
    #else
        #define DllInterface   __declspec( dllimport )
    #endif
#else
    #define DllInterface //not used
#endif

/**
 * @brief Maximum length of a search pattern.
 */
#define MAX_PATTERN_LEN 40

/**
 * @class PatternState
 * @brief Maintains state for a single search pattern using KMP algorithm.
 */
class DllInterface PatternState
{
public:
	char *m_pattern; /**< NUL‑terminated search string */
	int m_border[MAX_PATTERN_LEN+1]; /**< KMP failure function table */
	int m_pos; /**< current scan position */
	int m_found; /**< flag set when pattern is matched */
	/**
	 * @brief Initialize state with given pattern.
	 * @param pattern NUL‑terminated pattern string.
	 */
	void init(char *pattern);
	/**
	 * @brief Reset search state to beginning.
	 */
	void reset();
	/**
	 * @brief Feed next input character and check for match.
	 * @param ch Input character.
	 * @return 1 if pattern has just been found, 0 otherwise.
	 */
	int foundAtNextChar(char ch);
};

/**
 * @class PatternList
 * @brief A collection of `PatternState` objects for matching multiple patterns.
 */
class DllInterface PatternList
{
public:
	PatternState *m_patterns; /**< dynamically allocated array of states */
	int m_count; /**< number of patterns in the list */
	/**
	 * @brief Initialize list with given C-string patterns.
	 * @param patterns Array of NUL‑terminated strings.
	 * @param count Number of entries in @p patterns.
	 */
	void init(char **patterns, int count);
	/**
	 * @brief Feed character through every pattern state.
 * @param ch Input character to feed into each pattern state.
	 *
	 * The character is compared against all patterns managed by this list; if
	 * any of them report a match at the current position the function returns
	 * a positive result.
	 * @return 1 if any pattern matched on this character.
	 */
	int foundAtNextChar(char ch); 
	/**
	 * @brief Reset all contained pattern states.
	 */
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
