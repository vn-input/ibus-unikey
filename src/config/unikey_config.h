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

#if !defined (__IBUS_UNIKEY_CONFIG_H__)
#define __IBUS_UNIKEY_CONFIG_H__

/**
 * @file unikey_config.h
 * @brief Public configuration constants and API for ibus-unikey.
 *
 * Declares wrapper routines around GSettings plus constants and lookup maps
 * shared by the IBus engine and GTK setup utility.
 */

#include <map>
#include <string>
#include <gio/gio.h>

#include "ukengine.h"

/**
 * @brief Relative macro file path under user HOME.
 */
#define UNIKEY_MACRO_FILE        ".ibus/unikey/macro"

/**
 * @brief GSettings schema identifier used by ibus-unikey.
 */
#define UNIKEY_GSCHEMA_ID        "org.freedesktop.ibus.engine.unikey"

/** @name GSettings key names
 *  @{ */
#define CONFIG_INPUTMETHOD       "input-method"
#define CONFIG_OUTPUTCHARSET     "output-charset"
#define CONFIG_SPELLCHECK        "spell-check"
#define CONFIG_AUTORESTORENONVN  "auto-restore-non-vn"
#define CONFIG_MODERNSTYLE       "modern-style"
#define CONFIG_FREEMARKING       "free-marking"
#define CONFIG_MACROENABLED      "macro-enabled"
#define CONFIG_STANDALONEW       "standalone-w-as-uw"
#define CONFIG_MACRO_LASTIMPORTDIR "macro-last-import-dir"
#define CONFIG_MACRO_LASTEXPORTDIR "macro-last-export-dir"
/** @} */

/**
 * @brief Mapping from persisted input-method strings to engine enum + label.
 *
 * - key: value stored in GSettings
 * - first: internal ::UkInputMethod enum
 * - second: user-visible localized base label
 */
const std::map<const std::string, std::pair<UkInputMethod, const gchar*>> input_method_map {
    { "telex",   { UkTelex,        "Extend Telex" } },
    { "vni",     { UkVni,          "VNI" } },
    { "stelex",  { UkSimpleTelex,  "STelex" } },
    { "stelex2", { UkSimpleTelex2, "STelex 2" } },
};

/**
 * @brief Mapping from persisted output-charset strings to charset id + label.
 *
 * - key: value stored in GSettings
 * - first: conversion charset constant (CONV_CHARSET_*)
 * - second: user-visible localized base label
 */
const std::map<const std::string, std::pair<unsigned int, const gchar*>> output_charset_map {
    { "unicode", { CONV_CHARSET_XUTF8,       "Unicode" } },
    { "tcvn3",   { CONV_CHARSET_TCVN3,       "TCVN3" } },
    { "vni-win", { CONV_CHARSET_VNIWIN,      "VNI Win" } },
    { "viqr",    { CONV_CHARSET_VIQR,        "VIQR" } },
    { "bk-hcm2", { CONV_CHARSET_BKHCM2,      "BK HCM 2" } },
    { "cstr",    { CONV_CHARSET_UNI_CSTRING, "CString" } },
    { "ncr-dec", { CONV_CHARSET_UNIREF,      "NCR Decimal" } },
    { "ncr-hex", { CONV_CHARSET_UNIREF_HEX,  "NCR Hex" } },
};

/**
 * @brief Build absolute path to user macro file.
 * @return Newly-allocated gchar* from glib path API; caller must free with g_free.
 */
#define get_macro_file() (g_build_filename(g_getenv("HOME"), UNIKEY_MACRO_FILE, NULL))

/**
 * @brief Initialize configuration subsystem and underlying GSettings object.
 */
void ibus_unikey_config_init();

/**
 * @brief Register callback invoked when any setting key changes.
 *
 * @param cb Callback receiving changed key name and caller data.
 * @param user_data Opaque pointer forwarded to @p cb.
 */
void ibus_unikey_config_on_changed(void(*cb)(gchar *, gpointer), gpointer user_data);

/**
 * @brief Read a string setting.
 * @param name Setting key.
 * @param[out] result Receives newly-allocated string on success.
 * @return TRUE on success, FALSE if key missing/invalid.
 */
gboolean ibus_unikey_config_get_string(const gchar* name, gchar** result);

/**
 * @brief Write a string setting.
 * @param name Setting key.
 * @param value UTF-8 string value.
 */
void ibus_unikey_config_set_string(const gchar* name, const gchar* value);

/**
 * @brief Check whether the active ibus-unikey schema defines a given key.
 * @param name Setting key.
 * @return TRUE when the key exists in the schema, otherwise FALSE.
 */
gboolean ibus_unikey_config_has_key(const gchar* name);

/**
 * @brief Read a boolean setting.
 * @param name Setting key.
 * @param[out] result Receives boolean value on success.
 * @return TRUE on success, FALSE if key missing/invalid.
 */
gboolean ibus_unikey_config_get_boolean(const gchar* name, gboolean* result);

/**
 * @brief Write a boolean setting.
 * @param name Setting key.
 * @param value Boolean value.
 */
void ibus_unikey_config_set_boolean(const gchar* name, gboolean value);

#endif
