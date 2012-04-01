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
#include "common/darktable.h"
#include "develop/develop.h"
#include "develop/imageop.h"
#include "control/control.h"
#include "control/conf.h"
#include "dtgtk/togglebutton.h"
#include "dtgtk/slider.h"
#include "dtgtk/gradientslider.h"
#include "gui/gtk.h"
#include "gui/presets.h"
#include <xmmintrin.h>


#define CLIP(x) (((x)>=0)?((x)<=1.0?(x):1.0):0.0)
DT_MODULE(1)
#define MAX_ZONE_SYSTEM_SIZE	24

/** gui params. */
typedef struct dt_iop_zonesystem_params_t
{
  int size;
  float zone[MAX_ZONE_SYSTEM_SIZE+1];
}
dt_iop_zonesystem_params_t;

/** and pixelpipe data is just the same */
typedef struct dt_iop_zonesystem_params_t dt_iop_zonesystem_data_t;

/*
void init_presets (dt_iop_module_so_t *self)
{
//   DT_DEBUG_SQLITE3_EXEC(darktable.db, "begin", NULL, NULL, NULL);

  dt_gui_presets_add_generic(_("Fill-light 0.25EV with 4 zones"), self->op, self->version(), &(dt_iop_zonesystem_params_t){0.25,0.25,4.0} , sizeof(dt_iop_zonesystem_params_t), 1);
  dt_gui_presets_add_generic(_("Fill-shadow -0.25EV with 4 zones"), self->op, self->version(), &(dt_iop_zonesystem_params_t){-0.25,0.25,4.0} , sizeof(dt_iop_zonesystem_params_t), 1);

//   DT_DEBUG_SQLITE3_EXEC(darktable.db, "commit", NULL, NULL, NULL);
}
*/

typedef struct dt_iop_zonesystem_gui_data_t
{
  guchar *preview_buffer;
  int  preview_width,preview_height;
  GtkWidget *preview;
  GtkWidget   *zones;
  float press_x, press_y, mouse_x, mouse_y;
  gboolean hilite_zone;
  gboolean is_dragging;
  int current_zone;
  int zone_under_mouse;
  dt_pthread_mutex_t lock;
}
dt_iop_zonesystem_gui_data_t;


const char *name()
{
  return _("zone system");
}

int flags()
{
  return IOP_FLAGS_INCLUDE_IN_STYLES;
}

int
groups ()
{
  return IOP_GROUP_TONE;
}

/* get the zone index of pixel lightness from zonemap */
static inline int
_iop_zonesystem_zone_index_from_lightness (float lightness,float *zonemap,int size)
{
  for (int k=0; k<size-1; k++)
    if (zonemap[k+1] >= lightness)
      return k;
  return size-1;
}

/* calculate a zonemap with scale values for each zone based on controlpoints from param */
static inline void
_iop_zonesystem_calculate_zonemap (struct dt_iop_zonesystem_params_t *p, float *zonemap)
{
  int steps=0;
  int pk=0;

  for (int k=0; k<p->size; k++)
  {
    if((k>0 && k<p->size-1) && p->zone[k] == -1)
      steps++;
    else
    {
      /* set 0 and 1.0 for first and last element in zonesystem size, or the actually parameter value */
      zonemap[k] = k==0?0.0:k==(p->size-1)?1.0:p->zone[k];

      /* for each step from pk to k, calculate values
          for now this is linear distributed
      */
      for (int l=1; l<=steps; l++)
        zonemap[pk+l] = zonemap[pk]+(((zonemap[k]-zonemap[pk])/(steps+1))*l);

      /* store k into pk and reset zone steps for next range*/
      pk = k;
      steps = 0;
    }
  }
}

