#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>

#define IBUS_TYPE_UNIKEY_ENGINE (ibus_unikey_engine_get_type())

GType ibus_unikey_engine_get_type(void);
IBusComponent *ibus_unikey_get_component(void);

#endif

