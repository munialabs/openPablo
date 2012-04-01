/*
		This file is part of darktable,
		copyright (c) 2010 Henrik Andersson.

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
#include "develop/develop.h"
#include "develop/imageop.h"
#include "control/control.h"
#include "dtgtk/slider.h"
#include "dtgtk/resetlabel.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"

#include <gtk/gtk.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define CLIP(x) ((x<0)?0.0:(x>1.0)?1.0:x)
#define LCLIP(x) ((x<0)?0.0:(x>100.0)?100.0:x)
DT_MODULE(1)

typedef struct dt_iop_bloom_params_t
{
  float size;
  float threshold;
  float strength;
}
dt_iop_bloom_params_t;

typedef struct dt_iop_bloom_gui_data_t
{
  GtkVBox   *vbox;
  GtkWidget  *label1,*label2,*label3;			// size,threshold,strength
  GtkDarktableSlider *scale1,*scale2,*scale3;       // size,threshold,strength
}
dt_iop_bloom_gui_data_t;

typedef struct dt_iop_bloom_data_t
{
  float size;
  float threshold;
  float strength;
}
dt_iop_bloom_data_t;

const char *name()
{
  return _("bloom");
}

int flags()
{
  return IOP_FLAGS_INCLUDE_IN_STYLES | IOP_FLAGS_SUPPORTS_BLENDING;
}

int
groups ()
{
  return IOP_GROUP_EFFECT;
}


void init_key_accels(dt_iop_module_so_t *self)
{
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "size"));
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "threshold"));
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "strength"));
}

void connect_key_accels(dt_iop_module_t *self)
{
  dt_iop_bloom_gui_data_t *g = (dt_iop_bloom_gui_data_t*)self->gui_data;
  dt_accel_connect_slider_iop(self, "size", GTK_WIDGET(g->scale1));
  dt_accel_connect_slider_iop(self, "threshold", GTK_WIDGET(g->scale2));
  dt_accel_connect_slider_iop(self, "strength", GTK_WIDGET(g->scale3));
}

#define GAUSS(a,b,c,x) (a*pow(2.718281828,(-pow((x-b),2)/(pow(c,2)))))


void process (struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, void *ivoid, void *ovoid, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out)
{
  dt_iop_bloom_data_t *data = (dt_iop_bloom_data_t *)piece->data;
  float *in  = (float *)ivoid;
  float *out = (float *)ovoid;
  const int ch = piece->colors;

  /* gather light by threshold */
  float *blurlightness = malloc(roi_out->width*roi_out->height*sizeof(float));
  memset(blurlightness,0,(roi_out->width*roi_out->height*sizeof(float)));
  memcpy(out,in,roi_out->width*roi_out->height*ch*sizeof(float));

  int rad = 256*(fmin(100.0,data->size+1)/100.0);
  const int radius = MIN(256, ceilf(rad * roi_in->scale / piece->iscale));

  const float scale = 1.0 / exp2f ( -1.0*(fmin(100.0,data->strength+1)/100.0));

  /* get the thresholded lights into buffer */
#ifdef _OPENMP
  #pragma omp parallel for default(none) private(in, out) shared(ivoid, ovoid, roi_out, roi_in, data,blurlightness) schedule(static)
#endif
  for(int k=0; k<roi_out->width*roi_out->height; k++)
  {
    out = ((float *)ovoid) + ch*k;
    float L = out[0]*scale;
    if (L>data->threshold)
      blurlightness[k] = L;

    out +=ch;
  }


  /* horizontal blur into memchannel lightness */
  const int range = 2*radius+1;
  const int hr = range/2;

  const int size = roi_out->width>roi_out->height?roi_out->width:roi_out->height;
  float *scanline = malloc(size*sizeof(float));

  for(int iteration=0; iteration<8; iteration++)
  {
    int index=0;
    for(int y=0; y<roi_out->height; y++)
    {
      float L=0;
      int hits = 0;
      for(int x=-hr; x<roi_out->width; x++)
      {
        int op = x - hr-1;
        int np = x+hr;
        if(op>=0)
        {
          L-=blurlightness[index+op];
          hits--;
        }
        if(np < roi_out->width)
        {
          L+=blurlightness[index+np];
          hits++;
        }
        if(x>=0)
          scanline[x] = L/hits;
      }

      for (int x=0; x<roi_out->width; x++)
        blurlightness[index+x]=scanline[x];

      index+=roi_out->width;
    }

    /* vertical pass on blurlightness */
    const int opoffs = -(hr+1)*roi_out->width;
    const int npoffs = (hr)*roi_out->width;
    for(int x=0; x < roi_out->width; x++)
    {
      float L=0;
      int hits=0;
      int index = -hr*roi_out->width+x;
      for(int y=-hr; y<roi_out->height; y++)
      {
        int op=y-hr-1;
        int np= y + hr;

        if(op>=0)
        {
          L-=blurlightness[index+opoffs];
          hits--;
        }
        if(np < roi_out->height)
        {
          L+=blurlightness[index+npoffs];
          hits++;
        }
        if(y>=0)
          scanline[y] = L/hits;
        index += roi_out->width;
      }

      for (int y=0; y<roi_out->height; y++)
        blurlightness[y*roi_out->width+x]=scanline[y];

    }
  }

  /* screen blend lightness with orginal */

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(roi_out, in, out, data,blurlightness) schedule(static)
#endif
  for(int k=0; k<roi_out->width*roi_out->height; k++)
  {
    float *inp = in + ch*k;
    float *outp = out + ch*k;
    outp[0] = 100-(((100-inp[0])*(100- blurlightness[k]))/100); // Screen blend
    outp[1] = inp[1];
    outp[2] = inp[2];
  }

  if(scanline)
    free(scanline);
  if(blurlightness)
    free(blurlightness);
}

