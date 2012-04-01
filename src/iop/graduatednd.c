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
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#ifdef HAVE_GEGL
#include <gegl.h>
#endif
#include "develop/develop.h"
#include "develop/imageop.h"
#include "control/control.h"
#include "common/colorspaces.h"
#include "common/debug.h"
#include "dtgtk/slider.h"
#include "dtgtk/gradientslider.h"
#include "dtgtk/resetlabel.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include "gui/presets.h"
#include <xmmintrin.h>

#define CLIP(x) ((x<0.0f)?0.0f:(x>1.0f)?1.0f:x)
DT_MODULE(1)

typedef struct dt_iop_graduatednd_params_t
{
  float density;
  float compression;
  float rotation;
  float offset;
  float hue;
  float saturation;
}
dt_iop_graduatednd_params_t;

void init_presets (dt_iop_module_so_t *self)
{
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "begin", NULL, NULL, NULL);

  dt_gui_presets_add_generic(_("neutral grey ND2 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,0,0,50,0,0
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("neutral grey ND4 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    2,0,0,50,0,0
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("neutral grey ND8 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    3,0,0,50,0,0
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("neutral grey ND2 (hard)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,75,0,50,0,0
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("neutral grey ND4 (hard)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    2,75,0,50,0,0
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("neutral grey ND8 (hard)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    3,75,0,50,0,0
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("orange ND2 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,0,0,50,0.102439,0.8
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("yellow ND2 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,0,0,50,0.151220,0.5
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("purple ND2 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,0,0,50,0.824390,0.5
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("green ND2 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,0,0,50, 0.302439,0.5
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("red ND2 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,0,0,50,0,0.5
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("blue ND2 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    1,0,0,50,0.663415,0.5
  } , sizeof(dt_iop_graduatednd_params_t), 1);
  dt_gui_presets_add_generic(_("brown ND4 (soft)"), self->op, self->version(), &(dt_iop_graduatednd_params_t)
  {
    2,0,0,50,0.082927,0.25
  } , sizeof(dt_iop_graduatednd_params_t), 1);

  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "commit", NULL, NULL, NULL);
}

typedef struct dt_iop_graduatednd_gui_data_t
{
  GtkVBox   *vbox;
  GtkWidget  *label1,*label2,*label3,*label4,*label5,*label6;            			      // density, compression, rotation, offset, hue, saturation
  GtkDarktableSlider *scale1,*scale2,*scale3,*scale4;        // density, compression, rotation, offset
  GtkDarktableGradientSlider *gslider1,*gslider2;		// hue, saturation
}
dt_iop_graduatednd_gui_data_t;

typedef struct dt_iop_graduatednd_data_t
{
  float density;			          	// The density of filter 0-8 EV
  float compression;			        // Default 0% = soft and 100% = hard
  float rotation;		          	// 2*PI -180 - +180
  float offset;				            // Default 50%, centered, can be offsetted...
  float hue;                      // the hue
  float saturation;             // the saturation
}
dt_iop_graduatednd_data_t;

const char *name()
{
  return _("graduated density");
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
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "density"));
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "compression"));
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "rotation"));
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "split"));
}

void connect_key_accels(dt_iop_module_t *self)
{
  dt_iop_graduatednd_gui_data_t *g =
      (dt_iop_graduatednd_gui_data_t*)self->gui_data;

  dt_accel_connect_slider_iop(self, "density", GTK_WIDGET(g->scale1));
  dt_accel_connect_slider_iop(self, "compression", GTK_WIDGET(g->scale2));
  dt_accel_connect_slider_iop(self, "rotation", GTK_WIDGET(g->scale3));
  dt_accel_connect_slider_iop(self, "split", GTK_WIDGET(g->scale4));
}

static inline float
f (const float t, const float c, const float x)
{
  return (t/(1.0f + powf(c, -x*6.0f)) + (1.0f-t)*(x*.5f+.5f));
}

typedef struct dt_iop_vector_2d_t
{
  double x;
  double y;
} dt_iop_vector_2d_t;

// static int
// get_grab(float pointerx, float pointery, float zoom_scale){
//   const float radius = 5.0/zoom_scale;
//
//   //TODO add correct calculations
//   if(powf(pointerx-startx, 2)+powf(pointery, 2) <= powf(radius, 2)) return 2;    // x size
//   if(powf(pointerx, 2)+powf(pointery-starty, 2) <= powf(radius, 2)) return 4;    // y size
//   if(powf(pointerx, 2)+powf(pointery, 2) <= powf(radius, 2)) return 1;           // center
//   if(powf(pointerx-endx, 2)+powf(pointery, 2) <= powf(radius, 2)) return 8;      // x falloff
//   if(powf(pointerx, 2)+powf(pointery-endy, 2) <= powf(radius, 2)) return 16;     // y falloff
//
//   return 0;
// }

// static void
// draw_overlay(cairo_t *cr, float wd, float ht, int grab, float zoom_scale)
// {
//   const float radius_reg = 4.0/zoom_scale;
//
//   // top-left
//   cairo_arc(cr, 0.0, 0.0, radius_reg, 0.0, M_PI*2.0);
//   cairo_stroke(cr);
//
//   // bottom-right
//   cairo_arc(cr, wd, ht, radius_reg, 0.0, M_PI*2.0);
//   cairo_stroke(cr);
// }

void
gui_post_expose(struct dt_iop_module_t *self, cairo_t *cr, int32_t width, int32_t height, int32_t pointerx, int32_t pointery)
{

  dt_develop_t *dev             = self->dev;
//   dt_iop_graduatednd_gui_data_t *g = (dt_iop_graduatednd_gui_data_t *)self->gui_data;
//   dt_iop_graduatednd_params_t *p   = (dt_iop_graduatednd_params_t *)self->params;

  // general house keeping.
  int32_t zoom, closeup;
  float zoom_x, zoom_y;
  float wd = dev->preview_pipe->backbuf_width;
  float ht = dev->preview_pipe->backbuf_height;

// Commented out to allow this stub to be compilable with some versions of gcc ...
//  float bigger_side, smaller_side;
//  if(wd >= ht)
//  {
//    bigger_side = wd;
//    smaller_side = ht;
//  }
//  else
//  {
//    bigger_side = ht;
//    smaller_side = wd;
//  }
  DT_CTL_GET_GLOBAL(zoom_y, dev_zoom_y);
  DT_CTL_GET_GLOBAL(zoom_x, dev_zoom_x);
  DT_CTL_GET_GLOBAL(zoom, dev_zoom);
  DT_CTL_GET_GLOBAL(closeup, dev_closeup);
  float zoom_scale = dt_dev_get_zoom_scale(dev, zoom, closeup ? 2 : 1, 1);
  float pzx, pzy;
  dt_dev_get_pointer_zoom_pos(dev, pointerx, pointery, &pzx, &pzy);
  pzx += 0.5f;
  pzy += 0.5f;

  cairo_translate(cr, width/2.0, height/2.0);
  cairo_scale(cr, zoom_scale, zoom_scale);
  cairo_translate(cr, -.5f*wd-zoom_x*wd, -.5f*ht-zoom_y*ht);

  // now top-left is (0,0) and bottom-right is (wd,ht)

  //TODO calculate values for and execute cairo_translate() and cairo_rotate().
  //     of course the results can also be passed to draw_overlay() if it's easier to use them there instead.

  // finally draw the guides. if rotation and translation is done correctly it can be drawn statically in draw_overlay().
//   int grab = get_grab(pzx*wd*0.5, pzy*ht*0.5, zoom_scale); //FIXME use the calculated offsets in the 1st and 2nd paramter to get correct mouse positions
  cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND);
  cairo_set_line_width(cr, 3.0/zoom_scale);
  cairo_set_source_rgba(cr, .3, .3, .3, .8);
//   draw_overlay(cr, wd, ht, grab, zoom_scale); //FIXME removed to not annoy users as long as this is just a demo.
  cairo_set_line_width(cr, 1.0/zoom_scale);
  cairo_set_source_rgba(cr, .8, .8, .8, .8);
//   draw_overlay(cr, wd, ht, grab, zoom_scale); //FIXME removed to not annoy users as long as this is just a demo.

}

