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
#include "common/image_cache.h"
#include "common/imageio.h"
#include "common/imageio_module.h"
#include "common/imageio_jpeg.h"
#include "common/mipmap_cache.h"
#include "control/conf.h"
#include "control/jobs.h"
#include "libraw/libraw.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <limits.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <errno.h>
#include <xmmintrin.h>

#define DT_MIPMAP_CACHE_FILE_MAGIC 0xD71337
#define DT_MIPMAP_CACHE_FILE_VERSION 21
#define DT_MIPMAP_CACHE_DEFAULT_FILE_NAME "mipmaps"

#define DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE (1<<0)

struct dt_mipmap_buffer_dsc
{
  uint32_t width;
  uint32_t height;
  uint32_t size;
  uint32_t flags;
  /* NB: sizeof must be a multiple of 4*sizeof(float) */
}  __attribute__((packed));

// last resort mem alloc for dead images. sizeof(dt_mipmap_buffer_dsc) + dead image pixels (8x8)
// __m128 type for sse alignment.
static __m128 dt_mipmap_cache_static_dead_image[1 + 64];

static inline void
dead_image_8(dt_mipmap_buffer_t *buf)
{
  if(!buf->buf) return;
  struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)buf->buf - 1;
  dsc->width = dsc->height = 8;
  assert(dsc->size > 64*sizeof(uint32_t));
  const uint32_t X = 0xffffffffu;
  const uint32_t o = 0u;
  const uint32_t image[] =
  { o, o, o, o, o, o, o, o,
    o, o, X, X, X, X, o, o,
    o, X, o, X, X, o, X, o,
    o, X, X, X, X, X, X, o,
    o, o, X, o, o, X, o, o,
    o, o, o, o, o, o, o, o,
    o, o, X, X, X, X, o, o,
    o, o, o, o, o, o, o, o };
  memcpy(buf->buf, image, sizeof(uint32_t)*64);
}

static inline void
dead_image_f(dt_mipmap_buffer_t *buf)
{
  if(!buf->buf) return;
  struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)buf->buf - 1;
  dsc->width = dsc->height = 8;
  assert(dsc->size > 64*4*sizeof(float));
  const __m128 X = _mm_set1_ps(1.0f);
  const __m128 o = _mm_set1_ps(0.0f);
  const __m128 image[] =
  { o, o, o, o, o, o, o, o,
    o, o, X, X, X, X, o, o,
    o, X, o, X, X, o, X, o,
    o, X, X, X, X, X, X, o,
    o, o, X, o, o, X, o, o,
    o, o, o, o, o, o, o, o,
    o, o, X, X, X, X, o, o,
    o, o, o, o, o, o, o, o };
  memcpy(buf->buf, image, sizeof(__m128)*64);
}

static inline int32_t
buffer_is_broken(dt_mipmap_buffer_t *buf)
{
  if(!buf->buf) return 0;
  struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)buf->buf - 1;
  if(buf->width  != dsc->width) return 1;
  if(buf->height != dsc->height) return 2;
  // somewhat loose bound:
  if(buf->width*buf->height > dsc->size) return 3;
  return 0;
}

static inline uint32_t
get_key(const uint32_t imgid, const dt_mipmap_size_t size)
{
  // imgid can't be >= 2^29 (~500 million images)
  return (((uint32_t)size) << 29) | (imgid-1);
}

static inline uint32_t
get_imgid(const uint32_t key)
{
  return (key & 0x1fffffff) + 1;
}

static inline dt_mipmap_size_t
get_size(const uint32_t key)
{
  return (dt_mipmap_size_t)(key >> 29);
}

typedef struct _iterate_data_t
{
  FILE *f;
  uint8_t *blob;
  dt_mipmap_size_t mip;
}
_iterate_data_t;

static int
_write_buffer(const uint32_t key, const void *data, void *user_data)
{
  if(!data) return 1;
  struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)data;
  // too small to write. no error, but don't write.
  if(dsc->width <= 8 && dsc->height <= 8) return 0;

  _iterate_data_t *d = (_iterate_data_t *)user_data;
  int written = fwrite(&(d->mip), sizeof(dt_mipmap_size_t), 1, d->f);
  if(written != 1) return 1;
  written = fwrite(&key, sizeof(uint32_t), 1, d->f);
  if(written != 1) return 1;

  dt_mipmap_buffer_t buf;
  buf.width  = dsc->width;
  buf.height = dsc->height;
  buf.imgid  = get_imgid(key);
  buf.size   = get_size(key);
  // skip to next 8-byte alignment, for sse buffers.
  buf.buf    = (uint8_t *)(dsc+1);

  const int32_t length = dt_imageio_jpeg_compress(buf.buf, d->blob, buf.width, buf.height, MIN(100, MAX(10, dt_conf_get_int("database_cache_quality"))));
  written = fwrite(&length, sizeof(int32_t), 1, d->f);
  if(written != 1) return 1;
  written = fwrite(d->blob, sizeof(uint8_t), length, d->f);
  if(written != length) return 1;


//  fprintf(stderr, "[mipmap_cache] serializing image %u (%d x %d) with %d bytes in level %d\n", get_imgid(key), buf.width, buf.height, length, d->mip);

  return 0;
}