#define GAUSS(a,b,c,x) (a*pow(2.718281828,(-pow((x-b),2)/(pow(c,2)))))
void process (struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, void *ivoid, void *ovoid, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out)
{
  float *in;
  float *out;
  dt_iop_zonesystem_gui_data_t *g = NULL;
  dt_iop_zonesystem_data_t *data = (dt_iop_zonesystem_data_t*)piece->data;

  guchar *buffer = NULL;
  if( self->dev->gui_attached && piece->pipe->type == DT_DEV_PIXELPIPE_PREVIEW )
  {
    g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
    dt_pthread_mutex_lock(&g->lock);
    if(g->preview_buffer)
      g_free (g->preview_buffer);

    buffer = g->preview_buffer = g_malloc (roi_in->width*roi_in->height);
    g->preview_width=roi_out->width;
    g->preview_height=roi_out->height;
  }

  /* calculate zonemap */
  const int size = data->size;
  float zonemap[MAX_ZONE_SYSTEM_SIZE]= {-1};
  _iop_zonesystem_calculate_zonemap (data, zonemap);
  const int ch = piece->colors;

  /* if gui and have buffer lets gaussblur and fill buffer with zone indexes */
  if( self->dev->gui_attached && g && buffer)
  {
    /* setup gaussian kernel */
    const int radius = 8;
    const int rad = MIN(radius, ceilf(radius * roi_in->scale / piece->iscale));
    const int wd = 2*rad+1;
    float mat[wd*wd];
    float *m;
    const float sigma2 = (2.5*2.5)*(radius*roi_in->scale/piece->iscale)*(radius*roi_in->scale/piece->iscale);
    float weight = 0.0f;

    memset(mat, 0, wd*wd*sizeof(float));

    m = mat;
    for(int l=-rad; l<=rad; l++) for(int k=-rad; k<=rad; k++,m++)
        weight += *m = expf(- (l*l + k*k)/(2.f*sigma2));
    m = mat;
    for(int l=-rad; l<=rad; l++) for(int k=-rad; k<=rad; k++,m++)
        *m /= weight;

    /* gauss blur the L channel */
#ifdef _OPENMP
    #pragma omp parallel for default(none) private(in, out, m) shared(mat, ivoid, ovoid, roi_out, roi_in) schedule(static)
#endif
    for(int j=rad; j<roi_out->height-rad; j++)
    {
      in  = ((float *)ivoid) + ch*(j*roi_in->width  + rad);
      out = ((float *)ovoid) + ch*(j*roi_out->width + rad);
      for(int i=rad; i<roi_out->width-rad; i++)
      {
        for(int c=0; c<3; c++) out[c] = 0.0f;
        float sum = 0.0;
        m = mat;
        for(int l=-rad; l<=rad; l++)
        {
          float *inrow = in + ch*(l*roi_in->width-rad);
          for(int k=-rad; k<=rad; k++,inrow+=ch,m++)
            sum += *m * inrow[0];
        }
        out[0] = sum;
        out += ch;
        in += ch;
      }
    }

    /* create zonemap preview */
//     in  = (float *)ivoid;
    out = (float *)ovoid;
#ifdef _OPENMP
    #pragma omp parallel for default(none) shared(roi_out,out,buffer,g,zonemap) schedule(static)
#endif
    for (int k=0; k<roi_out->width*roi_out->height; k++)
    {
      buffer[k] = _iop_zonesystem_zone_index_from_lightness (out[ch*k]/100.0f, zonemap, size);
    }

    dt_pthread_mutex_unlock(&g->lock);
  }

  /* process the image */
  in  = (float *)ivoid;
  out = (float *)ovoid;

  const float rzscale = (size-1)/100.0f;

  float zonemap_offset[MAX_ZONE_SYSTEM_SIZE]= {-1};
  float zonemap_scale[MAX_ZONE_SYSTEM_SIZE]= {-1};

  // precompute scale and offset
  for (int k=0; k < size-1; k++) zonemap_scale[k]  = (zonemap[k+1]-zonemap[k])*(size-1);
  for (int k=0; k < size-1; k++) zonemap_offset[k] = 100.0f * ((k+1)*zonemap[k] - k*zonemap[k+1]) ;

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(roi_out, in, out, zonemap_scale,zonemap_offset) schedule(static)
#endif
  for (int j=0; j<roi_out->height; j++)
  for (int i=0; i<roi_out->width; i++)
  {
    /* remap lightness into zonemap and apply lightness */
    const float *inp = in + ch*(j*roi_out->width+i);
    float *outp = out + ch*(j*roi_out->width+i);

    const int rz = CLAMPS(inp[0]*rzscale, 0, size-2);  // zone index

    const float zs = ((rz > 0) ? (zonemap_offset[rz]/inp[0]) : 0) + zonemap_scale[rz];

    _mm_stream_ps(outp,_mm_mul_ps(_mm_load_ps(inp),_mm_set1_ps(zs)));
  }

  _mm_sfence();

}


void commit_params (struct dt_iop_module_t *self, dt_iop_params_t *p1, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  dt_iop_zonesystem_params_t *p = (dt_iop_zonesystem_params_t *)p1;
#ifdef HAVE_GEGL
  fprintf(stderr, "[zonesystem] TODO: implement gegl version!\n");
  // pull in new params to gegl
#else
  dt_iop_zonesystem_data_t *d = (dt_iop_zonesystem_data_t *)piece->data;
  d->size = p->size;
  for(int i=0; i<=MAX_ZONE_SYSTEM_SIZE; i++)
    d->zone[i] = p->zone[i];
#endif
}

