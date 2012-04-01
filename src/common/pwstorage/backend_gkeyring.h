// This file is part of darktable
// Copyright (c) 2010 Henrik Andersson <hean01@users.sourceforge.net>.

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

#ifndef __BACKEND_GNOME_KEYRING_H__
#define __BACKEND_GNOME_KEYRING_H__

#include "pwstorage.h"

typedef struct backend_gkeyring_context_t
{
  char dummy;
} backend_gkeyring_context_t;

/** Initializes a new kwallet backend context. */
const backend_gkeyring_context_t* dt_pwstorage_gkeyring_new ();
/** Store (key,value) pairs. */
gboolean dt_pwstorage_gkeyring_set (const gchar* slot, GHashTable* table);
/** Load (key,value) pairs. */
GHashTable* dt_pwstorage_gkeyring_get (const gchar* slot);

#endif
