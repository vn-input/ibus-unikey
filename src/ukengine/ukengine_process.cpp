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

#include "ukengine_internal.h"

int UkEngine::process(unsigned int keyCode, int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType)
{
    UkKeyEvent ev;
    prepareBuffer();
    m_backs = 0;
    m_changePos = m_current+1;
    m_pOutBuf = outBuf;
    m_pOutSize = &outSize;
    m_outputWritten = false;
    m_reverted = false;
    m_keyRestored = false;
    m_keyRestoring = false;
    m_outType = UkCharOutput;

    m_pCtrl->input.keyCodeToEvent(keyCode, ev);

    int ret;
    if (!m_toEscape) {
        ret = (this->*UkKeyProcList[ev.evType])(ev);
    }
    else {
        m_toEscape = false;
        if (m_current < 0 || ev.evType == vneNormal || ev.evType == vneEscChar) {
            ret = processAppend(ev);
        }
        else {
            m_current--;
            processAppend(ev);
            markChange(m_current); //this will assign m_backs to 1 and mark the character for output
            ret = 1;
        }
    }

    if ( m_pCtrl->vietKey &&
         m_current >= 0 && m_buffer[m_current].form == vnw_nonVn &&
         ev.chType == ukcVn &&
         (!m_pCtrl->options.spellCheckEnabled || m_singleMode) )
    {

        //The spell check has failed, but because we are in non-spellcheck mode,
        //we consider the new character as the beginning of a new word
        ret = processNoSpellCheck(ev);
        /*
        if ((!m_pCtrl->options.spellCheckEnabled || m_singleMode) || 
            ( !m_reverted && 
              (m_current < 1 || m_buffer[m_current-1].form != vnw_nonVn)) ) {

            ret = processNoSpellCheck(ev);
        }
        */
    }

    //we add key to key buffer only if that key has not caused a reset
    if (m_current >= 0) {
        ev.chType = m_pCtrl->input.getCharType(ev.keyCode);
        m_keyCurrent++;
        m_keyStrokes[m_keyCurrent].ev = ev;
        m_keyStrokes[m_keyCurrent].converted = (ret && !m_keyRestored);
    }

    if (ret == 0) {
        backs = 0;
        outSize = 0;
        outType = m_outType;
        return 0;
    }

    backs = m_backs;
    if (!m_outputWritten) {
        writeOutput(outBuf, outSize);
    }
    outType = m_outType;

    return ret;
}


//----------------------------------------------------------
// Returns 0 on success
//         error code otherwise
//  outBuf: buffer to write
//  outSize: [in] size of buffer in bytes
//           [out] bytes written to buffer
//----------------------------------------------------------
/**
 * @brief Encode the changed portion of the composition buffer into the target charset.
 *
 * @param outBuf Destination buffer for encoded output bytes.
 * @param outSize On entry, available space; on return, bytes produced.
 * @return Zero on success, or a conversion/output error code.
 */
int UkEngine::writeOutput(unsigned char *outBuf, int & outSize)
{
    StdVnChar stdChar;
    int i, bytesWritten;
    int ret = 1;

    /**
     * Buffered output stream backed directly by the caller-provided byte buffer.
     */
    StringBOStream os(outBuf, outSize);
    VnCharset *pCharset = VnCharsetLibObj.getVnCharset(m_pCtrl->charsetId);
    pCharset->startOutput();

    for (i = m_changePos; i <= m_current; i++) {
        if (m_buffer[i].vnSym != vnl_nonVnChar) {
            //process vn symbol
            stdChar = m_buffer[i].vnSym + VnStdCharOffset;
            if (m_buffer[i].caps)
                stdChar--;
            if (m_buffer[i].tone != 0)
                stdChar += m_buffer[i].tone * 2;
        }
        else {
            stdChar = IsoToStdVnChar(m_buffer[i].keyCode);
        }
    
        if (stdChar != INVALID_STD_CHAR)
            ret = pCharset->putChar(os, stdChar, bytesWritten);
    }

    outSize = os.getOutBytes();
    return (ret? 0 : VNCONV_OUT_OF_MEMORY);
}

