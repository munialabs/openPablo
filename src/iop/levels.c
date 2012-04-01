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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "iop/levels.h"
#include "gui/presets.h"
#include "develop/develop.h"
#include "control/control.h"
#include "gui/gtk.h"
#include "common/colorspaces.h"
#include "common/opencl.h"

#define DT_GUI_CURVE_EDITOR_INSET 5
#define DT_GUI_CURVE_INFL .3f

DT_MODULE(1)

const char *name()
{
  return _("levels");
}


int
groups ()
{
  return IOP_GROUP_TONE;
}

int
flags ()
{
  return IOP_FLAGS_SUPPORTS_BLENDING;
}

void process (struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, void *i, void *o, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out)
{
  const int ch = piece->colors;
  dt_iop_levels_data_t *d = (dt_iop_levels_data_t*)(piece->data);
#ifdef _OPENMP
#pragma omp parallel for default(none) shared(roi_out, i, o, d) schedule(static)
#endif
  for(int k=0; k<roi_out->height; k++)
  {
    float *in = ((float *)i) + k*ch*roi_out->width;
    float *out = ((float *)o) + k*ch*roi_out->width;
    for (int j=0; j<roi_out->width; j++,in+=ch,out+=ch)
    {
      float L_in = in[0] / 100.0;

      if(L_in <= d->in_low)
      {
        // Anything below the lower threshold just clips to zero
        out[0] = 0;
      }
      else if(L_in >= d->in_high)
      {
        float percentage = (L_in - d->in_low) / (d->in_high - d->in_low);
        out[0] = 100.0 * pow(percentage, d->in_inv_gamma);
      }
      else
      {
        // Within the expected input range we can use the lookup table
        float percentage = (L_in - d->in_low) / (d->in_high - d->in_low);
        //out[0] = 100.0 * pow(percentage, d->in_inv_gamma);
        out[0] = d->lut[CLAMP((int)(percentage * 0xfffful), 0, 0xffff)];
      }

      // Preserving contrast
      if(in[0] > 0.01f)
      {
        out[1] = in[1] * out[0]/in[0];
        out[2] = in[2] * out[0]/in[0];
      }
      else
      {
        out[1] = in[1] * out[0]/0.01f;
        out[2] = in[2] * out[0]/0.01f;
      }

    }
  }

}

//void init_presets (dt_iop_module_so_t *self)
//{
//  dt_iop_levels_params_t p;
//  p.levels_preset = 0;
//
//  p.levels[0] = 0;
//  p.levels[1] = 0.5;
//  p.levels[2] = 1;
//  dt_gui_presets_add_generic(_("unmodified"), self->op, self->version(), &p, sizeof(p), 1);
//}

void commit_params (struct dt_iop_module_t *self, dt_iop_params_t *p1,
                    dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  dt_iop_levels_data_t *d = (dt_iop_levels_data_t*)(piece->data);
  dt_iop_levels_params_t *p = (dt_iop_levels_params_t*)p1;

  // Building the lut for values in the [0,1] range
  d->in_low = p->levels[0];
  d->in_high = p->levels[2];
  float delta = (p->levels[2] - p->levels[0]) / 2.;
  float mid = p->levels[0] + delta;
  float tmp = (p->levels[1] - mid) / delta;
  d->in_inv_gamma = pow(10, tmp);

  for(unsigned int i = 0; i < 0x10000; i++)
  {
    float percentage = (float)i / (float)0xfffful;
    d->lut[i] = 100.0 * pow(percentage, d->in_inv_gamma);
  }
}

void init_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe,
                dt_dev_pixelpipe_iop_t *piece)
{
  // create part of the gegl pipeline
  dt_iop_levels_data_t *d =
      (dt_iop_levels_data_t *)malloc(sizeof(dt_iop_levels_data_t));
  piece->data = (void *)d;
}

void cleanup_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  // clean up everything again.
  free(piece->data);
}

void gui_update(struct dt_iop_module_t *self)
{
  // nothing to do, gui curve is read directly from params during expose event.
  gtk_widget_queue_draw(self->widget);
}

void reload_defaults(dt_iop_module_t *self)
{
  memcpy(self->params, self->default_params, sizeof(dt_iop_levels_params_t));
}

void init(dt_iop_module_t *module)
{
  module->params = malloc(sizeof(dt_iop_levels_params_t));
  module->default_params = malloc(sizeof(dt_iop_levels_params_t));
  module->default_enabled = 0;
  module->priority = 627; // module order created by iop_dependencies.py, do not edit!
  module->params_size = sizeof(dt_iop_levels_params_t);
  module->gui_data = NULL;
  dt_iop_levels_params_t tmp = (dt_iop_levels_params_t)
  {
    {0, 0.5, 1},
    0
  };
  memcpy(module->params, &tmp, sizeof(dt_iop_levels_params_t));
  memcpy(module->default_params, &tmp, sizeof(dt_iop_levels_params_t));
}

