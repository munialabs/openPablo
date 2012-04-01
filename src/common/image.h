/*
    This file is part of darktable,
    copyright (c) 2009--2011 johannes hanika.

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
#ifndef DT_IMAGE_H
#define DT_IMAGE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common/darktable.h"
#include "common/dtpthread.h"
#include <glib.h>
#include <inttypes.h>

/** define for max path/filename length */
#define DT_MAX_FILENAME_LEN 256
// TODO: separate into path/filename and store 256 for filename
#define DT_MAX_PATH_LEN 1024

/** return value of image io functions. */
typedef enum dt_imageio_retval_t
{
  DT_IMAGEIO_OK = 0,          // all good :)
  DT_IMAGEIO_FILE_NOT_FOUND,  // file has been lost
  DT_IMAGEIO_FILE_CORRUPTED,  // file contains garbage
  DT_IMAGEIO_CACHE_FULL       // dt's caches are full :(
}
dt_imageio_retval_t;

typedef enum
{
  // the first 0x7 in flags are reserved for star ratings.
  DT_IMAGE_DELETE = 1,
  DT_IMAGE_OKAY = 2,
  DT_IMAGE_NICE = 3,
  DT_IMAGE_EXCELLENT = 4,
  // this refers to the state of the mipf buffer and its source.
  DT_IMAGE_THUMBNAIL = 16,
  // set during import if the image is low-dynamic range, i.e. doesn't need demosaic, wb, highlight clipping etc.
  DT_IMAGE_LDR = 32,
  // set during import if the image is raw data, i.e. it needs demosaicing.
  DT_IMAGE_RAW = 64,
  // set during import if images is a high-dynamic range image..
  DT_IMAGE_HDR = 128,
  // set when marked for deletion
  DT_IMAGE_REMOVE = 256
}
dt_image_flags_t;

typedef struct dt_image_raw_parameters_t
{
  unsigned legacy    : 24;
  unsigned user_flip : 8; // +8 = 32 bits.
}
dt_image_raw_parameters_t;

// TODO: add color labels and such as cachable
// __attribute__ ((aligned (128)))
typedef struct dt_image_t
{
  // minimal exif data here (all in multiples of 4-byte to interface nicely with c++):
  int32_t exif_inited;
  int32_t orientation;
  float exif_exposure;
  float exif_aperture;
  float exif_iso;
  float exif_focal_length;
  float exif_focus_distance;
  float exif_crop;
  char exif_maker[32];
  char exif_model[32];
  char exif_lens[52];
  char exif_datetime_taken[20];
  char filename[DT_MAX_FILENAME_LEN];

  // common stuff
  int32_t width, height;
  // used by library
  int32_t num, flags, film_id, id;

  // FIXME: find out what this should do, and how
  int32_t dirty;

  uint32_t filters;  // demosaic pattern
  int32_t bpp;       // bytes per pixel
 
  dt_image_raw_parameters_t legacy_flip; // unfortunately needed to convert old bits to new flip module.
}
dt_image_t;