//---------------------------------------------
// Returns the number of backspaces needed to
// go back from last to first
//---------------------------------------------
/**
 * @brief Count how many output character positions a buffer slice occupies.
 *
 * Used to compute backspace counts when replacing already-emitted text.
 *
 * @param first Index of the first buffer entry in the sequence.
 * @param last Index of the last buffer entry in the sequence.
 * @return Number of output positions corresponding to that range.
 */
int UkEngine::getSeqSteps(int first, int last)
{
    StdVnChar stdChar;

    if (last < first)
        return 0;

    if (m_pCtrl->charsetId == CONV_CHARSET_XUTF8 || 
        m_pCtrl->charsetId == CONV_CHARSET_UNICODE)
        return (last - first +  1);


    /**
     * Counting-only output stream used to measure encoded length.
     */
    StringBOStream os(0, 0);
    int i, bytesWritten;

    VnCharset *pCharset = VnCharsetLibObj.getVnCharset(m_pCtrl->charsetId);
    pCharset->startOutput();

    for (i = first; i <= last; i++) {
        if (m_buffer[i].vnSym != vnl_nonVnChar) {
            //process vn symbol
            stdChar = m_buffer[i].vnSym + VnStdCharOffset;
            if (m_buffer[i].caps)
                stdChar--;
            if (m_buffer[i].tone != 0)
                stdChar += m_buffer[i].tone*2;
        }
        else {
            stdChar = m_buffer[i].keyCode;
        }
    
        if (stdChar != INVALID_STD_CHAR)
            pCharset->putChar(os, stdChar, bytesWritten);
    }
  
    int len = os.getOutBytes();
    if (m_pCtrl->charsetId == CONV_CHARSET_UNIDECOMPOSED)
        len = len / 2;
    return len;
}

//---------------------------------------------
/**
 * @brief Extend the changed range so output can be regenerated from `pos` onward.
 *
 * @param pos Earliest modified buffer position.
 */
void UkEngine::markChange(int pos)
{
    if (pos < m_changePos) {
        m_backs += getSeqSteps(pos, m_changePos-1);
        m_changePos = pos;
    }
}

//----------------------------------------------------------------
// Called from processBackspace to keep
// character buffer (m_buffer) and key stroke buffer in synch
//----------------------------------------------------------------
/**
 * @brief Keep the raw keystroke history aligned with the composition buffer after deletion.
 */
void UkEngine::synchKeyStrokeBuffer()
{
    //synchronize with key-stroke buffer
    if (m_keyCurrent >= 0)
        m_keyCurrent--;
    if (m_current >= 0 && m_buffer[m_current].form == vnw_empty) {
        //in character buffer, we have reached a word break,
        //so we also need to move key stroke pointer backward to corresponding word break
        while (m_keyCurrent >= 0 && m_keyStrokes[m_keyCurrent].ev.chType != ukcWordBreak)
        {
            m_keyCurrent--;
        }
    }
}

//---------------------------------------------
/**
 * @brief Handle backspace by deleting or rebalancing the current composition.
 *
 * @param backs Output count of physical backspaces to send to the application.
 * @param outBuf Buffer that receives replacement bytes when a recomposition is needed.
 * @param outSize On entry, available space; on return, bytes written.
 * @param outType Output classification for the generated data.
 * @return Non-zero when backspace triggered a visible recomposition.
 */