static int
dt_mipmap_cache_get_filename(
  gchar* mipmapfilename, size_t size)
{
  int r = -1;
  char* abspath = NULL;

  // Directory
  char cachedir[1024];
  dt_util_get_user_cache_dir(cachedir, sizeof(cachedir));

  // Build the mipmap filename
  const gchar *dbfilename = dt_database_get_path(darktable.db);
  if (!strcmp(dbfilename, ":memory:"))
  {
    snprintf(mipmapfilename, size, "%s", dbfilename);
    r = 0;
    goto exit;
  }

  abspath = malloc(PATH_MAX);
  if (!abspath)
    goto exit;

  if (!realpath(dbfilename, abspath))
    snprintf(abspath, PATH_MAX, "%s", dbfilename);

  GChecksum* chk = g_checksum_new(G_CHECKSUM_SHA1);
  g_checksum_update(chk, (guchar*)abspath, strlen(abspath));
  const gchar *filename = g_checksum_get_string(chk);

  if(!filename || filename[0] == '\0')
    snprintf(mipmapfilename, size, "%s/%s", cachedir, DT_MIPMAP_CACHE_DEFAULT_FILE_NAME);
  else
    snprintf(mipmapfilename, size, "%s/%s-%s", cachedir, DT_MIPMAP_CACHE_DEFAULT_FILE_NAME, filename);

  g_checksum_free(chk);
  r = 0;

exit:
    free(abspath);

    return r;
}

static int
dt_mipmap_cache_serialize(dt_mipmap_cache_t *cache)
{
  gchar dbfilename[1024];
  if (dt_mipmap_cache_get_filename(dbfilename, sizeof(dbfilename)))
  {
    fprintf(stderr, "[mipmap_cache] could not retrieve cache filename; not serializing\n");
    return 1;
  }
  if (!strcmp(dbfilename, ":memory:"))
  {
    fprintf(stderr, "[mipmap_cache] library is in memory; not serializing\n");
    return 0;
  }

  // only store smallest thumbs.
  const dt_mipmap_size_t mip = DT_MIPMAP_2;

  _iterate_data_t d;
  d.f = NULL;
  d.blob = (uint8_t *)malloc(cache->mip[mip].buffer_size);
  int written = 0;
  FILE *f = fopen(dbfilename, "wb");
  if(!f) goto write_error;
  d.f = f;
  // fprintf(stderr, "[mipmap_cache] serializing to `%s'\n", dbfilename);

  // write version info:
  const int32_t magic = DT_MIPMAP_CACHE_FILE_MAGIC + DT_MIPMAP_CACHE_FILE_VERSION;
  written = fwrite(&magic, sizeof(int32_t), 1, f);
  if(written != 1) goto write_error;

  for(int i=0;i<=mip;i++)
  {
    // print max sizes for this cache
    written = fwrite(&cache->mip[i].max_width, sizeof(int32_t), 1, f);
    if(written != 1) goto write_error;
    written = fwrite(&cache->mip[i].max_height, sizeof(int32_t), 1, f);
    if(written != 1) goto write_error;
  }

  for(int i=0;i<=mip;i++)
  {
    d.mip = (dt_mipmap_size_t)i;
    if(dt_cache_for_all(&cache->mip[i].cache, _write_buffer, &d)) goto write_error;
  }

  free(d.blob);
  fclose(f);
  return 0;

write_error:
  fprintf(stderr, "[mipmap_cache] serialization to `%s' failed!\n", dbfilename);
  if(f) fclose(f);
  free(d.blob);
  return 1;
}

