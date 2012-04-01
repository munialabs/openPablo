
/*
    This file is part of darktable,
    copyright (c) 2009--2011 johannes hanika, henrik andersson

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
#ifdef HAVE_GPHOTO2
#   include "common/camera_control.h"
#   include "views/capture.h"
#endif
#include "common/collection.h"
#include "common/image.h"
#include "common/image_cache.h"
#include "develop/develop.h"
#include "develop/imageop.h"
#include "dtgtk/label.h"
#include "dtgtk/button.h"
#include "gui/accelerators.h"
#include "gui/contrast.h"
#include "gui/gtk.h"

#include "gui/presets.h"
#include "control/control.h"
#include "control/jobs.h"
#include "control/conf.h"
#include "control/signal.h"
#include "views/view.h"
#include "common/styles.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pthread.h>


/*                                                                                         
 * NEW UI API                                                             
 */

#define DT_UI_PANEL_MODULE_SPACING 3

const char *_ui_panel_config_names[] = {
  "header",
  "toolbar_top",
  "toolbar_bottom",
  "left",
  "right",
  "bottom"
}; 

typedef struct dt_ui_t {
  /* container widgets */
  GtkWidget *containers[DT_UI_CONTAINER_SIZE];

  /* border widgets */
  GtkWidget *borders[DT_UI_BORDER_SIZE];

  /* panel widgets */
  GtkWidget *panels[DT_UI_PANEL_SIZE];

  /* center widget */
  GtkWidget *center;
  /* main widget */
  GtkWidget *main_window;
} dt_ui_t;

/* initialize the whole left panel */
static void _ui_init_panel_left(struct dt_ui_t *ui, GtkWidget *container);
/* initialize the whole right panel */
static void _ui_init_panel_right(dt_ui_t *ui, GtkWidget *container);
/* initialize the top container of panel */
static GtkWidget *_ui_init_panel_container_top(GtkWidget *container);
/* initialize the center container of panel */
static GtkWidget *_ui_init_panel_container_center(GtkWidget *container, gboolean left);
/* initialize the bottom container of panel */
static GtkWidget *_ui_init_panel_container_bottom(GtkWidget *container);
/* initialize the top container of panel */
static void _ui_init_panel_top(dt_ui_t *ui, GtkWidget *container);
/* intialize the center top panel */
static void _ui_init_panel_center_top(dt_ui_t *ui, GtkWidget *container);
/* initialize the center bottom panel */
static void _ui_init_panel_center_bottom(dt_ui_t *ui, GtkWidget *container);
/* initialize the bottom panel */
static void _ui_init_panel_bottom(dt_ui_t *ui, GtkWidget *container);
/* generic callback for redraw widget signals */
static void _ui_widget_redraw_callback(gpointer instance, GtkWidget *widget);

/*
 * OLD UI API
 */
static void init_widgets();

static void init_main_table(GtkWidget *container);

static void key_accel_changed(GtkAccelMap *object,
                              gchar *accel_path,
                              guint accel_key,
                              GdkModifierType accel_mods,
                              gpointer user_data)
{
  char path[256];

  // Updating all the stored accelerator keys/mods for key_pressed shortcuts

  dt_accel_path_view(path, 256, "filmstrip", "scroll forward");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.filmstrip_forward);
  dt_accel_path_view(path, 256, "filmstrip", "scroll back");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.filmstrip_back);

  // Lighttable
  dt_accel_path_view(path, 256, "lighttable", "scroll up");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.lighttable_up);
  dt_accel_path_view(path, 256, "lighttable", "scroll down");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.lighttable_down);
  dt_accel_path_view(path, 256, "lighttable", "scroll left");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.lighttable_left);
  dt_accel_path_view(path, 256, "lighttable", "scroll right");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.lighttable_right);
  dt_accel_path_view(path, 256, "lighttable", "scroll center");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.lighttable_center);
  dt_accel_path_view(path, 256, "lighttable", "preview");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.lighttable_preview);

  // Global
  dt_accel_path_global(path, 256, "toggle side borders");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.global_sideborders);

  dt_accel_path_global(path, 256, "toggle header");
  gtk_accel_map_lookup_entry(path,
                             &darktable.control->accels.global_header);

}

static gboolean brightness_key_accel_callback(GtkAccelGroup *accel_group,
                                          GObject *acceleratable, guint keyval,
                                          GdkModifierType modifier,
                                          gpointer data)
{
  if(data)
    dt_gui_brightness_increase();
  else
    dt_gui_brightness_decrease();

  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
  return TRUE;
}

static gboolean contrast_key_accel_callback(GtkAccelGroup *accel_group,
                                        GObject *acceleratable, guint keyval,
                                        GdkModifierType modifier,
                                        gpointer data)
{
  if(data)
    dt_gui_contrast_increase();
  else
    dt_gui_contrast_decrease();

  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
  return TRUE;
}

static gboolean fullscreen_key_accel_callback(GtkAccelGroup *accel_group,
                                          GObject *acceleratable, guint keyval,
                                          GdkModifierType modifier,
                                          gpointer data)
{
  GtkWidget *widget;
  int fullscreen;

  if(data)
  {
    widget = dt_ui_main_window(darktable.gui->ui);
    fullscreen = dt_conf_get_bool("ui_last/fullscreen");
    if(fullscreen) gtk_window_unfullscreen(GTK_WINDOW(widget));
    else           gtk_window_fullscreen  (GTK_WINDOW(widget));
    fullscreen ^= 1;
    dt_conf_set_bool("ui_last/fullscreen", fullscreen);
    dt_dev_invalidate(darktable.develop);
  }
  else
  {
    widget = dt_ui_main_window(darktable.gui->ui);
    gtk_window_unfullscreen(GTK_WINDOW(widget));
    fullscreen = 0;
    dt_conf_set_bool("ui_last/fullscreen", fullscreen);
    dt_dev_invalidate(darktable.develop);
  }

  /* redraw center view */
  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
  return TRUE;
}

static gboolean view_switch_key_accel_callback(GtkAccelGroup *accel_group,
                                          GObject *acceleratable, guint keyval,
                                          GdkModifierType modifier,
                                          gpointer data)
{
  dt_ctl_switch_mode();
  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
  return TRUE;
}

