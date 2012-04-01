/*
    This file is part of darktable,
    copyright (c) 2011 bruce guenter

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
#include "control/control.h"
#include "develop/imageop.h"
#include "dtgtk/slider.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <strings.h>

DT_MODULE(1)

typedef struct dt_iop_rawdenoise_params_t
{
  float threshold;
}
dt_iop_rawdenoise_params_t;

typedef struct dt_iop_rawdenoise_gui_data_t
{
  GtkDarktableSlider *threshold;
}
dt_iop_rawdenoise_gui_data_t;

typedef struct dt_iop_rawdenoise_data_t
{
  float threshold;
}
dt_iop_rawdenoise_data_t;

typedef struct dt_iop_rawdenoise_global_data_t
{
}
dt_iop_rawdenoise_global_data_t;

const char *name()
{
  return _("raw denoise");
}

int flags()
{
  return IOP_FLAGS_SUPPORTS_BLENDING;
}

int
groups ()
{
  return IOP_GROUP_CORRECT;
}

int
output_bpp(dt_iop_module_t *module, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  return sizeof(float);
}

void init_key_accels(dt_iop_module_so_t *self)
{
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "noise threshold"));
}

void connect_key_accels(dt_iop_module_t *self)
{
  dt_iop_rawdenoise_gui_data_t *g =
      (dt_iop_rawdenoise_gui_data_t*)self->gui_data;

  dt_accel_connect_slider_iop(self, "noise threshold",
                              GTK_WIDGET(g->threshold));
}

#if 0
static int
FC(const int row, const int col, const unsigned int filters)
{
  return filters >> (((row << 1 & 14) + (col & 1)) << 1) & 3;
}
#endif

// transposes image, it is faster to read columns than to write them.
static void
hat_transform(float *temp, const float *const base, int stride, int size, int scale)
{
  int i;
  const float *basep0;
  const float *basep1;
  const float *basep2;
  const int stxsc = stride*scale;

  basep0 = base;
  basep1 = base + stxsc;
  basep2 = base + stxsc;

  for (i=0; i < scale; i++, basep0+=stride, basep1-=stride, basep2+=stride)
    temp[i] = (*basep0 + *basep0 + *basep1 + *basep2)*0.25f;

  for (; i < size-scale; i++, basep0+=stride)
    temp[i] = ((*basep0)*2 + *(basep0-stxsc) + *(basep0+stxsc))*0.25f;

  basep1 = basep0 - stxsc;
  basep2 = base + stride*(size-2);

  for (; i < size; i++, basep0+=stride, basep1+=stride, basep2-=stride)
    temp[i] = (*basep0 + *basep0 + *basep1 + *basep2)*0.25f;
}

#define BIT16 65536.0

static void wavelet_denoise(const float *const in, float *const out, const dt_iop_roi_t *const roi, float threshold, uint32_t filters)
{
  int lev;
  static const float noise[] =
  { 0.8002,0.2735,0.1202,0.0585,0.0291,0.0152,0.0080,0.0044 };

  const int size = (roi->width/2+1) * (roi->height/2+1);
#if 0
  float maximum = 1.0;		/* FIXME */
  float black = 0.0;		/* FIXME */
  maximum *= BIT16;
  black *= BIT16;
  for (c=0; c<4; c++)
    cblack[c] *= BIT16;
