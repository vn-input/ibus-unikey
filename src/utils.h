#if !defined (__UTILS_H)
#define __UTILS_H

#include "ukengine.h"

IBusComponent* ibus_unikey_get_component();
int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize);
void unikey_set_default_options(UnikeyOptions *pOpt);

#endif
