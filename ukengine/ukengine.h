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
 * @brief Callback prototype used to query the current keyboard case state.
 *
 * The engine calls this function (if registered) before processing a key
 * event so it can adjust behaviour based on whether Shift or CapsLock are
 * active.  The implementation is provided by the host (e.g. setup UI or
 * platform-specific glue).
 *
 * @param pShiftPressed Pointer returned nonzero when Shift is held.
 * @param pCapslockOn   Pointer returned nonzero when CapsLock is on.
 */
typedef void (* CheckKeyboardCaseCb)(int *pShiftPressed, int *pCapslockOn);

struct KeyBufEntry {
    UkKeyEvent ev;
    bool converted;
};

class UkEngine
{
public:
    /**
     * @brief Construct a new `UkEngine` instance.
     *
     * Initializes internal buffers, state flags and prepares the engine to
     * accept key events.  After construction the caller should assign a
     * shared memory pointer via `setCtrlInfo()` before use.
     */
    UkEngine();
/**
 * @brief Store a pointer to the shared engine control memory.
 *
 * The engine reads options, macro tables and other global state from the
 * `UkSharedMem` structure supplied by the caller.  This must be called before
 * processing any key events.
 *
 * @param p Pointer to the shared memory region.
 */
    void setCtrlInfo(UkSharedMem *p)
    {
        m_pCtrl = p;
    }

/**
 * @brief Register a keyboard case query callback.
 *
 * The supplied function will be invoked before key processing so the engine
 * can know whether Shift or CapsLock are active (important for macro
 * detection and character mapping).
 *
 * @param pFunc Function pointer matching `CheckKeyboardCaseCb`.
 */
    void setCheckKbCaseFunc(CheckKeyboardCaseCb pFunc) 
    {
        m_keyCheckFunc = pFunc;
    }

/**
 * @brief Determine whether the engine is currently at a word boundary.
 *
 * Returns true when there are no characters in the internal buffer or the
 * most recent entry represents an empty (word‑separator) state.  This is used
 * by the IBus front end to make decisions about committing preedit text and
 * applying certain smart rules.
 */
    bool atWordBeginning();

/**
 * @brief Handle a key event and perform all engine logic.
 *
 * This is the primary entry point used by the host to feed keystrokes into
 * the engine.  The method updates internal buffers and may write output
 * characters, backspace counts, or commit actions into the supplied
 * parameters.  Various sub‑routines (tone, roof, macro expansion, etc.) are
 * invoked here.
 *
 * @param keyCode  Unicode code (or hardware code) of the pressed key.
 * @param backs    [out] number of backspace operations the caller should
 *                 perform before inserting new text.
 * @param outBuf   [out] buffer to receive committed output bytes.
 * @param outSize  [in,out] on entry contains space available, on return the
 *                 number of bytes written.
 * @param outType  [out] classification of the output (commit, preedit,
 *                 nothing, etc.).
 * @return 1 on success, 0 on failure.
 */
    int process(unsigned int keyCode, int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType);
/**
 * @brief Pass a key through without applying any filtering or conversion.
 *
 * Useful when the engine is temporarily disabled but the host still wishes
 * to feed events through the same interface.
 *
 * @param keyCode Code of the key to forward.
 */
    void pass(int keyCode); //just pass through without filtering
/**
 * @brief Enable "single mode" operation.
 *
 * In single mode the engine processes one character at a time, which is
 * useful for compatibility with certain input methods that don't support
 * multi‑key sequences.
 */
    void setSingleMode();

/**
 * @brief Handle a backspace key event.
 *
 * Adjusts the internal buffer appropriately and may request additional
 * backspaces or produce output to undo previous conversions.
 *
 * @param backs    [out] number of backspaces the caller should apply to the
 *                 document before inserting further text.
 * @param outBuf   [out] buffer for any text that must be re‑emitted.
 * @param outSize  [in,out] available space / bytes written.
 * @param outType  [out] output classification as with `process`.
 * @return 1 on success, 0 on failure.
 */
    int processBackspace(int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType);
/**
 * @brief Reset the engine state and clear the input buffer.
 *
 * This discards any pending composition and returns the engine to its initial
 * condition.  It is typically called when the input focus is changed or the
 * user explicitly resets the IME.
 */
    void reset();
/**
 * @brief Restore previously buffered keystrokes after a failed conversion.
 *
 * When a sequence is determined to be invalid or not subject to spell‑check,
 * this routine can replay the buffered keys so the host can insert them
 * literally.  Used by auto‑restore features.
 *
 * @param backs    [out] requested backspace count.
 * @param outBuf   [out] bytes to commit during restoration.
 * @param outSize  [in,out] space available / bytes produced.
 * @param outType  [out] classification of the output.
 * @return 1 on success.
 */
    int restoreKeyStrokes(int & backs, unsigned char *outBuf, int & outSize, UkOutputType & outType);