void init_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
#ifdef HAVE_GEGL
  // create part of the gegl pipeline
  piece->data = NULL;
#else
  piece->data = malloc(sizeof(dt_iop_zonesystem_data_t));
  memset(piece->data,0,sizeof(dt_iop_zonesystem_data_t));
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
//  dt_iop_module_t *module = (dt_iop_module_t *)self;
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
// dt_iop_zonesystem_params_t *p = (dt_iop_zonesystem_params_t *)module->params;
  gtk_widget_queue_draw (GTK_WIDGET (g->zones));
}

void init(dt_iop_module_t *module)
{
  module->params = malloc(sizeof(dt_iop_zonesystem_params_t));
  module->default_params = malloc(sizeof(dt_iop_zonesystem_params_t));
  module->default_enabled = 0;
  module->priority = 588; // module order created by iop_dependencies.py, do not edit!
  module->params_size = sizeof(dt_iop_zonesystem_params_t);
  module->gui_data = NULL;
  dt_iop_zonesystem_params_t tmp = (dt_iop_zonesystem_params_t)
  {
    10, {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
  };
  memcpy(module->params, &tmp, sizeof(dt_iop_zonesystem_params_t));
  memcpy(module->default_params, &tmp, sizeof(dt_iop_zonesystem_params_t));
}

void cleanup(dt_iop_module_t *module)
{
  free(module->gui_data);
  module->gui_data = NULL;
  free(module->params);
  module->params = NULL;
}

int button_released(struct dt_iop_module_t *self, double x, double y, int which, uint32_t state)
{
  self->request_color_pick=0;
  return 1;
}

static void _iop_zonesystem_redraw_preview_callback(gpointer instance, gpointer user_data);

static gboolean dt_iop_zonesystem_preview_expose(GtkWidget *widget, GdkEventExpose *event, dt_iop_module_t *self);

static gboolean dt_iop_zonesystem_bar_expose(GtkWidget *widget, GdkEventExpose *event, dt_iop_module_t *self);
static gboolean dt_iop_zonesystem_bar_motion_notify(GtkWidget *widget, GdkEventMotion *event, dt_iop_module_t *self);
static gboolean dt_iop_zonesystem_bar_leave_notify(GtkWidget *widget, GdkEventCrossing *event, dt_iop_module_t *self);
static gboolean dt_iop_zonesystem_bar_button_press(GtkWidget *widget, GdkEventButton *event, dt_iop_module_t *self);
static gboolean dt_iop_zonesystem_bar_button_release(GtkWidget *widget, GdkEventButton *event, dt_iop_module_t *self);
static gboolean dt_iop_zonesystem_bar_scrolled(GtkWidget *widget, GdkEventScroll *event, dt_iop_module_t *self);



void gui_init(struct dt_iop_module_t *self)
{
  self->gui_data = malloc(sizeof(dt_iop_zonesystem_gui_data_t));
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  g->preview_buffer = NULL;
  g->is_dragging = FALSE;
  g->hilite_zone = FALSE;
  g->preview_width=g->preview_height=0;

  dt_pthread_mutex_init(&g->lock, NULL);

  self->widget = gtk_vbox_new (FALSE,DT_GUI_IOP_MODULE_CONTROL_SPACING);

  /* create the zone preview widget */
  const int panel_width = MAX(-1, MIN(500, dt_conf_get_int("panel_width")));

  g->preview = gtk_drawing_area_new();
  g_signal_connect (G_OBJECT (g->preview), "expose-event", G_CALLBACK (dt_iop_zonesystem_preview_expose), self);
  gtk_widget_add_events (GTK_WIDGET (g->preview), GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK);
  gtk_widget_set_size_request(g->preview, panel_width * 0.8, panel_width * 0.8);

  /* create the zonesystem bar widget */
  g->zones = gtk_drawing_area_new();
  g_object_set (GTK_OBJECT(g->zones), "tooltip-text", _("lightness zones\nuse mouse scrollwheel to change the number of zones\nleft-click on a border to create a marker\nright-click on a marker to delete it"), (char *)NULL);
  g_signal_connect (G_OBJECT (g->zones), "expose-event", G_CALLBACK (dt_iop_zonesystem_bar_expose), self);
  g_signal_connect (G_OBJECT (g->zones), "motion-notify-event", G_CALLBACK (dt_iop_zonesystem_bar_motion_notify), self);
  g_signal_connect (G_OBJECT (g->zones), "leave-notify-event", G_CALLBACK (dt_iop_zonesystem_bar_leave_notify), self);
  g_signal_connect (G_OBJECT (g->zones), "button-press-event", G_CALLBACK (dt_iop_zonesystem_bar_button_press), self);
  g_signal_connect (G_OBJECT (g->zones), "button-release-event", G_CALLBACK (dt_iop_zonesystem_bar_button_release), self);
  g_signal_connect (G_OBJECT (g->zones), "scroll-event", G_CALLBACK (dt_iop_zonesystem_bar_scrolled), self);
  gtk_widget_add_events (GTK_WIDGET (g->zones), GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK);
  gtk_widget_set_size_request(g->zones, -1, 40);

  GtkWidget *aspect = gtk_aspect_frame_new(NULL, .5f, .5f, 1.0f, FALSE);
  gtk_frame_set_shadow_type(GTK_FRAME(aspect), GTK_SHADOW_NONE);
  gtk_container_add(GTK_CONTAINER(aspect), g->preview);
  gtk_box_pack_start (GTK_BOX (self->widget),aspect,TRUE,TRUE,0);
  gtk_box_pack_start (GTK_BOX (self->widget),g->zones,TRUE,TRUE,0);

  /* add signal handler for preview pipe finish to redraw the preview */
  dt_control_signal_connect(darktable.signals,
			    DT_SIGNAL_DEVELOP_PREVIEW_PIPE_FINISHED, 
			    G_CALLBACK(_iop_zonesystem_redraw_preview_callback), 
			    self);


}

void gui_cleanup(struct dt_iop_module_t *self)
{
  dt_control_signal_disconnect(darktable.signals, 
			       G_CALLBACK(_iop_zonesystem_redraw_preview_callback), 
			       self);

  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  dt_pthread_mutex_destroy(&g->lock);
  self->request_color_pick = 0;
  free(self->gui_data);
  self->gui_data = NULL;
}

#define DT_ZONESYSTEM_INSET 5
#define DT_ZONESYSTEM_BAR_SPLIT_WIDTH 0.0
#define DT_ZONESYSTEM_REFERENCE_SPLIT 0.30
static gboolean
dt_iop_zonesystem_bar_expose (GtkWidget *widget, GdkEventExpose *event, dt_iop_module_t *self)
{
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  dt_iop_zonesystem_params_t *p = (dt_iop_zonesystem_params_t *)self->params;

  const int inset = DT_ZONESYSTEM_INSET;
  int width = widget->allocation.width, height = widget->allocation.height;
  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(cst);

  /* clear background */
  cairo_set_source_rgb (cr, .15, .15, .15);
  cairo_paint(cr);


  /* translate and scale */
  width-=2*inset;
  height-=2*inset;
  cairo_save(cr);
  cairo_translate(cr, inset, inset);
  cairo_scale(cr,width,height);

  /* render the bars */
  float zonemap[MAX_ZONE_SYSTEM_SIZE]= {0};
  _iop_zonesystem_calculate_zonemap (p, zonemap);
  float s=(1./(p->size-2));
  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  for(int i=0; i<p->size-1; i++)
  {
    /* draw the reference zone */
    float z=s*i;
    cairo_rectangle (cr,(1./(p->size-1))*i,0,(1./(p->size-1)),DT_ZONESYSTEM_REFERENCE_SPLIT-DT_ZONESYSTEM_BAR_SPLIT_WIDTH);
    cairo_set_source_rgb (cr, z, z, z);
    cairo_fill (cr);

    /* draw zone mappings */
    cairo_rectangle (cr,
                     zonemap[i],DT_ZONESYSTEM_REFERENCE_SPLIT+DT_ZONESYSTEM_BAR_SPLIT_WIDTH,
                     (zonemap[i+1]-zonemap[i]),1.0-DT_ZONESYSTEM_REFERENCE_SPLIT);
    cairo_set_source_rgb (cr, z, z, z);
    cairo_fill (cr);

  }
  cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
  cairo_restore (cr);

  /* render zonebar control lines */
  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  cairo_set_line_width (cr, 1.);
  cairo_rectangle (cr,inset,inset,width,height);
  cairo_set_source_rgb (cr, .1,.1,.1);
  cairo_stroke (cr);
  cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);

  /* render control points handles */
  cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
  cairo_set_line_width (cr, 1.);
  const float arrw = 7.0f;
  for (int k=1; k<p->size-1; k++)
  {
    float nzw=zonemap[k+1]-zonemap[k];
    float pzw=zonemap[k]-zonemap[k-1];
    if (
      ( ((g->mouse_x/width) > (zonemap[k]-(pzw/2.0))) &&
        ((g->mouse_x/width) < (zonemap[k]+(nzw/2.0))) ) ||
      p->zone[k] != -1)
    {
      gboolean is_under_mouse = ((width*zonemap[k]) - arrw*.5f < g->mouse_x &&  (width*zonemap[k]) + arrw*.5f > g->mouse_x);

      cairo_move_to(cr, inset+(width*zonemap[k]), height+(2*inset)-1);
      cairo_rel_line_to(cr, -arrw*.5f, 0);
      cairo_rel_line_to(cr, arrw*.5f, -arrw);
      cairo_rel_line_to(cr, arrw*.5f, arrw);
      cairo_close_path(cr);

      if ( is_under_mouse )
        cairo_fill(cr);
      else
        cairo_stroke(cr);

    }
  }


  /* push mem surface into widget */
  cairo_destroy (cr);
  cairo_t *cr_pixmap = gdk_cairo_create (gtk_widget_get_window (widget));
  cairo_set_source_surface (cr_pixmap, cst, 0, 0);
  cairo_paint (cr_pixmap);
  cairo_destroy (cr_pixmap);
  cairo_surface_destroy (cst);

  return TRUE;
}