int
mouse_moved(struct dt_iop_module_t *self, double x, double y, int which)
{
  //TODO see vignette.c ...
  dt_control_queue_redraw_center();
  return 0;
}

int
button_pressed(struct dt_iop_module_t *self, double x, double y, int which, int type, uint32_t state)
{
  if(which == 1)
    return 1;
  return 0;
}

int button_released(struct dt_iop_module_t *self, double x, double y, int which, uint32_t state)
{
  if(which == 1)
    return 1;
  return 0;
}

void process (struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, void *ivoid, void *ovoid, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out)
{
  const dt_iop_graduatednd_data_t *data = (dt_iop_graduatednd_data_t *)piece->data;
  const int ch = piece->colors;

  const int ix= (roi_in->x);
  const int iy= (roi_in->y);
  const float iw=piece->buf_in.width*roi_out->scale;
  const float ih=piece->buf_in.height*roi_out->scale;
  const float hw=iw/2.0;
  const float hh=ih/2.0;
  const float hw_inv=1.0/hw;
  const float hh_inv=1.0/hh;
  const float v=(-data->rotation/180)*M_PI;
  const float sinv=sin(v);
  const float cosv=cos(v);
  const float filter_radie=sqrt((hh*hh)+(hw*hw))/hh;
  const float offset=data->offset/100.0*2;

  float color[3];
  hsl2rgb(color,data->hue,data->saturation,0.5);
  if (data->density < 0)
    for ( int l=0; l<3; l++ )
      color[l] = 1.0-color[l];

#if 1
  const float filter_compression = 1.0/filter_radie/(1.0-(0.5+(data->compression/100.0)*0.9/2.0))*0.5;
#else
  const float compression = data->compression/100.0f;
  const float t = 1.0f - .8f/(.8f + compression);
  const float c = 1.0f + 1000.0f*powf(4.0, compression);
#endif

  if (data->density > 0)
  {
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(roi_out, color, data, ivoid, ovoid) schedule(static)
#endif
  for(int y=0; y<roi_out->height; y++)
  {
    int k=roi_out->width*y*ch;
    const float *in = (float*)ivoid + k;
    float *out = (float*)ovoid + k;

    float length = (sinv * (-1.0+ix*hw_inv) - cosv * (-1.0+(iy+y)*hh_inv) - 1.0 + offset) * filter_compression;
    const float length_inc = sinv * hw_inv * filter_compression;

    __m128 c = _mm_set_ps(0,color[2],color[1],color[0]); 
    __m128 c1 = _mm_sub_ps(_mm_set1_ps(1.0f),c); 

    for(int x=0; x<roi_out->width; x++, in+=ch, out+=ch)
    {
      length += length_inc;

#if 1
      // !!! approximation is ok only when highest density is 8
      // for input x = (data->density * CLIP( 0.5+length ), calculate 2^x as (e^(ln2*x/8))^8
      // use exp2f approximation to calculate e^(ln2*x/8)
      // in worst case - density==8,CLIP(0.5-length) == 1.0 it gives 0.6% of error
      const float t = 0.693147181f /* ln2 */ * (data->density * CLIP( 0.5f+length )/8.0f);
      float d1 = t*t*0.5f;
      float d2 = d1*t*0.333333333f;
      float d3 = d2*t*0.25f;
      float d = 1+t+d1+d2+d3; /* taylor series for e^x till x^4 */
      __m128 density = _mm_set1_ps(d);
      density = _mm_mul_ps(density,density);
      density = _mm_mul_ps(density,density);
      density = _mm_mul_ps(density,density);
#else
      // use fair exp2f
      __m128 density = _mm_set1_ps(exp2f(data->density * CLIP( 0.5f+length )));
#endif
      
      /* max(0,in / (c + (1-c)*density)) */
      _mm_stream_ps(out,_mm_max_ps(_mm_set1_ps(0.0f),_mm_div_ps(_mm_load_ps(in),_mm_add_ps(c,_mm_mul_ps(c1,density)))));
    }
  }
  }
  else
  {
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(roi_out, color, data, ivoid, ovoid) schedule(static)
#endif
  for(int y=0; y<roi_out->height; y++)
  {
    int k=roi_out->width*y*ch;
    const float *in = (float*)ivoid + k;
    float *out = (float*)ovoid + k;

    float length = (sinv * (-1.0f+ix*hw_inv) - cosv * (-1.0f+(iy+y)*hh_inv) - 1.0f + offset) * filter_compression;
    const float length_inc = sinv * hw_inv * filter_compression;

    __m128 c = _mm_set_ps(0,color[2],color[1],color[0]); 
    __m128 c1 = _mm_sub_ps(_mm_set1_ps(1.0f),c); 

    for(int x=0; x<roi_out->width; x++, in+=ch, out+=ch)
    {
      length += length_inc;

#if 1
      // !!! approximation is ok only when lowest density is -8
      // for input x = (-data->density * CLIP( 0.5-length ), calculate 2^x as (e^(ln2*x/8))^8
      // use exp2f approximation to calculate e^(ln2*x/8)
      // in worst case - density==-8,CLIP(0.5-length) == 1.0 it gives 0.6% of error
      const float t = 0.693147181f /* ln2 */ * (-data->density * CLIP( 0.5f-length )/8.0f);
      float d1 = t*t*0.5f;
      float d2 = d1*t*0.333333333f;
      float d3 = d2*t*0.25f;
      float d = 1+t+d1+d2+d3; /* taylor series for e^x till x^4 */
      __m128 density = _mm_set1_ps(d);
      density = _mm_mul_ps(density,density);
      density = _mm_mul_ps(density,density);
      density = _mm_mul_ps(density,density);
#else
      __m128 density = _mm_set1_ps(exp2f(-data->density * CLIP( 0.5f-length )));
#endif

      /* max(0,in * (c + (1-c)*density)) */
      _mm_stream_ps(out,_mm_max_ps(_mm_set1_ps(0.0f),_mm_mul_ps(_mm_load_ps(in),_mm_add_ps(c,_mm_mul_ps(c1,density)))));
    }
  }
  }
  _mm_sfence();
}

