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
 * @file ukengine.cpp
 * @brief Core Vietnamese input engine implementation.
 *
 * This file contains the internal lookup tables, validation helpers, and key
 * processing routines used to transform raw keystrokes into Vietnamese text,
 * including tone placement, macro expansion, spell-check-aware restoration,
 * and charset-specific output generation.
 */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "keycons.h"

/*
#if defined(_WIN32)
#include "keyhook.h"
#endif
*/

#include "vnlexi.h"
#include "ukengine.h"

#include "charset.h"

using namespace std;

#include "ukengine_tables.h"
#include "ukengine_utils.h"

//----------------------------------------------------------
/**
 * @brief Process a character that may map to a Vietnamese symbol or action.
 *
 * @param ev Event to append and possibly reinterpret using map-char rules.
 * @return Non-zero when the mapped processing changed engine state.
 */
int UkEngine::processMapChar(UkKeyEvent & ev)
{
    int capsLockOn = 0;
    int shiftPressed = 0;
    if (m_keyCheckFunc)
        m_keyCheckFunc(&shiftPressed, &capsLockOn);

    if (capsLockOn)
        ev.vnSym = changeCase(ev.vnSym);

    int ret = processAppend(ev);
    if (!m_pCtrl->vietKey)
        return ret;

    if (m_current >= 0 && m_buffer[m_current].form != vnw_empty &&
        m_buffer[m_current].form != vnw_nonVn) {
        return 1;
    }

    if (m_current < 0)
        return 0;

    // mapChar doesn't apply
    m_current--;
    WordInfo & entry = m_buffer[m_current];

    bool undo = false;
    // test if undo is needed
    if (entry.form != vnw_empty && entry.form != vnw_nonVn) {
        VnLexiName prevSym = entry.vnSym;
        if (entry.caps) {
            prevSym = (VnLexiName)(prevSym - 1);
        }
        if (prevSym == ev.vnSym) {
            if (entry.form != vnw_c) {
                int vStart, vEnd, curTonePos, newTonePos, tone;
                VowelSeq vs, newVs;

                vEnd = m_current - entry.vOffset;
                vs = m_buffer[vEnd].vseq;
                vStart = vEnd - VSeqList[vs].len +1;
                curTonePos = vStart + getTonePosition(vs, vEnd == m_current);
                tone = m_buffer[curTonePos].tone;
                markChange(m_current);
                m_current--;

                //check if tone position is needed
                if (tone != 0 && m_current >= 0 &&
                    (m_buffer[m_current].form == vnw_v || m_buffer[m_current].form == vnw_cv)) {
                    newVs = m_buffer[m_current].vseq;
                    newTonePos = vStart + getTonePosition(newVs, true);
                    if (newTonePos != curTonePos) {
                        markChange(newTonePos);
                        m_buffer[newTonePos].tone = tone;
                        markChange(curTonePos);
                        m_buffer[curTonePos].tone = 0;
                    }
                }
            }
            else {
                markChange(m_current);
                m_current--;
            }
            undo = true;
        }
    }

    ev.evType = vneNormal;
    ev.chType = m_pCtrl->input.getCharType(ev.keyCode);
    ev.vnSym = IsoToVnLexi(ev.keyCode);
    ret = processAppend(ev);
    if (undo) {
        m_singleMode = false;
        m_reverted = true;
        return 1;
    }
    return ret;
}

//----------------------------------------------------------
/**
 * @brief Handle the Telex-specific `w` key, which may act as a hook key or mapped letter.
 *
 * @param ev Key event for `w` or `W`.
 * @return Non-zero when the event was consumed by Telex processing.
 */
int UkEngine::processTelexW(UkKeyEvent & ev)
{
    if (!m_pCtrl->vietKey)
        return processAppend(ev);

    int ret;
    static bool usedAsMapChar = false;
    int capsLockOn = 0;
    int shiftPressed = 0;
    if (m_keyCheckFunc)
        m_keyCheckFunc(&shiftPressed, &capsLockOn);

    if (usedAsMapChar) {
        ev.evType = vneMapChar;
        ev.vnSym = isupper(ev.keyCode)? vnl_Uh : vnl_uh;
        if (capsLockOn)
            ev.vnSym = changeCase(ev.vnSym);
        ev.chType = ukcVn;
        ret = processMapChar(ev);
        if (ret == 0) {
            if (m_current >= 0)
                m_current--;
            usedAsMapChar = false;
            ev.evType = vneHookAll;
            return processHook(ev);
        }
        return ret;
    }

    ev.evType = vneHookAll;
    usedAsMapChar = false;
    ret = processHook(ev);
    if (ret == 0) {
        if (m_current >= 0)
            m_current--;
        ev.evType = vneMapChar;
        ev.vnSym = isupper(ev.keyCode)? vnl_Uh : vnl_uh;
        if (capsLockOn)
            ev.vnSym = changeCase(ev.vnSym);
        ev.chType = ukcVn;
        usedAsMapChar = true;
        return processMapChar(ev);
    }
    return ret;
}