static gboolean dt_iop_zonesystem_bar_button_press(GtkWidget *widget, GdkEventButton *event, dt_iop_module_t *self)
{
  dt_iop_zonesystem_params_t *p = (dt_iop_zonesystem_params_t *)self->params;
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  const int inset = DT_ZONESYSTEM_INSET;
  int width = widget->allocation.width - 2*inset;/*, height = widget->allocation.height - 2*inset;*/

  /* calculate zonemap */
  float zonemap[MAX_ZONE_SYSTEM_SIZE]= {-1};
  _iop_zonesystem_calculate_zonemap(p,zonemap);

  /* translate mouse into zone index */
  int k = _iop_zonesystem_zone_index_from_lightness (g->mouse_x/width,zonemap,p->size);
  float zw = zonemap[k+1]-zonemap[k];
  if ((g->mouse_x/width)>zonemap[k]+(zw/2))
    k++;


  if (event->button == 1)
  {
    if (p->zone[k]==-1)
    {
      p->zone[k] = zonemap[k];
      dt_dev_add_history_item(darktable.develop, self, TRUE);
    }
    g->is_dragging = TRUE;
    g->current_zone = k;
  }
  else  if (event->button == 3)
  {
    /* clear the controlpoint */
    p->zone[k] = -1;
    dt_dev_add_history_item(darktable.develop, self, TRUE);
  }

  return TRUE;
}