static gboolean
borders_button_pressed (GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
  dt_ui_t *ui = (dt_ui_t*)user_data;
  const dt_view_t *cv = dt_view_manager_get_current_view(darktable.view_manager);
  char key[512];


  long which = (long)g_object_get_data(G_OBJECT(w),"border");
  switch(which)
  {
    case 0: // left border
    {
      g_snprintf(key, 512, "%s/ui/%s_visible", cv->module_name, _ui_panel_config_names[DT_UI_PANEL_LEFT]);
      dt_ui_panel_show(ui, DT_UI_PANEL_LEFT, !dt_conf_get_bool(key));
    } break;

    case 1:  // right border
    {
      g_snprintf(key, 512, "%s/ui/%s_visible", cv->module_name, _ui_panel_config_names[DT_UI_PANEL_RIGHT]);
      dt_ui_panel_show(ui, DT_UI_PANEL_RIGHT, !dt_conf_get_bool(key));
    } break;

    case 2: // top border
    {
      g_snprintf(key, 512, "%s/ui/%s_visible", cv->module_name, _ui_panel_config_names[DT_UI_PANEL_CENTER_TOP]);
      gboolean show = !dt_conf_get_bool(key);
      dt_ui_panel_show(ui, DT_UI_PANEL_CENTER_TOP, show);
      
      /* special case show header */
      g_snprintf(key, 512, "%s/ui/show_header", cv->module_name);
      if (dt_conf_get_bool(key)) 
        dt_ui_panel_show(ui, DT_UI_PANEL_TOP, show);

    } break;
   
    case 4:  // bottom border
    default:
    {
      g_snprintf(key, 512, "%s/ui/%s_visible", cv->module_name, _ui_panel_config_names[DT_UI_PANEL_CENTER_BOTTOM]);
      gboolean show = !dt_conf_get_bool(key);
      dt_ui_panel_show(ui, DT_UI_PANEL_CENTER_BOTTOM, show);
      dt_ui_panel_show(ui, DT_UI_PANEL_BOTTOM, show);
    } break;
  }

  gtk_widget_queue_draw(w);

  return TRUE;
}

static gboolean
_widget_focus_in_block_key_accelerators (GtkWidget *widget,GdkEventFocus *event,gpointer data)
{
  dt_control_key_accelerators_off (darktable.control);
  return FALSE;
}

static gboolean
_widget_focus_out_unblock_key_accelerators (GtkWidget *widget,GdkEventFocus *event,gpointer data)
{
  dt_control_key_accelerators_on (darktable.control);
  return FALSE;
}

void
dt_gui_key_accel_block_on_focus (GtkWidget *w)
{
  /* first off add focus change event mask */
  gtk_widget_add_events(w, GDK_FOCUS_CHANGE_MASK);

  /* conenct the signals */
  g_signal_connect (G_OBJECT (w), "focus-in-event", G_CALLBACK(_widget_focus_in_block_key_accelerators), (gpointer)w);
  g_signal_connect (G_OBJECT (w), "focus-out-event", G_CALLBACK(_widget_focus_out_unblock_key_accelerators), (gpointer)w);
}

static gboolean
expose_borders (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  // draw arrows on borders
  if(!dt_control_running()) return TRUE;
  long int which = (long int)user_data;
  float width = widget->allocation.width, height = widget->allocation.height;
  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(cst);
  GtkStyle *style = gtk_widget_get_style(dt_ui_center(darktable.gui->ui));
  cairo_set_source_rgb (cr,
                        .5f*style->bg[GTK_STATE_NORMAL].red/65535.0,
                        .5f*style->bg[GTK_STATE_NORMAL].green/65535.0,
                        .5f*style->bg[GTK_STATE_NORMAL].blue/65535.0
                       );
  // cairo_set_source_rgb (cr, .13, .13, .13);
  cairo_paint(cr);

  // draw scrollbar indicators
  int v = darktable.view_manager->current_view;
  dt_view_t *view = NULL;
  if(v >= 0 && v < darktable.view_manager->num_views) view = darktable.view_manager->view + v;
  // cairo_set_source_rgb (cr, .16, .16, .16);
  cairo_set_source_rgb (cr,
                        style->bg[GTK_STATE_NORMAL].red/65535.0,
                        style->bg[GTK_STATE_NORMAL].green/65535.0,
                        style->bg[GTK_STATE_NORMAL].blue/65535.0
                       );
  const float border = 0.3;
  if(!view) cairo_paint(cr);
  else
  {
    switch(which)
    {
      case 0:
      case 1: // left, right: vertical
        cairo_rectangle(cr, 0.0, view->vscroll_pos/view->vscroll_size * height, width, view->vscroll_viewport_size/view->vscroll_size * height);
        break;
      default:        // bottom, top: horizontal
        cairo_rectangle(cr, view->hscroll_pos/view->hscroll_size * width, 0.0, view->hscroll_viewport_size/view->hscroll_size * width, height);
        break;
    }
    cairo_fill(cr);
    switch(which)
    {
      case 0:
        cairo_rectangle(cr, (1.0-border)*width, 0.0, border*width, height);
        break;
      case 1:
        cairo_rectangle(cr, 0.0, 0.0, border*width, height);
        break;
      case 2:
        cairo_rectangle(cr, (1.0-border)*height, (1.0-border)*height, width-2*(1.0-border)*height, border*height);
        break;
      default:
        cairo_rectangle(cr, (1.0-border)*height, 0.0, width-2*(1.0-border)*height, border*height);
        break;
    }
    cairo_fill(cr);
  }

  // draw gui arrows.
  cairo_set_source_rgb (cr, .6, .6, .6);

  switch(which)
  {
    case 0: // left
      if(dt_ui_panel_visible(darktable.gui->ui, DT_UI_PANEL_LEFT))
      {
        cairo_move_to (cr, width, height/2-width);
        cairo_rel_line_to (cr, 0.0, 2*width);
        cairo_rel_line_to (cr, -width, -width);
      }
      else
      {
        cairo_move_to (cr, 0.0, height/2-width);
        cairo_rel_line_to (cr, 0.0, 2*width);
        cairo_rel_line_to (cr, width, -width);
      }
      break;
    case 1: // right
      if(dt_ui_panel_visible(darktable.gui->ui, DT_UI_PANEL_RIGHT))
      {
        cairo_move_to (cr, 0.0, height/2-width);
        cairo_rel_line_to (cr, 0.0, 2*width);
        cairo_rel_line_to (cr, width, -width);
      }
      else
      {
        cairo_move_to (cr, width, height/2-width);
        cairo_rel_line_to (cr, 0.0, 2*width);
        cairo_rel_line_to (cr, -width, -width);
      }
      break;
    case 2: // top
      if(dt_ui_panel_visible(darktable.gui->ui, DT_UI_PANEL_CENTER_TOP))
      {
        cairo_move_to (cr, width/2-height, height);
        cairo_rel_line_to (cr, 2*height, 0.0);
        cairo_rel_line_to (cr, -height, -height);
      }
      else
      {
        cairo_move_to (cr, width/2-height, 0.0);
        cairo_rel_line_to (cr, 2*height, 0.0);
        cairo_rel_line_to (cr, -height, height);
      }
      break;
    default: // bottom
      if(dt_ui_panel_visible(darktable.gui->ui, DT_UI_PANEL_CENTER_BOTTOM))
      {
        cairo_move_to (cr, width/2-height, 0.0);
        cairo_rel_line_to (cr, 2*height, 0.0);
        cairo_rel_line_to (cr, -height, height);
      }
      else
      {
        cairo_move_to (cr, width/2-height, height);
        cairo_rel_line_to (cr, 2*height, 0.0);
        cairo_rel_line_to (cr, -height, -height);
      }
      break;
  }
  cairo_close_path (cr);
  cairo_fill(cr);

  cairo_destroy(cr);
  cairo_t *cr_pixmap = gdk_cairo_create(gtk_widget_get_window(widget));
  cairo_set_source_surface (cr_pixmap, cst, 0, 0);
  cairo_paint(cr_pixmap);
  cairo_destroy(cr_pixmap);
  cairo_surface_destroy(cst);
  return TRUE;
}

