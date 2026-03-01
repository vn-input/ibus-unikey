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
 * @file engine.cpp
 * @brief Implementation for engine.cpp.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <libintl.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <string.h>
#include <ibus.h>

#include "unikey.h"
#include "vnconv.h"

#include "engine_private.h"
#include "unikey_config.h"

#define _(string) gettext(string)

/**
 * @file engine.cpp
 * @brief Main IBus engine implementation for unikey.
 *
 * Contains the GObject-based engine class, property handling, preedit logic,
 * and key event processing pipeline.
 */

#define CONVERT_BUF_SIZE 1024

static unsigned char WordBreakSyms[] =
{
    ',', ';', ':', '.', '\"', '\'', '!', '?', ' ',
    '<', '>', '=', '+', '-', '*', '/', '\\',
    '_', '~', '`', '@', '#', '$', '%', '^', '&', '(', ')', '{', '}', '[', ']',
    '|'
};

static IBusEngineClass* parent_class = NULL;

static IBusUnikeyEngine* unikey; // current (focus) unikey engine

/**
 * @brief Return the GType for the Unikey engine class.
 *
 * This implements the boilerplate required by the GObject type system.
 * @return `GType` for IBusUnikeyEngine.
 */
GType ibus_unikey_engine_get_type(void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof(IBusUnikeyEngineClass),
        (GBaseInitFunc)NULL,
        (GBaseFinalizeFunc)NULL,
        (GClassInitFunc)ibus_unikey_engine_class_init,
        NULL,
        NULL,
        sizeof(IBusUnikeyEngine),
        0,
        (GInstanceInitFunc)ibus_unikey_engine_init,
    };

    if (type == 0)
    {
        type = g_type_register_static(IBUS_TYPE_ENGINE,
                                      "IBusUnikeyEngine",
                                      &type_info,
                                      (GTypeFlags)0);
    }

    return type;
}

/**
 * @brief Perform module-level initialization when the engine is loaded.
 *
 * @param bus IBus bus connection passed by the caller.
 */
void ibus_unikey_init(IBusBus* bus)
{
    UnikeySetup();
    ibus_unikey_config_init();

    ibus_unikey_config_on_changed(ibus_unikey_config_value_changed, NULL);
}

/**
 * @brief Clean up resources when the engine is unloaded.
 */
void ibus_unikey_exit()
{
    UnikeyCleanup();
}

/**
 * @brief Class initializer invoked by the GType system.
 *
 * Sets up the engine class virtual methods and property hooks.
 *
 * @param klass Engine class object being initialized.
 */
static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass* klass)
{
    GObjectClass* object_class         = G_OBJECT_CLASS(klass);
    IBusObjectClass* ibus_object_class = IBUS_OBJECT_CLASS(klass);
    IBusEngineClass* engine_class      = IBUS_ENGINE_CLASS(klass);

    parent_class = (IBusEngineClass* )g_type_class_peek_parent(klass);

    object_class->constructor = ibus_unikey_engine_constructor;
    ibus_object_class->destroy = (IBusObjectDestroyFunc)ibus_unikey_engine_destroy;

    engine_class->process_key_event = ibus_unikey_engine_process_key_event;
    engine_class->reset             = ibus_unikey_engine_reset;
    engine_class->enable            = ibus_unikey_engine_enable;
    engine_class->disable           = ibus_unikey_engine_disable;
    engine_class->focus_in          = ibus_unikey_engine_focus_in;
    engine_class->focus_out         = ibus_unikey_engine_focus_out;
    engine_class->property_activate = ibus_unikey_engine_property_activate;
}

/**
 * @brief Instance initializer for new engine objects.
 *
 * @param unikey Newly-allocated engine instance.
 */
static void ibus_unikey_engine_init(IBusUnikeyEngine* unikey)
{
    ibus_unikey_engine_load_config(unikey);

    UnikeySetInputMethod(unikey->im);
    UnikeySetOutputCharset(unikey->oc);
    UnikeySetOptions(&unikey->ukopt);

    unikey->preeditstr = new std::string();
    ibus_unikey_engine_create_property_list(unikey);
}

/**
 * @brief Locate a property by key within an IBusPropList.
 *
 * @param list Property list to search.
 * @param key String key name to match.
 * @return Matching IBusProperty pointer or NULL if not found.
 */