void init_global(dt_iop_module_so_t *module)
{
  //const int program = 2; // basic.cl, from programs.conf
  dt_iop_levels_global_data_t *gd = (dt_iop_levels_global_data_t *)malloc(sizeof(dt_iop_levels_global_data_t));
  module->data = gd;
  //gd->kernel_levels = dt_opencl_create_kernel(program, "levels");      do not try to load kernel unless we have one
}

void cleanup_global(dt_iop_module_so_t *module)
{
  dt_iop_levels_global_data_t *gd = (dt_iop_levels_global_data_t *)module->data;
  dt_opencl_free_kernel(gd->kernel_levels);
  free(module->data);
  module->data = NULL;
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
  self->gui_data = malloc(sizeof(dt_iop_levels_gui_data_t));
  dt_iop_levels_gui_data_t *c = (dt_iop_levels_gui_data_t *)self->gui_data;

  c->mouse_x = c->mouse_y = -1.0;
  c->dragging = 0;
  self->widget = GTK_WIDGET(gtk_vbox_new(FALSE, 5));
  c->area = GTK_DRAWING_AREA(gtk_drawing_area_new());
  GtkWidget *asp = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, TRUE);
  gtk_box_pack_start(GTK_BOX(self->widget), asp, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(asp), GTK_WIDGET(c->area));
  gtk_drawing_area_size(c->area, 258, 150);
  g_object_set (GTK_OBJECT(c->area), "tooltip-text", _("drag handles to set black, grey, and white points.  operates on L channel."), (char *)NULL);

  gtk_widget_add_events(GTK_WIDGET(c->area), GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_LEAVE_NOTIFY_MASK);
  g_signal_connect (G_OBJECT (c->area), "expose-event",
                    G_CALLBACK (dt_iop_levels_expose), self);
  g_signal_connect (G_OBJECT (c->area), "button-press-event",
                    G_CALLBACK (dt_iop_levels_button_press), self);
  g_signal_connect (G_OBJECT (c->area), "button-release-event",
                    G_CALLBACK (dt_iop_levels_button_release), self);
  g_signal_connect (G_OBJECT (c->area), "motion-notify-event",
                    G_CALLBACK (dt_iop_levels_motion_notify), self);
  g_signal_connect (G_OBJECT (c->area), "leave-notify-event",
                    G_CALLBACK (dt_iop_levels_leave_notify), self);
}

void gui_cleanup(struct dt_iop_module_t *self)
{
  free(self->gui_data);
  self->gui_data = NULL;
}


static gboolean dt_iop_levels_leave_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  dt_iop_levels_gui_data_t *c = (dt_iop_levels_gui_data_t *)self->gui_data;
  c->mouse_x = c->mouse_y = -1.0;
  gtk_widget_queue_draw(widget);
  return TRUE;
}

static gboolean dt_iop_levels_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  dt_iop_levels_gui_data_t *c = (dt_iop_levels_gui_data_t *)self->gui_data;
  dt_iop_levels_params_t *p = (dt_iop_levels_params_t *)self->params;
  const int inset = DT_GUI_CURVE_EDITOR_INSET;
  int width = widget->allocation.width, height = widget->allocation.height;
  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(cst);
  // clear bg
  cairo_set_source_rgb (cr, .2, .2, .2);
  cairo_paint(cr);

  cairo_translate(cr, inset, inset);
  width -= 2*inset;
  height -= 2*inset;

  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb (cr, .1, .1, .1);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_stroke(cr);

  cairo_set_source_rgb (cr, .3, .3, .3);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);

  // draw grid
  cairo_set_line_width(cr, .4);
  cairo_set_source_rgb (cr, .1, .1, .1);
  dt_draw_vertical_lines(cr, 4, 0, 0, width, height);

  // Drawing the vertical line indicators
  cairo_set_line_width(cr, 2.);

  for(int k = 0; k < 3; k++)
  {
    if(k == c->handle_move && c->mouse_x > 0)
      cairo_set_source_rgb(cr, 1, 1, 1);
    else
      cairo_set_source_rgb(cr, .7, .7, .7);

    cairo_move_to(cr, width*p->levels[k], height);
    cairo_rel_line_to(cr, 0, -height);
    cairo_stroke(cr);
  }

  // draw x positions
  cairo_set_line_width(cr, 1.);
  const float arrw = 7.0f;
  for(int k=0; k<3; k++)
  {
    switch(k)
    {
    case 0:
      cairo_set_source_rgb(cr, 0, 0, 0);
      break;

    case 1:
      cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
      break;

    default:
      cairo_set_source_rgb(cr, 1, 1, 1);
      break;
    }

    cairo_move_to(cr, width*p->levels[k], height+inset-1);
    cairo_rel_line_to(cr, -arrw*.5f, 0);
    cairo_rel_line_to(cr, arrw*.5f, -arrw);
    cairo_rel_line_to(cr, arrw*.5f, arrw);
    cairo_close_path(cr);
    if(c->handle_move == k && c->mouse_x > 0)
      cairo_fill(cr);
    else
      cairo_stroke(cr);
  }

  cairo_translate(cr, 0, height);

  // draw lum histogram in background
  // only if the module is enabled
  if (self->enabled)
  {
    dt_develop_t *dev = darktable.develop;
    float *hist, hist_max;
    hist = dev->histogram_pre_levels;
    hist_max = dev->histogram_pre_levels_max;
    if(hist_max > 0)
    {
      cairo_save(cr);
      cairo_scale(cr, width/63.0, -(height-5)/(float)hist_max);
      cairo_set_source_rgba(cr, .2, .2, .2, 0.5);
      dt_draw_histogram_8(cr, hist, 3);
      cairo_restore(cr);
    }
  }

  // Cleaning up
  cairo_destroy(cr);
  cairo_t *cr_pixmap = gdk_cairo_create(gtk_widget_get_window(widget));
  cairo_set_source_surface (cr_pixmap, cst, 0, 0);
  cairo_paint(cr_pixmap);
  cairo_destroy(cr_pixmap);
  cairo_surface_destroy(cst);
  return TRUE;
}