static gboolean
expose (GtkWidget *da, GdkEventExpose *event, gpointer user_data)
{
  dt_control_expose(NULL);
  gdk_draw_drawable(da->window,
                    da->style->fg_gc[GTK_WIDGET_STATE(da)], darktable.gui->pixmap,
                    // Only copy the area that was exposed.
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);

  if(darktable.lib->proxy.colorpicker.module)
  {
    darktable.lib->proxy.colorpicker.update_panel(
        darktable.lib->proxy.colorpicker.module);
    darktable.lib->proxy.colorpicker.update_samples(
        darktable.lib->proxy.colorpicker.module);
  }

  // test quit cond (thread safe, 2nd pass)
  if(!dt_control_running())
  {
    dt_cleanup();
    gtk_main_quit();
  }
  return TRUE;
}

static gboolean
scrolled (GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
  dt_view_manager_scrolled(darktable.view_manager, event->x, event->y, event->direction == GDK_SCROLL_UP, event->state & 0xf);
  gtk_widget_queue_draw(widget);
  return TRUE;
}

static gboolean
borders_scrolled (GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
  dt_view_manager_border_scrolled(darktable.view_manager, event->x, event->y, (long int)user_data, event->direction == GDK_SCROLL_UP);
  gtk_widget_queue_draw(widget);
  return TRUE;
}

void quit()
{
  // thread safe quit, 1st pass:
  GtkWindow *win = GTK_WINDOW(dt_ui_main_window(darktable.gui->ui));
  gtk_window_iconify(win);

  GtkWidget *widget;
  widget = darktable.gui->widgets.left_border;
  g_signal_handlers_block_by_func (widget, expose_borders, (gpointer)0);
  widget = darktable.gui->widgets.right_border;
  g_signal_handlers_block_by_func (widget, expose_borders, (gpointer)1);
  widget = darktable.gui->widgets.top_border;
  g_signal_handlers_block_by_func (widget, expose_borders, (gpointer)2);
  widget = darktable.gui->widgets.bottom_border;
  g_signal_handlers_block_by_func (widget, expose_borders, (gpointer)3);

  dt_pthread_mutex_lock(&darktable.control->cond_mutex);
  dt_pthread_mutex_lock(&darktable.control->run_mutex);
  darktable.control->running = 0;
  dt_pthread_mutex_unlock(&darktable.control->run_mutex);
  dt_pthread_mutex_unlock(&darktable.control->cond_mutex);

  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
}

static gboolean _gui_switch_view_key_accel_callback(GtkAccelGroup *accel_group,
                                                GObject *acceleratable,
                                                guint keyval,
                                                GdkModifierType modifier,
                                                gpointer p)
{
  int view=(long int)p;
  dt_ctl_gui_mode_t mode=DT_MODE_NONE;
  /* do some setup before switch view*/
  switch (view)
  {
#ifdef HAVE_GPHOTO2
    case DT_GUI_VIEW_SWITCH_TO_TETHERING:
      // switching to capture view using "plugins/capture/current_filmroll" as session...
      // and last used camera
      if (dt_camctl_can_enter_tether_mode(darktable.camctl,NULL) )
      {
        dt_conf_set_int( "plugins/capture/mode", DT_CAPTURE_MODE_TETHERED);
        mode = DT_CAPTURE;
      }
      break;
#endif

    case DT_GUI_VIEW_SWITCH_TO_DARKROOM:
      mode = DT_DEVELOP;
      break;

    case DT_GUI_VIEW_SWITCH_TO_LIBRARY:
      mode = DT_LIBRARY;
      break;

  }

  /* try switch to mode */
  dt_ctl_switch_mode_to (mode);
  return TRUE;
}

static gboolean quit_callback(GtkAccelGroup *accel_group,
                          GObject *acceleratable, guint keyval,
                          GdkModifierType modifier)
{
  quit();
  return TRUE; // for the sake of completeness ...
}

static gboolean
configure (GtkWidget *da, GdkEventConfigure *event, gpointer user_data)
{
  static int oldw = 0;
  static int oldh = 0;
  //make our selves a properly sized pixmap if our window has been resized
  if (oldw != event->width || oldh != event->height)
  {
    //create our new pixmap with the correct size.
    GdkPixmap *tmppixmap = gdk_pixmap_new(da->window, event->width,  event->height, -1);
    //copy the contents of the old pixmap to the new pixmap.  This keeps ugly uninitialized
    //pixmaps from being painted upon resize
    int minw = oldw, minh = oldh;
    if(event->width  < minw) minw = event->width;
    if(event->height < minh) minh = event->height;
    gdk_draw_drawable(tmppixmap, da->style->fg_gc[GTK_WIDGET_STATE(da)], darktable.gui->pixmap, 0, 0, 0, 0, minw, minh);
    //we're done with our old pixmap, so we can get rid of it and replace it with our properly-sized one.
    g_object_unref(darktable.gui->pixmap);
    darktable.gui->pixmap = tmppixmap;
  }
  oldw = event->width;
  oldh = event->height;

  return dt_control_configure(da, event, user_data);
}

static gboolean
key_pressed_override (GtkWidget *w, GdkEventKey *event, gpointer user_data)
{
  // fprintf(stderr,"Key Press state: %d hwkey: %d\n",event->state, event->hardware_keycode);


  return dt_control_key_pressed_override(event->keyval,
                                         event->state & KEY_STATE_MASK);
}

static gboolean
key_pressed (GtkWidget *w, GdkEventKey *event, gpointer user_data)
{
  return dt_control_key_pressed(event->keyval, event->state & KEY_STATE_MASK);
}

static gboolean
key_released (GtkWidget *w, GdkEventKey *event, gpointer user_data)
{
  return dt_control_key_released(event->keyval, event->state & KEY_STATE_MASK);
}

static gboolean
button_pressed (GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
  dt_control_button_pressed(event->x, event->y, event->button, event->type, event->state & 0xf);
  gtk_widget_grab_focus(w);
  gtk_widget_queue_draw(w);
  return TRUE;
}

