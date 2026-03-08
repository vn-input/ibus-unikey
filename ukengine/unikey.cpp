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

// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/*------------------------------------------------------------------------------
UniKey - Open-source Vietnamese Keyboard
Copyright (C) 1998-2004 Pham Kim Long
Contact:
  longcz@yahoo.com
  http://unikey.sf.net

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

#include <ctype.h>
#include <memory.h>
#include <stdio.h>
#include <iostream>
#include "unikey.h"
#include "ukengine.h"
#include "usrkeymap.h"

using namespace std;

//---- exported variables for use in UkEnginge class ----
UkSharedMem *pShMem = 0;

UkEngine MyKbEngine;

int UnikeyCapsLockOn = 0;
int UnikeyShiftPressed = 0;
//----------------------------------------------------

unsigned char UnikeyBuf[1024];
int UnikeyBackspaces;
int UnikeyBufChars;
UkOutputType UnikeyOutput;

//--------------------------------------------
/**
 * @brief Selects the active input method for the Unikey engine.
 *
 * @param im Input method identifier.
 */
void UnikeySetInputMethod(UkInputMethod im)
{
  if (im == UkTelex || im == UkVni || im == UkSimpleTelex || im == UkSimpleTelex2) {
    pShMem->input.setIM(im);
    MyKbEngine.reset();
  }
  else if (im == UkUsrIM && pShMem->usrKeyMapLoaded) {
    //cout << "Switched to user mode\n"; //DEBUG
    pShMem->input.setIM(pShMem->usrKeyMap);
    MyKbEngine.reset();
  }

  //cout << "IM changed to: " << im << endl; //DEBUG
}


//--------------------------------------------
/**
 * @brief Updates keyboard case state used by the engine.
 *
 * @param shiftPressed Non-zero when Shift is currently pressed.
 * @param CapsLockOn Non-zero when Caps Lock is enabled.
 */
void UnikeySetCapsState(int shiftPressed, int CapsLockOn)
{
  //UnikeyCapsAll = (shiftPressed && !CapsLockOn) || (!shiftPressed && CapsLockOn);
  UnikeyCapsLockOn = CapsLockOn;
  UnikeyShiftPressed = shiftPressed;
}

//--------------------------------------------
/**
 * @brief Sets the output charset and resets composition state.
 *
 * @param charset Output charset identifier.
 * @return Always returns 1.
 */
int UnikeySetOutputCharset(int charset)
{
    pShMem->charsetId = charset;
    MyKbEngine.reset();
    return 1;
}

//--------------------------------------------
/**
 * @brief Applies runtime options to the shared engine state.
 *
 * @param pOpt Pointer to input options.
 */
void UnikeySetOptions(UnikeyOptions *pOpt)
{
  pShMem->options.freeMarking = pOpt->freeMarking;
  pShMem->options.modernStyle = pOpt->modernStyle;
  pShMem->options.macroEnabled = pOpt->macroEnabled;
  pShMem->options.useUnicodeClipboard = pOpt->useUnicodeClipboard;
  pShMem->options.alwaysMacro = pOpt->alwaysMacro;
  pShMem->options.spellCheckEnabled = pOpt->spellCheckEnabled;
  pShMem->options.autoNonVnRestore = pOpt->autoNonVnRestore;
}

//--------------------------------------------
/**
 * @brief Retrieves current runtime options.
 *
 * @param pOpt Output options structure.
 */
void UnikeyGetOptions(UnikeyOptions *pOpt)
{
  *pOpt = pShMem->options;
}

//--------------------------------------------
/**
 * @brief Populates an options structure with default values.
 *
 * @param pOpt Output options structure.
 */
void CreateDefaultUnikeyOptions(UnikeyOptions *pOpt)
{
  pOpt->freeMarking = 1;
  pOpt->modernStyle = 0;
  pOpt->macroEnabled = 0;
  pOpt->useUnicodeClipboard = 0;
  pOpt->alwaysMacro = 0;
  pOpt->spellCheckEnabled = 1;
  pOpt->autoNonVnRestore = 0;
}