static int
dt_mipmap_cache_deserialize(dt_mipmap_cache_t *cache)
{
  int32_t rd = 0;
  const dt_mipmap_size_t mip = DT_MIPMAP_2;
  uint8_t *blob = NULL;
  int file_width[mip+1], file_height[mip+1];

  gchar dbfilename[1024];
  if (dt_mipmap_cache_get_filename(dbfilename, sizeof(dbfilename)))
  {
    fprintf(stderr, "[mipmap_cache] could not retrieve cache filename; not deserializing\n");
    return 1;
  }
  if (!strcmp(dbfilename, ":memory:"))
  {
    fprintf(stderr, "[mipmap_cache] library is in memory; not deserializing\n");
    return 0;
  }

  FILE *f = fopen(dbfilename, "rb");
  if(!f) 
  {
    if (errno == ENOENT)
    {
      fprintf(stderr, "[mipmap_cache] cache is empty, file `%s' doesn't exist\n", dbfilename);
    }
    else
    {
      fprintf(stderr, "[mipmap_cache] failed to open the cache from `%s'\n", dbfilename);
    }
    goto read_finalize;
  }

  // read version info:
  const int32_t magic = DT_MIPMAP_CACHE_FILE_MAGIC + DT_MIPMAP_CACHE_FILE_VERSION;
  int32_t magic_file = 0;
  rd = fread(&magic_file, sizeof(int32_t), 1, f);
  if(rd != 1) goto read_error;
  if(magic_file != magic)
  {
    if(magic_file > DT_MIPMAP_CACHE_FILE_MAGIC && magic_file < magic)
        fprintf(stderr, "[mipmap_cache] cache version too old, dropping `%s' cache\n", dbfilename);
    else
        fprintf(stderr, "[mipmap_cache] invalid cache file, dropping `%s' cache\n", dbfilename);
    goto read_finalize;
  }

  for (int i=0; i<=mip; i++)
  {
    rd = fread(&file_width[i], sizeof(int32_t), 1, f);
    if(rd != 1) goto read_error;
    rd = fread(&file_height[i], sizeof(int32_t), 1, f);
    if(rd != 1) goto read_error;
    if(file_width[i]  != cache->mip[i].max_width ||
       file_height[i] != cache->mip[i].max_height)
    {
      fprintf(stderr, "[mipmap_cache] cache settings changed, dropping `%s' cache\n", dbfilename);
      goto read_finalize;
    }
  }
  blob = (uint8_t *)malloc(4*sizeof(uint8_t)*file_width[mip]*file_height[mip]);

  while(!feof(f))
  {
    int level = 0;
    rd = fread(&level, sizeof(int), 1, f);
    if (level > mip) break;
    
    int32_t key = 0;
    rd = fread(&key, sizeof(int32_t), 1, f);
    if(rd != 1) break; // first value is break only, goes to eof.
    int32_t length = 0;
    rd = fread(&length, sizeof(int32_t), 1, f);
//    fprintf(stderr, "[mipmap_cache] thumbnail for image %d length %d bytes (%d x %d) in level %d\n", get_imgid(key), length, file_width[level], file_height[level], level);
    if(rd != 1 || length > 4*sizeof(uint8_t)*file_width[level]*file_height[level])
      goto read_error;
    rd = fread(blob, sizeof(uint8_t), length, f);
    if(rd != length) goto read_error;

    dt_imageio_jpeg_t jpg;
    uint8_t *data = (uint8_t *)dt_cache_read_get(&cache->mip[level].cache, key);

    struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)data;
    if(dsc->flags & DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE)
    {
      if(dt_imageio_jpeg_decompress_header(blob, length, &jpg) ||
          (jpg.width > file_width[level] || jpg.height > file_height[level]) ||
          dt_imageio_jpeg_decompress(&jpg, data+sizeof(*dsc)))
      {
        fprintf(stderr, "[mipmap_cache] failed to decompress thumbnail for image %d!\n", get_imgid(key));
      }
      dsc->width = jpg.width;
      dsc->height = jpg.height;
      dsc->flags &= ~DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE;
      // these come write locked in case idata[3] == 1, so release that!
      dt_cache_write_release(&cache->mip[level].cache, key);
    }
    dt_cache_read_release(&cache->mip[level].cache, key);
  }

  fclose(f);
  free(blob);
  return 0;

read_error:
  fprintf(stderr, "[mipmap_cache] failed to recover the cache from `%s'\n", dbfilename);
read_finalize:
  if(f) fclose(f);
  free(blob);
  g_unlink(dbfilename);
  return 1;
}

static void _init_f(float   *buf, uint32_t *width, uint32_t *height, const uint32_t imgid);
static void _init_8(uint8_t *buf, uint32_t *width, uint32_t *height, const uint32_t imgid, const dt_mipmap_size_t size);

int32_t
dt_mipmap_cache_allocate(void *data, const uint32_t key, int32_t *cost, void **buf)
{
  dt_mipmap_cache_one_t *c = (dt_mipmap_cache_one_t *)data;
  // slot is exactly aligned with encapsulated cache's position and already allocated
  *cost = c->buffer_size;
  struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)*buf;
  // set width and height:
  dsc->width = c->max_width;
  dsc->height = c->max_height;
  dsc->size = c->buffer_size;
  dsc->flags = DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE;

  // fprintf(stderr, "[mipmap cache alloc] slot %d/%d for imgid %d size %d buffer size %d (%lX)\n", slot, c->cache.bucket_mask+1, get_imgid(key), get_size(key), c->buffer_size, (uint64_t)*buf);
  return 1;
}

#if 0
void
dt_mipmap_cache_deallocate(void *data, const uint32_t key, void *payload)
{
  // nothing. memory is only allocated once.
  // TODO: overwrite buffer with not-found image?
}
#endif


// callback for the imageio core to allocate memory.
// only needed for _F and _FULL buffers, as they change size
// with the input image. will allocate img->width*img->height*img->bpp bytes.
void*
dt_mipmap_cache_alloc(dt_image_t *img, dt_mipmap_size_t size, dt_mipmap_cache_allocator_t a)
{
  assert(size == DT_MIPMAP_FULL);

  struct dt_mipmap_buffer_dsc** dsc = (struct dt_mipmap_buffer_dsc**)a;

  const uint32_t buffer_size = 
      (img->width * img->height * img->bpp) +
      sizeof(**dsc);


  // buf might have been alloc'ed before,
  // so only check size and re-alloc if necessary:
  if(!(*dsc) || ((*dsc)->size < buffer_size) || ((void *)*dsc == (void *)dt_mipmap_cache_static_dead_image))
  {
    free(*dsc);
    *dsc = dt_alloc_align(64, buffer_size);
    // fprintf(stderr, "[mipmap cache] alloc for key %u %lX\n", get_key(img->id, size), (uint64_t)*buf);
    if(!(*dsc))
    {
      // return fallback: at least alloc size for a dead image:
      *dsc = (struct dt_mipmap_buffer_dsc *)dt_mipmap_cache_static_dead_image;
      // allocator holds the pointer. but imageio client is tricked to believe allocation failed:
      return NULL;
    }
    // set buffer size only if we're making it larger.
    (*dsc)->size = buffer_size;
  }
  (*dsc)->width = img->width;
  (*dsc)->height = img->height;
  (*dsc)->flags = DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE;

  // fprintf(stderr, "full buffer allocating img %u %d x %d = %u bytes (%lX)\n", img->id, img->width, img->height, buffer_size, (uint64_t)*buf);

  // trick the user into using a pointer without the header:
  return (*dsc)+1;
}