static gboolean
button_released (GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
  dt_control_button_released(event->x, event->y, event->button, event->state & 0xf);
  gtk_widget_queue_draw(w);
  return TRUE;
}

static gboolean
mouse_moved (GtkWidget *w, GdkEventMotion *event, gpointer user_data)
{
  dt_control_mouse_moved(event->x, event->y, event->state & 0xf);
  gint x, y;
  gdk_window_get_pointer(event->window, &x, &y, NULL);
  return TRUE;
}

static gboolean
center_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
  dt_control_mouse_leave();
  return TRUE;
}

static gboolean
center_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
  dt_control_mouse_enter();
  return TRUE;
}

int
dt_gui_gtk_init(dt_gui_gtk_t *gui, int argc, char *argv[])
{
  // unset gtk rc from kde:
  char path[1024], datadir[1024];
  dt_util_get_datadir(datadir, 1024);
  gchar *themefile = dt_conf_get_string("themefile");
  if(themefile && themefile[0] == '/') snprintf(path, 1023, "%s", themefile);
  else snprintf(path, 1023, "%s/%s", datadir, themefile ? themefile : "darktable.gtkrc");
  if(!g_file_test(path, G_FILE_TEST_EXISTS))
    snprintf(path, 1023, "%s/%s", DARKTABLE_DATADIR, themefile ? themefile : "darktable.gtkrc");
  (void)setenv("GTK2_RC_FILES", path, 1);

  /* lets zero mem */
  memset(gui,0,sizeof(dt_gui_gtk_t));

#if GLIB_MAJOR_VERSION <= 2
#if GLIB_MINOR_VERSION < 31
  if (!g_thread_supported ()) g_thread_init(NULL);
#endif
#endif
  gdk_threads_init();

  gdk_threads_enter();

  gtk_init (&argc, &argv);

  GtkWidget *widget;
  gui->ui = dt_ui_initialize(argc,argv);
  gui->pixmap = NULL;
  gui->center_tooltip = 0;
  gui->presets_popup_menu = NULL;
  
  if(g_file_test(path, G_FILE_TEST_EXISTS)) gtk_rc_parse (path);
  else
  {
    fprintf(stderr, "[gtk_init] could not find `%s' in . or %s!\n", themefile, datadir);
    g_free(themefile);
    return 1;
  }
  g_free(themefile);

  // Initializing the shortcut groups
  darktable.control->accelerators = gtk_accel_group_new();

  darktable.control->accelerator_list = NULL;

  // Connecting the callback to update keyboard accels for key_pressed
  g_signal_connect(G_OBJECT(gtk_accel_map_get()),
                   "changed",
                   G_CALLBACK(key_accel_changed),
                   NULL);

  // Initializing widgets
  init_widgets();

  // Adding the global shortcut group to the main window
  gtk_window_add_accel_group(GTK_WINDOW(dt_ui_main_window(darktable.gui->ui)),
                             darktable.control->accelerators);

  // set constant width from conf key
  int panel_width = dt_conf_get_int("panel_width");
  if(panel_width < 20 || panel_width > 500)
  {
    // fix for unset/insane values.
    panel_width = 300;
    dt_conf_set_int("panel_width", panel_width);
  }

  //  dt_gui_background_jobs_init();

  /* Have the delete event (window close) end the program */
  dt_util_get_datadir(datadir, 1024);
  snprintf(path, 1024, "%s/icons", datadir);
  gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (), path);

  widget = dt_ui_center(darktable.gui->ui);

  g_signal_connect (G_OBJECT (widget), "key-press-event",
                    G_CALLBACK (key_pressed), NULL);
  g_signal_connect (G_OBJECT (widget), "configure-event",
                    G_CALLBACK (configure), NULL);
  g_signal_connect (G_OBJECT (widget), "expose-event",
                    G_CALLBACK (expose), NULL);
  g_signal_connect (G_OBJECT (widget), "motion-notify-event",
                    G_CALLBACK (mouse_moved), NULL);
  g_signal_connect (G_OBJECT (widget), "leave-notify-event",
                    G_CALLBACK (center_leave), NULL);
  g_signal_connect (G_OBJECT (widget), "enter-notify-event",
                    G_CALLBACK (center_enter), NULL);
  g_signal_connect (G_OBJECT (widget), "button-press-event",
                    G_CALLBACK (button_pressed), NULL);
  g_signal_connect (G_OBJECT (widget), "button-release-event",
                    G_CALLBACK (button_released), NULL);
  g_signal_connect (G_OBJECT (widget), "scroll-event",
                    G_CALLBACK (scrolled), NULL);
  // TODO: left, right, top, bottom:
  //leave-notify-event

  widget = darktable.gui->widgets.left_border;
  g_signal_connect (G_OBJECT (widget), "expose-event", G_CALLBACK (expose_borders), (gpointer)0);
  g_signal_connect (G_OBJECT (widget), "button-press-event", G_CALLBACK (borders_button_pressed), darktable.gui->ui);
  g_signal_connect (G_OBJECT (widget), "scroll-event", G_CALLBACK (borders_scrolled), (gpointer)0);
  g_object_set_data(G_OBJECT (widget), "border", (gpointer)0);
  widget = darktable.gui->widgets.right_border;
  g_signal_connect (G_OBJECT (widget), "expose-event", G_CALLBACK (expose_borders), (gpointer)1);
  g_signal_connect (G_OBJECT (widget), "button-press-event", G_CALLBACK (borders_button_pressed), darktable.gui->ui);
  g_signal_connect (G_OBJECT (widget), "scroll-event", G_CALLBACK (borders_scrolled), (gpointer)1);
  g_object_set_data(G_OBJECT (widget), "border", (gpointer)1);
  widget = darktable.gui->widgets.top_border;
  g_signal_connect (G_OBJECT (widget), "expose-event", G_CALLBACK (expose_borders), (gpointer)2);
  g_signal_connect (G_OBJECT (widget), "button-press-event", G_CALLBACK (borders_button_pressed), darktable.gui->ui);
  g_signal_connect (G_OBJECT (widget), "scroll-event", G_CALLBACK (borders_scrolled), (gpointer)2);
  g_object_set_data(G_OBJECT (widget), "border", (gpointer)2);
  widget = darktable.gui->widgets.bottom_border;
  g_signal_connect (G_OBJECT (widget), "expose-event", G_CALLBACK (expose_borders), (gpointer)3);
  g_signal_connect (G_OBJECT (widget), "button-press-event", G_CALLBACK (borders_button_pressed), darktable.gui->ui);
  g_signal_connect (G_OBJECT (widget), "scroll-event", G_CALLBACK (borders_scrolled), (gpointer)3);
  g_object_set_data(G_OBJECT (widget), "border", (gpointer)3);
  dt_gui_presets_init();

  widget = dt_ui_center(darktable.gui->ui);
  GTK_WIDGET_UNSET_FLAGS (widget, GTK_DOUBLE_BUFFERED);
  // GTK_WIDGET_SET_FLAGS (widget, GTK_DOUBLE_BUFFERED);
  GTK_WIDGET_SET_FLAGS   (widget, GTK_APP_PAINTABLE);

  // TODO: make this work as: libgnomeui testgnome.c
  /*  GtkContainer *box = GTK_CONTAINER(darktable.gui->widgets.plugins_vbox);
  GtkScrolledWindow *swin = GTK_SCROLLED_WINDOW(darktable.gui->
                                                widgets.right_scrolled_window);
  gtk_container_set_focus_vadjustment (box, gtk_scrolled_window_get_vadjustment (swin));
  */
  dt_ctl_get_display_profile(widget, &darktable.control->xprofile_data, &darktable.control->xprofile_size);

  // register keys for view switching
  dt_accel_register_global(NC_("accel", "capture view"), GDK_t, 0);
  dt_accel_register_global(NC_("accel", "lighttable view"), GDK_l, 0);
  dt_accel_register_global(NC_("accel", "darkroom view"), GDK_d, 0);

  dt_accel_connect_global(
      "capture view",
      g_cclosure_new(G_CALLBACK(_gui_switch_view_key_accel_callback),
                     (gpointer)DT_GUI_VIEW_SWITCH_TO_TETHERING, NULL));
  dt_accel_connect_global(
      "lighttable view",
      g_cclosure_new(G_CALLBACK(_gui_switch_view_key_accel_callback),
                     (gpointer)DT_GUI_VIEW_SWITCH_TO_LIBRARY, NULL));
  dt_accel_connect_global(
      "darkroom view",
      g_cclosure_new(G_CALLBACK(_gui_switch_view_key_accel_callback),
                     (gpointer)DT_GUI_VIEW_SWITCH_TO_DARKROOM, NULL));

  // register_keys for applying styles
  init_styles_key_accels();
  connect_styles_key_accels();
  // register ctrl-q to quit:
  dt_accel_register_global(NC_("accel", "quit"), GDK_q, GDK_CONTROL_MASK);

  dt_accel_connect_global(
      "quit",
      g_cclosure_new(G_CALLBACK(quit_callback), NULL, NULL));

  // Contrast and brightness accelerators
  dt_accel_register_global(NC_("accel", "increase brightness"),
                           GDK_F10, 0);
  dt_accel_register_global(NC_("accel", "decrease brightness"),
                           GDK_F9, 0);
  dt_accel_register_global(NC_("accel", "increase contrast"),
                           GDK_F8, 0);
  dt_accel_register_global(NC_("accel", "decrease contrast"),
                           GDK_F7, 0);

  dt_accel_connect_global(
      "increase brightness",
      g_cclosure_new(G_CALLBACK(brightness_key_accel_callback),
                     (gpointer)1, NULL));
  dt_accel_connect_global(
      "decrease brightness",
      g_cclosure_new(G_CALLBACK(brightness_key_accel_callback),
                     (gpointer)0, NULL));
  dt_accel_connect_global(
      "increase contrast",
      g_cclosure_new(G_CALLBACK(contrast_key_accel_callback),
                     (gpointer)1, NULL));
  dt_accel_connect_global(
      "decrease contrast",
      g_cclosure_new(G_CALLBACK(contrast_key_accel_callback),
                     (gpointer)0, NULL));

  // Full-screen accelerators
  dt_accel_register_global(NC_("accel", "toggle fullscreen"), GDK_F11, 0);
  dt_accel_register_global(NC_("accel", "leave fullscreen"), GDK_Escape, 0);

  dt_accel_connect_global(
      "toggle fullscreen",
      g_cclosure_new(G_CALLBACK(fullscreen_key_accel_callback),
                     (gpointer)1, NULL));
  dt_accel_connect_global(
      "leave fullscreen",
      g_cclosure_new(G_CALLBACK(fullscreen_key_accel_callback),
                     (gpointer)0, NULL));

  // Side-border hide/show
  dt_accel_register_global(NC_("accel", "toggle side borders"), GDK_Tab, 0);

  // toggle view of header 
  dt_accel_register_global(NC_("accel", "toggle header"),
                           GDK_h, GDK_CONTROL_MASK);

  // View-switch
  dt_accel_register_global(NC_("accel", "switch view"), GDK_period, 0);

  dt_accel_connect_global(
      "switch view",
      g_cclosure_new(G_CALLBACK(view_switch_key_accel_callback), NULL, NULL));

  darktable.gui->reset = 0;
  for(int i=0; i<3; i++) darktable.gui->bgcolor[i] = 0.1333;

  /* apply contrast to theme */
  dt_gui_contrast_init ();

  return 0;
}

