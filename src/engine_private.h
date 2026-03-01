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

typedef struct _IBusUnikeyEngine       IBusUnikeyEngine;
typedef struct _IBusUnikeyEngineClass  IBusUnikeyEngineClass;

struct _IBusUnikeyEngine
{
    IBusEngine parent;

/* members */
    IBusPropList* prop_list;

    UkInputMethod im; // input method
    unsigned int  oc; // output charset
    UnikeyOptions ukopt;
    gboolean process_w_at_begin;

    gboolean last_key_with_shift;

    std::string* preeditstr;
};

struct _IBusUnikeyEngineClass
{
    IBusEngineClass parent;
};

// prototype
static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass* kclass);
static void ibus_unikey_engine_init(IBusUnikeyEngine* unikey);

static GObject* ibus_unikey_engine_constructor(GType type,
                                               guint n_construct_params,
                                               GObjectConstructParam* construct_params);

static void ibus_unikey_engine_destroy(IBusUnikeyEngine* unikey);
static gboolean ibus_unikey_engine_process_key_event(IBusEngine* engine,
                                                     guint keyval,
                                                     guint keycode,
                                                     guint modifiers);

static void ibus_unikey_engine_focus_in(IBusEngine* engine);
static void ibus_unikey_engine_focus_out(IBusEngine* engine);
static void ibus_unikey_engine_reset(IBusEngine* engine);
static void ibus_unikey_engine_enable(IBusEngine* engine);
static void ibus_unikey_engine_disable(IBusEngine* engine);
static void ibus_unikey_engine_load_config(IBusUnikeyEngine* unikey);
static void ibus_unikey_config_value_changed(gchar* name, gpointer user_data);
static void ibus_unikey_engine_property_activate(IBusEngine* engine,
                                                 const gchar* prop_name,
                                                 guint prop_state);

static gboolean ibus_unikey_engine_process_key_event_preedit(IBusEngine* engine,
                                                             guint keyval,
                                                             guint keycode,
                                                             guint modifiers);

static void ibus_unikey_engine_create_property_list(IBusUnikeyEngine* unikey);

static void ibus_unikey_engine_update_preedit_string(IBusEngine *engine, const gchar *string, gboolean visible);
static void ibus_unikey_engine_erase_chars(IBusEngine *engine, int num_chars);

static int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize);

#endif

