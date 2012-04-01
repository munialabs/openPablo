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

#include "common/darktable.h"
#include "common/debug.h"
#include "common/exif.h"
#include "common/image.h"
#include "common/image_cache.h"
#include "control/conf.h"
#include "develop/develop.h"

#include <sqlite3.h>

int32_t
dt_image_cache_allocate(void *data, const uint32_t key, int32_t *cost, void **buf)
{
  dt_image_cache_t *c = (dt_image_cache_t *)data;
  const uint32_t hash = key; // == image id
  const uint32_t slot = hash & c->cache.bucket_mask;
  *cost = sizeof(dt_image_t);

  dt_image_t *img = c->images + slot;
  // load stuff from db and store in cache:
  char *str;
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select id, film_id, width, height, filename, maker, model, lens, exposure, aperture, iso, focal_length, datetime_taken, flags, crop, orientation, focus_distance, raw_parameters from images where id = ?1", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, key);
  if(sqlite3_step(stmt) == SQLITE_ROW)
  {
    img->id      = sqlite3_column_int(stmt, 0);
    img->film_id = sqlite3_column_int(stmt, 1);
    img->width   = sqlite3_column_int(stmt, 2);
    img->height  = sqlite3_column_int(stmt, 3);
    img->filename[0] = img->exif_maker[0] = img->exif_model[0] = img->exif_lens[0] =
        img->exif_datetime_taken[0] = '\0';
    str = (char *)sqlite3_column_text(stmt, 4);
    if(str) g_strlcpy(img->filename,   str, 512);
    str = (char *)sqlite3_column_text(stmt, 5);
    if(str) g_strlcpy(img->exif_maker, str, 32);
    str = (char *)sqlite3_column_text(stmt, 6);
    if(str) g_strlcpy(img->exif_model, str, 32);
    str = (char *)sqlite3_column_text(stmt, 7);
    if(str) g_strlcpy(img->exif_lens,  str, 52);
    img->exif_exposure = sqlite3_column_double(stmt, 8);
    img->exif_aperture = sqlite3_column_double(stmt, 9);
    img->exif_iso = sqlite3_column_double(stmt, 10);
    img->exif_focal_length = sqlite3_column_double(stmt, 11);
    str = (char *)sqlite3_column_text(stmt, 12);
    if(str) g_strlcpy(img->exif_datetime_taken, str, 20);
    img->flags = sqlite3_column_int(stmt, 13);
    img->exif_crop = sqlite3_column_double(stmt, 14);
    img->orientation = sqlite3_column_int(stmt, 15);
    img->exif_focus_distance = sqlite3_column_double(stmt,16);
    if(img->exif_focus_distance >= 0 && img->orientation >= 0) img->exif_inited = 1;
    uint32_t tmp = sqlite3_column_int(stmt, 17);
    memcpy(&img->legacy_flip, &tmp, sizeof(dt_image_raw_parameters_t));

    // buffer size?
    if(img->flags & DT_IMAGE_LDR)
      img->bpp = 4*sizeof(float);
    else if(img->flags & DT_IMAGE_HDR)
    {
      if(img->flags & DT_IMAGE_RAW)
        img->bpp = sizeof(float);
      else
        img->bpp = 4*sizeof(float);
    }
    else // raw
      img->bpp = sizeof(uint16_t);
  }
  else fprintf(stderr, "[image_cache_allocate] failed to open image from database: %s\n", sqlite3_errmsg(dt_database_get(darktable.db)));
  sqlite3_finalize(stmt);

  *buf = c->images + slot;
  return 0; // no write lock required, we inited it all right here.
}

void
dt_image_cache_deallocate(void *data, const uint32_t key, void *payload)
{
  // don't free. memory is only allocated once.
  dt_image_t *img = (dt_image_t *)payload;

  // but reset all the stuff. not strictly necessary, but experience tells
  // it might be best to make sure star ratings and such don't spill.
  //
  // note that no flushing to xmp takes place here, it is only done
  // when write_release is called.
  dt_image_init(img);
}

