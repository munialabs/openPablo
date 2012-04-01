/*
    This file is part of darktable,
    copyright (c) 2010 henrik andersson.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DT_STYLES_H
#define DT_STYLES_H

#include "common/darktable.h"

#include <sqlite3.h>
#include <glib.h>
#include <inttypes.h>

/** The definition of styles are copied historystack to
        reproduce an development style such as sepia, cross process
        etc.
*/

typedef struct dt_style_t
{
  gchar *name;
  gchar *description;
} dt_style_t;

typedef struct dt_style_item_t
{
  int num;
  gchar *name;
} dt_style_item_t;

/** creates a new style from specified image, items are the history stack number of items to include in style */
void dt_styles_create_from_image (const char *name,const char *description,int32_t imgid,GList *items);

/** applies the style to selection of images */
void dt_styles_apply_to_selection (const char *name,gboolean duplicate);

/** applies the style to image by imgid*/
void dt_styles_apply_to_image (const char *name,gboolean dulpicate,int32_t imgid);

/** delete a style by name */
void dt_styles_delete_by_name (const char *name);

/** check if style exists by name*/
gboolean dt_styles_exists (const char *name);

/** get a list of styles based on filter string */
GList *dt_styles_get_list (const char *filter);

/** get a list of items for a named style */
GList *dt_styles_get_item_list (const char *name);

/** get a description of a named style */
gchar *dt_styles_get_description (const char *name);

/** save style to file */
void dt_styles_save_to_file(const char *style_name,const char *filedir);

/** load style from file */
void dt_styles_import_from_file(const char *style_path);

/** register global style accelerators at start time */
void init_styles_key_accels();
/** connect global style accelerators at start time */
void connect_styles_key_accels();
#endif
