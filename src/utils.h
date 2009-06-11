#if !defined (__UTILS_H)
#define __UTILS_H

IBusComponent* ibus_unikey_get_component();
int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize);

#endif