// callback for the cache backend to initialize payload pointers
int32_t
dt_mipmap_cache_allocate_dynamic(void *data, const uint32_t key, int32_t *cost, void **buf)
{
  // for full image buffers
  struct dt_mipmap_buffer_dsc* dsc = *buf;
  // alloc mere minimum for the header + broken image buffer:
  if(!dsc)
  {
    *buf = dt_alloc_align(16, sizeof(*dsc)+sizeof(float)*4*64);
    // fprintf(stderr, "[mipmap cache] alloc dynamic for key %u %lX\n", key, (uint64_t)*buf);
    if(!(*buf))
    {
      fprintf(stderr, "[mipmap cache] memory allocation failed!\n");
      exit(1);
    }
    dsc = *buf;
    dsc->width = 0;
    dsc->height = 0;
    dsc->size = sizeof(*dsc)+sizeof(float)*4*64;
  }
  assert(dsc->size >= sizeof(*dsc));
  dsc->flags = DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE;

  // cost is just flat one for the buffer, as the buffers might have different sizes,
  // to make sure quota is meaningful.
  *cost = 1;
  // fprintf(stderr, "dummy allocing %lX\n", (uint64_t)*buf);
  return 1; // request write lock
}

#if 0
void
dt_mipmap_cache_deallocate_dynamic(void *data, const uint32_t key, void *payload)
{
  // don't clean up anything, as we are re-allocating. 
}
#endif

static uint32_t
nearest_power_of_two(const uint32_t value)
{
  uint32_t rc = 1;
  while(rc < value) rc <<= 1;
  return rc;
}

void dt_mipmap_cache_init(dt_mipmap_cache_t *cache)
{
  // make sure static memory is initialized
  struct dt_mipmap_buffer_dsc *dsc = (struct dt_mipmap_buffer_dsc *)dt_mipmap_cache_static_dead_image;
  dead_image_f((dt_mipmap_buffer_t *)(dsc+1));

  // adjust numbers to be large enough to hold what mem limit suggests.
  // we want at least 100MB, and consider 2G just still reasonable.
  const uint32_t max_mem = CLAMPS(dt_conf_get_int("cache_memory"), 100u<<20, 2u<<30)/5;
  const uint32_t parallel = CLAMP(dt_conf_get_int ("worker_threads"), 1, 8);
  const int32_t max_size = 2048, min_size = 32;
  int32_t wd = darktable.thumbnail_width;
  int32_t ht = darktable.thumbnail_height;
  wd = CLAMPS(wd, min_size, max_size);
  ht = CLAMPS(ht, min_size, max_size);
  // round up to a multiple of 8, so we can divide by two 3 times
  if(wd & 0xf) wd = (wd & ~0xf) + 0x10;
  if(ht & 0xf) ht = (ht & ~0xf) + 0x10;
  // cache these, can't change at runtime:
  cache->mip[DT_MIPMAP_F].max_width  = wd;
  cache->mip[DT_MIPMAP_F].max_height = ht;
  cache->mip[DT_MIPMAP_F-1].max_width  = wd;
  cache->mip[DT_MIPMAP_F-1].max_height = ht;
  for(int k=DT_MIPMAP_F-2;k>=DT_MIPMAP_0;k--)
  {
    cache->mip[k].max_width  = cache->mip[k+1].max_width  / 2;
    cache->mip[k].max_height = cache->mip[k+1].max_height / 2;
  }

  for(int k=0;k<=DT_MIPMAP_F;k++)
  {
    // buffer stores width and height + actual data
    const int width  = cache->mip[k].max_width;
    const int height = cache->mip[k].max_height;
    if(k == DT_MIPMAP_F)
      cache->mip[k].buffer_size = (4 + 4 * width * height)*sizeof(float);
    else
      cache->mip[k].buffer_size = (4 + width * height)*sizeof(uint32_t);
    cache->mip[k].size = k;
    // level of parallelism also gives minimum size (which is twice that)
    // is rounded to a power of two by the cache anyways, we might as well.
    uint32_t thumbnails = MAX(2*parallel, nearest_power_of_two((uint32_t)((float)max_mem/cache->mip[k].buffer_size)));
    while(thumbnails > 2*parallel && thumbnails * cache->mip[k].buffer_size > max_mem) thumbnails /= 2;

    // try to utilize that memory well (use 90% quota), the hopscotch paper claims good scalability up to
    // even more than that.
    dt_cache_init(&cache->mip[k].cache, thumbnails, parallel, 64, 0.9f*thumbnails*cache->mip[k].buffer_size);

    // might have been rounded to power of two:
    thumbnails = dt_cache_capacity(&cache->mip[k].cache);
    cache->mip[k].buf = dt_alloc_align(64, thumbnails * cache->mip[k].buffer_size);
    dt_cache_static_allocation(&cache->mip[k].cache, (uint8_t *)cache->mip[k].buf, cache->mip[k].buffer_size);
    dt_cache_set_allocate_callback(&cache->mip[k].cache,
        dt_mipmap_cache_allocate, &cache->mip[k]);
    // dt_cache_set_cleanup_callback(&cache->mip[k].cache,
        // &dt_mipmap_cache_deallocate, &cache->mip[k]);


    dt_print(DT_DEBUG_CACHE,
        "[mipmap_cache_init] cache has % 5d entries for mip %d (% 4.02f MB).\n",
        thumbnails, k, thumbnails * cache->mip[k].buffer_size/(1024.0*1024.0));
  }
  // full buffer needs dynamic alloc:
  const int full_entries = 2*parallel;
  int32_t max_mem_bufs = nearest_power_of_two(full_entries);

  // for this buffer, because it can be very busy during import, we want the minimum
  // number of entries in the hashtable to be 16, but leave the quota as is. the dynamic
  // alloc/free properties of this cache take care that no more memory is required.
  dt_cache_init(&cache->mip[DT_MIPMAP_FULL].cache, MAX(16, max_mem_bufs), parallel, 64, 0.9f*max_mem_bufs);
  dt_cache_set_allocate_callback(&cache->mip[DT_MIPMAP_FULL].cache,
      dt_mipmap_cache_allocate_dynamic, &cache->mip[DT_MIPMAP_FULL]);
  // dt_cache_set_cleanup_callback(&cache->mip[DT_MIPMAP_FULL].cache,
      // &dt_mipmap_cache_deallocate_dynamic, &cache->mip[DT_MIPMAP_FULL]);
  cache->mip[DT_MIPMAP_FULL].buffer_size = 0;
  cache->mip[DT_MIPMAP_FULL].size = DT_MIPMAP_FULL;
  cache->mip[DT_MIPMAP_FULL].buf = NULL;

  dt_mipmap_cache_deserialize(cache);
}