//----------------------------------------------------------
/**
 * @brief Detect VIQR escape sequences that force literal punctuation output.
 *
 * @param ev Punctuation key event being tested for escape handling.
 * @return Non-zero when a backslash escape was emitted instead of composition.
 */
int UkEngine::checkEscapeVIQR(UkKeyEvent & ev)
{
    if (m_current < 0)
        return 0;
    WordInfo & entry = m_buffer[m_current];
    int escape = 0;
    if (entry.form == vnw_v || entry.form == vnw_cv) {
        switch (ev.keyCode) {
        case '^':
            escape = (entry.vnSym == vnl_a || entry.vnSym == vnl_o || entry.vnSym == vnl_e);
            break;
        case '(':
            escape = (entry.vnSym == vnl_a);
            break;
        case '+':
            escape = (entry.vnSym == vnl_o || entry.vnSym == vnl_u);
            break;
        case '\'':
        case '`':
        case '?':
        case '~':
        case '.':
            escape = (entry.tone == 0);
            break;
        }
    }
    // entry is in a non‑Vietnamese form; apply special escape rules
    else if (entry.form == vnw_nonVn) {
        unsigned char ch = toupper(entry.keyCode);
        switch (ev.keyCode) {
        case '^':
            escape = (ch == 'A' || ch == 'O' || ch == 'E');
            break;
        case '(':
            escape = (ch == 'A');
            break;
        case '+':
            escape = (ch == 'O' || ch == 'U');
            break;
        case '\'':
        case '`':
        case '?':
        case '~':
        case '.':
            escape = (ch == 'A' || ch == 'E' || ch == 'I' ||
                      ch == 'O' || ch == 'U' || ch == 'Y');
            break;
        }
    }
  
    if (escape) {
        m_current++;
        WordInfo *p = &m_buffer[m_current];
        p->form = (ev.chType == ukcWordBreak) ? vnw_empty : vnw_nonVn;
        p->c1Offset = p->c2Offset = p->vOffset = -1;
        p->keyCode = '?';
        p->vnSym = vnl_nonVnChar;

        m_current++;
        p++;
        p->form = (ev.chType == ukcWordBreak) ? vnw_empty : vnw_nonVn;
        p->c1Offset = p->c2Offset = p->vOffset = -1;
        p->keyCode = ev.keyCode;
        p->vnSym = vnl_nonVnChar;

        //write output
        m_pOutBuf[0] = '\\';
        m_pOutBuf[1] = ev.keyCode;
        *m_pOutSize = 2;
        m_outputWritten = true;
    }
    return escape;
}

//----------------------------------------------------------
/**
 * @brief Append a normalized key event into the current composition buffer.
 *
 * @param ev Event to append or dispatch based on character class.
 * @return Non-zero when appending produced visible output changes.
 */
int UkEngine::processAppend(UkKeyEvent & ev)
{
    int ret = 0;
    switch (ev.chType) {
    case ukcReset:
#if defined(_WIN32)
        if (ev.keyCode == ENTER_CHAR) {
            if (m_pCtrl->options.macroEnabled && macroMatch(ev))
                return 1;
        }
#endif
        reset();
        return 0;
    case ukcWordBreak:
        m_singleMode = false;
        return processWordEnd(ev);
    case ukcNonVn:
        {
            if (m_pCtrl->vietKey && m_pCtrl->charsetId == CONV_CHARSET_VIQR && checkEscapeVIQR(ev))
                return 1;

            m_current++;
            WordInfo & entry = m_buffer[m_current];
            entry.form = (ev.chType == ukcWordBreak) ? vnw_empty : vnw_nonVn;
            entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
            entry.keyCode = ev.keyCode;
            entry.vnSym = vnToLower(ev.vnSym);
            entry.tone = 0;
            entry.caps = (entry.vnSym != ev.vnSym);
            if (!m_pCtrl->vietKey || m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING)
                return 0;
            markChange(m_current);
            return 1;
        }
    case ukcVn:
        {
            if (IsVnVowel[ev.vnSym]) {
                VnLexiName v = (VnLexiName)StdVnNoTone[vnToLower(ev.vnSym)];
                if (m_current >= 0 && m_buffer[m_current].form == vnw_c &&
                      ((m_buffer[m_current].cseq == cs_q && v == vnl_u) ||
                      (m_buffer[m_current].cseq == cs_g && v == vnl_i))) {
                    return appendConsonnant(ev); //process u after q, i after g as consonnants
                }
                return appendVowel(ev);
            }
            return appendConsonnant(ev);
        }
        break;
    }

    return ret;
}