static IBusProperty* find_prop_from_list(IBusPropList* list, const char* key)
{
    for (guint i = 0; i < list->properties->len ; i++)
    {
        IBusProperty* prop = ibus_prop_list_get(list, i);
        if (prop == NULL)
            return NULL;
        if (strcmp(ibus_property_get_key(prop), key) == 0)
            return prop;
    }
    return NULL;
}

/**
 * @brief Synchronize UI property objects with current engine options.
 *
 * @param unikey Engine instance whose properties are being updated.
 */
static void ibus_unikey_engine_update_property_list(IBusUnikeyEngine* unikey)
{
    bool b;
    IBusProperty* prop;

    b = unikey->ukopt.spellCheckEnabled;
    prop = find_prop_from_list(unikey->prop_list, CONFIG_SPELLCHECK);
    if (prop != NULL)
    {
        ibus_property_set_state(prop,
                (b == 1) ? PROP_STATE_CHECKED:PROP_STATE_UNCHECKED);
    }

    b = unikey->ukopt.autoNonVnRestore;
    prop = find_prop_from_list(unikey->prop_list, CONFIG_AUTORESTORENONVN);
    if (prop != NULL)
    {
        ibus_property_set_state(prop,
                (b == 1) ? PROP_STATE_CHECKED:PROP_STATE_UNCHECKED);
    }

    b = unikey->ukopt.macroEnabled;
    prop = find_prop_from_list(unikey->prop_list, CONFIG_MACROENABLED);
    if (prop != NULL)
    {
        ibus_property_set_state(prop,
                (b == 1) ? PROP_STATE_CHECKED:PROP_STATE_UNCHECKED);
    }
}

/**
 * @brief Load saved configuration values into the engine instance.
 *
 * @param unikey Engine instance to populate from persisted settings.
 */
static void ibus_unikey_engine_load_config(IBusUnikeyEngine* unikey)
{
    gchar* str;
    gboolean b;

    auto im = input_method_map.at("telex").first;
    if (ibus_unikey_config_get_string(CONFIG_INPUTMETHOD, &str))
    {
        auto p = input_method_map.at(std::string(str));
        im = p.first;
        g_free(str);
    }
    unikey->im = im;

    auto oc = output_charset_map.at("unicode").first;
    if (ibus_unikey_config_get_string(CONFIG_OUTPUTCHARSET, &str))
    {
        auto p = output_charset_map.at(std::string(str));
        oc = p.first;
        g_free(str);
    }
    unikey->oc = oc;

    if (ibus_unikey_config_get_boolean(CONFIG_FREEMARKING, &b))
        unikey->ukopt.freeMarking = b;

    if (ibus_unikey_config_get_boolean(CONFIG_MODERNSTYLE, &b))
        unikey->ukopt.modernStyle = b;

    if (ibus_unikey_config_get_boolean(CONFIG_MACROENABLED, &b))
        unikey->ukopt.macroEnabled = b;

    if (ibus_unikey_config_get_boolean(CONFIG_SPELLCHECK, &b))
        unikey->ukopt.spellCheckEnabled = b;

    if (ibus_unikey_config_get_boolean(CONFIG_AUTORESTORENONVN, &b))
        unikey->ukopt.autoNonVnRestore = b;

    if (ibus_unikey_config_get_boolean(CONFIG_STANDALONEW, &b))
        unikey->process_w_at_begin = b;

    // load macro
    gchar* fn = get_macro_file();
    UnikeyLoadMacroTable(fn);
    g_free(fn);
}

/**
 * @brief Custom GObject constructor override for the engine.
 *
 * Used to intercept construction and perform any engine-specific setup.
 *
 * @param type GType of object being constructed (should be IBusUnikeyEngine).
 * @param n_construct_params Number of construction parameters.
 * @param construct_params Array of construction parameters.
 * @return Newly created GObject instance.
 */
static GObject* ibus_unikey_engine_constructor(GType type,
                                               guint n_construct_params,
                                               GObjectConstructParam* construct_params)
{
    IBusUnikeyEngine* unikey;

    unikey = (IBusUnikeyEngine*)
        G_OBJECT_CLASS(parent_class)->constructor(type,
                                                  n_construct_params,
                                                  construct_params);

    return (GObject*)unikey;
}

/**
 * @brief Destructor called when an engine object is freed.
 *
 * Releases allocated memory and forwards call to parent class.
 *
 * @param unikey Engine instance being destroyed.
 */
static void ibus_unikey_engine_destroy(IBusUnikeyEngine* unikey)
{
    delete unikey->preeditstr;
    g_object_unref(unikey->prop_list);

    IBUS_OBJECT_CLASS(parent_class)->destroy((IBusObject*)unikey);
}