void dt_mipmap_cache_cleanup(dt_mipmap_cache_t *cache)
{
  dt_mipmap_cache_serialize(cache);
  for(int k=0;k<=DT_MIPMAP_F;k++)
  {
    dt_cache_cleanup(&cache->mip[k].cache);
    // now mem is actually freed, not during cache cleanup
    free(cache->mip[k].buf);
  }
  dt_cache_cleanup(&cache->mip[DT_MIPMAP_FULL].cache);
}

void dt_mipmap_cache_print(dt_mipmap_cache_t *cache)
{
  for(int k=0; k<(int)DT_MIPMAP_FULL; k++)
  {
    printf("[mipmap_cache] level %d fill %.2f/%.2f MB (%.2f%% in %u/%u buffers)\n", k, cache->mip[k].cache.cost/(1024.0*1024.0),
      cache->mip[k].cache.cost_quota/(1024.0*1024.0),
      100.0f*(float)cache->mip[k].cache.cost/(float)cache->mip[k].cache.cost_quota,
      dt_cache_size(&cache->mip[k].cache),
      dt_cache_capacity(&cache->mip[k].cache));
  }
  const int k = DT_MIPMAP_FULL;
  printf("[mipmap_cache] level [full] fill %d/%d slots (%.2f%% in %u/%u buffers)\n", cache->mip[k].cache.cost,
    cache->mip[k].cache.cost_quota,
    100.0f*(float)cache->mip[k].cache.cost/(float)cache->mip[k].cache.cost_quota,
    dt_cache_size(&cache->mip[k].cache),
    dt_cache_capacity(&cache->mip[k].cache));
  printf("\n\n");
  // very verbose stats about locks/users
  //dt_cache_print(&cache->mip[DT_MIPMAP_3].cache);
}