//----------------------------------------------------------
/**
 * @brief Append a vowel and update vowel-sequence and tone metadata.
 *
 * @param ev Vowel key event to merge into the current word.
 * @return Non-zero when the append requires output updates.
 */
int UkEngine::appendVowel(UkKeyEvent & ev)
{
    bool autoCompleted = false;

    m_current++;
    WordInfo & entry = m_buffer[m_current];

    VnLexiName lowerSym = vnToLower(ev.vnSym);
    VnLexiName canSym = (VnLexiName)StdVnNoTone[lowerSym];

    entry.vnSym = canSym;
    entry.caps = (lowerSym != ev.vnSym);
    entry.tone = (lowerSym - canSym)/2;
    entry.keyCode = ev.keyCode;

    if (m_current == 0 || !m_pCtrl->vietKey) {
        entry.form = vnw_v;
        entry.c1Offset = entry.c2Offset = -1;
        entry.vOffset = 0;
        entry.vseq = lookupVSeq(canSym);

        if (!m_pCtrl->vietKey || 
            ((m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING) && isalpha(entry.keyCode)) ) {
            return 0;
        }
        markChange(m_current);
        return 1;
    }
  
    WordInfo & prev = m_buffer[m_current-1];
    VowelSeq vs, newVs;
    ConSeq cs;
    int prevTonePos;
    int tone, newTone, tonePos, newTonePos;

    switch (prev.form) {

    case vnw_empty:
        entry.form = vnw_v;
        entry.c1Offset = entry.c2Offset = -1;
        entry.vOffset = 0;
        entry.vseq = newVs = lookupVSeq(canSym);
        break;

    case vnw_nonVn:
    case vnw_cvc:
    case vnw_vc:
        entry.form = vnw_nonVn;
        entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
        break;

    case vnw_v:
    case vnw_cv:
        vs = prev.vseq;
        prevTonePos = (m_current - 1) - (VSeqList[vs].len - 1) + getTonePosition(vs, true);
        tone = m_buffer[prevTonePos].tone;
    
        if (lowerSym != canSym && tone != 0) //new sym has a tone, but there's is already a preceeding tone
            newVs = vs_nil;
        else {
            if (VSeqList[vs].len == 3)
                newVs = vs_nil;
            // two-length sequence: include candidate symbol in lookup
            else if (VSeqList[vs].len == 2)
                newVs = lookupVSeq(VSeqList[vs].v[0], VSeqList[vs].v[1], canSym);
            else
                newVs = lookupVSeq(VSeqList[vs].v[0], canSym);
        }

        if (newVs != vs_nil && prev.form == vnw_cv) {
            cs = m_buffer[m_current - 1 - prev.c1Offset].cseq;
            if (!isValidCV(cs, newVs))
                newVs = vs_nil;
        }

        if (newVs == vs_nil) {
            entry.form = vnw_nonVn;
            entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
            break;
        }
    
        entry.form = prev.form;
        if (prev.form == vnw_cv)
            entry.c1Offset = prev.c1Offset + 1;
        else
            entry.c1Offset = -1;
        entry.c2Offset = -1;
        entry.vOffset = 0;
        entry.vseq = newVs;
        entry.tone = 0;
        
        newTone = (lowerSym - canSym)/2;
        if (tone == 0) {
            if (newTone != 0) {
                tone = newTone;
                tonePos = getTonePosition(newVs, true) + ((m_current - 1) - VSeqList[vs].len + 1);
                markChange(tonePos);
                m_buffer[tonePos].tone = tone;
                return 1;
            }
        }
        else {
            newTonePos = getTonePosition(newVs, true) + ((m_current - 1) - VSeqList[vs].len + 1);
            if (newTonePos != prevTonePos) {
                markChange(prevTonePos);
                m_buffer[prevTonePos].tone = 0;
                markChange(newTonePos);
                if (newTone != 0)
                    tone = newTone;
                m_buffer[newTonePos].tone = tone;
                return 1;
            }
            if (newTone != 0 && newTone != tone) {
                tone = newTone;
                markChange(prevTonePos);
                m_buffer[prevTonePos].tone = tone;
                return 1;
            }

        }

        break;
    case vnw_c:
        newVs = lookupVSeq(canSym);
        cs = prev.cseq;
        if (!isValidCV(cs, newVs)) {
            entry.form = vnw_nonVn;
            entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
            break;
        }

        entry.form = vnw_cv;
        entry.c1Offset = 1;
        entry.c2Offset = -1;
        entry.vOffset = 0;
        entry.vseq = newVs;

        if (cs == cs_gi && prev.tone != 0) {
            if (entry.tone == 0)
                entry.tone = prev.tone;
            markChange(m_current - 1);
            prev.tone = 0;
            return 1;
        }
    
        break;
  }

    if (!autoCompleted &&
        (m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING) && 
        isalpha(entry.keyCode)) {
        return 0;
    }

    markChange(m_current);
    return 1;
}

