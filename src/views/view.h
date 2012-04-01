/*
    This file is part of darktable,
    copyright (c) 2009--2010 johannes hanika.

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
#ifndef DT_VIEW_H
#define DT_VIEW_H

#include "common/image.h"
#include <inttypes.h>
#include <gui/gtk.h>
#include <gmodule.h>
#include <cairo.h>
#include <sqlite3.h>

/** avilable views flags, a view should return it's type and
    is also used in modules flags available in src/libs to
    control which view the module should be available in also
    which placement in the panels the module have.
*/
enum dt_view_type_flags_t {
  DT_VIEW_LIGHTTABLE = 1,
  DT_VIEW_DARKROOM = 2,
  DT_VIEW_TETHERING = 4,
};

/**
 * main dt view module (as lighttable or darkroom)
 */
struct dt_view_t;
typedef struct dt_view_t
{
  char module_name[64];
  // dlopened module
  GModule *module;
  // custom data for module
  void *data;
  // width and height of allocation
  uint32_t width, height;
  // scroll bar control
  float vscroll_size, vscroll_viewport_size, vscroll_pos;
  float hscroll_size, hscroll_viewport_size, hscroll_pos;
  const char *(*name)     (struct dt_view_t *self); // get translatable name
  uint32_t (*view)        (struct dt_view_t *self); // get the view type
  void (*init)            (struct dt_view_t *self); // init *data
  void (*cleanup)         (struct dt_view_t *self); // cleanup *data
  void (*expose)          (struct dt_view_t *self, cairo_t *cr, int32_t width, int32_t height, int32_t pointerx, int32_t pointery); // expose the module (gtk callback)
  int  (*try_enter)       (struct dt_view_t *self); // test if enter can succeed.
  void (*enter)           (struct dt_view_t *self); // mode entered, this module got focus. return non-null on failure.
  void (*leave)           (struct dt_view_t *self); // mode left (is called after the new try_enter has succeded).
  void (*reset)           (struct dt_view_t *self); // reset default appearance

  // event callbacks:
  int  (*mouse_enter)     (struct dt_view_t *self);
  int  (*mouse_leave)     (struct dt_view_t *self);
  int  (*mouse_moved)     (struct dt_view_t *self, double x, double y, int which);
  int  (*button_released) (struct dt_view_t *self, double x, double y, int which, uint32_t state);
  int  (*button_pressed)  (struct dt_view_t *self, double x, double y, int which, int type, uint32_t state);
  int  (*key_pressed)     (struct dt_view_t *self, guint key, guint state);
  int  (*key_released)    (struct dt_view_t *self, guint key, guint state);
  void (*configure)       (struct dt_view_t *self, int width, int height);
  void (*scrolled)        (struct dt_view_t *self, double x, double y, int up, int state); // mouse scrolled in view
  void (*border_scrolled) (struct dt_view_t *self, double x, double y, int which, int up); // mouse scrolled on left/right/top/bottom border (which 0123).

  // keyboard accel callbacks
  void (*init_key_accels)(struct dt_view_t *self);
  void (*connect_key_accels)(struct dt_view_t *self);

  GSList *accel_closures;
}
dt_view_t;

typedef enum dt_view_image_over_t
{
  DT_VIEW_DESERT = 0,
  DT_VIEW_STAR_1 = 1,
  DT_VIEW_STAR_2 = 2,
  DT_VIEW_STAR_3 = 3,
  DT_VIEW_STAR_4 = 4,
  DT_VIEW_STAR_5 = 5,
  DT_VIEW_REJECT = 6
}
dt_view_image_over_t;

/** expose an image, set image over flags. */
void
dt_view_image_expose(
    dt_view_image_over_t *image_over,
    uint32_t index,
    cairo_t *cr,
    int32_t width,
    int32_t height,
    int32_t zoom,
    int32_t px,
    int32_t py);

/** Set the selection bit to a given value for the specified image */
void dt_view_set_selection(int imgid, int value);
/** toggle selection of given image. */
void dt_view_toggle_selection(int imgid);

#define DT_VIEW_MAX_MODULES 10
/**
 * holds all relevant data needed to manage the view
 * modules.
 */