void
dt_mipmap_cache_read_get(
    dt_mipmap_cache_t *cache,
    dt_mipmap_buffer_t *buf,
    const uint32_t imgid,
    const dt_mipmap_size_t mip,
    const dt_mipmap_get_flags_t flags)
{
  const uint32_t key = get_key(imgid, mip);
  if(flags == DT_MIPMAP_TESTLOCK)
  {
    // simple case: only get and lock if it's there.
    struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)dt_cache_read_testget(&cache->mip[mip].cache, key);
    if(dsc)
    {
      buf->width  = dsc->width;
      buf->height = dsc->height;
      buf->imgid  = imgid;
      buf->size   = mip;
      // skip to next 8-byte alignment, for sse buffers.
      buf->buf    = (uint8_t *)(dsc+1);
    }
    else
    {
      // set to NULL if failed.
      buf->width = buf->height = 0;
      buf->imgid = 0;
      buf->size  = DT_MIPMAP_NONE;
      buf->buf   = NULL;
    }
  }
  else if(flags == DT_MIPMAP_PREFETCH)
  {
    // and opposite: prefetch without locking
    if(mip > DT_MIPMAP_FULL || mip < DT_MIPMAP_0) return;
    dt_job_t j;
    dt_image_load_job_init(&j, imgid, mip);
    // if the job already exists, make it high-priority, if not, add it:
    if(dt_control_revive_job(darktable.control, &j) < 0)
      dt_control_add_job(darktable.control, &j);
  }
  else if(flags == DT_MIPMAP_BLOCKING)
  {
    // simple case: blocking get
    struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)dt_cache_read_get(&cache->mip[mip].cache, key);
    if(!dsc)
    {
      // should never happen for anything but full images which have been moved.
      assert(mip == DT_MIPMAP_FULL);
      // fprintf(stderr, "[mipmap cache get] no data in cache for imgid %u size %d!\n", imgid, mip);
      // sorry guys, no image for you :(
      buf->width = buf->height = 0;
      buf->imgid = 0;
      buf->size  = DT_MIPMAP_NONE;
      buf->buf   = NULL;
    }
    else
    {
      // fprintf(stderr, "[mipmap cache get] found data in cache for imgid %u size %d\n", imgid, mip);
      // uninitialized?
      //assert(dsc->flags & DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE || dsc->size == 0);
      if(dsc->flags & DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE)
      {
        // fprintf(stderr, "[mipmap cache get] now initializing buffer for img %u mip %d!\n", imgid, mip);
        // we're write locked here, as requested by the alloc callback.
        // now fill it with data:
        if(mip == DT_MIPMAP_FULL)
        {
          // load the image:
          // make sure we access the r/w lock as shortly as possible!
          dt_image_t buffered_image;
          const dt_image_t *cimg = dt_image_cache_read_get(darktable.image_cache, imgid);
          buffered_image = *cimg;
          // dt_image_t *img = dt_image_cache_write_get(darktable.image_cache, cimg);
          // dt_image_cache_write_release(darktable.image_cache, img, DT_IMAGE_CACHE_RELAXED);
          dt_image_cache_read_release(darktable.image_cache, cimg);
          char filename[DT_MAX_PATH_LEN];
          dt_image_full_path(buffered_image.id, filename, DT_MAX_PATH_LEN);
          dt_mipmap_cache_allocator_t a = (dt_mipmap_cache_allocator_t)&dsc;
          struct dt_mipmap_buffer_dsc* prvdsc = dsc;
          dt_imageio_retval_t ret = dt_imageio_open(&buffered_image, filename, a);
          if(dsc != prvdsc)
          {
            // fprintf(stderr, "[mipmap cache] realloc %lX\n", (uint64_t)data);
            // write back to cache, too.
            // in case something went wrong, still keep the buffer and return it to the hashtable
            // so we don't produce mem leaks or unnecessary mem fragmentation.
            dt_cache_realloc(&cache->mip[mip].cache, key, 1, (void*)dsc);
          }
          if(ret != DT_IMAGEIO_OK)
          {
            // fprintf(stderr, "[mipmap read get] error loading image: %d\n", ret);
            // 
            // we can only return a zero dimension buffer if the buffer has been allocated.
            // in case dsc couldn't be allocated and points to the static buffer, it contains
            // a dead image already.
            if((void *)dsc != (void *)dt_mipmap_cache_static_dead_image) dsc->width = dsc->height = 0;
          }
          else
          {
            // swap back new image data:
            cimg = dt_image_cache_read_get(darktable.image_cache, imgid);
            dt_image_t *img = dt_image_cache_write_get(darktable.image_cache, cimg);
            *img = buffered_image;
            // fprintf(stderr, "[mipmap read get] initializing full buffer img %u with %u %u -> %d %d (%lX)\n", imgid, data[0], data[1], img->width, img->height, (uint64_t)data);
            // don't write xmp for this (we only changed db stuff):
            dt_image_cache_write_release(darktable.image_cache, img, DT_IMAGE_CACHE_RELAXED);
            dt_image_cache_read_release(darktable.image_cache, img);
          }
        }
        else if(mip == DT_MIPMAP_F)
          _init_f((float *)(dsc+1), &dsc->width, &dsc->height, imgid);
        else
          _init_8((uint8_t *)(dsc+1), &dsc->width, &dsc->height, imgid, mip);
        dsc->flags &= ~DT_MIPMAP_BUFFER_DSC_FLAG_GENERATE;
        // drop the write lock
        dt_cache_write_release(&cache->mip[mip].cache, key);
        /* raise signal that mipmaps has been flushed to cache */
        dt_control_signal_raise(darktable.signals, DT_SIGNAL_DEVELOP_MIPMAP_UPDATED);
      }
      buf->width  = dsc->width;
      buf->height = dsc->height;
      buf->imgid  = imgid;
      buf->size   = mip;
      buf->buf = (uint8_t *)(dsc+1);
      if(dsc->width == 0 || dsc->height == 0)
      {
        // fprintf(stderr, "[mipmap cache get] got a zero-sized image for img %u mip %d!\n", imgid, mip);
        if(mip < DT_MIPMAP_F)       dead_image_8(buf);
        else if(mip == DT_MIPMAP_F) dead_image_f(buf);
        else buf->buf = NULL; // full images with NULL buffer have to be handled, indicates `missing image'
      }
    }
  }
  else if(flags == DT_MIPMAP_BEST_EFFORT)
  {
    // best-effort, might also return NULL.
    // never decrease mip level for float buffer or full image:
    dt_mipmap_size_t min_mip = (mip >= DT_MIPMAP_F) ? mip : DT_MIPMAP_0;
    for(int k=mip;k>=min_mip && k>=0;k--)
    {
      // already loaded?
      dt_mipmap_cache_read_get(cache, buf, imgid, k, DT_MIPMAP_TESTLOCK);
      if(buf->buf && buf->width > 0 && buf->height > 0) return;
      // didn't succeed the first time? prefetch for later!
      if(mip == k)
        dt_mipmap_cache_read_get(cache, buf, imgid, mip, DT_MIPMAP_PREFETCH);
    }
    // fprintf(stderr, "[mipmap cache get] image not found in cache: imgid %u mip %d!\n", imgid, mip);
    // nothing found :(
    buf->buf   = NULL;
    buf->imgid = 0;
    buf->size  = DT_MIPMAP_NONE;
    buf->width = buf->height = 0;
  }
}