//----------------------------------------------------------
/**
 * @brief Append a consonant and update syllable structure metadata.
 *
 * @param ev Consonant key event to merge into the current word.
 * @return Non-zero when the append changes emitted text.
 */
int UkEngine::appendConsonnant(UkKeyEvent & ev)
{
    bool complexEvent = false;
    m_current++;
    WordInfo & entry = m_buffer[m_current];

    VnLexiName lowerSym = vnToLower(ev.vnSym);

    entry.vnSym = lowerSym;
    entry.caps = (lowerSym != ev.vnSym);
    entry.keyCode = ev.keyCode;
    entry.tone = 0;

    if (m_current == 0 || !m_pCtrl->vietKey) {
        entry.form = vnw_c;
        entry.c1Offset = 0;
        entry.c2Offset = -1;
        entry.vOffset = -1;
        entry.cseq = lookupCSeq(lowerSym);
        if (!m_pCtrl->vietKey || m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING)
            return 0;
        markChange(m_current);
        return 1;
    }

    ConSeq cs, newCs, c1;
    VowelSeq vs, newVs;
    bool isValid;

    WordInfo & prev = m_buffer[m_current-1];

    switch (prev.form) {
    case vnw_nonVn:
        entry.form = vnw_nonVn;
        entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
        if (m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING)
            return 0;
        markChange(m_current);
        return 1;
    case vnw_empty:
        entry.form = vnw_c;
        entry.c1Offset = 0;
        entry.c2Offset = -1;
        entry.vOffset = -1;
        entry.cseq = lookupCSeq(lowerSym);
        if (m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING)
            return 0;
        markChange(m_current);
        return 1;
    case vnw_v:
    case vnw_cv:
        vs = prev.vseq;
        newVs = vs;
        if (vs == vs_uoh || vs == vs_uho) {
            newVs = vs_uhoh;
        }

        c1 = cs_nil;
        if (prev.c1Offset != -1)
            c1 = m_buffer[m_current-1-prev.c1Offset].cseq;

        newCs = lookupCSeq(lowerSym);
        isValid = isValidCVC(c1, newVs, newCs);

        if (isValid) {
            //check u+o -> u+o+
            if (vs == vs_uho) {
                markChange(m_current-1);
                prev.vnSym = vnl_oh;
                prev.vseq = vs_uhoh;
                complexEvent = true;
            }
            // special case: previous vowel sequence was 'uoh'
            else if (vs == vs_uoh) {
                markChange(m_current-2);
                m_buffer[m_current-2].vnSym = vnl_uh;
                m_buffer[m_current-2].vseq = vs_uh;
                prev.vseq = vs_uhoh;
                complexEvent = true;
            }

            if (prev.form == vnw_v) {
                entry.form = vnw_vc;
                entry.c1Offset = -1;
                entry.c2Offset = 0;
                entry.vOffset = 1;
            }
            else { //prev == vnw_cv
                entry.form = vnw_cvc;
                entry.c1Offset = prev.c1Offset + 1;
                entry.c2Offset = 0;
                entry.vOffset = 1;
            }
            entry.cseq = newCs;

            //reposition tone if needed
            int oldIdx = (m_current-1) - (VSeqList[vs].len - 1) + getTonePosition(vs, true);
            if (m_buffer[oldIdx].tone != 0) {
                int newIdx = (m_current-1) - (VSeqList[newVs].len - 1) + getTonePosition(newVs, false);
                if (newIdx != oldIdx) {
                    markChange(newIdx);
                    m_buffer[newIdx].tone = m_buffer[oldIdx].tone;
                    markChange(oldIdx);
                    m_buffer[oldIdx].tone = 0;
                    return 1;
                }
            }
        }
        else {
            entry.form = vnw_nonVn;
            entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
        }

        if (complexEvent) {
            return 1;
        }

        if (m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING)
            return 0;
        markChange(m_current);
        return 1;
    case vnw_c:
    case vnw_vc:
    case vnw_cvc:
        cs = prev.cseq;
        if (CSeqList[cs].len == 3)
            newCs = cs_nil;
        // two-character consonant sequence: include lowerSym in lookup
        else if (CSeqList[cs].len == 2)
            newCs = lookupCSeq(CSeqList[cs].c[0], CSeqList[cs].c[1], lowerSym);
        else
            newCs = lookupCSeq(CSeqList[cs].c[0], lowerSym);
        
        if (newCs != cs_nil && (prev.form == vnw_vc || prev.form == vnw_cvc)) {
            // Check CVC combination
            c1 = cs_nil;
            if (prev.c1Offset != -1)
                c1 = m_buffer[m_current-1-prev.c1Offset].cseq;

            int vIdx = (m_current - 1) - prev.vOffset;
            vs = m_buffer[vIdx].vseq;
            isValid = isValidCVC(c1, vs, newCs);

            if (!isValid)
                newCs = cs_nil;
        }

        if (newCs == cs_nil) {
            entry.form = vnw_nonVn;
            entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
        }
        else {
            if (prev.form == vnw_c) {
                entry.form = vnw_c;
                entry.c1Offset = 0;
                entry.c2Offset = -1;
                entry.vOffset = -1;
            }
            // previous form was vowel+consonant: set current offsets accordingly
            else if (prev.form == vnw_vc) {
                entry.form = vnw_vc;
                entry.c1Offset = -1;
                entry.c2Offset = 0;
                entry.vOffset = prev.vOffset + 1;
            }
            else { //vnw_cvc
                entry.form = vnw_cvc;
                entry.c1Offset = prev.c1Offset + 1;
                entry.c2Offset = 0;
                entry.vOffset = prev.vOffset + 1;
            }
            entry.cseq = newCs;
        }
        if (m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING)
            return 0;
        markChange(m_current);
        return 1;
    }

    if (m_pCtrl->charsetId != CONV_CHARSET_UNI_CSTRING)
        return 0;
    markChange(m_current);
    return 1;
}

