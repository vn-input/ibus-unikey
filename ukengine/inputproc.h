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
#ifndef __UK_INPUT_PROCESSOR_H
#define __UK_INPUT_PROCESSOR_H

/**
 * @file inputproc.h
 * @brief Definitions for keyboard input classification and mapping.
 */

#include "keycons.h"
#include "vnlexi.h"

#if defined(_WIN32)
    #define DllExport   __declspec( dllexport )
    #define DllImport   __declspec( dllimport )
    #if defined(UNIKEYHOOK)
        #define DllInterface   __declspec( dllexport )
    #else
        #define DllInterface   __declspec( dllimport )
    #endif
#else
    #define DllInterface //not used
    #define DllExport
    #define DllImport
#endif

/**
 * @enum UkKeyEvName
 * @brief Classification of key events recognized by the input processor.
 *
 * Values represent roof/hook/tone markers, special mappings and other
 * character categories used during Vietnamese input method processing.
 */
enum UkKeyEvName {
  vneRoofAll, /**< roof diacritic (all vowels) */
  vneRoof_a,  /**< roof on 'a' */
  vneRoof_e,  /**< roof on 'e' */
  vneRoof_o,  /**< roof on 'o' */
  vneHookAll, /**< hook diacritic (all vowels) */
  vneHook_uo, /**< hook for 'u' or 'o' */
  vneHook_u,  /**< hook on 'u' */
  vneHook_o,  /**< hook on 'o' */
  vneBowl,    /**< bowl character (Ă/ă) */
  vneDd,      /**< đ character */
  vneTone0,   /**< neutral tone */
  vneTone1,   /**< acute tone */
  vneTone2,   /**< grave tone */
  vneTone3,   /**< hook tone */
  vneTone4,   /**< tilde tone */
  vneTone5,   /**< dot tone */
  vne_telex_w, /**< special Telex mapping for 'w' */
  vneMapChar, /**< map bracket keys to 'ư'/'ơ' */
  vneEscChar, /**< escape character event */
  vneNormal,  /**< no special category */
  vneCount    /**< number of event types */
};

/**
 * @enum UkCharType
 * @brief Character classification used during input processing.
 */
enum UkCharType {
  ukcVn,       /**< Vietnamese character */
  ukcWordBreak,/**< delimiter or word-breaking symbol */
  ukcNonVn,    /**< non-Vietnamese character */
  ukcReset     /**< reset state indicator */
};

/**
 * @struct UkKeyEvent
 * @brief Representation of a processed key stroke for the engine.
 *
 * Fields are filled by the input processor to describe the event type,
 * character classification, associated Vietnamese symbol and any tone.
 */
struct UkKeyEvent {
  int evType;         /**< event type (UkKeyEvName) */
  UkCharType chType;  /**< character classification */
  VnLexiName vnSym;   /**< Vietnamese symbol (valid if chType==ukcVn) */
  unsigned int keyCode; /**< raw keycode from keyboard */
  int tone;           /**< tone value, meaningful for vowels */
};

/**
 * @struct UkKeyMapping
 * @brief Simple mapping from an ASCII key to a UkKeyEvName or symbol index.
 */
struct UkKeyMapping {
    unsigned char key; /**< input key character */
    int action;        /**< mapped action/event code */
};

///////////////////////////////////////////
/**
 * @class UkInputProcessor
 * @brief Encapsulates keyboard input classification and mapping logic.
 *
 * The processor converts raw keycodes into `UkKeyEvent` structures and
 * maintains the current input method (Telex, VNI, etc.).  Initialization
 * must be performed via `init()` rather than the constructor to support
 * placement in shared memory.
 */
class UkInputProcessor {

public:
  /**
   * @brief Initialize table and defaults.
   */
  void init();

  /**
   * @brief Retrieve current input method.
   */
  UkInputMethod getIM()
  {
    return m_im;
  }

  /**
   * @brief Translate a keycode into a key event.
   * @param keyCode Raw keycode value.
   * @param[out] ev Event structure to populate.
   */
  void keyCodeToEvent(unsigned int keyCode, UkKeyEvent & ev);

  /**
   * @brief Convert keycode to a symbolic representation (e.g. letter).
   */
  void keyCodeToSymbol(unsigned int keyCode, UkKeyEvent & ev);

  /**
   * @brief Select a predefined input method.
   * @return 0 on success, negative on error.
   */
  int setIM(UkInputMethod im);

  /**
   * @brief Provide custom key map array (256 entries).
   */
  int setIM(int map[256]);

  /**
   * @brief Export current key map.
   */
  void getKeyMap(int map[256]);

  /**
   * @brief Classify raw keycode as UkCharType.
   */
  UkCharType getCharType(unsigned int keyCode);

protected:
  static bool m_classInit; /**< whether classifier tables are initialized */

  UkInputMethod m_im;       /**< currently selected input method */
  int m_keyMap[256];        /**< key mapping table */

  void useBuiltIn(UkKeyMapping *map); /**< load built-in mapping */

};

void UkResetKeyMap(int keyMap[256]);
void SetupInputClassifierTable();

DllInterface extern UkKeyMapping TelexMethodMapping[];
DllInterface extern UkKeyMapping SimpleTelexMethodMapping[];
DllInterface extern UkKeyMapping VniMethodMapping[];
DllInterface extern UkKeyMapping VIQRMethodMapping[];
DllInterface extern UkKeyMapping MsViMethodMapping[];

extern VnLexiName IsoVnLexiMap[];
/**
 * @brief Maps an ISO key code to the lexical category used by Unikey.
 *
 * @param keyCode Raw key code.
 * @return Lexical classification for the key.
 */
inline VnLexiName IsoToVnLexi(unsigned int keyCode)
{
    return (keyCode >= 256)? vnl_nonVnChar : IsoVnLexiMap[keyCode];
}

#endif