/**
 * @brief Reset the internal preedit buffer of @p engine.
 *
 * Clears the displayed preedit text and resets the Unikey internal buffer.
 *
 * @param engine IBus engine instance whose buffer should be reset.
 */
static void ibus_unikey_buffer_reset(IBusEngine* engine)
{
    unikey = (IBusUnikeyEngine*)engine;

    ibus_engine_hide_preedit_text(engine);
    unikey->preeditstr->clear();
    UnikeyResetBuf();
}

/**
 * @brief Commit current preedit buffer content to IBus.
 *
 * Sends the contents of @p engine's preedit string as committed text to the
 * client application and then resets the buffer.
 *
 * @param engine IBus engine instance performing the commit.
 */
static void ibus_unikey_buffer_commit(IBusEngine* engine)
{
    unikey = (IBusUnikeyEngine*)engine;

    if (unikey->preeditstr->length() > 0)
    {
        IBusText *text;
        text = ibus_text_new_from_static_string(unikey->preeditstr->c_str());
        ibus_engine_commit_text(engine, text);
    }

    ibus_unikey_buffer_reset(engine);
}

/**
 * @brief Handler invoked when the engine gets input focus.
 *
 * Registers engine properties with the IBus daemon.
 *
 * @param engine IBus engine gaining focus.
 */
static void ibus_unikey_engine_focus_in(IBusEngine* engine)
{

    unikey = (IBusUnikeyEngine*)engine;
    ibus_engine_register_properties(engine, unikey->prop_list);

    parent_class->focus_in(engine);
}

/**
 * @brief Handler invoked when the engine loses focus.
 *
 * Clears the preedit buffer.
 *
 * @param engine IBus engine losing focus.
 */
static void ibus_unikey_engine_focus_out(IBusEngine* engine)
{

    ibus_unikey_buffer_reset(engine);
    parent_class->focus_out(engine);
}

/**
 * @brief Reset state such as preedit and input method for @p engine.
 *
 * This is invoked when the engine receives a reset request from IBus.
 *
 * @param engine IBus engine instance to reset.
 */
static void ibus_unikey_engine_reset(IBusEngine* engine)
{

    ibus_unikey_buffer_reset(engine);
    parent_class->reset(engine);
}

/**
 * @brief Enable input processing on @p engine (called when engine activated).
 *
 * @param engine IBus engine to enable.
 */
static void ibus_unikey_engine_enable(IBusEngine* engine)
{

    parent_class->enable(engine);
}

/**
 * @brief Disable input processing on @p engine (called when engine deactivated).
 *
 * @param engine IBus engine to disable.
 */
static void ibus_unikey_engine_disable(IBusEngine* engine)
{

    parent_class->disable(engine);
}

/**
 * @brief Callback invoked when a configuration key changes.
 *
 * Reloads the config into the current engine and updates properties.
 *
 * @param name Name of the changed setting.
 * @param user_data Opaque pointer formerly passed to registration (unused).
 */
static void ibus_unikey_config_value_changed(gchar* name, gpointer user_data)
{

    ibus_unikey_engine_load_config(unikey);

    UnikeySetInputMethod(unikey->im);
    UnikeySetOutputCharset(unikey->oc);
    UnikeySetOptions(&unikey->ukopt);

    ibus_unikey_engine_update_property_list(unikey);
}

/**
 * @brief Handler for activation of a property in the UI.
 *
 * Properties include toggles for spellcheck, macros, etc.
 *
 * @param engine Engine instance receiving the activation.
 * @param prop_name Key of the property.
 * @param prop_state New state value of the property.
 */
static void ibus_unikey_engine_property_activate(IBusEngine* engine,
                                                 const gchar* prop_name,
                                                 guint prop_state)
{

    unikey = (IBusUnikeyEngine*)engine;

    if (strcmp(prop_name, "more-settings") == 0)
    {
        int ret = 0;

        ret = system(LIBEXECDIR "/ibus-setup-unikey &");
        if (ret == -1)
        {
	    g_print("Failed to open ibus-setup-unikey");
        }
        return;
    }

    if (strcmp(prop_name, CONFIG_SPELLCHECK) == 0)
    {
        unikey->ukopt.spellCheckEnabled = prop_state > 0;
        ibus_unikey_config_set_boolean(prop_name, prop_state > 0);
    }
/**
 * @brief if.
 *
 * @param CONFIG_AUTORESTORENONVN Parameter.
 * @return Return value.
 */
    else if (strcmp(prop_name, CONFIG_AUTORESTORENONVN) == 0)
    {
        unikey->ukopt.autoNonVnRestore = prop_state > 0;
        ibus_unikey_config_set_boolean(prop_name, prop_state > 0);
    }
/**
 * @brief if.
 *
 * @param CONFIG_MACROENABLED Parameter.
 * @return Return value.
 */
    else if (strcmp(prop_name, CONFIG_MACROENABLED) == 0)
    {
        unikey->ukopt.macroEnabled = prop_state > 0;
        ibus_unikey_config_set_boolean(prop_name, prop_state > 0);
    }
}