static void
density_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)self->params;
  p->density = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

static void
compression_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)self->params;
  p->compression = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

static void
rotation_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)self->params;
  p->rotation= dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}


static void
offset_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)self->params;
  p->offset= dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}


void commit_params (struct dt_iop_module_t *self, dt_iop_params_t *p1, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)p1;
#ifdef HAVE_GEGL
  fprintf(stderr, "[velvia] TODO: implement gegl version!\n");
  // pull in new params to gegl
#else
  dt_iop_graduatednd_data_t *d = (dt_iop_graduatednd_data_t *)piece->data;
  d->density = p->density;
  d->compression = p->compression;
  d->rotation = p->rotation;
  d->offset = p->offset;
  d->hue = p->hue;
  d->saturation = p->saturation;
#endif
}

void init_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
#ifdef HAVE_GEGL
  // create part of the gegl pipeline
  piece->data = NULL;
#else
  piece->data = malloc(sizeof(dt_iop_graduatednd_data_t));
  memset(piece->data,0,sizeof(dt_iop_graduatednd_data_t));
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
  dt_iop_graduatednd_gui_data_t *g = (dt_iop_graduatednd_gui_data_t *)self->gui_data;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)module->params;
  dtgtk_slider_set_value (g->scale1, p->density);
  dtgtk_slider_set_value (g->scale2, p->compression);
  dtgtk_slider_set_value (g->scale3, p->rotation);
  dtgtk_slider_set_value (g->scale4, p->offset);
  dtgtk_gradient_slider_set_value(g->gslider1,p->hue);
  dtgtk_gradient_slider_set_value(g->gslider2,p->saturation);
}

