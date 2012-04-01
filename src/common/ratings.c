/*
    This file is part of darktable,
    copyright (c) 2011 henrik andersson.

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
#include "common/darktable.h"
#include "common/collection.h"
#include "common/debug.h"
#include "common/image_cache.h"
#include "control/control.h"
#include "control/conf.h"
#include "gui/gtk.h"


void dt_ratings_apply_to_selection (int rating)
{
  uint32_t count = dt_collection_get_selected_count(darktable.collection);
  if (count)
  {
    dt_control_log(ngettext("applying rating %d to %d image", "applying rating %d to %d images", count), rating, count);
#if 0 // not updating cache
    gchar query[1024]={0};
    g_snprintf(query,1024,
	       "update images set flags=(images.flags & ~7) | (7 & %d) where id in (select imgid from selected_images)",
	       rating
	       );
    DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), query, NULL, NULL, NULL);
#endif

    /* for each selected image update rating */
    sqlite3_stmt *stmt;
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select imgid from selected_images", -1, &stmt, NULL);
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
      const dt_image_t *cimg = dt_image_cache_read_get(darktable.image_cache, sqlite3_column_int(stmt, 0));
      dt_image_t *image = dt_image_cache_write_get(darktable.image_cache, cimg);
      image->flags = (image->flags & ~0x7) | (0x7 & rating);
      // synch through:
      dt_image_cache_write_release(darktable.image_cache, image, DT_IMAGE_CACHE_SAFE);
      dt_image_cache_read_release(darktable.image_cache, image);
    }
    sqlite3_finalize(stmt);

    /* redraw view */
    dt_control_queue_redraw_center();
  }
  else
    dt_control_log(_("no images selected to apply rating"));

}