/**
 * @brief Create and initialize the engine property list.
 *
 * Populates toggle and action properties exposed in the language bar menu.
 *
 * @param unikey Engine instance whose property list should be created.
 */
static void ibus_unikey_engine_create_property_list(IBusUnikeyEngine* unikey)
{
    IBusProperty* prop;
    IBusText* label;

    if (unikey->prop_list != NULL)
        return;
    unikey->prop_list = ibus_prop_list_new();
    g_object_ref_sink(unikey->prop_list);

    // spellcheck property
    label = ibus_text_new_from_static_string(_("Enable spell check"));
    prop = ibus_property_new(CONFIG_SPELLCHECK,
                             PROP_TYPE_TOGGLE,
                             label,
                             "",
                             NULL,
                             TRUE,
                             TRUE,
                             (unikey->ukopt.spellCheckEnabled==1)?
                             PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                             NULL);
    if (ibus_prop_list_update_property(unikey->prop_list, prop) == false)
        ibus_prop_list_append(unikey->prop_list, prop);

    // auto restore property
    label = ibus_text_new_from_static_string(_("Auto restore non Vietnamese word"));
    prop = ibus_property_new(CONFIG_AUTORESTORENONVN,
                             PROP_TYPE_TOGGLE,
                             label,
                             "",
                             NULL,
                             TRUE,
                             TRUE,
                             (unikey->ukopt.autoNonVnRestore==1)?
                             PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                             NULL);
    if (ibus_prop_list_update_property(unikey->prop_list, prop) == false)
        ibus_prop_list_append(unikey->prop_list, prop);

    // macroEnabled property
    label = ibus_text_new_from_static_string(_("Enable Macro"));
    prop = ibus_property_new(CONFIG_MACROENABLED,
                             PROP_TYPE_TOGGLE,
                             label,
                             "",
                             NULL,
                             TRUE,
                             TRUE,
                             (unikey->ukopt.macroEnabled==1)?
                             PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                             NULL);
    if (ibus_prop_list_update_property(unikey->prop_list, prop) == false)
        ibus_prop_list_append(unikey->prop_list, prop);

    // more setting property
    label = ibus_text_new_from_static_string(_("More settings..."));
    prop = ibus_property_new("more-settings",
                             PROP_TYPE_NORMAL,
                             label,
                             "",
                             NULL,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);
    if (ibus_prop_list_update_property(unikey->prop_list, prop) == false)
        ibus_prop_list_append(unikey->prop_list, prop);
}

/**
 * @brief Update the preedit string shown to the user.
 *
 * Underlines the text and optionally displays it.
 *
 * @param engine Engine instance.
 * @param string UTF-8 text to present.
 * @param visible Whether the preedit should be visible.
 */
static void ibus_unikey_engine_update_preedit_string(IBusEngine *engine, const gchar *string, gboolean visible)
{

    IBusText *text;

    text = ibus_text_new_from_static_string(string);

    // underline text
    ibus_text_append_attribute(text, IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE, 0, -1);

    // update and display text
    ibus_engine_update_preedit_text_with_mode(engine, text, ibus_text_get_length(text), visible, IBUS_ENGINE_PREEDIT_COMMIT);
}

/**
 * @brief Erase `count` characters from the preedit string.
 *
 * Properly handles UTF-8 multi-byte sequences.
 *
 * @param engine Engine whose buffer is modified.
 * @param count Number of characters to remove.
 */
static void ibus_unikey_engine_erase_chars(IBusEngine *engine, int count)
{

    int i = unikey->preeditstr->length();

    while (i > 0 && count > 0) {
        unsigned char code = unikey->preeditstr->at(i-1);

        // count down if code is the first byte of utf-8 char
        // REF: http://en.wikipedia.org/wiki/UTF-8
        if (code >> 6 != 2) { // ignore 10xxxxxx
            count--;
        }
        i--;
    }
    unikey->preeditstr->erase(i);
}

