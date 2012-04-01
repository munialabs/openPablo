/*
    This file is part of darktable,
    copyright (c) 2009--2010 johannes hanika.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "common/darktable.h"
#include "common/imageio_module.h"
#include "common/colorspaces.h"
#include "control/conf.h"
#include "dtgtk/slider.h"
#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <inttypes.h>
#include <zlib.h>

DT_MODULE(1)

typedef struct dt_imageio_png_t
{
  int max_width, max_height;
  int width, height;
  int bpp;
  FILE *f;
  png_structp png_ptr;
  png_infop info_ptr;
}
dt_imageio_png_t;

typedef struct dt_imageio_png_gui_t
{
  GtkToggleButton *b8, *b16;
}
dt_imageio_png_gui_t;

/* Write EXIF data to PNG file.
 * Code copied from DigiKam's libs/dimg/loaders/pngloader.cpp.
 * The EXIF embeding is defined by ImageMagicK.
 * It is documented in the ExifTool page:
 * http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/PNG.html
 *
 * ..and in turn copied from ufraw. thanks to udi and colleagues
 * for making useful code much more readable and discoverable ;)
 */

static void PNGwriteRawProfile(png_struct *ping,
                               png_info *ping_info, char *profile_type, guint8 *profile_data,
                               png_uint_32 length)
{
  png_textp text;
  long i;
  guint8 *sp;
  png_charp dp;
  png_uint_32 allocated_length, description_length;

  const guint8 hex[16] =
  {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  text = png_malloc(ping, sizeof(png_text));
  description_length = strlen(profile_type);
  allocated_length = length*2 + (length >> 5) + 20 + description_length;

  text[0].text = png_malloc(ping, allocated_length);
  text[0].key = png_malloc(ping, 80);
  text[0].key[0] = '\0';

  g_strlcat(text[0].key, "Raw profile type ", 80);
  g_strlcat(text[0].key, profile_type, 80);

  sp = profile_data;
  dp = text[0].text;
  *dp++='\n';

  g_strlcpy(dp, profile_type, allocated_length);

  dp += description_length;
  *dp++='\n';
  *dp='\0';

  g_snprintf(dp, allocated_length-strlen(text[0].text), "%8lu ", (unsigned long int)length);

  dp += 8;

  for (i=0; i < (long) length; i++)
  {
    if (i%36 == 0)
      *dp++='\n';

    *(dp++) = hex[((*sp >> 4) & 0x0f)];
    *(dp++) = hex[((*sp++ ) & 0x0f)];
  }

  *dp++='\n';
  *dp='\0';
  text[0].text_length = (dp-text[0].text);
  text[0].compression = -1;

  if (text[0].text_length <= allocated_length)
    png_set_text(ping, ping_info,text, 1);

  png_free(ping, text[0].text);
  png_free(ping, text[0].key);
  png_free(ping, text);
}

int
write_image (dt_imageio_png_t *p, const char *filename, const void *in_void, void *exif, int exif_len, int imgid)
{
  const int width = p->width, height = p->height;
  const uint8_t *in = (uint8_t *)in_void;
  FILE *f = fopen(filename, "wb");
  if (!f) return 1;

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
  {
    fclose(f);
    return 1;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    fclose(f);
    png_destroy_write_struct(&png_ptr, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    fclose(f);
    png_destroy_write_struct(&png_ptr, NULL);
    return 1;
  }

  png_init_io(png_ptr, f);

  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
  png_set_compression_mem_level(png_ptr, 8);
  png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(png_ptr, 15);
  png_set_compression_method(png_ptr, 8);
  png_set_compression_buffer_size(png_ptr, 8192);

  png_set_IHDR(png_ptr, info_ptr, width, height,
               p->bpp, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);

  // png_bytep row_pointer = (png_bytep) in;
  png_byte row[6*width];
  // unsigned long rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  if(p->bpp > 8)
  {
    for (int y = 0; y < height; y++)
    {
      for(int x=0; x<width; x++) for(int k=0; k<3; k++)
        {
          uint16_t pix = ((uint16_t *)in)[4*width*y + 4*x + k];
          uint16_t swapped = (0xff00 & (pix<<8)) | (pix>>8);
          ((uint16_t *)row)[3*x+k] = swapped;
        }
      png_write_row(png_ptr, row);
    }
  }
  else
  {
    for (int y = 0; y < height; y++)
    {
      for(int x=0; x<width; x++) for(int k=0; k<3; k++) row[3*x+k] = in[4*width*y + 4*x + k];
      png_write_row(png_ptr, row);
    }
  }

  PNGwriteRawProfile(png_ptr, info_ptr, "exif", exif, exif_len);

  // TODO: embed icc profile!

  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(f);
  return 0;
}

int read_header(const char *filename, dt_imageio_png_t *png)
{
  png->f = fopen(filename, "rb");

  if(!png->f) return 1;

  const unsigned int NUM_BYTES_CHECK = 8;
  png_byte dat[NUM_BYTES_CHECK];

  int cnt = fread(dat, 1, NUM_BYTES_CHECK, png->f);

  if (cnt != NUM_BYTES_CHECK || png_sig_cmp(dat, (png_size_t) 0, NUM_BYTES_CHECK))
  {
    fclose(png->f);
    return 1;
  }

  png->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png->png_ptr)
  {
    fclose(png->f);
    return 1;
  }

  png->info_ptr = png_create_info_struct(png->png_ptr);
  if (!png->info_ptr)
  {
    fclose(png->f);
    png_destroy_read_struct(&png->png_ptr, NULL, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png->png_ptr)))
  {
    fclose(png->f);
    png_destroy_read_struct(&png->png_ptr, NULL, NULL);
    return 1;
  }

  png_init_io(png->png_ptr, png->f);

  // we checked some bytes
  png_set_sig_bytes(png->png_ptr, NUM_BYTES_CHECK);

  // image info
  png_read_info(png->png_ptr, png->info_ptr);

  uint32_t bit_depth = png_get_bit_depth(png->png_ptr, png->info_ptr);
  uint32_t color_type = png_get_color_type(png->png_ptr, png->info_ptr);

  // image input transformations

  // palette => rgb
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png->png_ptr);

  // 1, 2, 4 bit => 8 bit
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png->png_ptr);

  // strip alpha channel
  if (color_type & PNG_COLOR_MASK_ALPHA)
    png_set_strip_alpha(png->png_ptr);

  // grayscale => rgb
  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png->png_ptr);

  // png->bytespp = 3*bit_depth/8;
  png->width  = png_get_image_width(png->png_ptr, png->info_ptr);
  png->height = png_get_image_height(png->png_ptr, png->info_ptr);

  return 0;
}

