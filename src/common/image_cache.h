/*
    This file is part of darktable,
    copyright (c) 2011 johannes hanika.

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
#ifndef DT_IMAGE_CACHE_H
#define DT_IMAGE_CACHE_H

#include "common/cache.h"
#include "common/image.h"

typedef struct dt_image_cache_t
{
  // one fat block of dt_image_t, to assign `dynamic' void* in cache to.
  dt_image_t *images;
  dt_cache_t cache;
}
dt_image_cache_t;

// what to do if an image struct is
// released after writing.
typedef enum dt_image_cache_write_mode_t
{
  // always write to database and xmp
  DT_IMAGE_CACHE_SAFE = 0,
  // only write to db and do xmp only during shutdown
  DT_IMAGE_CACHE_RELAXED = 1
}
dt_image_cache_write_mode_t;

void dt_image_cache_init   (dt_image_cache_t *cache);
void dt_image_cache_cleanup(dt_image_cache_t *cache);
void dt_image_cache_print  (dt_image_cache_t *cache);

// blocks until it gets the image struct with this id for reading.
// also does the sql query if the image is not in cache atm.
// if id < 0, a newly wiped image struct shall be returned (for import).
// this will silently start the garbage collector and free long-unused
// cachelines to free up space if necessary.
// if an entry is swapped out like this in the background, this is the latest
// point where sql and xmp can be synched (unsafe setting).
const dt_image_t*
dt_image_cache_read_get(
    dt_image_cache_t *cache,
    const uint32_t imgid);

// same as read_get, but doesn't block and returns NULL if the image
// is currently unavailable.
const dt_image_t*
dt_image_cache_read_testget(
    dt_image_cache_t *cache,
    const uint32_t imgid);

// drops the read lock on an image struct
void
dt_image_cache_read_release(
    dt_image_cache_t *cache,
    const dt_image_t *img);

// augments the already acquired read lock on an image to write the struct.
// blocks until all readers have stepped back from this image (all but one,
// which is assumed to be this thread)
dt_image_t*
dt_image_cache_write_get(
    dt_image_cache_t *cache,
    const dt_image_t *img);

// drops the write priviledges on an image struct.
// thtis triggers a write-through to sql, and if the setting
// is present, also to xmp sidecar files (safe setting).
void
dt_image_cache_write_release(
    dt_image_cache_t *cache,
    dt_image_t *img,
    dt_image_cache_write_mode_t mode);

// remove the image from the cache
void
dt_image_cache_remove(
    dt_image_cache_t *cache,
    const uint32_t imgid);

#endif