void dt_gui_gtk_cleanup(dt_gui_gtk_t *gui)
{
  g_free(darktable.control->xprofile_data);
  darktable.control->xprofile_size = 0;
}

void dt_gui_gtk_run(dt_gui_gtk_t *gui)
{
  GtkWidget *widget = dt_ui_center(darktable.gui->ui);
  darktable.gui->pixmap = gdk_pixmap_new(widget->window, widget->allocation.width, widget->allocation.height, -1);
  /* start the event loop */
  gtk_main ();
  gdk_threads_leave();
}

void init_widgets()
{

  GtkWidget* container;
  GtkWidget* widget;

  // Creating the main window
  widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  darktable.gui->ui->main_window = widget;
  gtk_window_set_default_size(GTK_WINDOW(widget), 900, 500);

  gtk_window_set_icon_name(GTK_WINDOW(widget), "darktable");
  gtk_window_set_title(GTK_WINDOW(widget), "Darktable");

  g_signal_connect (G_OBJECT (widget), "delete_event",
                    G_CALLBACK (quit), NULL);
  g_signal_connect (G_OBJECT (widget), "key-press-event",
                    G_CALLBACK (key_pressed_override), NULL);
  g_signal_connect (G_OBJECT (widget), "key-release-event",
                    G_CALLBACK (key_released), NULL);

  container = widget;

  // Adding the outermost vbox
  widget = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(container), widget);
  gtk_widget_show(widget);

  /* connect to signal redraw all */
  dt_control_signal_connect(darktable.signals,
                            DT_SIGNAL_CONTROL_REDRAW_ALL,
                            G_CALLBACK(_ui_widget_redraw_callback),
                            darktable.gui->ui->main_window);

  container = widget;

  // Initializing the top border
  widget = gtk_drawing_area_new();
  darktable.gui->widgets.top_border = widget;
  gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);
  gtk_widget_set_size_request(widget, -1, 10);
  gtk_widget_set_app_paintable(widget, TRUE);
  gtk_widget_set_events(widget,
                        GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_ENTER_NOTIFY_MASK
                        | GDK_LEAVE_NOTIFY_MASK
                        | GDK_STRUCTURE_MASK
                        | GDK_SCROLL_MASK);
  gtk_widget_show(widget);

  // Initializing the main table
  init_main_table(container);

  // Initializing the bottom border
  widget = gtk_drawing_area_new();
  darktable.gui->widgets.bottom_border = widget;
  gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);
  gtk_widget_set_size_request(widget, -1, 10);
  gtk_widget_set_app_paintable(widget, TRUE);
  gtk_widget_set_events(widget,
                        GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_ENTER_NOTIFY_MASK
                        | GDK_LEAVE_NOTIFY_MASK
                        | GDK_STRUCTURE_MASK
                        | GDK_SCROLL_MASK);
  gtk_widget_show(widget);

  // Showing everything
  gtk_widget_show_all(dt_ui_main_window(darktable.gui->ui));

  /* hide panels depending on last ui state */
  for(int k=0;k<DT_UI_PANEL_SIZE;k++)
  {
    /* prevent show all */
    gtk_widget_set_no_show_all(GTK_WIDGET(darktable.gui->ui->containers[k]), TRUE);

    /* check last visible state of panel */
    char key[512];
    g_snprintf(key, 512, "ui_last/%s/visible", _ui_panel_config_names[k]);
    
    /* if no key, lets default to TRUE*/
    if(!dt_conf_key_exists(key))
      dt_conf_set_bool(key,TRUE);

    if (!dt_conf_get_bool(key))
      gtk_widget_set_visible(darktable.gui->ui->panels[k],FALSE);

  }

}

