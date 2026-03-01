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

/**
 * @file engine.h
 * @brief Public declarations for engine.h.
 */
#ifndef __ENGINE_H__
#define __ENGINE_H__

/**
 * @file engine.h
 * @brief Public IBus engine interface for the unikey module.
 *
 * Declares initialization and type registration functions used by IBus when
 * loading this engine plugin.
 */

#include <ibus.h>

#define IBUS_TYPE_UNIKEY_ENGINE (ibus_unikey_engine_get_type())

/**
 * @brief ibus_unikey_init.
 *
 * @param bus Description of bus.
 */
void ibus_unikey_init(IBusBus* bus);
/**
 * @brief ibus_unikey_exit.
 *
 */
void ibus_unikey_exit();
/**
 * @brief ibus_unikey_engine_get_type.
 *
 */
GType ibus_unikey_engine_get_type();

#endif