void init(dt_iop_module_t *module)
{
  module->params = malloc(sizeof(dt_iop_graduatednd_params_t));
  module->default_params = malloc(sizeof(dt_iop_graduatednd_params_t));
  module->default_enabled = 0;
  module->priority = 254; // module order created by iop_dependencies.py, do not edit!
  module->params_size = sizeof(dt_iop_graduatednd_params_t);
  module->gui_data = NULL;
  dt_iop_graduatednd_params_t tmp = (dt_iop_graduatednd_params_t)
  {
    2.0,0,0,50,0,0
  };
  memcpy(module->params, &tmp, sizeof(dt_iop_graduatednd_params_t));
  memcpy(module->default_params, &tmp, sizeof(dt_iop_graduatednd_params_t));
}

void cleanup(dt_iop_module_t *module)
{
  free(module->gui_data);
  module->gui_data = NULL;
  free(module->params);
  module->params = NULL;
}

static void
hue_callback(GtkDarktableGradientSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)self->params;
  dt_iop_graduatednd_gui_data_t *g = (dt_iop_graduatednd_gui_data_t *)self->gui_data;

  double hue=dtgtk_gradient_slider_get_value(g->gslider1);
  //fprintf(stderr," hue: %f, saturation: %f\n",hue,dtgtk_gradient_slider_get_value(g->gslider2));
  double saturation=1.0;
  float color[3];
  hsl2rgb(color,hue,saturation,0.5);

  GdkColor c;
  c.red=color[0]*65535.0;
  c.green=color[1]*65535.0;
  c.blue=color[2]*65535.0;

  dtgtk_gradient_slider_set_stop(g->gslider2,1.0,c);  // Update saturation end color

  if(self->dt->gui->reset)
    return;
  gtk_widget_draw(GTK_WIDGET(g->gslider2),NULL);

  p->hue = dtgtk_gradient_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

