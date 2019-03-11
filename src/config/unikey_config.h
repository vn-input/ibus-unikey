#if !defined (__IBUS_UNIKEY_CONFIG_H__)
#define __IBUS_UNIKEY_CONFIG_H__

#include <map>
#include <string>
#include <gio/gio.h>

#include "ukengine.h"

#define UNIKEY_MACRO_FILE        ".ibus/unikey/macro"

#define UNIKEY_GSCHEMA_ID        "org.freedesktop.ibus.engine.unikey"

#define CONFIG_INPUTMETHOD       "input-method"
#define CONFIG_OUTPUTCHARSET     "output-charset"
#define CONFIG_SPELLCHECK        "spell-check"
#define CONFIG_AUTORESTORENONVN  "auto-restore-non-vn"
#define CONFIG_MODERNSTYLE       "modern-style"
#define CONFIG_FREEMARKING       "free-marking"
#define CONFIG_MACROENABLED      "macro-enabled"
#define CONFIG_STANDALONEW       "standalone-w-as-uw"

const std::map<const std::string, std::pair<UkInputMethod, const gchar*>> input_method_map {
    { "telex",   { UkTelex,        "Extend Telex" } },
    { "vni",     { UkVni,          "VNI" } },
    { "stelex",  { UkSimpleTelex,  "STelex" } },
    { "stelex2", { UkSimpleTelex2, "STelex 2" } },
};

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

#define get_macro_file() (g_build_filename(g_getenv("HOME"), UNIKEY_MACRO_FILE, NULL))

void ibus_unikey_config_init();

void ibus_unikey_config_on_changed(void(*cb)(gchar *, gpointer), gpointer user_data);

gboolean ibus_unikey_config_get_string(const gchar* name, gchar** result);
void ibus_unikey_config_set_string(const gchar* name, const gchar* value);

gboolean ibus_unikey_config_get_boolean(const gchar* name, gboolean* result);
void ibus_unikey_config_set_boolean(const gchar* name, gboolean value);

#endif