void
dt_image_cache_init(dt_image_cache_t *cache)
{
  // the image cache does no serialization.
  // (unsafe. data should be in db/xmp, not in any other additional cache,
  // also, it should be relatively fast to get the image_t structs from sql.)
  // TODO: actually an independent conf var?
  //       too large: dangerous and wasteful?
  //       can we get away with a fixed size?
  const uint32_t max_mem = 50*1024*1024;
  uint32_t num = (uint32_t)(1.5f*max_mem/sizeof(dt_image_t));
  dt_cache_init(&cache->cache, num, 16, 64, max_mem);
  dt_cache_set_allocate_callback(&cache->cache, &dt_image_cache_allocate,   cache);
  dt_cache_set_cleanup_callback (&cache->cache, &dt_image_cache_deallocate, cache);

  // might have been rounded to power of two:
  num = dt_cache_capacity(&cache->cache);
  cache->images = dt_alloc_align(64, sizeof(dt_image_t)*num);
  dt_print(DT_DEBUG_CACHE, "[image_cache] has %d entries\n", num);
  // initialize first image as empty data:
  dt_image_init(cache->images);
  for(uint32_t k=1;k<num;k++)
  {
    // optimized initialization (avoid accessing conf):
    memcpy(cache->images + k, cache->images, sizeof(dt_image_t));
  }
}

void
dt_image_cache_cleanup(dt_image_cache_t *cache)
{
  dt_cache_cleanup(&cache->cache);
  free(cache->images);
}

void dt_image_cache_print(dt_image_cache_t *cache)
{
  printf("[image cache] fill %.2f/%.2f MB (%.2f%%)\n", cache->cache.cost/(1024.0*1024.0),
      cache->cache.cost_quota/(1024.0*1024.0),
      (float)cache->cache.cost/(float)cache->cache.cost_quota);
}

const dt_image_t*
dt_image_cache_read_get(
    dt_image_cache_t *cache,
    const uint32_t imgid)
{
  if(imgid <= 0) return NULL;
  return (const dt_image_t *)dt_cache_read_get(&cache->cache, imgid);
}

const dt_image_t*
dt_image_cache_read_testget(
    dt_image_cache_t *cache,
    const uint32_t imgid)
{
  if(imgid <= 0) return NULL;
  return (const dt_image_t *)dt_cache_read_testget(&cache->cache, imgid);
}

// drops the read lock on an image struct
void
dt_image_cache_read_release(
    dt_image_cache_t *cache,
    const dt_image_t *img)
{
  if(!img || img->id <= 0) return;
  // just force the dt_image_t struct to make sure it has been locked before.
  dt_cache_read_release(&cache->cache, img->id);
}

// augments the already acquired read lock on an image to write the struct.
// blocks until all readers have stepped back from this image (all but one,
// which is assumed to be this thread)
dt_image_t*
dt_image_cache_write_get(
    dt_image_cache_t *cache,
    const dt_image_t *img)
{
  if(!img) return NULL;
  // just force the dt_image_t struct to make sure it has been locked for reading before.
  return (dt_image_t *)dt_cache_write_get(&cache->cache, img->id);
}


// drops the write priviledges on an image struct.
// this triggers a write-through to sql, and if the setting
// is present, also to xmp sidecar files (safe setting).
void
dt_image_cache_write_release(
    dt_image_cache_t *cache,
    dt_image_t *img,
    dt_image_cache_write_mode_t mode)
{
  if(img->id <= 0) return;
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db),
      "update images set width = ?1, height = ?2, maker = ?3, model = ?4, "
      "lens = ?5, exposure = ?6, aperture = ?7, iso = ?8, focal_length = ?9, "
      "focus_distance = ?10, film_id = ?11, datetime_taken = ?12, flags = ?13, "
      "crop = ?14, orientation = ?15, raw_parameters = ?16 where id = ?17", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, img->width);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, img->height);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 3, img->exif_maker, strlen(img->exif_maker), SQLITE_STATIC);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 4, img->exif_model, strlen(img->exif_model), SQLITE_STATIC);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 5, img->exif_lens,  strlen(img->exif_lens),  SQLITE_STATIC);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 6, img->exif_exposure);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 7, img->exif_aperture);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 8, img->exif_iso);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 9, img->exif_focal_length);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 10, img->exif_focus_distance);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 11, img->film_id);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 12, img->exif_datetime_taken, strlen(img->exif_datetime_taken), SQLITE_STATIC);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 13, img->flags);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 14, img->exif_crop);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 15, img->orientation);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 16, *(uint32_t*)(&img->legacy_flip));
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 17, img->id);
  int rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) fprintf(stderr, "[image_cache_write_release] sqlite3 error %d\n", rc);
  sqlite3_finalize(stmt);
  
  // TODO: make this work in relaxed mode, too.
  if(mode == DT_IMAGE_CACHE_SAFE)
  {
    // rest about sidecars:
    // also synch dttags file:
    dt_image_write_sidecar_file(img->id);
  }
  dt_cache_write_release(&cache->cache, img->id);
}


// remove the image from the cache
void
dt_image_cache_remove(
    dt_image_cache_t *cache,
    const uint32_t imgid)
{
  dt_cache_remove(&cache->cache, imgid);
}



