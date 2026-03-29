// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
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

#ifndef __UK_VALIDATION_H
#define __UK_VALIDATION_H

/**
 * @file uk_validation.h
 * @brief Validation helpers for Vietnamese syllable structure.
 *
 * This module provides pure predicates for checking whether consonant-vowel
 * combinations follow Vietnamese spelling rules. Extracted from ukengine.cpp
 * as part of the legacy cleanup effort (Phase 3A).
 */

#include "vnlexi.h"

/**
 * @brief Check whether a leading consonant sequence can precede a vowel sequence.
 *
 * @param c Candidate leading consonant sequence.
 * @param v Candidate vowel sequence.
 * @return `true` when the combination is allowed by Vietnamese spelling rules.
 */
bool isValidCV(ConSeq c, VowelSeq v);

/**
 * @brief Check whether a vowel sequence can take a trailing consonant sequence.
 *
 * @param v Candidate vowel sequence.
 * @param c Candidate trailing consonant sequence.
 * @return `true` when the final combination is valid.
 */
bool isValidVC(VowelSeq v, ConSeq c);

/**
 * @brief Validate a full consonant-vowel-consonant syllable structure.
 *
 * @param c1 Leading consonant sequence.
 * @param v Middle vowel sequence.
 * @param c2 Trailing consonant sequence.
 * @return `true` when the syllable is accepted by the engine rules.
 */
bool isValidCVC(ConSeq c1, VowelSeq v, ConSeq c2);

#endif // __UK_VALIDATION_H