int UkEngine::processBackspace(int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType)
{
    outType = UkCharOutput;
    if (!m_pCtrl->vietKey || m_current < 0) {
        backs = 0;
        outSize = 0;
        return 0;
    }

    m_backs = 0;
    m_changePos = m_current + 1;
    markChange(m_current);

    if (m_current == 0 || 
        m_buffer[m_current].form == vnw_empty ||
        m_buffer[m_current].form == vnw_nonVn ||
        m_buffer[m_current].form == vnw_c ||
        m_buffer[m_current-1].form == vnw_c ||
        m_buffer[m_current-1].form == vnw_cvc ||
        m_buffer[m_current-1].form == vnw_vc) {

        m_current--;
        backs = m_backs;
        outSize = 0;
        synchKeyStrokeBuffer();
        return (backs > 1);
    }
  
    VowelSeq vs, newVs;
    int curTonePos, newTonePos, tone, vStart, vEnd;

    vEnd = m_current - m_buffer[m_current].vOffset;
    vs = m_buffer[vEnd].vseq;
    vStart = vEnd - VSeqList[vs].len + 1;
    newVs = m_buffer[m_current-1].vseq;
    curTonePos = vStart +  getTonePosition(vs, vEnd == m_current);
    newTonePos = vStart + getTonePosition(newVs, true);
    tone = m_buffer[curTonePos].tone;

    if (tone == 0 || curTonePos == newTonePos || 
        (curTonePos == m_current && m_buffer[m_current].tone != 0)) {
        m_current--;
        backs = m_backs;
        outSize = 0;
        synchKeyStrokeBuffer();
        return (backs > 1);
    }

    markChange(newTonePos);
    m_buffer[newTonePos].tone = tone;
    markChange(curTonePos);
    m_buffer[curTonePos].tone = 0;
    m_current--;
    synchKeyStrokeBuffer();
    backs = m_backs;
    writeOutput(outBuf, outSize);
    return 1;
}

//------------------------------------------------
/**
 * @brief Clear composition and keystroke state for the current session.
 */
void UkEngine::reset()
{
    m_current = -1;
    m_keyCurrent = -1;
    m_singleMode = false;
    m_toEscape = false;
}

//------------------------------------------------
/**
 * @brief Drop the remembered raw keystroke history.
 */
void UkEngine::resetKeyBuf()
{
    m_keyCurrent = -1;
}

//------------------------------------------------
UkEngine::UkEngine()
{
    if (!m_classInit) {
        engineClassInit();
        m_classInit = true;
    }
    m_pCtrl = 0;
    m_bufSize = MAX_UK_ENGINE;
    m_keyBufSize = MAX_UK_ENGINE;
    m_current = -1;
    m_keyCurrent = -1;
    m_singleMode = false;
    m_keyCheckFunc = 0;
    m_reverted = false;
    m_toEscape = false;
    m_keyRestored = false;
}

//----------------------------------------------------
// make sure there are at least 10 entries available
//----------------------------------------------------
/**
 * @brief Reclaim space in the composition and keystroke buffers before appending more data.
 */
void UkEngine::prepareBuffer()
{
    int rid;
    //prepare symbol buffer
    if (m_current >= 0 && m_current + 10 >= m_bufSize) {
        // Get rid of at least half of the current entries
        // don't get rid from the middle of a word.
        for (rid = m_current/2; m_buffer[rid].form != vnw_empty && rid < m_current; rid++);
        if (rid == m_current) {
            m_current = -1;
        }
        else {
            rid++;
            memmove(m_buffer, m_buffer+rid, (m_current-rid+1)*sizeof(WordInfo));
            m_current -= rid;
        }
    }

    //prepare key stroke buffer
    if (m_keyCurrent > 0 && m_keyCurrent + 1 >= m_keyBufSize) {
        // Get rid of at least half of the current entries
        rid = m_keyCurrent/2;
        memmove(m_keyStrokes, m_keyStrokes + rid, (m_keyCurrent-rid+1)*sizeof(m_keyStrokes[0]));
        m_keyCurrent -= rid;
    }

}

#define ENTER_CHAR 13
enum VnCaseType {VnCaseNoChange, VnCaseAllCapital, VnCaseAllSmall};

//----------------------------------------------------
/**
 * @brief Expand the most recent word if it matches a user-defined macro.
 *
 * @param ev Word-ending event that triggers macro lookup.
 * @return Non-zero when a macro expansion was emitted.
 */