void
dt_mipmap_cache_write_get(
    dt_mipmap_cache_t *cache,
    dt_mipmap_buffer_t *buf)
{
  assert(buf->imgid > 0);
  assert(buf->size >= DT_MIPMAP_0);
  assert(buf->size <  DT_MIPMAP_NONE);
  // simple case: blocking write get
  struct dt_mipmap_buffer_dsc* dsc = (struct dt_mipmap_buffer_dsc*)dt_cache_write_get(&cache->mip[buf->size].cache, get_key(buf->imgid, buf->size));
  buf->width  = dsc->width;
  buf->height = dsc->height;
  buf->buf    = (uint8_t *)(dsc+1);
  // these have already been set in read_get
  // buf->imgid  = imgid;
  // buf->size   = mip;
}

void
dt_mipmap_cache_read_release(
    dt_mipmap_cache_t *cache,
    dt_mipmap_buffer_t *buf)
{
  if(buf->size == DT_MIPMAP_NONE || buf->buf == NULL) return;
  assert(buf->imgid > 0);
  assert(buf->size >= DT_MIPMAP_0);
  assert(buf->size <  DT_MIPMAP_NONE);
  dt_cache_read_release(&cache->mip[buf->size].cache, get_key(buf->imgid, buf->size));
  buf->size = DT_MIPMAP_NONE;
  buf->buf  = NULL;
}

// drop a write lock, read will still remain.
void
dt_mipmap_cache_write_release(
    dt_mipmap_cache_t *cache,
    dt_mipmap_buffer_t *buf)
{
  if(buf->size == DT_MIPMAP_NONE || buf->buf == NULL) return;
  assert(buf->imgid > 0);
  assert(buf->size >= DT_MIPMAP_0);
  assert(buf->size <  DT_MIPMAP_NONE);
  dt_cache_write_release(&cache->mip[buf->size].cache, get_key(buf->imgid, buf->size));
  buf->size = DT_MIPMAP_NONE;
  buf->buf  = NULL;
}



// return the closest mipmap size
dt_mipmap_size_t
dt_mipmap_cache_get_matching_size(
    const dt_mipmap_cache_t *cache,
    const int32_t width,
    const int32_t height)
{
  // find `best' match to width and height.
  uint32_t error = 0xffffffff;
  dt_mipmap_size_t best = DT_MIPMAP_NONE;
  for(int k=DT_MIPMAP_0;k<DT_MIPMAP_F;k++)
  {
    uint32_t new_error = abs(cache->mip[k].max_width + cache->mip[k].max_height
                       - width - height);
    if(new_error < error)
    {
      best = k;
      error = new_error;
    }
  }
  return best;
}

void
dt_mipmap_cache_remove(
    dt_mipmap_cache_t *cache,
    const uint32_t imgid)
{
  // get rid of all ldr thumbnails:
  for(int k=DT_MIPMAP_0;k<DT_MIPMAP_F;k++)
  {
    const uint32_t key = get_key(imgid, k);
    dt_cache_remove(&cache->mip[k].cache, key);
  }
}

static void
_init_f(
    float          *out,
    uint32_t       *width,
    uint32_t       *height,
    const uint32_t  imgid)
{
  const uint32_t wd = *width, ht = *height;

  /* do not even try to process file if it isnt available */
  char filename[2048] = {0};
  dt_image_full_path(imgid, filename, 2048);
  if (strlen(filename) == 0 || !g_file_test(filename, G_FILE_TEST_EXISTS))
  {
    *width = *height = 0;
    return;
  }

  dt_mipmap_buffer_t buf;
  dt_mipmap_cache_read_get(darktable.mipmap_cache, &buf, imgid, DT_MIPMAP_FULL, DT_MIPMAP_BLOCKING);

  // lock image after we have the buffer, we might need to lock the image struct for
  // writing during raw loading, to write to width/height.
  const dt_image_t *image = dt_image_cache_read_get(darktable.image_cache, imgid);

  dt_iop_roi_t roi_in, roi_out;
  roi_in.x = roi_in.y = 0;
  roi_in.width = image->width;
  roi_in.height = image->height;
  roi_in.scale = 1.0f;
  
  roi_out.x = roi_out.y = 0;
  roi_out.scale = fminf(wd/(float)image->width, ht/(float)image->height);
  roi_out.width  = roi_out.scale * roi_in.width;
  roi_out.height = roi_out.scale * roi_in.height;

  if(!buf.buf)
  {
    dt_control_log(_("image `%s' is not available!"), image->filename);
    dt_image_cache_read_release(darktable.image_cache, image);
    *width = *height = 0;
    return;
  }

  assert(!buffer_is_broken(&buf));

  if(image->filters)
  {
    // demosaic during downsample
    if(image->bpp == sizeof(float))
      dt_iop_clip_and_zoom_demosaic_half_size_f(
          out, (const float *)buf.buf,
          &roi_out, &roi_in, roi_out.width, roi_in.width,
          dt_image_flipped_filter(image), 1.0f);
    else
      dt_iop_clip_and_zoom_demosaic_half_size(
          out, (const uint16_t *)buf.buf,
          &roi_out, &roi_in, roi_out.width, roi_in.width,
          dt_image_flipped_filter(image));
  }
  else
  {
    // downsample
    dt_iop_clip_and_zoom(out, (const float *)buf.buf,
          &roi_out, &roi_in, roi_out.width, roi_in.width);
  }
  dt_image_cache_read_release(darktable.image_cache, image);
  dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);

  *width  = roi_out.width;
  *height = roi_out.height;
}


// dummy functions for `export' to mipmap buffers:
typedef struct _dummy_data_t
{
  dt_imageio_module_data_t head;
  uint8_t *buf;
}
_dummy_data_t;

static int
_bpp(dt_imageio_module_data_t *data)
{
  return 8;
}

