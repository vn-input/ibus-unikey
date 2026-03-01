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
 * @file engine_private.h
 * @brief Public declarations for engine_private.h.
 */
#ifndef __ENGINE_PRIVATE_H__
#define __ENGINE_PRIVATE_H__

/**
 * @file engine_private.h
 * @brief Internal declarations for the IBus Unikey engine implementation.
 */

#include <string>
#include <ibus.h>
#include "unikey.h"
#include "vnconv.h"

/**
 * @typedef IBusUnikeyEngine
 * @brief Opaque type for the Unikey engine instance.
 */
typedef struct _IBusUnikeyEngine       IBusUnikeyEngine;
/**
 * @typedef IBusUnikeyEngineClass
 * @brief Opaque type for the Unikey engine class structure.
 */
typedef struct _IBusUnikeyEngineClass  IBusUnikeyEngineClass;

/**
 * @struct _IBusUnikeyEngine
 * @brief Instance data for the Unikey IBus engine object.
 */
struct _IBusUnikeyEngine
{
    IBusEngine parent;

    /* members */
    IBusPropList* prop_list; /**< list of engine properties exposed to UI */

    UkInputMethod im; /**< current input method selection */
    unsigned int  oc; /**< output charset identifier */
    UnikeyOptions ukopt; /**< engine options structure */
    gboolean process_w_at_begin; /**< whether to treat 'w' specially at word start */

    gboolean last_key_with_shift; /**< last key event included Shift modifier */

    std::string* preeditstr; /**< current preedit buffer string */
};

/**
 * @struct _IBusUnikeyEngineClass
 * @brief Class structure for the Unikey engine GObject.
 */
struct _IBusUnikeyEngineClass
{
    IBusEngineClass parent;
};

// prototypes for internal engine routines
/**
 * @brief GType class initialization callback.
 * @param kclass Class pointer being initialized.
 */
static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass* kclass);
/**
 * @brief GType instance initialization callback.
 * @param unikey Newly created engine instance.
 */
static void ibus_unikey_engine_init(IBusUnikeyEngine* unikey);

/**
 * @brief Custom constructor for the Unikey engine
 * @param type Parameter.
 * @param n_construct_params Parameter.
 * @param construct_params Parameter.
 */
static GObject* ibus_unikey_engine_constructor(GType type,
                                               guint n_construct_params,
                                               GObjectConstructParam* construct_params);

/**
 * @brief Cleanup invoked when the Unikey engine object is destroyed.
 * @param unikey Parameter.
 */
static void ibus_unikey_engine_destroy(IBusUnikeyEngine* unikey);
/**
 * @brief Primary key event handler called by IBus.
 * @param engine Parameter.
 * @param keyval Parameter.
 * @param keycode Parameter.
 * @param modifiers Parameter.
 */
static gboolean ibus_unikey_engine_process_key_event(IBusEngine* engine,
                                                     guint keyval,
                                                     guint keycode,
                                                     guint modifiers);

/**
 * @brief Called when engine receives input focus.
 * @param engine Parameter.
 */
static void ibus_unikey_engine_focus_in(IBusEngine* engine);
/**
 * @brief Called when engine loses input focus.
 * @param engine Parameter.
 */
static void ibus_unikey_engine_focus_out(IBusEngine* engine);
/**
 * @brief Reset internal engine state (e.g. preedit) to initial.
 * @param engine Parameter.
 */
static void ibus_unikey_engine_reset(IBusEngine* engine);
/**
 * @brief Enable the engine (e.g. when user activates it).
 * @param engine Parameter.
 */
static void ibus_unikey_engine_enable(IBusEngine* engine);
/**
 * @brief Disable the engine (e.g. when user switches away).
 * @param engine Parameter.
 */
static void ibus_unikey_engine_disable(IBusEngine* engine);
/**
 * @brief Load persisted configuration values into the engine instance.
 * @param unikey Parameter.
 */
static void ibus_unikey_engine_load_config(IBusUnikeyEngine* unikey);
/**
 * @brief Callback invoked when a configuration key changes.
 * @param name Parameter.
 * @param user_data Parameter.
 */
static void ibus_unikey_config_value_changed(gchar* name, gpointer user_data);
/**
 * @brief Handler for activation of engine ui properties.
 * @param engine Parameter.
 * @param prop_name Parameter.
 * @param prop_state Parameter.
 */
static void ibus_unikey_engine_property_activate(IBusEngine* engine,
                                                 const gchar* prop_name,
                                                 guint prop_state);

/**
 * @brief Process key event when preedit buffer is active.
 * @param engine Parameter.
 * @param keyval Parameter.
 * @param keycode Parameter.
 * @param modifiers Parameter.
 */
static gboolean ibus_unikey_engine_process_key_event_preedit(IBusEngine* engine,
                                                             guint keyval,
                                                             guint keycode,
                                                             guint modifiers);

/**
 * @brief Build the engine's property list structure used by the GUI.
 * @param unikey Parameter.
 */
static void ibus_unikey_engine_create_property_list(IBusUnikeyEngine* unikey);

/**
 * @brief Helper to update the preedit string shown to the user.
 * @param engine Parameter.
 * @param string Parameter.
 * @param visible Parameter.
 */
static void ibus_unikey_engine_update_preedit_string(IBusEngine *engine, const gchar *string, gboolean visible);
/**
 * @brief Erase @p num_chars characters from the preedit buffer.
 * @param engine Parameter.
 * @param num_chars Parameter.
 */
static void ibus_unikey_engine_erase_chars(IBusEngine *engine, int num_chars);

/**
 * @brief Convert Latin-1 bytes to UTF-8, storing result in @p dst.
 *
 * @param dst Destination buffer (should be large enough).
 * @param src Source Latin-1 data.
 * @param inSize Number of input bytes.
 * @param[out] pOutSize Receives number of bytes written to dst.
 * @return 0 on success, negative on error.
 */
static int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize);

#endif