#endif
  float *const fimg = malloc(size*4*sizeof *fimg);


  const int nc = 4;
  for (int c=0; c<nc; c++)	/* denoise R,G1,B,G3 individually */
  {
    // zero lowest quarter part
    memset(fimg,0,size*sizeof(float));

    // adjust for odd width and height
    const int halfwidth  = roi->width / 2  + (roi->width & (~(c >> 1)) & 1) ;
    const int halfheight = roi->height / 2 + (roi->height & (~c) & 1) ;

#ifdef _OPENMP
    #pragma omp parallel for default(none) shared(c) schedule(static)
#endif
    for (int row=c&1; row<roi->height; row+=2)
    {
      float *fimgp = fimg + size + row/2 * halfwidth;
      int col = (c&2)>>1;
      const float *inp = in + row*roi->width + col;
      for (; col<roi->width; col+=2, fimgp++, inp+=2)
        *fimgp = sqrt(*inp);
    }

    int lastpass;

    for (lev=0; lev < 5; lev++)
    {
      const int pass1 = size*((lev & 1)*2 + 1);
      const int pass2 = 2*size;
      const int pass3 = 4*size - pass1;

      // filter horizontally and transpose
#ifdef _OPENMP
      #pragma omp parallel for default(none) shared(lev) schedule(static)
#endif
      for (int col=0; col < halfwidth; col++)
      {
        hat_transform(fimg+pass2+col*halfheight, fimg+pass1+col, halfwidth, halfheight, 1 << lev);
      }
      // filter vertically and transpose back
#ifdef _OPENMP
      #pragma omp parallel for default(none) shared(lev) schedule(static)
#endif
      for (int row=0; row < halfheight; row++)
      {
        hat_transform(fimg+pass3+row*halfwidth, fimg+pass2+row, halfheight, halfwidth, 1 << lev);
      }

      const float thold = threshold * noise[lev];
#ifdef _OPENMP
      #pragma omp parallel for default(none) shared(lev) 
#endif
      for (int i=0; i < halfwidth*halfheight; i++)
      {
        float *fimgp = fimg + i;
        const float diff = fimgp[pass1] - fimgp[pass3];
        fimgp[0] += copysignf(fmaxf(fabsf(diff) - thold,0.0f),diff);
      }

      lastpass = pass3;
    }
#ifdef _OPENMP
    #pragma omp parallel for default(none) shared(c,lastpass) schedule(static)
#endif
    for (int row=c&1; row<roi->height; row+=2)
    {
      const float *fimgp = fimg + row/2 * halfwidth;
      int col = (c&2)>>1;
      float *outp = out + row*roi->width + col;
      for (; col<roi->width; col+=2, fimgp++, outp+=2)
      {
        float d = fimgp[0] + fimgp[lastpass];
        *outp = d * d;
      }
    }
  }
#if 0
  /* FIXME: Haven't ported this part yet */
  if (filters && colors == 3)	/* pull G1 and G3 closer together */
  {
    float *window[4];
    int wlast, blk[2];
    float mul[2];
    float thold = threshold/512;
    for (row=0; row < 2; row++)
    {
      mul[row] = 0.125 * pre_mul[FC(row+1,0) | 1] / pre_mul[FC(row,0) | 1];
      blk[row] = cblack[FC(row,0) | 1];
    }
    for (i=0; i < 4; i++)
      window[i] = fimg + width*i;
    for (wlast=-1, row=1; row < height-1; row++)
    {
      while (wlast < row+1)
      {
        for (wlast++, i=0; i < 4; i++)
          window[(i+3) & 3] = window[i];
        for (col = FC(wlast,1) & 1; col < width; col+=2)
          window[2][col] = BAYER(wlast,col);
      }
      for (col = (FC(row,0) & 1)+1; col < width-1; col+=2)
      {
        float avg = ( window[0][col-1] + window[0][col+1] +
                      window[2][col-1] + window[2][col+1] - blk[~row & 1]*4 )
                    * mul[row & 1] + (window[1][col] + blk[row & 1]) * 0.5;
        avg = avg < 0 ? 0 : sqrt(avg);
        float diff = sqrt(BAYER(row,col)) - avg;
        if      (diff < -thold) diff += thold;
        else if (diff >  thold) diff -= thold;
        else diff = 0;
        BAYER(row,col) = SQR(avg+diff);
      }
    }
  }
#endif
  free(fimg);
}