static int
_write_image(
    dt_imageio_module_data_t *data,
    const char               *filename,
    const void               *in,
    void                     *exif,
    int                       exif_len,
    int                       imgid)
{
  _dummy_data_t *d = (_dummy_data_t *)data;
  memcpy(d->buf, in, data->width*data->height*sizeof(uint32_t));
  return 0;
}

static void 
_init_8(
    uint8_t                *buf,
    uint32_t               *width,
    uint32_t               *height,
    const uint32_t          imgid,
    const dt_mipmap_size_t  size)
{
  const uint32_t wd = *width, ht = *height;  

  /* do not even try to process file if it isnt available */
  char filename[2048] = {0};
  dt_image_full_path(imgid, filename, 2048);
  if (strlen(filename) == 0 || !g_file_test(filename, G_FILE_TEST_EXISTS))
  {
    *width = *height = 0;
    return;
  }

  const int altered = dt_image_altered(imgid);
  int res = 1;

  const dt_image_t *cimg = dt_image_cache_read_get(darktable.image_cache, imgid);
  const int orientation = dt_image_orientation(cimg);
  // the orientation for this camera is not read correctly from exiv2, so we need
  // to go the full libraw path (as the thumbnail will be flipped the wrong way round)
  const int incompatible = !strncmp(cimg->exif_maker, "Phase One", 9);
  dt_image_cache_read_release(darktable.image_cache, cimg);

  // try to load the embedded thumbnail first
  if(!altered && !dt_conf_get_bool("never_use_embedded_thumb") && !incompatible)
  {
    int ret;
    char filename[DT_MAX_PATH_LEN];
    dt_image_full_path(imgid, filename, DT_MAX_PATH_LEN);
    const char *c = filename + strlen(filename);
    while(*c != '.' && c > filename) c--;
    if(!strcasecmp(c, ".jpg"))
    {
      // try to load jpg
      dt_imageio_jpeg_t jpg;
      if(!dt_imageio_jpeg_read_header(filename, &jpg))
      {
        uint8_t *tmp = (uint8_t *)malloc(sizeof(uint8_t)*jpg.width*jpg.height*4);
        if(!dt_imageio_jpeg_read(&jpg, tmp))
        {
          // scale to fit
          dt_iop_flip_and_zoom_8(tmp, jpg.width, jpg.height, buf, wd, ht, orientation, width, height);
          res = 0;
        }
        free(tmp);
      }
    }
    else
    {
      // raw image thumbnail
      libraw_data_t *raw = libraw_init(0);
      libraw_processed_image_t *image = NULL;
      ret = libraw_open_file(raw, filename);
      if(ret) goto libraw_fail;
      ret = libraw_unpack_thumb(raw);
      if(ret) goto libraw_fail;
      ret = libraw_adjust_sizes_info_only(raw);
      if(ret) goto libraw_fail;

      image = libraw_dcraw_make_mem_thumb(raw, &ret);
      if(!image || ret) goto libraw_fail;
      const int orientation = raw->sizes.flip;
      if(image->type == LIBRAW_IMAGE_JPEG)
      {
        // JPEG: decode (directly rescaled to mip4)
        dt_imageio_jpeg_t jpg;
        if(dt_imageio_jpeg_decompress_header(image->data, image->data_size, &jpg)) goto libraw_fail;
        uint8_t *tmp = (uint8_t *)malloc(sizeof(uint8_t)*jpg.width*jpg.height*4);
        if(dt_imageio_jpeg_decompress(&jpg, tmp))
        {
          free(tmp);
          goto libraw_fail;
        }
        // scale to fit
        dt_iop_flip_and_zoom_8(tmp, jpg.width, jpg.height, buf, wd, ht, orientation, width, height);

        free(tmp);
        res = 0;
      }
      else
      {
        // bmp
        dt_iop_flip_and_zoom_8(image->data, image->width, image->height, buf, wd, ht, orientation, width, height);
        res = 0;
      }

      // clean up raw stuff.
      libraw_recycle(raw);
      libraw_close(raw);
      free(image);
      if(0)
      {
libraw_fail:
        // fprintf(stderr,"[imageio] %s: %s\n", filename, libraw_strerror(ret));
        libraw_close(raw);
        res = 1;
      }
    }
  }

  if(res)
  {
    // try the real thing: rawspeed + pixelpipe
    dt_imageio_module_format_t format;
    _dummy_data_t dat;
    format.bpp = _bpp;
    format.write_image = _write_image;
    dat.head.max_width  = wd;
    dat.head.max_height = ht;
    dat.buf = buf;
    // export with flags: ignore exif (don't load from disk), don't swap byte order, and don't do hq processing
    res = dt_imageio_export_with_flags(imgid, "unused", &format, (dt_imageio_module_data_t *)&dat, 1, 1, 0);
    if(!res)
    {
      // might be smaller, or have a different aspect than what we got as input.
      *width  = dat.head.width;
      *height = dat.head.height;
    }
  }

  // fprintf(stderr, "[mipmap init 8] export image %u finished (sizes %d %d => %d %d)!\n", imgid, wd, ht, dat.head.width, dat.head.height);

  // any errors?
  if(res)
  {
    // fprintf(stderr, "[mipmap_cache] could not process thumbnail!\n");
    *width = *height = 0;
    return;
  }

  // TODO: various speed optimizations:
  // TODO: also init all smaller mips!
  // TODO: use mipf, but:
  // TODO: if output is cropped, don't use mipf!
}