// image buffer operations:
/** inits basic values to sensible defaults. */
void dt_image_init(dt_image_t *img);
/** returns non-zero if the image contains low-dynamic range data. */
int dt_image_is_ldr(const dt_image_t *img);
/** returns non-zero if the image contains mosaic data. */
int dt_image_is_raw(const dt_image_t *img);
/** returns non-zero if the image contains float data. */
int dt_image_is_hdr(const dt_image_t *img);
/** returns the full path name where the image was imported from. */
void dt_image_full_path(const int imgid, char *pathname, int len);
/** returns the portion of the path used for the film roll name. */
const char *dt_image_film_roll_name(const char *path);
/** returns the film roll name, i.e. without the path. */
void dt_image_film_roll(const dt_image_t *img, char *pathname, int len);
/** appends version numbering for duplicated images. */
void dt_image_path_append_version(int imgid, char *pathname, const int len);
/** prints a one-line exif information string. */
void dt_image_print_exif(const dt_image_t *img, char *line, int len);
/** imports a new image from raw/etc file and adds it to the data base and image cache. */
uint32_t dt_image_import(const int32_t film_id, const char *filename, gboolean override_ignore_jpegs);
/** removes the given image from the database. */
void dt_image_remove(const int32_t imgid);
/** duplicates the given image in the database. */
int32_t dt_image_duplicate(const int32_t imgid);
/** flips the image, clock wise, if given flag. */
void dt_image_flip(const int32_t imgid, const int32_t cw);
void dt_image_set_flip(const int32_t imgid, const int32_t user_flip);
/** returns 1 if there is history data found for this image, 0 else. */
int dt_image_altered(const uint32_t imgid);
/** returns the orientation bits of the image, exif or user override, if set. */
static inline int dt_image_orientation(const dt_image_t *img)
{
  return img->orientation > 0 ? img->orientation : 0;
}
/** returns the (flipped) filter string for the demosaic pattern. */
static inline uint32_t
dt_image_flipped_filter(const dt_image_t *img)
{
  // from the dcraw source code documentation:
  //
  //   0x16161616:     0x61616161:     0x49494949:     0x94949494:

  //   0 1 2 3 4 5     0 1 2 3 4 5     0 1 2 3 4 5     0 1 2 3 4 5
  // 0 B G B G B G   0 G R G R G R   0 G B G B G B   0 R G R G R G
  // 1 G R G R G R   1 B G B G B G   1 R G R G R G   1 G B G B G B
  // 2 B G B G B G   2 G R G R G R   2 G B G B G B   2 R G R G R G
  // 3 G R G R G R   3 B G B G B G   3 R G R G R G   3 G B G B G B
  //
  // orient:     0               5               6               3
  // orient:     6               0               3               5
  // orient:     5               3               0               6
  // orient:     3               6               5               0
  //
  // orientation: &1 : flip y    &2 : flip x    &4 : swap x/y
  //
  // if image height is odd (and flip y), need to switch pattern by one row:
  // 0x16161616 <-> 0x61616161
  // 0x49494949 <-> 0x94949494
  //
  // if image width is odd (and flip x), need to switch pattern by one column:
  // 0x16161616 <-> 0x49494949
  // 0x61616161 <-> 0x94949494

  const int orient = dt_image_orientation(img);
  int filters = img->filters;
  if((orient & 1) && (img->height & 1))
  {
    switch(filters)
    {
      case 0x16161616u:
        filters = 0x49494949u;
        break;
      case 0x49494949u:
        filters = 0x16161616u;
        break;
      case 0x61616161u:
        filters = 0x94949494u;
        break;
      case 0x94949494u:
        filters = 0x61616161u;
        break;
      default:
        filters = 0;
        break;
    }
  }
  if((orient & 2) && (img->width & 1))
  {
    switch(filters)
    {
      case 0x16161616u:
        filters = 0x61616161u;
        break;
      case 0x49494949u:
        filters = 0x94949494u;
        break;
      case 0x61616161u:
        filters = 0x16161616u;
        break;
      case 0x94949494u:
        filters = 0x49494949u;
        break;
      default:
        filters = 0;
        break;
    }
  }
  switch(filters)
  {
    case 0:
      // no mosaic is no mosaic, even rotated:
      return 0;
    case 0x16161616u:
      switch(orient)
      {
        case 5:
          return 0x61616161u;
        case 6:
          return 0x49494949u;
        case 3:
          return 0x94949494u;
        default:
          return 0x16161616u;
      }
      break;
    case 0x61616161u:
      switch(orient)
      {
        case 6:
          return 0x16161616u;
        case 3:
          return 0x49494949u;
        case 5:
          return 0x94949494u;
        default:
          return 0x61616161u;
      }
      break;
    case 0x49494949u:
      switch(orient)
      {
        case 3:
          return 0x61616161u;
        case 6:
          return 0x94949494u;
        case 5:
          return 0x16161616u;
        default:
          return 0x49494949u;
      }
      break;
    default: // case 0x94949494u:
      switch(orient)
      {
        case 6:
          return 0x61616161u;
        case 5:
          return 0x49494949u;
        case 3:
          return 0x16161616u;
        default:
          return 0x94949494u;
      }
      break;
  }
}
/** return the raw orientation, from jpg orientation. */
static inline int
dt_image_orientation_to_flip_bits(const int orient)
{
  switch(orient)
  {
    case 1:
      return 0 | 0 | 0;
    case 2:
      return 0 | 2 | 0;
    case 3:
      return 0 | 2 | 1;
    case 4:
      return 0 | 0 | 1;
    case 5:
      return 4 | 0 | 0;
    case 6:
      return 4 | 2 | 0;
    case 7:
      return 4 | 2 | 1;
    case 8:
      return 4 | 0 | 1;
    default:
      return 0;
  }
}

// xmp functions:
void dt_image_write_sidecar_file(int imgid);
void dt_image_synch_xmp(const int selected);
void dt_image_synch_all_xmp(const gchar *pathname);

#endif
