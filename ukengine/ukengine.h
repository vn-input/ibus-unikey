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

#ifndef __UKENGINE_H
#define __UKENGINE_H

/**
 * @file ukengine.h
 * @brief Core engine classes, shared-memory structs, and processing API.
 */

#include "charset.h"
#include "vnlexi.h"
#include "inputproc.h"
#include "mactab.h"

//This is a shared object among processes, do not put any pointer in it
struct UkSharedMem {
    //states
    int initialized;
    int vietKey;

    UnikeyOptions options;
    UkInputProcessor input;
    int usrKeyMapLoaded;
    int usrKeyMap[256];
    int charsetId;

    CMacroTable macStore;
};

#define MAX_UK_ENGINE 128

enum VnWordForm {vnw_nonVn, vnw_empty, vnw_c, vnw_v, vnw_cv, vnw_vc, vnw_cvc};

/**
 * @brief void.
 *
 * @param pCapslockOn Parameter.
 */
typedef void (* CheckKeyboardCaseCb)(int *pShiftPressed, int *pCapslockOn);

struct KeyBufEntry {
    UkKeyEvent ev;
    bool converted;
};

class UkEngine
{
public:
    UkEngine();
/**
 * @brief setCtrlInfo.
 *
 * @param p Parameter.
 */
    void setCtrlInfo(UkSharedMem *p)
    {
        m_pCtrl = p;
    }

/**
 * @brief setCheckKbCaseFunc.
 *
 * @param pFunc Parameter.
 */
    void setCheckKbCaseFunc(CheckKeyboardCaseCb pFunc) 
    {
        m_keyCheckFunc = pFunc;
    }

/**
 * @brief atWordBeginning.
 *
 */
    bool atWordBeginning();

/**
 * @brief process.
 *
 * @param keyCode Parameter.
 * @param backs Parameter.
 * @param outBuf Parameter.
 * @param outSize Parameter.
 * @param outType Parameter.
 */
    int process(unsigned int keyCode, int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType);
/**
 * @brief pass.
 *
 * @param keyCode Parameter.
 */
    void pass(int keyCode); //just pass through without filtering
/**
 * @brief setSingleMode.
 *
 */
    void setSingleMode();

/**
 * @brief processBackspace.
 *
 * @param backs Parameter.
 * @param outBuf Parameter.
 * @param outSize Parameter.
 * @param outType Parameter.
 */
    int processBackspace(int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType);
/**
 * @brief reset.
 *
 */
    void reset();
/**
 * @brief restoreKeyStrokes.
 *
 * @param backs Parameter.
 * @param outBuf Parameter.
 * @param outSize Parameter.
 * @param outType Parameter.
 */
    int restoreKeyStrokes(int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType);

    //following methods must be public just to enable the use of pointers to them
    //they should not be called from outside.
/**
 * @brief processTone.
 *
 * @param ev Parameter.
 */
    int processTone(UkKeyEvent & ev);
/**
 * @brief processRoof.
 *
 * @param ev Parameter.
 */
    int processRoof(UkKeyEvent & ev);
/**
 * @brief processHook.
 *
 * @param ev Parameter.
 */
    int processHook(UkKeyEvent & ev);
/**
 * @brief processAppend.
 *
 * @param ev Parameter.
 */
    int processAppend(UkKeyEvent & ev);
/**
 * @brief appendVowel.
 *
 * @param ev Parameter.
 */
    int appendVowel(UkKeyEvent & ev);
/**
 * @brief appendConsonnant.
 *
 * @param ev Parameter.
 */
    int appendConsonnant(UkKeyEvent & ev);
/**
 * @brief processDd.
 *
 * @param ev Parameter.
 */
    int processDd(UkKeyEvent & ev);
/**
 * @brief processMapChar.
 *
 * @param ev Parameter.
 */
    int processMapChar(UkKeyEvent & ev);
/**
 * @brief processTelexW.
 *
 * @param ev Parameter.
 */
    int processTelexW(UkKeyEvent & ev);
/**
 * @brief processEscChar.
 *
 * @param ev Parameter.
 */
    int processEscChar(UkKeyEvent & ev);

protected:
    static bool m_classInit;
    CheckKeyboardCaseCb m_keyCheckFunc;
    UkSharedMem *m_pCtrl;

    int m_changePos;
    int m_backs;
    int m_bufSize;
    int m_current;
    int m_singleMode;

    int m_keyBufSize;
    //unsigned int m_keyStrokes[MAX_UK_ENGINE];
    KeyBufEntry m_keyStrokes[MAX_UK_ENGINE];
    int m_keyCurrent;
    bool m_toEscape;

    //varables valid in one session
    unsigned char *m_pOutBuf;
    int *m_pOutSize;
    bool m_outputWritten;
    bool m_reverted;
    bool m_keyRestored;
    bool m_keyRestoring;
    UkOutputType m_outType;
  
    struct WordInfo {
        //info for word ending at this position
        VnWordForm form;
        int c1Offset, vOffset, c2Offset;

        union {
            VowelSeq vseq;
            ConSeq cseq;
        };

        //info for current symbol
        int caps, tone;
        //canonical symbol, after caps, tone are removed
        //for non-Vn, vnSym == -1
        VnLexiName vnSym;
        int keyCode;
    };

    WordInfo m_buffer[MAX_UK_ENGINE];

/**
 * @brief processHookWithUO.
 *
 * @param ev Parameter.
 */
    int processHookWithUO(UkKeyEvent & ev);
/**
 * @brief macroMatch.
 *
 * @param ev Parameter.
 */
    int macroMatch(UkKeyEvent & ev);
/**
 * @brief markChange.
 *
 * @param pos Parameter.
 */
    void markChange(int pos);
/**
 * @brief prepareBuffer.
 *
 */
    void prepareBuffer(); //make sure we have a least 10 entries available
/**
 * @brief writeOutput.
 *
 * @param outBuf Parameter.
 * @param outSize Parameter.
 */
    int writeOutput(unsigned char *outBuf, int & outSize);
    //int getSeqLength(int first, int last);
/**
 * @brief getSeqSteps.
 *
 * @param first Parameter.
 * @param last Parameter.
 */
    int getSeqSteps(int first, int last);
/**
 * @brief getTonePosition.
 *
 * @param vs Parameter.
 * @param terminated Parameter.
 */
    int getTonePosition(VowelSeq vs, bool terminated);
/**
 * @brief resetKeyBuf.
 *
 */
    void resetKeyBuf();
/**
 * @brief checkEscapeVIQR.
 *
 * @param ev Parameter.
 */
    int checkEscapeVIQR(UkKeyEvent & ev);
/**
 * @brief processNoSpellCheck.
 *
 * @param ev Parameter.
 */
    int processNoSpellCheck(UkKeyEvent & ev);
/**
 * @brief processWordEnd.
 *
 * @param ev Parameter.
 */
    int processWordEnd(UkKeyEvent & ev);
/**
 * @brief synchKeyStrokeBuffer.
 *
 */
    void synchKeyStrokeBuffer();
/**
 * @brief lastWordHasVnMark.
 *
 */
    bool lastWordHasVnMark();
/**
 * @brief lastWordIsNonVn.
 *
 */
    bool lastWordIsNonVn();
};

/**
 * @brief SetupUnikeyEngine.
 *
 */
void SetupUnikeyEngine();

#endif