int UkEngine::macroMatch(UkKeyEvent & ev)
{
    int capsLockOn = 0;
    int shiftPressed = 0;
    if (m_keyCheckFunc)
        m_keyCheckFunc(&shiftPressed, &capsLockOn);

    if (shiftPressed && (ev.keyCode ==' ' || ev.keyCode == ENTER_CHAR))
        return 0;

    const StdVnChar *pMacText = NULL;
    StdVnChar key[MAX_MACRO_KEY_LEN+1];
    StdVnChar *pKeyStart;

    // Use static macro text so we can gain a bit of performance
    // by avoiding memory allocation each time this function is called
    static StdVnChar macroText[MAX_MACRO_TEXT_LEN+1];

    int i, j;

    i = m_current;
    while (i >= 0 && (m_current-i + 1) < MAX_MACRO_KEY_LEN) {
        while (i>=0 && m_buffer[i].form != vnw_empty && (m_current-i + 1) < MAX_MACRO_KEY_LEN)
            i--;
        if (i>=0 && m_buffer[i].form != vnw_empty)
            return 0;

        if (i>=0) {
            if (m_buffer[i].vnSym != vnl_nonVnChar) {
                key[0] = m_buffer[i].vnSym + VnStdCharOffset;
                if (m_buffer[i].caps)
                    key[0]--;
                key[0] += m_buffer[i].tone*2;
            }
            else
                key[0] = m_buffer[i].keyCode;
        }

        for (j=i+1; j<=m_current; j++) {
            if (m_buffer[j].vnSym != vnl_nonVnChar) {
                key[j-i] = m_buffer[j].vnSym + VnStdCharOffset;
                if (m_buffer[j].caps)
                    key[j-i]--;
                key[j-i] += m_buffer[j].tone*2;
            }
            else
                key[j-i] = m_buffer[j].keyCode;
        }
        key[m_current-i+1] = 0;
        //search macro table
        pMacText = m_pCtrl->macStore.lookup(key+1);
        if (pMacText) {
            i++; //mark the position where change is needed
            pKeyStart = key + 1;
            break;
        }
        if (i>=0) {
            pMacText = m_pCtrl->macStore.lookup(key);
            if (pMacText) {
                pKeyStart = key;
                break;
            }
        }
        i--;
    }

    if (!pMacText) {
        return 0;
    }

    markChange(i);

    // determine the form of macro replacements: ALL CAPITALS, First Character Capital, or no change
    VnCaseType macroCase;
    if (IS_STD_VN_LOWER(*pKeyStart)) {
        macroCase = VnCaseAllSmall;
    }
    // first character uppercase – determine overall case of the macro
    else if (IS_STD_VN_UPPER(*pKeyStart)) {
        macroCase = VnCaseAllCapital;
        for (i=1; pKeyStart[i]; i++) {
            if (IS_STD_VN_LOWER(pKeyStart[i])) {
                macroCase = VnCaseNoChange;
            }
        }
    }
    else macroCase = VnCaseNoChange;

    // Convert case of macro text according to macroCase
    int charCount = 0;
    while (pMacText[charCount] != 0)
        charCount++;

    for (i = 0; i < charCount; i++)
    {
        if (macroCase == VnCaseAllCapital)
            macroText[i] = StdVnToUpper(pMacText[i]);
        // convert to lowercase if macro is all small
        else if (macroCase == VnCaseAllSmall)
            macroText[i] = StdVnToLower(pMacText[i]);
        else
            macroText[i] = pMacText[i];
    }

    // Convert to target output charset
    int outSize;
    int maxOutSize = *m_pOutSize;
    int inLen = charCount * sizeof(StdVnChar);
    VnConvert(CONV_CHARSET_VNSTANDARD, m_pCtrl->charsetId,
	        (UKBYTE *) macroText, (UKBYTE *)m_pOutBuf,
	        &inLen, &maxOutSize);
    outSize = maxOutSize;

    //write the last input character
    StdVnChar vnChar;
    if (outSize < *m_pOutSize) {
        maxOutSize = *m_pOutSize - outSize;
        if (ev.vnSym != vnl_nonVnChar)
            vnChar = ev.vnSym + VnStdCharOffset;
        else
            vnChar = ev.keyCode;
        inLen = sizeof(StdVnChar);
        VnConvert(CONV_CHARSET_VNSTANDARD, m_pCtrl->charsetId,
	            (UKBYTE *) &vnChar, ((UKBYTE *)m_pOutBuf) + outSize,
	            &inLen, &maxOutSize);
        outSize += maxOutSize;
    }
    int backs = m_backs; //store m_backs before calling reset
    reset();
    m_outputWritten = true;
    m_backs = backs;
    *m_pOutSize = outSize;
    return 1;
}

