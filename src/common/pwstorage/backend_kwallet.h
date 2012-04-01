// This file is part of darktable
// Copyright (c) 2010 Tobias Ellinghaus <houz@gmx.de>.

// darktable is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// darktable is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with darktable.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __BACKEND_KWALLET_H__
#define __BACKEND_KWALLET_H__

#include "pwstorage.h"
#include <glib.h>
#include <dbus/dbus-glib.h>

/** kwallet backend context */
typedef struct backend_kwallet_context_t
{
  // Connection to the DBus session bus.
  DBusGConnection* connection;

  // Proxy to the kwallet DBus service.
  DBusGProxy* proxy;

  // The name of the wallet we've opened. Set during init_kwallet().
  gchar* wallet_name;
} backend_kwallet_context_t;

/** Initializes a new kwallet backend context. */
const backend_kwallet_context_t* dt_pwstorage_kwallet_new();
/** Cleanup and destroy kwallet backend context. */
// void dt_pwstorage_kwallet_destroy(const backend_kwallet_context_t *context); // doesn't do anything
/** Store (key,value) pairs. */
gboolean dt_pwstorage_kwallet_set(const gchar* slot, GHashTable* table);
/** Load (key,value) pairs. */
GHashTable* dt_pwstorage_kwallet_get(const gchar* slot);

#endif
