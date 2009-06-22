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
// For some unknown reasons, the functions in this file cannot be exported
// We had to move them to convert.cpp.
// TODO: inspect this problem later!
/*
#include "stdafx.h"
#include "vnconv.h"

char *ErrTable[VNCONV_LAST_ERROR] = 
{"No error",
 "Unknown error",
 "Invalid charset",
 "Error opening input file",
 "Error opening output file",
 "Error writing to output stream",
 "Not enough memory",
};

DllExport const char * VnConvErrMsg(int errCode)
{
	if (errCode < 0 || errCode >= VNCONV_LAST_ERROR)
		errCode = VNCONV_UNKNOWN_ERROR;
	return ErrTable[errCode];
}

*/