//----------------------------------------------------
/**
 * @brief Replay original keystrokes for the last word when conversion should be undone.
 *
 * @param backs Output number of backspaces needed before replaying the raw keys.
 * @param outBuf Buffer receiving the restored keystrokes.
 * @param outSize On entry, available space; on return, bytes written.
 * @param outType Output classification for the restored data.
 * @return Non-zero when restoration succeeded.
 */
int UkEngine::restoreKeyStrokes(int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType)
{
    outType = UkKeyOutput;
    if (!lastWordHasVnMark()) {
        backs = 0;
        outSize = 0;
        return 0;
    }

    m_backs = 0;
    m_changePos = m_current+1;

    int keyStart;
    bool converted = false;
    for (keyStart = m_keyCurrent; keyStart >= 0 && m_keyStrokes[keyStart].ev.chType != ukcWordBreak; keyStart--) {
        if (m_keyStrokes[keyStart].converted) {
            converted = true;
        }
    }
    keyStart++;

    if (!converted) {
        //no key stroke has been converted, so it doesn't make sense to restore key strokes
        backs = 0;
        outSize = 0;
        return 0;
    }

    //int i = m_current;
    while (m_current >=0 && m_buffer[m_current].form != vnw_empty)
        m_current--;
    markChange(m_current+1);
    backs = m_backs;

    int count;
    int i;
    UkKeyEvent ev;
    m_keyRestoring = true;
    for (i=keyStart, count = 0; i <= m_keyCurrent; i++) {
        if (count<outSize) {
            outBuf[count++] = (unsigned char)m_keyStrokes[i].ev.keyCode;
        }
        m_pCtrl->input.keyCodeToSymbol(m_keyStrokes[i].ev.keyCode, ev);
        m_keyStrokes[i].converted = false;
        processAppend(ev);
    }
    outSize = count;
    m_keyRestoring = false;

    return 1;
}

//--------------------------------------------------
/**
 * @brief Force subsequent processing into single-key mode.
 */
void UkEngine::setSingleMode()
{
    m_singleMode = true;
}

//--------------------------------------------------
/**
 * @brief Initialize global lookup tables required by the UniKey engine runtime.
 */
void SetupUnikeyEngine()
{
    SetupInputClassifierTable();
    int i;
    VnLexiName lexi;

    //Calculate IsoStdVnCharMap
    for (i=0; i < 256; i++) {
        IsoStdVnCharMap[i] = i;
    }

    for (i=0; SpecialWesternChars[i]; i++) {
        IsoStdVnCharMap[SpecialWesternChars[i]] = (vnl_lastChar + i) + VnStdCharOffset;
    }

    for (i=0; i < 256; i++) {
        if ((lexi = IsoToVnLexi(i)) != vnl_nonVnChar) {
            IsoStdVnCharMap[i] = lexi + VnStdCharOffset;
        }
    }
}

//--------------------------------------------------
/**
 * @brief Report whether the engine is currently positioned at a word boundary.
 *
 * @return `true` when no active word is being composed.
 */
bool UkEngine::atWordBeginning()
{
    return (m_current < 0 || m_buffer[m_current].form == vnw_empty);
}

//--------------------------------------------------
// Check for macro first, if there's a match, expand macro. If not:
// Spell-check, if is valid Vietnamese, return normally, if not:
// restore key strokes if auto-restore is enabled
//--------------------------------------------------
/**
 * @brief Finalize the current word when a separator key is entered.
 *
 * @param ev Word-break event such as space or punctuation.
 * @return Non-zero when finalization emitted output or macro expansion.
 */