#if 0
int dt_imageio_png_read_assure_8(dt_imageio_png_t *png)
{
  if (setjmp(png_jmpbuf(png->png_ptr)))
  {
    fclose(png->f);
    png_destroy_read_struct(&png->png_ptr, NULL, NULL);
    return 1;
  }
  uint32_t bit_depth = png_get_bit_depth(png->png_ptr, png->info_ptr);
  // strip down to 8 bit channels
  if (bit_depth == 16)
    png_set_strip_16(png->png_ptr);

  return 0;
}
#endif

int read_image (dt_imageio_png_t *png, uint8_t *out)
{
  if (setjmp(png_jmpbuf(png->png_ptr)))
  {
    fclose(png->f);
    png_destroy_read_struct(&png->png_ptr, NULL, NULL);
    return 1;
  }
  // reflect changes
  png_read_update_info(png->png_ptr, png->info_ptr);

  png_bytep row_pointer = (png_bytep) out;
  unsigned long rowbytes = png_get_rowbytes(png->png_ptr, png->info_ptr);

  for (int y = 0; y < png->height; y++)
  {
    png_read_row(png->png_ptr, row_pointer, NULL);
    row_pointer += rowbytes;
  }

  png_read_end(png->png_ptr, png->info_ptr);
  png_destroy_read_struct(&png->png_ptr, &png->info_ptr, NULL);

  fclose(png->f);
  return 0;
}

void*
get_params(dt_imageio_module_format_t *self, int *size)
{
  *size = 5*sizeof(int);
  dt_imageio_png_t *d = (dt_imageio_png_t *)malloc(sizeof(dt_imageio_png_t));
  memset(d, 0, sizeof(dt_imageio_png_t));
  d->bpp = dt_conf_get_int("plugins/imageio/format/png/bpp");
  if(d->bpp < 12) d->bpp = 8;
  else            d->bpp = 16;
  return d;
}

void
free_params(dt_imageio_module_format_t *self, void *params)
{
  free(params);
}

int
set_params(dt_imageio_module_format_t *self, void *params, int size)
{
  if(size != 5*sizeof(int)) return 1;
  dt_imageio_png_t *d = (dt_imageio_png_t *)params;
  dt_imageio_png_gui_t *g = (dt_imageio_png_gui_t *)self->gui_data;
  if(d->bpp < 12) gtk_toggle_button_set_active(g->b8, TRUE);
  else            gtk_toggle_button_set_active(g->b16, TRUE);
  dt_conf_set_int("plugins/imageio/format/png/bpp", d->bpp);
  return 0;
}

int bpp(dt_imageio_png_t *p)
{
  return p->bpp;
}

const char*
mime(dt_imageio_png_t *data)
{
  return "image/png";
}

const char*
extension(dt_imageio_module_data_t *data)
{
  return "png";
}

const char*
name ()
{
  return _("8/16-bit png");
}

static void
radiobutton_changed (GtkRadioButton *radiobutton, gpointer user_data)
{
  long int bpp = (long int)user_data;
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
    dt_conf_set_int("plugins/imageio/format/png/bpp", bpp);
}

void init(dt_imageio_module_format_t *self) {}
void cleanup(dt_imageio_module_format_t *self) {}

// TODO: some quality/compression stuff?
void gui_init (dt_imageio_module_format_t *self)
{
  dt_imageio_png_gui_t *gui = (dt_imageio_png_gui_t *)malloc(sizeof(dt_imageio_png_gui_t));
  self->gui_data = (void *)gui;
  int bpp = dt_conf_get_int("plugins/imageio/format/png/bpp");
  self->widget = gtk_hbox_new(TRUE, 5);
  GtkWidget *radiobutton = gtk_radio_button_new_with_label(NULL, _("8-bit"));
  gui->b8 = GTK_TOGGLE_BUTTON(radiobutton);
  gtk_box_pack_start(GTK_BOX(self->widget), radiobutton, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(radiobutton), "toggled", G_CALLBACK(radiobutton_changed), (gpointer)8);
  if(bpp < 12) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton), TRUE);
  radiobutton = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radiobutton), _("16-bit"));
  gui->b16 = GTK_TOGGLE_BUTTON(radiobutton);
  gtk_box_pack_start(GTK_BOX(self->widget), radiobutton, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(radiobutton), "toggled", G_CALLBACK(radiobutton_changed), (gpointer)16);
  if(bpp >= 12) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton), TRUE);
}

void gui_cleanup (dt_imageio_module_format_t *self)
{
  free(self->gui_data);
}

void gui_reset (dt_imageio_module_format_t *self) {}