//----------------------------------------------------------
/**
 * @brief Mark the next key so it is inserted literally instead of being transformed.
 *
 * @param ev Escape key event.
 * @return Result of appending the escape-triggering character.
 */
int UkEngine::processEscChar(UkKeyEvent & ev)
{
    if (m_pCtrl->vietKey && 
        m_current >=0 && m_buffer[m_current].form != vnw_empty && m_buffer[m_current].form != vnw_nonVn) {
        m_toEscape = true;
    }
    return processAppend(ev);
}

//----------------------------------------------------------
/**
 * @brief Feed one key through the append path without extra conversion logic.
 *
 * @param keyCode Raw key code to store in the internal buffer.
 */
void UkEngine::pass(int keyCode)
{
    UkKeyEvent ev;
    m_pCtrl->input.keyCodeToEvent(keyCode, ev);
    processAppend(ev);
}

//---------------------------------------------
// This can be called only after other processing have been done.
// The new event is supposed to be put into m_buffer already
//---------------------------------------------
/**
 * @brief Reinterpret the current word as literal Vietnamese input when spell-check is bypassed.
 *
 * @param ev Event that caused the spell-check fallback.
 * @return Non-zero when the fallback produced output changes.
 */
int UkEngine::processNoSpellCheck(UkKeyEvent & ev)
{
    WordInfo & entry = m_buffer[m_current];
    if (IsVnVowel[entry.vnSym]) {
        entry.form = vnw_v;
        entry.vOffset = 0;
        entry.vseq = lookupVSeq(entry.vnSym);
        entry.c1Offset = entry.c2Offset = -1;
    }
    else {
        entry.form = vnw_c;
        entry.c1Offset = 0;
        entry.c2Offset = -1;
        entry.vOffset = -1;
        entry.cseq = lookupCSeq(entry.vnSym);
    }

    if (ev.evType == vneNormal &&
        ((entry.keyCode >= 'a' && entry.keyCode <= 'z') || 
         (entry.keyCode >= 'A' && entry.keyCode <= 'Z') ) )
        return 0;
    markChange(m_current);
    return 1;
}
//----------------------------------------------------------
/**
 * @brief Process one raw key code through the full UniKey composition pipeline.
 *
 * @param keyCode Raw input key code.
 * @param backs Output count of backspaces required before inserting replacement text.
 * @param outBuf Buffer that receives generated output bytes.
 * @param outSize On entry, capacity of `outBuf`; on return, bytes written.
 * @param outType Output classification describing the meaning of `outBuf`.
 * @return Non-zero when the engine handled the key and produced state/output updates.
 */