static void
strength_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_bloom_params_t *p = (dt_iop_bloom_params_t *)self->params;
  p->strength = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

static void
threshold_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_bloom_params_t *p = (dt_iop_bloom_params_t *)self->params;
  p->threshold = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

static void
size_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_bloom_params_t *p = (dt_iop_bloom_params_t *)self->params;
  p->size= dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

void commit_params (struct dt_iop_module_t *self, dt_iop_params_t *p1, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  dt_iop_bloom_params_t *p = (dt_iop_bloom_params_t *)p1;
#ifdef HAVE_GEGL
  fprintf(stderr, "[bloom] TODO: implement gegl version!\n");
  // pull in new params to gegl
#else
  dt_iop_bloom_data_t *d = (dt_iop_bloom_data_t *)piece->data;
  d->strength = p->strength;
  d->size = p->size;
  d->threshold = p->threshold;
#endif
}

void init_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
#ifdef HAVE_GEGL
  // create part of the gegl pipeline
  piece->data = NULL;
#else
  piece->data = malloc(sizeof(dt_iop_bloom_data_t));
  memset(piece->data,0,sizeof(dt_iop_bloom_data_t));
  self->commit_params(self, self->default_params, pipe, piece);
#endif
}

void cleanup_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
#ifdef HAVE_GEGL
  // clean up everything again.
  (void)gegl_node_remove_child(pipe->gegl, piece->input);
  // no free necessary, no data is alloc'ed
#else
  free(piece->data);
#endif
}

void gui_update(struct dt_iop_module_t *self)
{
  dt_iop_module_t *module = (dt_iop_module_t *)self;
  dt_iop_bloom_gui_data_t *g = (dt_iop_bloom_gui_data_t *)self->gui_data;
  dt_iop_bloom_params_t *p = (dt_iop_bloom_params_t *)module->params;
  dtgtk_slider_set_value(g->scale1, p->size);
  dtgtk_slider_set_value(g->scale2, p->threshold);
  dtgtk_slider_set_value(g->scale3, p->strength);
}

void init(dt_iop_module_t *module)
{
  module->params = malloc(sizeof(dt_iop_bloom_params_t));
  module->default_params = malloc(sizeof(dt_iop_bloom_params_t));
  module->default_enabled = 0;
  module->priority = 450; // module order created by iop_dependencies.py, do not edit!
  module->params_size = sizeof(dt_iop_bloom_params_t);
  module->gui_data = NULL;
  dt_iop_bloom_params_t tmp = (dt_iop_bloom_params_t)
  {
    20,90,25
  };
  memcpy(module->params, &tmp, sizeof(dt_iop_bloom_params_t));
  memcpy(module->default_params, &tmp, sizeof(dt_iop_bloom_params_t));
}

void cleanup(dt_iop_module_t *module)
{
  free(module->gui_data);
  module->gui_data = NULL;
  free(module->params);
  module->params = NULL;
}

void gui_init(struct dt_iop_module_t *self)
{
  self->gui_data = malloc(sizeof(dt_iop_bloom_gui_data_t));
  dt_iop_bloom_gui_data_t *g = (dt_iop_bloom_gui_data_t *)self->gui_data;
  dt_iop_bloom_params_t *p = (dt_iop_bloom_params_t *)self->params;

  self->widget = GTK_WIDGET(gtk_hbox_new(FALSE, 0));
  g->vbox = GTK_VBOX(gtk_vbox_new(FALSE, DT_GUI_IOP_MODULE_CONTROL_SPACING));
  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(g->vbox), TRUE, TRUE, 5);

  g->scale1 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR, 0.0, 100.0, 1.0, p->size, 0));
  g->scale2 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR, 0.0, 100.0, 1.0, p->threshold, 0));
  g->scale3 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR, 0.0, 100.0, 1.0, p->strength, 0));
  dtgtk_slider_set_format_type(g->scale1,DARKTABLE_SLIDER_FORMAT_PERCENT);
  dtgtk_slider_set_format_type(g->scale2,DARKTABLE_SLIDER_FORMAT_PERCENT);
  dtgtk_slider_set_format_type(g->scale3,DARKTABLE_SLIDER_FORMAT_PERCENT);
  dtgtk_slider_set_label(g->scale1,_("size"));
  dtgtk_slider_set_unit(g->scale1,"%");
  dtgtk_slider_set_label(g->scale2,_("threshold"));
  dtgtk_slider_set_unit(g->scale2,"%");
  dtgtk_slider_set_label(g->scale3,_("strength"));
  dtgtk_slider_set_unit(g->scale3,"%");

  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale1), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale2), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale3), TRUE, TRUE, 0);
  g_object_set(G_OBJECT(g->scale1), "tooltip-text", _("the size of bloom"), (char *)NULL);
  g_object_set(G_OBJECT(g->scale2), "tooltip-text", _("the threshold of light"), (char *)NULL);
  g_object_set(G_OBJECT(g->scale3), "tooltip-text", _("the strength of bloom"), (char *)NULL);

  g_signal_connect (G_OBJECT (g->scale1), "value-changed",
                    G_CALLBACK (size_callback), self);
  g_signal_connect (G_OBJECT (g->scale2), "value-changed",
                    G_CALLBACK (threshold_callback), self);
  g_signal_connect (G_OBJECT (g->scale3), "value-changed",
                    G_CALLBACK (strength_callback), self);
}

void gui_cleanup(struct dt_iop_module_t *self)
{
  free(self->gui_data);
  self->gui_data = NULL;
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