// code from x-unikey, for convert charset that not is XUtf-8
/**
 * @brief Convert ISO‑8859-1 (Latin-1) bytes to UTF-8.
 *
 * @param dst Destination buffer (must be large enough to hold output).
 * @param src Source Latin-1 byte sequence.
 * @param inSize Number of input bytes.
 * @param[out] pOutSize On entry size of @p dst; on return bytes remaining.
 * @return nonzero if conversion succeeded (destination had enough room), 0 otherwise.
 */
int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize)
{

    int i;
    int outLeft;
    unsigned char ch;

    outLeft = *pOutSize;

    for (i=0; i<inSize; i++)
    {
        ch = *src++;
        if (ch < 0x80)
        {
            outLeft -= 1;
            if (outLeft >= 0)
                *dst++ = ch;
        }
        else
        {
            outLeft -= 2;
            if (outLeft >= 0)
            {
                *dst++ = (0xC0 | ch >> 6);
                *dst++ = (0x80 | (ch & 0x3F));
            }
        }
    }

    *pOutSize = outLeft;
    return (outLeft >= 0);
}


/**
 * @brief Process a key event, routing through preedit handling.
 *
 * This is the primary entry point IBus invokes for keyboard events.
 *
 * @param engine Engine instance receiving the key.
 * @param keyval Unicode code point of key pressed.
 * @param keycode Hardware keycode.
 * @param modifiers Modifier mask (Shift/Ctrl/Alt, etc.).
 * @return TRUE if event was handled, FALSE to propagate.
 */
static gboolean ibus_unikey_engine_process_key_event(IBusEngine* engine,
                                                     guint keyval,
                                                     guint keycode,
                                                     guint modifiers)
{

    static gboolean tmp;

    unikey = (IBusUnikeyEngine*)engine;

    tmp = ibus_unikey_engine_process_key_event_preedit(engine, keyval, keycode, modifiers);

    // check last keyevent with shift
    if (keyval >= IBUS_space && keyval <=IBUS_asciitilde)
    {
        unikey->last_key_with_shift = modifiers & IBUS_SHIFT_MASK;
    }
    else
    {
        unikey->last_key_with_shift = false;
    } // end check last keyevent with shift

    return tmp;
}

/**
 * @brief Internal preedit-specific key event processor.
 *
 * Called by the public process_key_event to handle editing state.
 *
 * @param engine Engine instance.
 * @param keyval Unicode key value.
 * @param keycode Physical keycode.
 * @param modifiers Modifier flags.
 * @return TRUE if event consumed, FALSE otherwise.
 */