void init_main_table(GtkWidget *container)
{
  GtkWidget *widget;

  // Creating the table
  widget = gtk_table_new(3, 5, FALSE);
  gtk_box_pack_start(GTK_BOX(container), widget, TRUE, TRUE, 0);
  gtk_widget_show(widget);

  container = widget;

  // Adding the left border
  widget = gtk_drawing_area_new();
  darktable.gui->widgets.left_border = widget;

  gtk_widget_set_size_request(widget, 10, -1);
  gtk_widget_set_app_paintable(widget, TRUE);
  gtk_widget_set_events(widget,
                        GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_ENTER_NOTIFY_MASK
                        | GDK_LEAVE_NOTIFY_MASK
                        | GDK_STRUCTURE_MASK
                        | GDK_SCROLL_MASK);
  gtk_table_attach(GTK_TABLE(container), widget, 0, 1, 0, 2,
                   GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show(widget);

  // Adding the right border
  widget = gtk_drawing_area_new();
  darktable.gui->widgets.right_border = widget;

  gtk_widget_set_size_request(widget, 10, -1);
  gtk_widget_set_app_paintable(widget, TRUE);
  gtk_widget_set_events(widget,
                        GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_ENTER_NOTIFY_MASK
                        | GDK_LEAVE_NOTIFY_MASK
                        | GDK_STRUCTURE_MASK
                        | GDK_SCROLL_MASK);
  gtk_table_attach(GTK_TABLE(container), widget, 4, 5, 0, 2,
                   GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show(widget);

  /* initialize the top container */
  _ui_init_panel_top(darktable.gui->ui, container);

  /* 
   * initialize the center top/center/bottom 
   */
  widget = gtk_vbox_new(FALSE, 0);
  gtk_table_attach(GTK_TABLE(container), widget, 2, 3, 1, 2,
                   GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

  /* intiialize the center top panel */
  _ui_init_panel_center_top(darktable.gui->ui, widget);
  
  /* setup center drawing area */
  GtkWidget *cda = gtk_drawing_area_new();
  gtk_widget_set_size_request(cda, -1, 500);
  gtk_widget_set_app_paintable(cda, TRUE);
  gtk_widget_set_events(cda,
            GDK_POINTER_MOTION_MASK
            | GDK_POINTER_MOTION_HINT_MASK
            | GDK_BUTTON_PRESS_MASK
            | GDK_BUTTON_RELEASE_MASK
            | GDK_ENTER_NOTIFY_MASK
            | GDK_LEAVE_NOTIFY_MASK);
  gtk_widget_set_can_focus(cda, TRUE);
  gtk_widget_set_visible(cda, TRUE);

  gtk_box_pack_start(GTK_BOX(widget), cda, TRUE, TRUE, 0);
  darktable.gui->ui->center = cda;

  /* center should redraw when signal redraw center is raised*/
  dt_control_signal_connect(darktable.signals, 
            DT_SIGNAL_CONTROL_REDRAW_CENTER, 
            G_CALLBACK(_ui_widget_redraw_callback), 
            darktable.gui->ui->center);

  /* initialize the center bottom panel */
  _ui_init_panel_center_bottom(darktable.gui->ui, widget);

  /* initialize the bottom panel */
  _ui_init_panel_bottom(darktable.gui->ui, container);

  /* initialize  left panel */
  _ui_init_panel_left(darktable.gui->ui, container);

  /* initialize right panel */
  _ui_init_panel_right(darktable.gui->ui, container);
}

/*
 * NEW UI API
 */
dt_ui_t *dt_ui_initialize(int argc, char **argv)
{
  dt_ui_t *ui=g_malloc(sizeof(dt_ui_t));
  memset(ui,0,sizeof(dt_ui_t));
  return ui;
}

void dt_ui_destroy(struct dt_ui_t *ui)
{
  g_free(ui);
}

void dt_ui_container_add_widget(dt_ui_t *ui, const dt_ui_container_t c, GtkWidget *w)
{
  //  if(!GTK_IS_BOX(ui->containers[c])) return;
  g_return_if_fail(GTK_IS_BOX(ui->containers[c]));
  switch(c)
  {
    /* if box is right lets pack at end for nicer alignment */
    case DT_UI_CONTAINER_PANEL_TOP_RIGHT:
    case DT_UI_CONTAINER_PANEL_CENTER_TOP_RIGHT:
    case DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_RIGHT:
      gtk_box_pack_end(GTK_BOX(ui->containers[c]),w,FALSE,FALSE,2);
      break;

    /* if box is center we want it to fill as much as it can */
    case DT_UI_CONTAINER_PANEL_TOP_CENTER:
    case DT_UI_CONTAINER_PANEL_CENTER_TOP_CENTER:
    case DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_CENTER:
    case DT_UI_CONTAINER_PANEL_BOTTOM:
      gtk_box_pack_start(GTK_BOX(ui->containers[c]),w,TRUE,TRUE,2);
      break;

    default:
    {
      gtk_box_pack_start(GTK_BOX(ui->containers[c]),w,FALSE,FALSE,2);
    }  break;
  }
  gtk_widget_show_all(w);
}

void dt_ui_container_focus_widget(dt_ui_t *ui, const dt_ui_container_t c, GtkWidget *w)
{
  //if(!GTK_IS_CONTAINER(ui->containers[c])) return;
  g_return_if_fail(GTK_IS_CONTAINER(ui->containers[c]));
  gtk_container_set_focus_child(GTK_CONTAINER(ui->containers[c]), w);
  gtk_widget_queue_draw(ui->containers[c]);
}

void dt_ui_container_clear(struct dt_ui_t *ui, const dt_ui_container_t c)
{
  g_return_if_fail(GTK_IS_CONTAINER(ui->containers[c]));
  gtk_container_foreach(GTK_CONTAINER(ui->containers[c]), (GtkCallback)gtk_widget_destroy, (gpointer)c);
}

void dt_ui_toggle_panels_visibility(struct dt_ui_t *ui)
{
  char key[512];
  const dt_view_t *cv = dt_view_manager_get_current_view(darktable.view_manager);
  g_snprintf(key, 512, "%s/ui/panel_collaps_state",cv->module_name);
  uint32_t state = dt_conf_get_int(key);
  
  if (state)
  {
    /* restore previous panel view states */
    for (int k=0;k<DT_UI_PANEL_SIZE;k++)
      dt_ui_panel_show(ui, k, (state>>k)&1);
    
    /* reset state */
    state = 0;
  }
  else
  {
    /* store current panel view state */
    for (int k=0;k<DT_UI_PANEL_SIZE;k++)
      state |= (uint32_t)(dt_ui_panel_visible(ui, k))<<k;
    
    /* hide all panels */
    for (int k=0;k<DT_UI_PANEL_SIZE;k++)
      dt_ui_panel_show(ui, k, FALSE);
  }

  /* store new state */
  dt_conf_set_int(key, state);
}

void dt_ui_restore_panels(dt_ui_t *ui)
{
  /* restore visible state of panels for current view */
  const dt_view_t *cv = dt_view_manager_get_current_view(darktable.view_manager);
  char key[512];

  /* restore from a previous collapse all panel state if enabled */
  g_snprintf(key, 512, "%s/ui/panel_collaps_state",cv->module_name);
  uint32_t state = dt_conf_get_int(key);
  if (state)
  {
    /* restore previous panel view states */
    for (int k=0;k<DT_UI_PANEL_SIZE;k++)
      dt_ui_panel_show(ui, k, (state>>k)&1);
    
    /* clear state */
    dt_conf_set_int(key, 0);
  }
  else
  {
    /* restore the visibile state of panels */
    for (int k=0;k<DT_UI_PANEL_SIZE;k++)
    {
      g_snprintf(key, 512, "%s/ui/%s_visible",cv->module_name, _ui_panel_config_names[k]);
      if (dt_conf_key_exists(key))
        gtk_widget_set_visible(ui->panels[k], dt_conf_get_bool(key));
      else
        gtk_widget_set_visible(ui->panels[k], 1);
    }
  }
}

void dt_ui_panel_show(dt_ui_t *ui,const dt_ui_panel_t p, gboolean show)
{
  //if(!GTK_IS_WIDGET(ui->panels[p])) return;
  g_return_if_fail(GTK_IS_WIDGET(ui->panels[p]));

  const dt_view_t *cv = dt_view_manager_get_current_view(darktable.view_manager);
  char key[512];
  g_snprintf(key, 512, "%s/ui/%s_visible",cv->module_name, _ui_panel_config_names[p]);
  dt_conf_set_bool(key, show);

  if(show)
    gtk_widget_show(ui->panels[p]);
  else
    gtk_widget_hide(ui->panels[p]);
}

gboolean dt_ui_panel_visible(dt_ui_t *ui,const dt_ui_panel_t p)
{
  //if(!GTK_IS_WIDGET(ui->panels[p])) return FALSE;
  g_return_val_if_fail(GTK_IS_WIDGET(ui->panels[p]),FALSE);
  return gtk_widget_get_visible(ui->panels[p]);
}

GtkWidget *dt_ui_center(dt_ui_t *ui)
{
  return ui->center;
}

GtkWidget *dt_ui_main_window(dt_ui_t *ui)
{
  return ui->main_window;
}

static GtkWidget * _ui_init_panel_container_top(GtkWidget *container)
{
  GtkWidget *w = gtk_vbox_new(FALSE, DT_UI_PANEL_MODULE_SPACING);
  gtk_box_pack_start(GTK_BOX(container),w,FALSE,FALSE,4);
  return w;
}

static GtkWidget * _ui_init_panel_container_center(GtkWidget *container, gboolean left)
{
  GtkWidget *widget;
  GtkAdjustment *a[4];

  a[0] = GTK_ADJUSTMENT(gtk_adjustment_new(0,0,100,1,10,10));
  a[1] = GTK_ADJUSTMENT(gtk_adjustment_new(0,0,100,1,10,10));
  a[2] = GTK_ADJUSTMENT(gtk_adjustment_new(0,0,100,1,10,10));
  a[3] = GTK_ADJUSTMENT(gtk_adjustment_new(0,0,100,1,10,10));

  /* create the scrolled window */
  widget = gtk_scrolled_window_new(a[0],a[1]);
  gtk_widget_set_can_focus(widget, TRUE);
  gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(widget), left?GTK_CORNER_TOP_LEFT:GTK_CORNER_TOP_RIGHT);
  gtk_box_pack_start(GTK_BOX(container), widget, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (widget,dt_conf_get_int("panel_width")-5-13, -1);

  /* create the scrolled viewport */
  container = widget;
  widget = gtk_viewport_new(a[2],a[3]);
  gtk_viewport_set_shadow_type(GTK_VIEWPORT(widget), GTK_SHADOW_NONE);
  gtk_container_set_resize_mode(GTK_CONTAINER(widget), GTK_RESIZE_QUEUE);
  gtk_container_add(GTK_CONTAINER(container), widget);

  /* create the container */
  container = widget;
  widget = gtk_vbox_new(FALSE, DT_UI_PANEL_MODULE_SPACING);
  gtk_widget_set_name(widget, "plugins_vbox_left");
  gtk_widget_set_size_request (widget,0, -1);
  gtk_container_add(GTK_CONTAINER(container),widget);

  return widget;
}

static GtkWidget * _ui_init_panel_container_bottom(GtkWidget *container)
{
  GtkWidget *w = gtk_vbox_new(FALSE, DT_UI_PANEL_MODULE_SPACING);
  gtk_box_pack_start(GTK_BOX(container),w,FALSE,FALSE,DT_UI_PANEL_MODULE_SPACING);
  return w;
}
     
static void _ui_init_panel_left(dt_ui_t *ui, GtkWidget *container)
{
  GtkWidget *widget;
  
  /* create left panel main widget and add it to ui */
  widget = ui->panels[DT_UI_PANEL_LEFT] = gtk_alignment_new(.5, .5, 1, 1);
  gtk_widget_set_name(widget, "left");
  gtk_alignment_set_padding(GTK_ALIGNMENT(widget), 0, 0, 5, 0);
  gtk_table_attach(GTK_TABLE(container), widget, 1, 2, 1, 2,
	    GTK_SHRINK, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);

  /* set panel width */
  gtk_widget_set_size_request(widget,dt_conf_get_int("panel_width"), -1);
    
  // Adding the vbox which will containt TOP,CENTER,BOTTOM                                                                                                                                                                             
  container = widget;
  widget = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(container), widget);
  
  /* add top,center,bottom*/
  container = widget;
  ui->containers[DT_UI_CONTAINER_PANEL_LEFT_TOP] = _ui_init_panel_container_top(container);
  ui->containers[DT_UI_CONTAINER_PANEL_LEFT_CENTER] = _ui_init_panel_container_center(container, FALSE);
  ui->containers[DT_UI_CONTAINER_PANEL_LEFT_BOTTOM] = _ui_init_panel_container_bottom(container);
  
  /* lets show all widgets */
  gtk_widget_show_all(ui->panels[DT_UI_PANEL_LEFT]);  
}

static void _ui_init_panel_right(dt_ui_t *ui, GtkWidget *container)
{
  GtkWidget *widget;

  /* create left panel main widget and add it to ui */
  widget = ui->panels[DT_UI_PANEL_RIGHT] = gtk_alignment_new(.5, .5, 1, 1);
  gtk_widget_set_name(widget, "right");
  gtk_alignment_set_padding(GTK_ALIGNMENT(widget), 0, 0, 0, 5);
  gtk_table_attach(GTK_TABLE(container), widget, 3, 4, 1, 2,
            GTK_SHRINK, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);

  /* set panel width */
  gtk_widget_set_size_request(widget,dt_conf_get_int("panel_width"), -1);

  // Adding the vbox which will containt TOP,CENTER,BOTTOM                                                                                                                                                                             
  container = widget;
  widget = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(container), widget);
  gtk_widget_set_size_request(widget, 0, -1);

  /* add top,center,bottom*/
  container = widget;
  ui->containers[DT_UI_CONTAINER_PANEL_RIGHT_TOP] = _ui_init_panel_container_top(container);
  ui->containers[DT_UI_CONTAINER_PANEL_RIGHT_CENTER] = _ui_init_panel_container_center(container, TRUE);
  ui->containers[DT_UI_CONTAINER_PANEL_RIGHT_BOTTOM] = _ui_init_panel_container_bottom(container);

  /* lets show all widgets */
  gtk_widget_show_all(ui->panels[DT_UI_PANEL_RIGHT]);
}

static void _ui_init_panel_top(dt_ui_t *ui, GtkWidget *container)
{
  GtkWidget *widget;

  /* create the panel box */
  ui->panels[DT_UI_PANEL_TOP] = widget = gtk_hbox_new(FALSE, 0);
  gtk_table_attach(GTK_TABLE(container), widget, 1, 4, 0, 1,
                   GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_SHRINK, 0, 0);

  /* add container for top left */
  ui->containers[DT_UI_CONTAINER_PANEL_TOP_LEFT] = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_TOP_LEFT], FALSE, FALSE,  DT_UI_PANEL_MODULE_SPACING);

  /* add container for top center */
  ui->containers[DT_UI_CONTAINER_PANEL_TOP_CENTER] = gtk_hbox_new(TRUE,0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_TOP_CENTER], TRUE, TRUE, DT_UI_PANEL_MODULE_SPACING);

  /* add container for top right */
  ui->containers[DT_UI_CONTAINER_PANEL_TOP_RIGHT] = gtk_hbox_new(FALSE,0);
  gtk_box_pack_end(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_TOP_RIGHT], FALSE, FALSE, DT_UI_PANEL_MODULE_SPACING);

}