static gboolean
dt_iop_zonesystem_bar_button_release (GtkWidget *widget, GdkEventButton *event, dt_iop_module_t *self)
{
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  if (event->button == 1)
  {
    g->is_dragging = FALSE;
  }
  return TRUE;
}

static gboolean
dt_iop_zonesystem_bar_scrolled (GtkWidget *widget, GdkEventScroll *event, dt_iop_module_t *self)
{
  dt_iop_zonesystem_params_t *p = (dt_iop_zonesystem_params_t *)self->params;
  int cs = p->size;
  if(event->direction == GDK_SCROLL_UP)
    p->size+=2;
  else if(event->direction == GDK_SCROLL_DOWN)
    p->size-=2;

  /* sanity checks */
  p->size = p->size>MAX_ZONE_SYSTEM_SIZE?MAX_ZONE_SYSTEM_SIZE:p->size;
  p->size = p->size<4?4:p->size;

  p->zone[cs] = -1;
  dt_dev_add_history_item(darktable.develop, self, TRUE);
  return TRUE;
}

static gboolean
dt_iop_zonesystem_bar_leave_notify(GtkWidget *widget, GdkEventCrossing *event, dt_iop_module_t *self)
{
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  g->hilite_zone = FALSE;
  gtk_widget_queue_draw (g->preview);
  return TRUE;
}

