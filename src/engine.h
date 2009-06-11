#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>

#define IBUS_TYPE_UNIKEY_ENGINE (ibus_unikey_engine_get_type())

void ibus_unikey_init(IBusBus* bus);
void ibus_unikey_exit();
GType ibus_unikey_engine_get_type();

#endif