static gboolean dt_iop_levels_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  dt_iop_levels_gui_data_t *c = (dt_iop_levels_gui_data_t *)self->gui_data;
  dt_iop_levels_params_t *p = (dt_iop_levels_params_t *)self->params;
  const int inset = DT_GUI_CURVE_EDITOR_INSET;
  int height = widget->allocation.height - 2*inset, width = widget->allocation.width - 2*inset;
  if(!c->dragging)
  {
    c->mouse_x = CLAMP(event->x - inset, 0, width);
    c->drag_start_percentage = (p->levels[1] - p->levels[0])
                               / (p->levels[2] - p->levels[0]);
  }
  c->mouse_y = CLAMP(event->y - inset, 0, height);

  if(c->dragging)
  {
    if(c->handle_move >= 0 && c->handle_move < 3)
    {
      const float mx = (CLAMP(event->x - inset, 0, width)) / (float)width;

      float min_x = 0;
      float max_x = 1;

      // Determining the minimum and maximum bounds for the drag handles
      switch(c->handle_move)
      {
      case 0:
        max_x = fminf(p->levels[2] - (0.05 / c->drag_start_percentage),
                      1);
        max_x = fminf((p->levels[2] * (1 - c->drag_start_percentage) - 0.05)
                      / (1 - c->drag_start_percentage),
                      max_x);
        break;

      case 1:
        min_x = p->levels[0] + 0.05;
        max_x = p->levels[2] - 0.05;
        break;

      case 2:
        min_x = fmaxf((0.05 / c->drag_start_percentage) + p->levels[0],
                      0);
        min_x = fmaxf((p->levels[0] * (1 - c->drag_start_percentage) + 0.05)
                      / (1 - c->drag_start_percentage),
                      min_x);
        break;
      }

      p->levels[c->handle_move] =
          fminf(max_x, fmaxf(min_x, mx));

      if(c->handle_move != 1)
        p->levels[1] = p->levels[0] + (c->drag_start_percentage
                                       * (p->levels[2] - p->levels[0]));
    }
    dt_dev_add_history_item(darktable.develop, self, TRUE);
  }
  else
  {
    c->handle_move = 0;
    const float mx = CLAMP(event->x - inset, 0, width)/(float)width;
    float dist = fabsf(p->levels[0] - mx);
    for(int k=1; k<3; k++)
    {
      float d2 = fabsf(p->levels[k] - mx);
      if(d2 < dist)
      {
        c->handle_move = k;
        dist = d2;
      }
    }
  }
  gtk_widget_queue_draw(widget);

  gint x, y;
  gdk_window_get_pointer(event->window, &x, &y, NULL);
  return TRUE;
}

static gboolean dt_iop_levels_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  // set active point
  if(event->button == 1)
  {
    dt_iop_module_t *self = (dt_iop_module_t *)user_data;
    dt_iop_levels_gui_data_t *c = (dt_iop_levels_gui_data_t *)self->gui_data;
    c->dragging = 1;
    return TRUE;
  }
  return FALSE;
}

static gboolean dt_iop_levels_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  if(event->button == 1)
  {
    dt_iop_module_t *self = (dt_iop_module_t *)user_data;
    dt_iop_levels_gui_data_t *c = (dt_iop_levels_gui_data_t *)self->gui_data;
    c->dragging = 0;
    return TRUE;
  }
  return FALSE;
}