int UkEngine::processWordEnd(UkKeyEvent & ev)
{
    if (m_pCtrl->options.macroEnabled && macroMatch(ev))
        return 1;

    if (!m_pCtrl->options.spellCheckEnabled || m_singleMode || m_current < 0 || m_keyRestoring) {
        m_current++;
        WordInfo & entry = m_buffer[m_current];
        entry.form = vnw_empty;
        entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
        entry.keyCode = ev.keyCode;
        entry.vnSym = vnToLower(ev.vnSym);
        entry.caps = (entry.vnSym != ev.vnSym);
        return 0;
    }

    int outSize = 0;
    if (m_pCtrl->options.autoNonVnRestore && lastWordIsNonVn()) {
        outSize = *m_pOutSize;
        if (restoreKeyStrokes(m_backs, m_pOutBuf, outSize, m_outType)) {
            m_keyRestored = true;
            m_outputWritten = true;
        }
    }

    m_current++;
    WordInfo & entry = m_buffer[m_current];
    entry.form = vnw_empty;
    entry.c1Offset = entry.c2Offset = entry.vOffset = -1;
    entry.keyCode = ev.keyCode;
    entry.vnSym = vnToLower(ev.vnSym);
    entry.caps = (entry.vnSym != ev.vnSym);
    
    if (m_keyRestored && outSize < *m_pOutSize) {
        m_pOutBuf[outSize] = ev.keyCode;
        outSize++;
        *m_pOutSize = outSize;
        return 1;
    }
 
    return 0;
}

//---------------------------------------------------------------------------
// Test if last word is a non-Vietnamese word, so that
// the engine can restore key strokes if it is indeed not a Vietnamese word
//---------------------------------------------------------------------------
/**
 * @brief Decide whether the most recent word should be treated as non-Vietnamese.
 *
 * @return `true` when the buffered word violates Vietnamese spelling/composition rules.
 */
bool UkEngine::lastWordIsNonVn()
{
    if (m_current < 0)
        return false;

    switch (m_buffer[m_current].form) {
        case vnw_nonVn:
            return true;
        case vnw_empty:
        case vnw_c:
            return false;
        case vnw_v:
        case vnw_cv:
            return !VSeqList[m_buffer[m_current].vseq].complete;
        case vnw_vc:
        case vnw_cvc: {
            int vIndex = m_current - m_buffer[m_current].vOffset;
            VowelSeq vs = m_buffer[vIndex].vseq;
            if (!VSeqList[vs].complete)
                return true;
            ConSeq cs = m_buffer[m_current].cseq;
            ConSeq c1 = cs_nil;
            if (m_buffer[m_current].c1Offset != -1)
                c1 = m_buffer[m_current-m_buffer[m_current].c1Offset].cseq;

            if (!isValidCVC(c1, vs, cs)) {
                return true;
            }

            int tonePos = (vIndex - VSeqList[vs].len + 1) + getTonePosition(vs, false);
            int tone = m_buffer[tonePos].tone;
            if ((cs == cs_c || cs == cs_ch || cs == cs_p || cs == cs_t) &&
                (tone == 2 || tone == 3 || tone == 4))
            {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------
// Test if last word has a Vietnamese mark, that is tones, decorators
//---------------------------------------------------------------------------
/**
 * @brief Check whether the current word contains any Vietnamese-specific mark.
 *
 * @return `true` when the word includes tones or decorated base letters.
 */
bool UkEngine::lastWordHasVnMark()
{
    for (int i=m_current; i>=0 && m_buffer[i].form != vnw_empty; i--) {
        VnLexiName sym = m_buffer[i].vnSym;
        if (sym != vnl_nonVnChar) {
            if (IsVnVowel[sym]) {
                if (m_buffer[i].tone)
                    return true;
            }
            if (sym != StdVnRootChar[sym] )
                return true;
        }
    }
    return false;
}