static void _ui_init_panel_bottom(dt_ui_t *ui, GtkWidget *container)
{
  GtkWidget *widget;

  /* create the panel box */
  ui->panels[DT_UI_PANEL_BOTTOM] = widget = gtk_hbox_new(FALSE, 0);
  gtk_table_attach(GTK_TABLE(container), widget, 1, 4, 2, 3,
            GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_SHRINK, 0, 0); 

  /* add the container */
  ui->containers[DT_UI_CONTAINER_PANEL_BOTTOM] = gtk_hbox_new(TRUE,0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_BOTTOM], TRUE, TRUE,  DT_UI_PANEL_MODULE_SPACING);
}


static void _ui_init_panel_center_top(dt_ui_t *ui, GtkWidget *container)
{
  GtkWidget *widget;

  /* create the panel box */
  ui->panels[DT_UI_PANEL_CENTER_TOP] = widget = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

  /* add container for center top left */
  ui->containers[DT_UI_CONTAINER_PANEL_CENTER_TOP_LEFT] = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_CENTER_TOP_LEFT], FALSE, FALSE, DT_UI_PANEL_MODULE_SPACING);

  /* add container for center top center */
  ui->containers[DT_UI_CONTAINER_PANEL_CENTER_TOP_CENTER] = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_CENTER_TOP_CENTER], TRUE, TRUE, DT_UI_PANEL_MODULE_SPACING);

  /* add container for center top right */
  ui->containers[DT_UI_CONTAINER_PANEL_CENTER_TOP_RIGHT] = gtk_hbox_new(FALSE,0);
  gtk_box_pack_end(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_CENTER_TOP_RIGHT], FALSE, FALSE, DT_UI_PANEL_MODULE_SPACING);

}

static void _ui_init_panel_center_bottom(dt_ui_t *ui, GtkWidget *container)
{
  GtkWidget *widget;

  /* create the panel box */
  ui->panels[DT_UI_PANEL_CENTER_BOTTOM] = widget = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

  /* adding the center bottom left toolbox */
  ui->containers[DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_LEFT] = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_LEFT], TRUE, TRUE, DT_UI_PANEL_MODULE_SPACING);
  
  /* adding the center box */
  ui->containers[DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_CENTER] = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_CENTER], FALSE, TRUE, DT_UI_PANEL_MODULE_SPACING);

  /* adding the right toolbox */
  ui->containers[DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_RIGHT] = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(widget), ui->containers[DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_RIGHT], TRUE, TRUE, DT_UI_PANEL_MODULE_SPACING);

}

/* this is always called on gui thread !!! */
static void _ui_widget_redraw_callback(gpointer instance, GtkWidget *widget)
{
  //  g_return_if_fail(GTK_IS_WIDGET(widget) && gtk_widget_is_drawable(widget));
  gboolean i_own_lock = dt_control_gdk_lock();
  gtk_widget_queue_draw(widget);
  if(i_own_lock) dt_control_gdk_unlock();

}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
