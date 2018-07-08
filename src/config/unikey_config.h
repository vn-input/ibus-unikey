#if !defined (__IBUS_UNIKEY_CONFIG_H__)
#define __IBUS_UNIKEY_CONFIG_H__

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

extern const gchar*          Unikey_IMNames[];
extern const UkInputMethod   Unikey_IM[];
extern const unsigned int    NUM_INPUTMETHOD;
extern const gchar*          Unikey_OCNames[];
extern const unsigned int    Unikey_OC[];
extern const unsigned int    NUM_OUTPUTCHARSET;

#define get_macro_file() (g_build_filename(g_getenv("HOME"), UNIKEY_MACRO_FILE, NULL))

void ibus_unikey_config_init();

void ibus_unikey_config_on_changed(void(*cb)(gchar *, gpointer), gpointer user_data);

gboolean ibus_unikey_config_get_string(const gchar* name, gchar** result);
void ibus_unikey_config_set_string(const gchar* name, const gchar* value);

gboolean ibus_unikey_config_get_boolean(const gchar* name, gboolean* result);
void ibus_unikey_config_set_boolean(const gchar* name, gboolean value);

#endif