    //following methods must be public just to enable the use of pointers to them
    //they should not be called from outside.
/**
 * @brief Apply a tone diacritic to the most recent vowel in the buffer.
 *
 * The event describes which tone key was pressed; the engine updates its
 * internal representation accordingly.
 *
 * @param ev Key event triggering the tone insertion.
 * @return 1 if the tone was applied, 0 otherwise.
 */
    int processTone(UkKeyEvent & ev);
/**
 * @brief Handle a roof diacritic key (e.g. '^') on the current character.
 *
 * Updates vowel sequence information and may adjust output accordingly.
 *
 * @param ev Key event corresponding to a roof input.
 * @return 1 if handled.
 */
    int processRoof(UkKeyEvent & ev);
/**
 * @brief Process a hook or bowl diacritic (e.g. '`' or 'o') input.
 *
 * May call `processHookWithUO` for special cases.  Adjusts the engine state
 * to apply the diacritic to the appropriate character.
 *
 * @param ev Input event describing the hook key.
 * @return 1 if applied.
 */
    int processHook(UkKeyEvent & ev);
/**
 * @brief Append a raw character to the internal composition buffer.
 *
 * This helper is used when an input key does not correspond to a diacritic or
 * special rule; the event is simply buffered for later processing.
 *
 * @param ev Key event whose character should be appended.
 * @return 1 on success.
 */
    int processAppend(UkKeyEvent & ev);
/**
 * @brief Append a vowel character and update vowel sequence tracking.
 *
 * Called from `processAppend` when the appended character is recognized as a
 * Vietnamese vowel.  Handles special sequencing rules.
 *
 * @param ev Key event for the vowel character.
 * @return 1 on success.
 */
    int appendVowel(UkKeyEvent & ev);
/**
 * @brief Append a consonant character to the composition buffer.
 *
 * Updates state such as whether the last symbol was non‑Vietnamese.
 *
 * @param ev Key event representing the consonant.
 * @return 1 on success.
 */
    int appendConsonnant(UkKeyEvent & ev);
/**
 * @brief Handle the special "dd" combination that produces the letter đ/Đ.
 *
 * When the user types 'd' twice under certain rules this function maps the
 * sequence to the appropriate Vietnamese character.
 *
 * @param ev Key event corresponding to the second 'd'.
 * @return 1 if conversion occurred.
 */
    int processDd(UkKeyEvent & ev);
/**
 * @brief Map a character according to the user-defined key map table.
 *
 * Consults `m_pCtrl->usrKeyMap` and replaces the event with the mapped action
 * if one exists.
 *
 * @param ev Original key event; may be modified in-place.
 * @return 1 if mapping applied.
 */
    int processMapChar(UkKeyEvent & ev);
/**
 * @brief Handle Telex-style 'w' input rules used for certain diacritics.
 *
 * In Telex mode pressing 'w' after a vowel may add a diacritic or convert to
 * another letter; this routine encapsulates that behaviour.
 *
 * @param ev Key event for the 'w' character.
 * @return 1 if Telex transformation occurred.
 */
    int processTelexW(UkKeyEvent & ev);
/**
 * @brief Process an escape character which forces literal output.
 *
 * Certain input modes allow prefixing a keystroke with an escape to bypass the
 * normal conversion logic; this function handles such events.
 *
 * @param ev Escape key event received by the engine.
 * @return 1 if the escape was consumed.
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
 * @brief Specialised hook handling when preceding letter is 'u' or 'o'.
 *
 * On sequences such as "uo" or "uo" the rules for applying hooks/bowls are
 * slightly different; this helper encapsulates that logic.
 *
 * @param ev Key event for the hook character.
 * @return 1 if handled.
 */
    int processHookWithUO(UkKeyEvent & ev);
/**
 * @brief Check whether the current keystroke sequence matches a macro.
 *
 * If a macro is found and expansion is appropriate, the routine performs the
 * substitution in-place and returns a nonzero value so the caller can handle
 * the output according to the engine's macro rules.
 *
 * @param ev Key event to include in macro matching.
 * @return nonzero if a macro expansion occurred.
 */
    int macroMatch(UkKeyEvent & ev);
/**
 * @brief Mark the given buffer position as having been modified.
 *
 * Used internally to track where the last transformation occurred so that
 * subsequent processing can resume from the correct location.
 *
 * @param pos Index in the internal buffer to flag as changed.
 */
    void markChange(int pos);
/**
 * @brief Ensure the internal composition buffer has sufficient space.
 *
 * Allocates or shifts the buffer as necessary so that at least 10 entries are
 * available for new keystrokes.
 */
    void prepareBuffer(); //make sure we have a least 10 entries available
/**
 * @brief Flush pending output characters into the provided buffer.
 *
 * Copies any characters accumulated in the engine's internal output buffer to
 * the caller's buffer and updates the size accordingly.
 *
 * @param outBuf Destination buffer for output bytes.
 * @param outSize [in,out] available space on entry, bytes written on return.
 * @return number of bytes actually written.
 */
    int writeOutput(unsigned char *outBuf, int & outSize);
    //int getSeqLength(int first, int last);
/**
 * @brief Compute the number of intermediate steps between two buffer indices.
 *
 * Used for navigating vowel/consonant sequence structures during processing.
 *
 * @param first Starting index in the internal sequence array.
 * @param last  Ending index.
 * @return Step count between the two positions.
 */
    int getSeqSteps(int first, int last);
/**
 * @brief Locate the tone diacritic position within a vowel sequence.
 *
 * Takes a vowel sequence enum and returns the offset where a tone mark should
 * be inserted.  The `terminated` flag indicates whether the sequence is
 * considered complete.
 *
 * @param vs        Vowel sequence identifier.
 * @param terminated True if the sequence has been terminated by a consonant.
 * @return Offset index for the tone character, or -1 if none.
 */
    int getTonePosition(VowelSeq vs, bool terminated);
/**
 * @brief Clear and reinitialize the internal keystroke history buffer.
 *
 * Called when the engine needs to discard stored key events, e.g. on reset.
 */
    void resetKeyBuf();
/**
 * @brief Handle VIQR escape sequences for literal character entry.
 *
 * When VIQR mode is active, certain prefix characters (like '\\') instruct
 * the engine to treat the following keystroke literally.  This routine
 * examines the event and performs the necessary adjustment.
 *
 * @param ev Input key event to examine.
 * @return 1 if the event was consumed as an escape.
 */
    int checkEscapeVIQR(UkKeyEvent & ev);
/**
 * @brief Process a key event without applying Vietnamese spell‑checking rules.
 *
 * Used in modes where the user explicitly disables automatic corrections; the
 * input is buffered but not validated.
 *
 * @param ev Key event to process.
 * @return 1 if the key was added to the buffer.
 */
    int processNoSpellCheck(UkKeyEvent & ev);
/**
 * @brief Handle tasks that occur when a word boundary is reached.
 *
 * This may include macro triggering, spell-check lookups, and buffer
 * housekeeping.  It is typically invoked when a space or punctuation is
 * entered.
 *
 * @param ev Key event representing the boundary character.
 * @return 1 if processing succeeded.
 */
    int processWordEnd(UkKeyEvent & ev);
/**
 * @brief Synchronize the keystroke history buffer with the current state.
 *
 * Ensures that the internal record of recent key events matches the
 * position and content of `m_buffer`, which is important after operations
 * like macro expansion or restoration.
 */
    void synchKeyStrokeBuffer();
/**
 * @brief Determine if the last completed word contains any Vietnamese marks.
 *
 * Returns true when the previous word included a diacritic such as a tone or
 * roof, which affects input rules for following characters.
 */
    bool lastWordHasVnMark();
/**
 * @brief Check whether the last completed word consisted solely of non‑Vietnamese
 * characters.
 *
 * This is used to decide whether to enter Vietnamese processing when the
 * user resumes typing in ASCII.
 */
    bool lastWordIsNonVn();
};

/**
 * @brief Perform one-time initialization of global engine resources.
 *
 * This function sets up static tables and other data structures used by the
 * `UkEngine` class.  It should be called once during program startup.
 */
void SetupUnikeyEngine();

#endif
