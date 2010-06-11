// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/*------------------------------------------------------------------------------
UniKey - Open-source Vietnamese Keyboard
Copyright (C) 2000-2005 Pham Kim Long
Contact:
  unikey@gmail.com
  http://unikey.org

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

#ifndef __UNIKEY_H
#define __UNIKEY_H

#include "keycons.h"

/*----------------------------------------------------
Initialization steps:
   1. UnikeySetup: This will initialized Unikey module,
      with default options, input method = TELEX, output format = UTF-8
   2. If you want a different settings:
     + Call UnikeySetInputMethod to change input method
     + Call UnikeySetOutputVIQR/UTF8 to chang output format
     + Call UnikeySetOptions to change extra options

Key event handling:

- Call UnikeyFilter when a key event occurs, examine results in
    + UnikeyBackspaces: number of backspaces that need to be sent
    + UnikeyBufChars: number of chars in buffer that need to be sent
    + UnikeyAnsiBuf: buffer containing output characters.
    + UnikeyUniBuf: not used

  You should also call UnikeySetCapsState() before calling UnikeyFilter.

  To make this module portable across platforms, UnikeyFilter should not
  be called on special keys: Enter, Tab, movement keys, delete, backspace...

- Special events:
    + Call UnikeyResetBuf to reset the engine's state in situations such as:
      focus lost, movement keys: arrow keys, pgup, pgdown....
    + If a backspace is received, call UnikeyBackspacePress,
      then examine the result:
      UnikeyBackspaces is the number of backspaces actually required to
      remove one character.

Clean up:
- When the Engine is no longer needed, call UnikeyCleanup
------------------------------------------------------*/

#if defined(__cplusplus)
extern "C" {
#endif
    extern unsigned char UnikeyBuf[];
    extern int UnikeyBackspaces;
    extern int UnikeyBufChars;
    extern UkOutputType UnikeyOutput;

  void UnikeySetup(); // always call this first
  void UnikeyCleanup(); // call this when unloading unikey module
  
  // call this to reset Unikey's state when focus, context is changed or
  // some control key is pressed
  void UnikeyResetBuf(); 

 // main handler, call every time a character input is received
  void UnikeyFilter(unsigned int ch);
  void UnikeyPutChar(unsigned int ch); // put new char without filtering

  // call this before UnikeyFilter for correctly processing some TELEX shortcuts
  void UnikeySetCapsState(int shiftPressed, int CapsLockOn);

 // call this when backspace is pressed
  void UnikeyBackspacePress();

  // call this to restore to original key strokes
  void UnikeyRestoreKeyStrokes();

 //set extra options
  void UnikeySetOptions(UnikeyOptions *pOpt); 
  void CreateDefaultUnikeyOptions(UnikeyOptions *pOpt);

  void UnikeyGetOptions(UnikeyOptions *pOpt);

  // set input method
  //   im: TELEX_INPUT, VNI_INPUT, VIQR_INPUT, VIQR_STAR_INPUT
  void UnikeySetInputMethod(UkInputMethod im);
  // set output format
  //  void UnikeySetOutputVIQR();
  // void UnikeySetOutputUTF8();
  int UnikeySetOutputCharset(int charset);

  int UnikeyLoadMacroTable(const char *fileName);
  int UnikeyLoadUserKeyMap(const char *fileName);

  //call this to enable typing vietnamese even in a non-vn sequence
  //e.g: GD&DDT,QDDND...
  //The engine will return to normal mode when a word-break occurs.
  void UnikeySetSingleMode();

  bool UnikeyAtWordBeginning();
#if defined(__cplusplus)
}
#endif

#endif