//--------------------------------------------
/**
 * @brief Reports current Shift/CapsLock status to the caller.
 *
 * @param pShiftPressed Output Shift state.
 * @param pCapsLockOn Output Caps Lock state.
 */
void UnikeyCheckKbCase(int *pShiftPressed, int *pCapsLockOn)
{
  *pShiftPressed = UnikeyShiftPressed;
  *pCapsLockOn = UnikeyCapsLockOn;
}

//--------------------------------------------
/**
 * @brief Initializes shared state and the core Unikey engine.
 */
void UnikeySetup()
{
    SetupUnikeyEngine();
    pShMem = new UkSharedMem;
    pShMem->input.init();
    pShMem->macStore.init();
    pShMem->vietKey = 1;
    pShMem->usrKeyMapLoaded = 0;
    MyKbEngine.setCtrlInfo(pShMem);
    MyKbEngine.setCheckKbCaseFunc(&UnikeyCheckKbCase);
    UnikeySetInputMethod(UkTelex);
    UnikeySetOutputCharset(CONV_CHARSET_XUTF8);
    pShMem->initialized = 1;
    CreateDefaultUnikeyOptions(&pShMem->options);
}

//--------------------------------------------
/**
 * @brief Releases shared resources used by the Unikey engine.
 */
void UnikeyCleanup()
{
  delete pShMem;
}

//--------------------------------------------
/**
 * @brief Feeds one key input into the engine processing pipeline.
 *
 * @param ch Input key code.
 */
void UnikeyFilter(unsigned int ch)
{
  UnikeyBufChars = sizeof(UnikeyBuf);
  MyKbEngine.process(ch, UnikeyBackspaces, UnikeyBuf, UnikeyBufChars, UnikeyOutput);
}

//--------------------------------------------
/**
 * @brief Forwards one character directly to output without composition.
 *
 * @param ch Character code to pass through.
 */
void UnikeyPutChar(unsigned int ch)
{
  MyKbEngine.pass(ch);
  UnikeyBufChars = 0;
  UnikeyBackspaces = 0;
}

//--------------------------------------------
/**
 * @brief Clears current composition buffer state.
 */
void UnikeyResetBuf()
{
  MyKbEngine.reset();
}

//--------------------------------------------
/**
 * @brief Switches engine into single-character input mode.
 */
void UnikeySetSingleMode()
{
  MyKbEngine.setSingleMode();
}

//--------------------------------------------
/**
 * @brief Processes a backspace action through the engine.
 */
void UnikeyBackspacePress()
{
  UnikeyBufChars = sizeof(UnikeyBuf);
  MyKbEngine.processBackspace(UnikeyBackspaces, UnikeyBuf, UnikeyBufChars, UnikeyOutput);
  //  printf("Backspaces: %d\n",UnikeyBackspaces);
}

//--------------------------------------------
/**
 * @brief Loads macro definitions from a file.
 *
 * @param fileName Macro table file path.
 * @return Non-zero on success, 0 on failure.
 */
int UnikeyLoadMacroTable(const char *fileName)
{
  return pShMem->macStore.loadFromFile(fileName);
}

//--------------------------------------------
/**
 * @brief Loads a user-defined key mapping file.
 *
 * @param fileName User keymap file path.
 * @return 1 if loaded successfully, otherwise 0.
 */
int UnikeyLoadUserKeyMap(const char *fileName)
{
  if (UkLoadKeyMap(fileName, pShMem->usrKeyMap)) {
    //cout << "User key map loaded!\n"; //DEBUG
    pShMem->usrKeyMapLoaded = 1;
    return 1;
  }
  return 0;
}

//--------------------------------------------
/**
 * @brief Restores original keystrokes for current composed text.
 */
void UnikeyRestoreKeyStrokes()
{
    UnikeyBufChars = sizeof(UnikeyBuf);
    MyKbEngine.restoreKeyStrokes(UnikeyBackspaces, UnikeyBuf, UnikeyBufChars, UnikeyOutput);
}

/**
 * @brief Checks whether the engine is currently at word start.
 *
 * @return true when at a word boundary, otherwise false.
 */
bool UnikeyAtWordBeginning()
{
    return MyKbEngine.atWordBeginning();
}