static gboolean ibus_unikey_engine_process_key_event_preedit(IBusEngine* engine,
                                                             guint keyval,
                                                             guint keycode,
                                                             guint modifiers)
{

    if (modifiers & IBUS_RELEASE_MASK)
    {
        return false;
    }

/**
 * @brief if.
 *
 * @param keyval Parameter.
 * @return Return value.
 */
    else if (modifiers & IBUS_CONTROL_MASK
             || modifiers & IBUS_MOD1_MASK // alternate mask
             || keyval == IBUS_Control_L
             || keyval == IBUS_Control_R
             || keyval == IBUS_Tab
             || keyval == IBUS_Return
             || keyval == IBUS_Delete
             || keyval == IBUS_KP_Enter
             || (keyval >= IBUS_Home && keyval <= IBUS_Insert)
             || (keyval >= IBUS_KP_Home && keyval <= IBUS_KP_Delete)
        )
    {
        ibus_unikey_buffer_commit(engine);
        return false;
    }

/**
 * @brief if.
 *
 * @param keyval Parameter.
 * @return Return value.
 */
    else if ((keyval >= IBUS_Caps_Lock && keyval <= IBUS_Hyper_R)
            || (!(modifiers & IBUS_SHIFT_MASK) && (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R))  // when press one shift key
        )
    {
        return false;
    }

    // capture BackSpace
/**
 * @brief if.
 *
 * @param keyval Parameter.
 * @return Return value.
 */
    else if (keyval == IBUS_BackSpace)
    {
        UnikeyBackspacePress();

        if (UnikeyBackspaces == 0 || unikey->preeditstr->empty())
        {
            return false;
        }
        else
        {
            if (unikey->preeditstr->length() <= (guint)UnikeyBackspaces)
            {
                ibus_unikey_buffer_reset(engine);
            }
            else
            {
                ibus_unikey_engine_erase_chars(engine, UnikeyBackspaces);
                ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
            }

            // change tone position after press backspace
            if (UnikeyBufChars > 0)
            {
                if (unikey->oc == CONV_CHARSET_XUTF8)
                {
                    unikey->preeditstr->append((const gchar*)UnikeyBuf, UnikeyBufChars);
                }
                else
                {
                    static unsigned char buf[CONVERT_BUF_SIZE];
                    int bufSize = CONVERT_BUF_SIZE;

                    latinToUtf(buf, UnikeyBuf, UnikeyBufChars, &bufSize);
                    unikey->preeditstr->append((const gchar*)buf, CONVERT_BUF_SIZE - bufSize);
                }

                ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
            }
        }
        return true;
    } // end capture BackSpace

/**
 * @brief if.
 *
 * @param keyval Parameter.
 * @return Return value.
 */
    else if (keyval >=IBUS_KP_Multiply && keyval <=IBUS_KP_9)
    {
        ibus_unikey_buffer_commit(engine);
        return false;
    }

    // capture ascii printable char
/**
 * @brief if.
 *
 * @param keyval Parameter.
 * @return Return value.
 */
    else if ((keyval >= IBUS_space && keyval <=IBUS_asciitilde)
            || keyval == IBUS_Shift_L || keyval == IBUS_Shift_R) // sure this have IBUS_SHIFT_MASK
    {
        UnikeySetCapsState(modifiers & IBUS_SHIFT_MASK, modifiers & IBUS_LOCK_MASK);

        // process keyval

        if ((unikey->im == UkTelex || unikey->im == UkSimpleTelex2)
            && unikey->process_w_at_begin == false
            && UnikeyAtWordBeginning()
            && (keyval == IBUS_w || keyval == IBUS_W))
        {
            UnikeyPutChar(keyval);
            unikey->preeditstr->append(keyval==IBUS_w?"w":"W");
            ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
            return true;
        }

        // shift + space, shift + shift event
        if ((unikey->last_key_with_shift == false && modifiers & IBUS_SHIFT_MASK
                    && keyval == IBUS_space && !UnikeyAtWordBeginning())
            || (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R) // (&& modifiers & IBUS_SHIFT_MASK), sure this have IBUS_SHIFT_MASK
           )
        {
            UnikeyRestoreKeyStrokes();
        } // end shift + space, shift + shift event

        else
        {
            UnikeyFilter(keyval);
        }
        // end process keyval

        // process result of ukengine
        if (UnikeyBackspaces > 0)
        {
            if (unikey->preeditstr->length() <= (guint)UnikeyBackspaces)
            {
                unikey->preeditstr->clear();
            }
            else
            {
                ibus_unikey_engine_erase_chars(engine, UnikeyBackspaces);
            }
        }

        if (UnikeyBufChars > 0)
        {
            if (unikey->oc == CONV_CHARSET_XUTF8)
            {
                unikey->preeditstr->append((const gchar*)UnikeyBuf, UnikeyBufChars);
            }
            else
            {
                static unsigned char buf[CONVERT_BUF_SIZE];
                int bufSize = CONVERT_BUF_SIZE;

                latinToUtf(buf, UnikeyBuf, UnikeyBufChars, &bufSize);
                unikey->preeditstr->append((const gchar*)buf, CONVERT_BUF_SIZE - bufSize);
            }
        }
/**
 * @brief if.
 *
 * @param keyval Parameter.
 * @return Return value.
 */
        else if (keyval != IBUS_Shift_L && keyval != IBUS_Shift_R) // if ukengine not process
        {
            static int n;
            static char s[6];

            n = g_unichar_to_utf8(keyval, s); // convert ucs4 to utf8 char
            unikey->preeditstr->append(s, n);
        }
        // end process result of ukengine

        // commit string: if need
        if (unikey->preeditstr->length() > 0)
        {
            static guint i;
            for (i = 0; i < sizeof(WordBreakSyms); i++)
            {
                if (WordBreakSyms[i] == unikey->preeditstr->at(unikey->preeditstr->length()-1)
                    && WordBreakSyms[i] == keyval)
                {
                    ibus_unikey_buffer_commit(engine);
                    return true;
                }
            }
        }
        // end commit string

        ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
        return true;
    } //end capture printable char

    // non process key
    ibus_unikey_buffer_commit(engine);
    return false;
}

