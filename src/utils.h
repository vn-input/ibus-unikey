#if !defined (__UTILS_H)
#define __UTILS_H

#include "ukengine.h"
#include "engine_const.h"

IBusComponent* ibus_unikey_get_component();
int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize);
void unikey_create_default_options(UnikeyOptions *pOpt);

#define get_macro_file() (g_build_filename(g_getenv("HOME"), UNIKEY_MACRO_FILE, NULL))

#endif
