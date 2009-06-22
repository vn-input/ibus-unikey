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

#include "pattern.h"

//////////////////////////////////////////////////
// Pattern matching (based on KPM algorithm)
//////////////////////////////////////////////////

//----------------------------
void PatternState::reset()
{
	m_pos = 0;
	m_found = 0;
}

//----------------------------
void PatternState::init(char *pattern)
{
	m_pos = 0;
	m_found = 0;
	m_pattern = pattern;

	int i=0, j=-1;
    m_border[i]=j;
    while (m_pattern[i])
    {
        while (j>=0 && m_pattern[i]!=m_pattern[j]) j=m_border[j];
        i++; j++;
        m_border[i]=j;
    }
}

//-----------------------------------------------------
//get next input char, returns 1 if pattern is found.
//-----------------------------------------------------
int PatternState::foundAtNextChar(char ch)
{
	int ret = 0;
	//int j = m_pos;
	while (m_pos>=0 && ch!=m_pattern[m_pos]) m_pos=m_border[m_pos];
	m_pos++;
	if (m_pattern[m_pos]==0) {
		m_found++;
		m_pos = m_border[m_pos];
		ret = 1;
	}
	return ret;
}

//-----------------------------------------------------
void PatternList::init(char **patterns, int count)
{
	m_count = count;
	delete [] m_patterns;
	m_patterns = new PatternState[count];
	for (int i=0; i<count; i++)
		m_patterns[i].init(patterns[i]);
}

//-----------------------------------------------------
// return the order number of the pattern that is found.
// If more than 1 pattern is found, returns any pattern
// Returns -1 if no pattern is found
//-----------------------------------------------------
int PatternList::foundAtNextChar(char ch)
{
	int patternFound = -1;
	for (int i=0; i<m_count; i++) {
		if (m_patterns[i].foundAtNextChar(ch))
			patternFound = i;
	}
	return patternFound;
}

//-----------------------------------------------------
void PatternList::reset()
{
	for (int i=0; i<m_count; i++)
		m_patterns[i].reset();
}