static void
saturation_callback(GtkDarktableGradientSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)self->params;

  p->saturation = dtgtk_gradient_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}


void gui_init(struct dt_iop_module_t *self)
{
  self->gui_data = malloc(sizeof(dt_iop_graduatednd_gui_data_t));
  dt_iop_graduatednd_gui_data_t *g = (dt_iop_graduatednd_gui_data_t *)self->gui_data;
  dt_iop_graduatednd_params_t *p = (dt_iop_graduatednd_params_t *)self->params;

  self->widget = GTK_WIDGET(gtk_hbox_new(FALSE, 0));
  g->vbox = GTK_VBOX(gtk_vbox_new(FALSE, DT_GUI_IOP_MODULE_CONTROL_SPACING));
  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(g->vbox), TRUE, TRUE, 5);


  /* adding the labels */

  g->label5 = dtgtk_reset_label_new(_("hue"), self, &p->hue, sizeof(float));
  g->label6 = dtgtk_reset_label_new(_("saturation"), self, &p->saturation, sizeof(float));

  g->scale1 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR,-8.0, 8.0, 0.1, p->density, 2));
  g->scale2 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR,0.0, 100.0, 1.0, p->compression, 0));
  dtgtk_slider_set_format_type(g->scale2,DARKTABLE_SLIDER_FORMAT_PERCENT);
  g->scale3 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR,-180, 180,0.5, p->rotation, 2));
  g->scale4 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR,0.0, 100.0, 1.0, p->offset, 0));
  dtgtk_slider_set_format_type(g->scale4,DARKTABLE_SLIDER_FORMAT_PERCENT);

  dtgtk_slider_set_label(g->scale1,_("density"));
  dtgtk_slider_set_unit(g->scale1,"EV");
  dtgtk_slider_set_label(g->scale2,_("compression"));
  dtgtk_slider_set_unit(g->scale2,"%");
  dtgtk_slider_set_label(g->scale3,_("rotation"));
  dtgtk_slider_set_unit(g->scale3,"°");
  dtgtk_slider_set_label(g->scale4,_("split"));
  dtgtk_slider_set_unit(g->scale4,"%");

  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale1), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale2), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale3), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale4), TRUE, TRUE, 0);

  /* hue slider */
  int lightness=32768;
  g->gslider1=DTGTK_GRADIENT_SLIDER(dtgtk_gradient_slider_new_with_color((GdkColor)
  {
    0,lightness,0,0
  },(GdkColor)
  {
    0,lightness,0,0
  }));
  dtgtk_gradient_slider_set_stop(g->gslider1,0.166,(GdkColor)
  {
    0,lightness,lightness,0
  });
  dtgtk_gradient_slider_set_stop(g->gslider1,0.332,(GdkColor)
  {
    0,0,lightness,0
  });
  dtgtk_gradient_slider_set_stop(g->gslider1,0.498,(GdkColor)
  {
    0,0,lightness,lightness
  });
  dtgtk_gradient_slider_set_stop(g->gslider1,0.664,(GdkColor)
  {
    0,0,0,lightness
  });
  dtgtk_gradient_slider_set_stop(g->gslider1,0.83,(GdkColor)
  {
    0,lightness,0,lightness
  });
  g_object_set(G_OBJECT(g->gslider1), "tooltip-text", _("select the hue tone of filter"), (char *)NULL);
  g_signal_connect (G_OBJECT (g->gslider1), "value-changed",
                    G_CALLBACK (hue_callback), self);

  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->gslider1), TRUE, TRUE, 0);

  /* saturation slider */
  g->gslider2=DTGTK_GRADIENT_SLIDER(dtgtk_gradient_slider_new_with_color((GdkColor)
  {
    0,lightness,lightness,lightness
  },(GdkColor)
  {
    0,lightness,lightness,lightness
  }));
  g_object_set(G_OBJECT(g->gslider2), "tooltip-text", _("select the saturation of filter"), (char *)NULL);
  g_signal_connect (G_OBJECT (g->gslider2), "value-changed",
                    G_CALLBACK (saturation_callback), self);

  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->gslider2), TRUE, TRUE, 0);


  g_object_set(G_OBJECT(g->scale1), "tooltip-text", _("the density in EV for the filter"), (char *)NULL);
  /* xgettext:no-c-format */
  g_object_set(G_OBJECT(g->scale2), "tooltip-text", _("compression of graduation:\n0% = soft, 100% = hard"), (char *)NULL);
  g_object_set(G_OBJECT(g->scale3), "tooltip-text", _("rotation of filter -180 to 180 degrees"), (char *)NULL);
  g_object_set(G_OBJECT(g->scale4), "tooltip-text", _("offset of filter in angle of rotation"), (char *)NULL);

  g_signal_connect (G_OBJECT (g->scale1), "value-changed",
                    G_CALLBACK (density_callback), self);
  g_signal_connect (G_OBJECT (g->scale2), "value-changed",
                    G_CALLBACK (compression_callback), self);
  g_signal_connect (G_OBJECT (g->scale3), "value-changed",
                    G_CALLBACK (rotation_callback), self);
  g_signal_connect (G_OBJECT (g->scale4), "value-changed",
                    G_CALLBACK (offset_callback), self);
}

void gui_cleanup(struct dt_iop_module_t *self)
{
  free(self->gui_data);
  self->gui_data = NULL;
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
