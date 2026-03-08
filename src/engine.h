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
 * @brief Initialize the Unikey IBus engine.
 *
 * Registers the engine type with the given `IBusBus` instance.  This must be
 * called before the engine is used by IBus.
 *
 * @param bus Pointer to the `IBusBus` that will host the engine.
 */
void ibus_unikey_init(IBusBus* bus);
/**
 * @brief Clean up global resources used by the Unikey engine.
 *
 * Called when the plugin is being unloaded; after this returns the engine
 * should no longer be referenced.
 */
void ibus_unikey_exit();
/**
 * @brief Return the GType for the Unikey engine object.
 *
 * Used internally by the IBus type system during registration and object
 * creation.
 */
GType ibus_unikey_engine_get_type();

#endif