typedef struct dt_view_manager_t
{
  dt_view_t view[DT_VIEW_MAX_MODULES];
  int32_t current_view, num_views;

  /* reusable db statements 
   * TODO: reconsider creating a common/database helper API
   *       instead of having this spread around in sources..
   */
  struct {
    /* select num from history where imgid = ?1*/
    sqlite3_stmt *have_history;
    /* select * from selected_images where imgid = ?1 */
    sqlite3_stmt *is_selected;
    /* delete from selected_images where imgid = ?1 */
    sqlite3_stmt *delete_from_selected;
    /* insert into selected_images values (?1) */
    sqlite3_stmt *make_selected;
    /* select color from color_labels where imgid=?1 */
    sqlite3_stmt *get_color;

  } statements;


  /*
   * Proxy
   */
  struct {

    /* view toolbox proxy object */
    struct {
      struct dt_lib_module_t *module;
      void (*add)(struct dt_lib_module_t *,GtkWidget *);
    } view_toolbox;

    /* module toolbox proxy object */
    struct {
      struct dt_lib_module_t *module;
      void (*add)(struct dt_lib_module_t *,GtkWidget *);
    } module_toolbox;

    /* module collection proxy object */
    struct {
      struct dt_lib_module_t *module;
      void (*update)(struct dt_lib_module_t *);
    } module_collect;

    /* filmstrip proxy object */
    struct {
      struct dt_lib_module_t *module;
      void (*scroll_to_image)(struct dt_lib_module_t *, gint imgid, gboolean activate);
      int32_t (*activated_image)(struct dt_lib_module_t *);
    } filmstrip;

    /* lighttable view proxy object */
    struct {
      struct dt_lib_module_t *module;
      void (*set_zoom)(struct dt_lib_module_t *module, gint zoom);
    } lighttable;

    /* tethering view proxy object */
    struct {
      struct dt_view_t *view;
      uint32_t (*get_film_id)(const dt_view_t *view);
      const char *(*get_session_filename)(const dt_view_t *view, const char *filename);
      const char *(*get_session_path)(const dt_view_t *view);
      const char *(*get_job_code)(const dt_view_t *view);
      void (*set_job_code)(const dt_view_t *view, const char *name);
    } tethering;

  } proxy;


}
dt_view_manager_t;

void dt_view_manager_init(dt_view_manager_t *vm);
void dt_view_manager_cleanup(dt_view_manager_t *vm);

/** return translated name. */
const char *dt_view_manager_name (dt_view_manager_t *vm);
/** switch to this module. returns non-null if the module fails to change. */
int dt_view_manager_switch(dt_view_manager_t *vm, int k);
/** expose current module. */
void dt_view_manager_expose(dt_view_manager_t *vm, cairo_t *cr, int32_t width, int32_t height, int32_t pointerx, int32_t pointery);
/** reset current view. */
void dt_view_manager_reset(dt_view_manager_t *vm);
/** get current view of the view manager. */
const dt_view_t *dt_view_manager_get_current_view(dt_view_manager_t *vm);

void dt_view_manager_mouse_enter     (dt_view_manager_t *vm);
void dt_view_manager_mouse_leave     (dt_view_manager_t *vm);
void dt_view_manager_mouse_moved     (dt_view_manager_t *vm, double x, double y, int which);
int dt_view_manager_button_released  (dt_view_manager_t *vm, double x, double y, int which, uint32_t state);
int dt_view_manager_button_pressed   (dt_view_manager_t *vm, double x, double y, int which, int type, uint32_t state);
int dt_view_manager_key_pressed      (dt_view_manager_t *vm, guint key, guint state);
int dt_view_manager_key_released     (dt_view_manager_t *vm, guint key, guint state);
void dt_view_manager_configure       (dt_view_manager_t *vm, int width, int height);
void dt_view_manager_scrolled        (dt_view_manager_t *vm, double x, double y, int up, int state);
void dt_view_manager_border_scrolled (dt_view_manager_t *vm, double x, double y, int which, int up);

/** add widget to the current view toolbox */
void dt_view_manager_view_toolbox_add(dt_view_manager_t *vm,GtkWidget *tool);

/** add widget to the current module toolbox */
void dt_view_manager_module_toolbox_add(dt_view_manager_t *vm,GtkWidget *tool);

/** load module to view managers list, if still space. return slot number on success. */
int dt_view_manager_load_module(dt_view_manager_t *vm, const char *mod);
/** load a view module */
int dt_view_load_module(dt_view_t *view, const char *module);
/** unload, cleanup */
void dt_view_unload_module(dt_view_t *view);
/** set scrollbar positions, gui method. */
void dt_view_set_scrollbar(dt_view_t *view, float hpos, float hsize, float hwinsize, float vpos, float vsize, float vwinsize);

/*
 * Tethering View PROXY
 */
/** get the current filmroll id for tethering session */
int32_t dt_view_tethering_get_film_id(const dt_view_manager_t *vm);
/** get the current session path for tethering session */
const char *dt_view_tethering_get_session_path(const dt_view_manager_t *vm);
/** get the current session filename for tethering session */
const char *dt_view_tethering_get_session_filename(const dt_view_manager_t *vm, const char *filename);
/** set the current jobcode for tethering session */
void dt_view_tethering_set_job_code(const dt_view_manager_t *vm, const char *name);
/** get the current jobcode for tethering session */
const char *dt_view_tethering_get_job_code(const dt_view_manager_t *vm);

/** update the collection module */
void dt_view_collection_update(const dt_view_manager_t *vm);

/*
 * NEW filmstrip api
 */
/** scrolls filmstrip to the specified image */
void dt_view_filmstrip_scroll_to_image(dt_view_manager_t *vm, const int imgid, gboolean activate);
/** get the imageid from last filmstrip activate request */
int32_t dt_view_filmstrip_get_activated_imgid(dt_view_manager_t *vm);

/** sets the lighttable image in row zoom */
void dt_view_lighttable_set_zoom(dt_view_manager_t *vm, gint zoom);

/** set active image */
void dt_view_filmstrip_set_active_image(dt_view_manager_t *vm,int iid);
/** prefetch the next few images in film strip, from selected on. 
    TODO: move to control ?
*/
void dt_view_filmstrip_prefetch();



#endif
