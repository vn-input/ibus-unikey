#ifndef __CONFIG_UTILS_H__
#define __CONFIG_UTILS_H__

#include "engine_const.h"

void read_config(void* data, UnikeyMainSetupOptions* opt);
void write_config(void* data, UnikeyMainSetupOptions* opt);
int force_engine_to_reload_config();

#endif