void process(struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, void *ivoid, void *ovoid, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out)
{
  dt_iop_rawdenoise_data_t *d = (dt_iop_rawdenoise_data_t *)piece->data;
  if (d->threshold > 0.0)
    wavelet_denoise(ivoid, ovoid, roi_in, d->threshold, dt_image_flipped_filter(&piece->pipe->image));
  else
    memcpy(ovoid, ivoid, roi_out->width * roi_out->height * sizeof(float));
}

void reload_defaults(dt_iop_module_t *module)
{
  // init defaults:
  dt_iop_rawdenoise_params_t tmp = (dt_iop_rawdenoise_params_t)
  {
    0.01
  };
  memcpy(module->params, &tmp, sizeof(dt_iop_rawdenoise_params_t));
  memcpy(module->default_params, &tmp, sizeof(dt_iop_rawdenoise_params_t));

  // can't be switched on for non-raw images:
  if(dt_image_is_raw(&module->dev->image_storage)) module->hide_enable_button = 0;
  else module->hide_enable_button = 1;
}

void init(dt_iop_module_t *module)
{
  module->data = NULL;
  module->params = malloc(sizeof(dt_iop_rawdenoise_params_t));
  module->default_params = malloc(sizeof(dt_iop_rawdenoise_params_t));
  module->default_enabled = 0;

  // raw denoise must come just before demosaicing.
  module->priority = 78; // module order created by iop_dependencies.py, do not edit!
  module->params_size = sizeof(dt_iop_rawdenoise_params_t);
  module->gui_data = NULL;
}

void cleanup(dt_iop_module_t *module)
{
  free(module->gui_data);
  module->gui_data = NULL;
  free(module->params);
  module->params = NULL;
  free(module->data);
  module->data = NULL;
}

void commit_params (struct dt_iop_module_t *self, dt_iop_params_t *params, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  dt_iop_rawdenoise_params_t *p = (dt_iop_rawdenoise_params_t *)params;
  dt_iop_rawdenoise_data_t *d = (dt_iop_rawdenoise_data_t *)piece->data;
  if (!(pipe->image.flags & DT_IMAGE_RAW) || pipe->type == DT_DEV_PIXELPIPE_PREVIEW)
    piece->enabled = 0;
  d->threshold = p->threshold;
}

void init_pipe(struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  piece->data = malloc(sizeof(dt_iop_rawdenoise_data_t));
  self->commit_params(self, self->default_params, pipe, piece);
}

void cleanup_pipe(struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  free(piece->data);
}

void gui_update(dt_iop_module_t *self)
{
  dt_iop_module_t *module = (dt_iop_module_t *)self;
  dt_iop_rawdenoise_gui_data_t *g = (dt_iop_rawdenoise_gui_data_t *)self->gui_data;
  dt_iop_rawdenoise_params_t *p = (dt_iop_rawdenoise_params_t *)module->params;
  dtgtk_slider_set_value(g->threshold, p->threshold);
}

static void
threshold_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_rawdenoise_params_t *p = (dt_iop_rawdenoise_params_t *)self->params;
  p->threshold = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

void gui_init(dt_iop_module_t *self)
{
  self->gui_data = malloc(sizeof(dt_iop_rawdenoise_gui_data_t));
  dt_iop_rawdenoise_gui_data_t *g = (dt_iop_rawdenoise_gui_data_t *)self->gui_data;
  dt_iop_rawdenoise_params_t *p = (dt_iop_rawdenoise_params_t *)self->params;

  self->widget = GTK_WIDGET(gtk_vbox_new(FALSE, 0));

  g->threshold = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR, 0.0, 0.1, 0.001, p->threshold, 3));
  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(g->threshold), TRUE, TRUE, 0);
  dtgtk_slider_set_label(g->threshold, _("noise threshold"));
  g_signal_connect(G_OBJECT(g->threshold), "value-changed", G_CALLBACK(threshold_callback), self);
}

void gui_cleanup(dt_iop_module_t *self)
{
  free(self->gui_data);
  self->gui_data = NULL;
}