static gboolean
dt_iop_zonesystem_bar_motion_notify (GtkWidget *widget, GdkEventMotion *event, dt_iop_module_t *self)
{
  dt_iop_zonesystem_params_t *p = (dt_iop_zonesystem_params_t *)self->params;
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  const int inset = DT_ZONESYSTEM_INSET;
  int width = widget->allocation.width - 2*inset, height = widget->allocation.height - 2*inset;

  /* record mouse position within control */
  g->mouse_x = CLAMP(event->x - inset, 0, width);
  g->mouse_y = CLAMP(height - 1 - event->y + inset, 0, height);

  g->zone_under_mouse = (g->mouse_x/width) / (1.0/(p->size-1));

  if (g->is_dragging)
  {
    /* calculate zonemap */
    float zonemap[MAX_ZONE_SYSTEM_SIZE]= {-1};
    _iop_zonesystem_calculate_zonemap (p,zonemap);

    if ( (g->mouse_x/width) > zonemap[g->current_zone-1] &&  (g->mouse_x/width) < zonemap[g->current_zone+1] )
    {
      p->zone[g->current_zone] = (g->mouse_x/width);
      dt_dev_add_history_item(darktable.develop, self, TRUE);
    }
  }
  else
    g->hilite_zone = (g->mouse_y<(height/2.0))?TRUE:FALSE;

  gtk_widget_queue_draw (self->widget);
  gtk_widget_queue_draw (g->preview);
  return TRUE;
}


static gboolean
dt_iop_zonesystem_preview_expose (GtkWidget *widget, GdkEventExpose *event, dt_iop_module_t *self)
{
  const int inset = 2;
  int width = widget->allocation.width, height = widget->allocation.height;

  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;
  dt_iop_zonesystem_params_t *p = (dt_iop_zonesystem_params_t *)self->params;

  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(cst);

  /* clear background */
  GtkStateType state = gtk_widget_get_state(self->expander);
  GtkStyle *style = gtk_widget_get_style(self->expander);
  cairo_set_source_rgb (cr, style->bg[state].red/65535.0, style->bg[state].green/65535.0, style->bg[state].blue/65535.0);
  cairo_paint (cr);

  width -= 2*inset;
  height -= 2*inset;
  cairo_translate(cr, inset, inset);

  dt_pthread_mutex_lock(&g->lock);
  if( g->preview_buffer && self->enabled)
  {
    /* calculate the zonemap */
    float zonemap[MAX_ZONE_SYSTEM_SIZE]= {-1};
    _iop_zonesystem_calculate_zonemap (p,zonemap);

    /* let's generate a pixbuf from pixel zone buffer */
    guchar *image = g_malloc ((g->preview_width*g->preview_height)*4);
    for (int k=0; k<g->preview_width*g->preview_height; k++)
    {
      int zone = 255*CLIP (((1.0/(p->size-1))*g->preview_buffer[k]));
      image[4*k+2] = (g->hilite_zone && g->preview_buffer[k]==g->zone_under_mouse)?255:zone;
      image[4*k+1] = (g->hilite_zone && g->preview_buffer[k]==g->zone_under_mouse)?255:zone;
      image[4*k+0] = (g->hilite_zone && g->preview_buffer[k]==g->zone_under_mouse)?0:zone;
    }
    dt_pthread_mutex_unlock(&g->lock);

    const int wd = g->preview_width, ht = g->preview_height;
    const float scale = fminf(width/(float)wd, height/(float)ht);
    const int stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, wd);
    cairo_surface_t *surface = cairo_image_surface_create_for_data (image, CAIRO_FORMAT_RGB24, wd, ht, stride);
    cairo_translate(cr, width/2.0, height/2.0f);
    cairo_scale(cr, scale, scale);
    cairo_translate(cr, -.5f*wd, -.5f*ht);

    cairo_rectangle(cr, 1, 1, wd-2, ht-2);
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_GOOD);
    cairo_fill_preserve(cr);
    cairo_surface_destroy (surface);

    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgb(cr, .1, .1, .1);
    cairo_stroke(cr);

    g_free(image);
  }
  else
    dt_pthread_mutex_unlock(&g->lock);

  cairo_destroy(cr);
  cairo_t *cr_pixmap = gdk_cairo_create(gtk_widget_get_window(widget));
  cairo_set_source_surface (cr_pixmap, cst, 0, 0);
  cairo_paint(cr_pixmap);
  cairo_destroy(cr_pixmap);
  cairo_surface_destroy(cst);

  return TRUE;
}

void _iop_zonesystem_redraw_preview_callback(gpointer instance, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  dt_iop_zonesystem_gui_data_t *g = (dt_iop_zonesystem_gui_data_t *)self->gui_data;  

  dt_control_queue_redraw_widget(g->preview);
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
