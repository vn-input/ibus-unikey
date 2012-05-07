#if !defined (__UTILS_H)
#define __UTILS_H

#include "ukengine.h"
#include "engine_const.h"

extern const gchar*          Unikey_IMNames[];
extern const UkInputMethod   Unikey_IM[];
extern const unsigned int    NUM_INPUTMETHOD;
extern const gchar*          Unikey_OCNames[];
extern const unsigned int    Unikey_OC[];
extern const unsigned int    NUM_OUTPUTCHARSET;

IBusComponent* ibus_unikey_get_component();
int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize);

#define get_macro_file() (g_build_filename(g_getenv("HOME"), UNIKEY_MACRO_FILE, NULL))

gboolean ibus_unikey_config_get_string(IBusConfig* config,
                                    const gchar* section,
                                    const gchar* name,
                                    gchar** result);
void ibus_unikey_config_set_string(IBusConfig* config,
                                    const gchar* section,
                                    const gchar* name,
                                    const gchar* value);

gboolean ibus_unikey_config_get_boolean(IBusConfig* config,
                                    const gchar* section,
                                    const gchar* name,
                                    gboolean* result);
void ibus_unikey_config_set_boolean(IBusConfig* config,
                                    const gchar* section,
                                    const gchar* name,
                                    gboolean value);

#if !IBUS_CHECK_VERSION(1,3,99)
char* ibus_property_get_key(IBusProperty *prop);
#endif

#endif
