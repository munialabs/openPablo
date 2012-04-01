/*
    This file is part of darktable,
    copyright (c) 2009--2011 johannes hanika, henrik andersson.

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
#  include "config.h"
#endif
#include "common/darktable.h"
#include "control/control.h"
#include "control/conf.h"
#include "develop/develop.h"
#include "common/image_cache.h"
#include "common/imageio.h"
#include "common/debug.h"
#include "views/view.h"
#include "gui/gtk.h"
#include "gui/contrast.h"
#include "gui/draw.h"

#ifdef GDK_WINDOWING_QUARTZ
#  include <Carbon/Carbon.h>
#  include <ApplicationServices/ApplicationServices.h>
#  include <CoreServices/CoreServices.h>
#endif

#ifdef G_OS_WIN32
#  include <windows.h>
#endif

#include <stdlib.h>
#include <strings.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

void dt_ctl_settings_default(dt_control_t *c)
{
  dt_conf_set_string ("database", "library.db");

  dt_conf_set_int  ("config_version", DT_CONFIG_VERSION);
  dt_conf_set_bool ("write_sidecar_files", TRUE);
  dt_conf_set_bool ("ask_before_delete", TRUE);
  dt_conf_set_int  ("parallel_export", 1);
  dt_conf_set_int  ("worker_threads", 2);
  dt_conf_set_int  ("cache_memory", 536870912);
  dt_conf_set_int  ("database_cache_quality", 89);

  dt_conf_set_bool ("ui_last/fullscreen", FALSE);
  dt_conf_set_int  ("ui_last/view", DT_MODE_NONE);

  dt_conf_set_int  ("ui_last/window_x",      0);
  dt_conf_set_int  ("ui_last/window_y",      0);
  dt_conf_set_int  ("ui_last/window_w",    900);
  dt_conf_set_int  ("ui_last/window_h",    500);

  dt_conf_set_int  ("ui_last/panel_left",   -1);
  dt_conf_set_int  ("ui_last/panel_right",  -1);
  dt_conf_set_int  ("ui_last/panel_top",     0);
  dt_conf_set_int  ("ui_last/panel_bottom",  0);

  dt_conf_set_int  ("ui_last/expander_library",     1<<DT_LIBRARY);
  dt_conf_set_int  ("ui_last/expander_navigation", -1);
  dt_conf_set_int  ("ui_last/expander_histogram",  -1);
  dt_conf_set_int  ("ui_last/expander_history",    -1);

  dt_conf_set_int  ("ui_last/initial_rating", DT_LIB_FILTER_STAR_1);

  // import settings
  dt_conf_set_string ("capture/camera/storage/basedirectory", "$(PICTURES_FOLDER)/Darktable");
  dt_conf_set_string ("capture/camera/storage/subpath", "$(YEAR)$(MONTH)$(DAY)_$(JOBCODE)");
  dt_conf_set_string ("capture/camera/storage/namepattern", "$(YEAR)$(MONTH)$(DAY)_$(SEQUENCE).$(FILE_EXTENSION)");
  dt_conf_set_string ("capture/camera/import/jobcode", "noname");

  dt_conf_set_int  ("plugins/collection/film_id",           1);
  dt_conf_set_int  ("plugins/collection/filter_flags",      3);
  dt_conf_set_int  ("plugins/collection/query_flags",       3);
  dt_conf_set_int  ("plugins/collection/rating",            1);
  dt_conf_set_int  ("plugins/lighttable/collect/num_rules", 0);
  dt_conf_set_int  ("plugins/collection/sort",              0);
  dt_conf_set_bool ("plugins/collection/descending",        0);

  // reasonable thumbnail res:
  dt_conf_set_int  ("plugins/lighttable/thumbnail_width", 1300);
  dt_conf_set_int  ("plugins/lighttable/thumbnail_height", 1000);
}

void dt_ctl_settings_init(dt_control_t *s)
{
  // same thread as init
  s->gui_thread = pthread_self();
  // init global defaults.
  dt_pthread_mutex_init(&(s->global_mutex), NULL);
  dt_pthread_mutex_init(&(s->image_mutex), NULL);

  s->global_settings.version = DT_VERSION;

  // TODO: move the mouse_over_id of lighttable to something general in control: gui-thread selected img or so?
  s->global_settings.lib_image_mouse_over_id = -1;

  // TODO: move these to darkroom settings blob:
  s->global_settings.dev_closeup = 0;
  s->global_settings.dev_zoom_x = 0;
  s->global_settings.dev_zoom_y = 0;
  s->global_settings.dev_zoom = DT_ZOOM_FIT;

  memcpy(&(s->global_defaults), &(s->global_settings), sizeof(dt_ctl_settings_t));
}

// There are systems where absolute paths don't start with '/' (like Windows). Since the bug which introduced absolute paths to
// the db was fixed before a Windows build was available this shouldn't matter though.
static void dt_control_sanitize_database()
{
  sqlite3_stmt *stmt;
  sqlite3_stmt *innerstmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select id, filename from images where filename like '/%'", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "update images set filename = ?1 where id = ?2", -1, &innerstmt, NULL);
  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    int id = sqlite3_column_int(stmt, 0);
    const char* path = (const char*)sqlite3_column_text(stmt, 1);
    gchar* filename = g_path_get_basename (path);
    DT_DEBUG_SQLITE3_BIND_TEXT(innerstmt, 1, filename, -1, SQLITE_TRANSIENT);
    DT_DEBUG_SQLITE3_BIND_INT(innerstmt, 2, id);
    sqlite3_step(innerstmt);
    sqlite3_reset(innerstmt);
    sqlite3_clear_bindings(innerstmt);
    g_free(filename);
  }
  sqlite3_finalize(stmt);
  sqlite3_finalize(innerstmt);

  // TODO: Create in attached memory database
  // temporary stuff for some ops, need this for some reason with newer sqlite3:
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table memory.color_labels_temp (imgid integer primary key)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table memory.tmp_selection (imgid integer)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table memory.tagquery1 (tagid integer, name varchar, count integer)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table memory.tagquery2 (tagid integer, name varchar, count integer)", NULL, NULL, NULL);
}

int dt_control_load_config(dt_control_t *c)
{
  dt_conf_set_int("ui_last/view", DT_MODE_NONE);
  int width  = dt_conf_get_int("ui_last/window_w");
  int height = dt_conf_get_int("ui_last/window_h");
  gint x = dt_conf_get_int("ui_last/window_x");
  gint y = dt_conf_get_int("ui_last/window_y");
  GtkWidget *widget = dt_ui_main_window(darktable.gui->ui);
  gtk_window_move(GTK_WINDOW(widget),x,y);
  gtk_window_resize(GTK_WINDOW(widget), width, height);
  int fullscreen = dt_conf_get_bool("ui_last/fullscreen");
  if(fullscreen) gtk_window_fullscreen  (GTK_WINDOW(widget));
  else           gtk_window_unfullscreen(GTK_WINDOW(widget));
  return 0;
}

int dt_control_write_config(dt_control_t *c)
{
  GtkWidget *widget = dt_ui_main_window(darktable.gui->ui);
  gint x, y;
  gtk_window_get_position(GTK_WINDOW(widget), &x, &y);
  dt_conf_set_int ("ui_last/window_x",  x);
  dt_conf_set_int ("ui_last/window_y",  y);
  dt_conf_set_int ("ui_last/window_w",  widget->allocation.width);
  dt_conf_set_int ("ui_last/window_h",  widget->allocation.height);

  sqlite3_stmt *stmt;
  dt_pthread_mutex_lock(&(darktable.control->global_mutex));
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "update settings set settings = ?1 where rowid = 1", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_BLOB(stmt, 1, &(darktable.control->global_settings), sizeof(dt_ctl_settings_t), SQLITE_STATIC);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  dt_pthread_mutex_unlock(&(darktable.control->global_mutex));
  return 0;
}

// Get the display ICC profile of the monitor associated with the widget.
// For X display, uses the ICC profile specifications version 0.2 from
// http://burtonini.com/blog/computers/xicc
// Based on code from Gimp's modules/cdisplay_lcms.c
#ifdef GDK_WINDOWING_QUARTZ
typedef struct
{
  guchar *data;
  gsize   len;
}
ProfileTransfer;

enum
{
  openReadSpool  = 1, /* start read data process         */
  openWriteSpool = 2, /* start write data process        */
  readSpool      = 3, /* read specified number of bytes  */
  writeSpool     = 4, /* write specified number of bytes */
  closeSpool     = 5  /* complete data transfer process  */
};

#ifndef __LP64__
static OSErr dt_ctl_lcms_flatten_profile(SInt32  command,
    SInt32 *size, void *data, void *refCon)
{
  // ProfileTransfer *transfer = static_cast<ProfileTransfer*>(refCon);
  ProfileTransfer *transfer = (ProfileTransfer *)refCon;

  switch (command)
  {
    case openWriteSpool:
      g_return_val_if_fail(transfer->data==NULL && transfer->len==0, -1);
      break;

    case writeSpool:
      transfer->data = (guchar *)
                       g_realloc(transfer->data, transfer->len + *size);
      memcpy(transfer->data + transfer->len, data, *size);
      transfer->len += *size;
      break;

    default:
      break;
  }
  return 0;
}
#endif /* __LP64__ */
#endif /* GDK_WINDOWING_QUARTZ */

void dt_ctl_get_display_profile(GtkWidget *widget,
                                guint8 **buffer, gint *buffer_size)
{
  // thanks to ufraw for this!
  *buffer = NULL;
  *buffer_size = 0;
#if defined GDK_WINDOWING_X11
  GdkScreen *screen = gtk_widget_get_screen(widget);
  if ( screen==NULL )
    screen = gdk_screen_get_default();
  int monitor = gdk_screen_get_monitor_at_window (screen, widget->window);
  char *atom_name;
  if (monitor > 0)
    atom_name = g_strdup_printf("_ICC_PROFILE_%d", monitor);
  else
    atom_name = g_strdup("_ICC_PROFILE");

  GdkAtom type = GDK_NONE;
  gint format = 0;
  gdk_property_get(gdk_screen_get_root_window(screen),
                   gdk_atom_intern(atom_name, FALSE), GDK_NONE,
                   0, 64 * 1024 * 1024, FALSE,
                   &type, &format, buffer_size, buffer);
  g_free(atom_name);

#elif defined GDK_WINDOWING_QUARTZ
  GdkScreen *screen = gtk_widget_get_screen(widget);
  if ( screen==NULL )
    screen = gdk_screen_get_default();
  int monitor = gdk_screen_get_monitor_at_window(screen, widget->window);

  CMProfileRef prof = NULL;
  CMGetProfileByAVID(monitor, &prof);
  if ( prof==NULL )
    return;

  ProfileTransfer transfer = { NULL, 0 };
  //The following code does not work on 64bit OSX.  Disable if we are compiling there.
#ifndef __LP64__
  Boolean foo;
  CMFlattenProfile(prof, 0, dt_ctl_lcms_flatten_profile, &transfer, &foo);
  CMCloseProfile(prof);
#endif
  *buffer = transfer.data;
  *buffer_size = transfer.len;

#elif defined G_OS_WIN32
  (void)widget;
  HDC hdc = GetDC (NULL);
  if ( hdc==NULL )
    return;

  DWORD len = 0;
  GetICMProfile (hdc, &len, NULL);
  gchar *path = g_new (gchar, len);

  if (GetICMProfile (hdc, &len, path))
  {
    gsize size;
    g_file_get_contents(path, (gchar**)buffer, &size, NULL);
    *buffer_size = size;
  }
  g_free (path);
  ReleaseDC (NULL, hdc);
#endif
}

void dt_control_create_database_schema()
{
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table settings (settings blob)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table film_rolls (id integer primary key, datetime_accessed char(20), folder varchar(1024))", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table images (id integer primary key, film_id integer, width int, height int, filename varchar, maker varchar, model varchar, lens varchar, exposure real, aperture real, iso real, focal_length real, focus_distance real, datetime_taken char(20), flags integer, output_width integer, output_height integer, crop real, raw_parameters integer, raw_denoise_threshold real, raw_auto_bright_threshold real, raw_black real, raw_maximum real, caption varchar, description varchar, license varchar, sha1sum char(40), orientation integer)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table selected_images (imgid integer)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table history (imgid integer, num integer, module integer, operation varchar(256), op_params blob, enabled integer,blendop_params blob)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table tags (id integer primary key, name varchar, icon blob, description varchar, flags integer)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table tagxtag (id1 integer, id2 integer, count integer, primary key(id1, id2))", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table tagged_images (imgid integer, tagid integer, primary key(imgid, tagid))", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table styles (name varchar,description varchar)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table style_items (styleid integer,num integer,module integer,operation varchar(256),op_params blob,enabled integer,blendop_params blob)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table color_labels (imgid integer, color integer)", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "create table meta_data (id integer,key integer,value varchar)", NULL, NULL, NULL);
}

void dt_control_init(dt_control_t *s)
{
  dt_ctl_settings_init(s);

  // s->last_expose_time = dt_get_wtime();
  s->key_accelerators_on = 1;
  s->log_pos = s->log_ack = 0;
  s->log_busy = 0;
  s->log_message_timeout_id = 0;
  dt_pthread_mutex_init(&(s->log_mutex), NULL);
  s->progress = 200.0f;

  dt_conf_set_int("ui_last/view", DT_MODE_NONE);

  // if config is old, replace with new defaults.
  if(DT_CONFIG_VERSION > dt_conf_get_int("config_version"))
    dt_ctl_settings_default(s);

  pthread_cond_init(&s->cond, NULL);
  dt_pthread_mutex_init(&s->cond_mutex, NULL);
  dt_pthread_mutex_init(&s->queue_mutex, NULL);
  dt_pthread_mutex_init(&s->run_mutex, NULL);

  int k;
  for(k=0; k<DT_CONTROL_MAX_JOBS; k++) s->idle[k] = k;
  s->idle_top = DT_CONTROL_MAX_JOBS;
  s->queued_top = 0;
  // start threads
  s->num_threads = CLAMP(dt_conf_get_int ("worker_threads"), 1, 8);
  s->thread = (pthread_t *)malloc(sizeof(pthread_t)*s->num_threads);
  dt_pthread_mutex_lock(&s->run_mutex);
  s->running = 1;
  dt_pthread_mutex_unlock(&s->run_mutex);
  for(k=0; k<s->num_threads; k++)
    pthread_create(&s->thread[k], NULL, dt_control_work, s);
  for(k=0; k<DT_CTL_WORKER_RESERVED; k++)
  {
    s->new_res[k] = 0;
    pthread_create(&s->thread_res[k], NULL, dt_control_work_res, s);
  }
  s->button_down = 0;
  s->button_down_which = 0;

  // init database schema:
  int rc;
  sqlite3_stmt *stmt;
  // unsafe, fast write:
  // DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "PRAGMA synchronous=off", NULL, NULL, NULL);
  // free memory on disk if we call the line below:
  // DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "PRAGMA auto_vacuum=INCREMENTAL", NULL, NULL, NULL);
  // DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "PRAGMA incremental_vacuum(0)", NULL, NULL, NULL);
  rc = sqlite3_prepare_v2(dt_database_get(darktable.db), "select settings from settings", -1, &stmt, NULL);
  if(rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
  {
#if 1 // global settings not needed anymore
    dt_pthread_mutex_lock(&(darktable.control->global_mutex));
    darktable.control->global_settings.version = -1;
    const void *set = sqlite3_column_blob(stmt, 0);
    int len = sqlite3_column_bytes(stmt, 0);
    if(len == sizeof(dt_ctl_settings_t)) memcpy(&(darktable.control->global_settings), set, len);
#endif
    sqlite3_finalize(stmt);

#if 1
    if(darktable.control->global_settings.version != DT_VERSION)
    {
      fprintf(stderr, "[load_config] wrong version %d (should be %d), substituting defaults.\n", darktable.control->global_settings.version, DT_VERSION);
      memcpy(&(darktable.control->global_settings), &(darktable.control->global_defaults), sizeof(dt_ctl_settings_t));
      dt_pthread_mutex_unlock(&(darktable.control->global_mutex));
      // drop all, restart. TODO: freeze this version or have update facility!
      sqlite3_exec(dt_database_get(darktable.db), "drop table settings", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table film_rolls", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table images", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table selected_images", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table mipmaps", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table mipmap_timestamps", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table history", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table tags", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table tagxtag", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table tagged_images", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table styles", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table style_items", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table meta_data", NULL, NULL, NULL);
      goto create_tables;
    }
    else
    {
      // silly check if old table is still present:
      sqlite3_exec(dt_database_get(darktable.db), "delete from color_labels where imgid=0", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "insert into color_labels values (0, 0)", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "insert into color_labels values (0, 1)", NULL, NULL, NULL);
      DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select max(color) from color_labels where imgid=0", -1, &stmt, NULL);
      int col = 0;
      // still the primary key option set?
      if(sqlite3_step(stmt) == SQLITE_ROW) col = MAX(col, sqlite3_column_int(stmt, 0));
      sqlite3_finalize(stmt);
      if(col != 1) sqlite3_exec(dt_database_get(darktable.db), "drop table color_labels", NULL, NULL, NULL);

      // insert new tables, if not there (statement will just fail if so):
      sqlite3_exec(dt_database_get(darktable.db), "create table color_labels (imgid integer, color integer)", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table mipmaps", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "drop table mipmap_timestamps", NULL, NULL, NULL);

      sqlite3_exec(dt_database_get(darktable.db), "create table styles (name varchar,description varchar)", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "create table style_items (styleid integer,num integer,module integer,operation varchar(256),op_params blob,enabled integer)", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "create table meta_data (id integer,key integer,value varchar)", NULL, NULL, NULL);

      // add columns where needed. will just fail otherwise:
      sqlite3_exec(dt_database_get(darktable.db), "alter table images add column orientation integer", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "update images set orientation = -1 where orientation is NULL", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "alter table images add column focus_distance real", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "update images set focus_distance = -1 where focus_distance is NULL", NULL, NULL, NULL);

      // add column for blendops
      sqlite3_exec(dt_database_get(darktable.db), "alter table history add column blendop_params blob", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "alter table style_items add column blendop_params blob", NULL, NULL, NULL);
      sqlite3_exec(dt_database_get(darktable.db), "alter table presets add column blendop_params blob", NULL, NULL, NULL);

      dt_pthread_mutex_unlock(&(darktable.control->global_mutex));
    }
#endif
  }
  else
  {
    // db not yet there, create it
    sqlite3_finalize(stmt);
create_tables:
    dt_control_create_database_schema();
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "insert into settings (settings) values (?1)", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_BLOB(stmt, 1, &(darktable.control->global_defaults), sizeof(dt_ctl_settings_t), SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }

  dt_control_sanitize_database();
}

void dt_control_key_accelerators_on(struct dt_control_t *s)
{
  gtk_window_add_accel_group(GTK_WINDOW(dt_ui_main_window(darktable.gui->ui)),
                             darktable.control->accelerators);
  if(!s->key_accelerators_on)
    s->key_accelerators_on = 1;
}

void dt_control_key_accelerators_off(struct dt_control_t *s)
{
  gtk_window_remove_accel_group(
      GTK_WINDOW(dt_ui_main_window(darktable.gui->ui)),
      darktable.control->accelerators);
  s->key_accelerators_on = 0;
}


int dt_control_is_key_accelerators_on(struct dt_control_t *s)
{
  return  s->key_accelerators_on;
}

void dt_control_change_cursor(dt_cursor_t curs)
{
  GtkWidget *widget = dt_ui_main_window(darktable.gui->ui);
  GdkCursor* cursor = gdk_cursor_new(curs);
  gdk_window_set_cursor(widget->window, cursor);
  gdk_cursor_destroy(cursor);
}

int dt_control_running()
{
  // FIXME: when shutdown, run_mutex is not inited anymore!
  dt_control_t *s = darktable.control;
  dt_pthread_mutex_lock(&s->run_mutex);
  int running = s->running;
  dt_pthread_mutex_unlock(&s->run_mutex);
  return running;
}

void dt_control_shutdown(dt_control_t *s)
{
  dt_pthread_mutex_lock(&s->cond_mutex);
  dt_pthread_mutex_lock(&s->run_mutex);
  s->running = 0;
  dt_pthread_mutex_unlock(&s->run_mutex);
  dt_pthread_mutex_unlock(&s->cond_mutex);
  pthread_cond_broadcast(&s->cond);

  // gdk_threads_leave();
  int k;
  for(k=0; k<s->num_threads; k++)
    // pthread_kill(s->thread[k], 9);
    pthread_join(s->thread[k], NULL);
  for(k=0; k<DT_CTL_WORKER_RESERVED; k++)
    // pthread_kill(s->thread_res[k], 9);
    pthread_join(s->thread_res[k], NULL);
  // gdk_threads_enter();
}

void dt_control_cleanup(dt_control_t *s)
{
  // vacuum TODO: optional?
  // DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "PRAGMA incremental_vacuum(0)", NULL, NULL, NULL);
  // DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "vacuum", NULL, NULL, NULL);
  dt_pthread_mutex_destroy(&s->queue_mutex);
  dt_pthread_mutex_destroy(&s->cond_mutex);
  dt_pthread_mutex_destroy(&s->log_mutex);
  dt_pthread_mutex_destroy(&s->run_mutex);
}

void dt_control_job_init(dt_job_t *j, const char *msg, ...)
{
  memset(j, 0, sizeof(dt_job_t));
#ifdef DT_CONTROL_JOB_DEBUG
  va_list ap;
  va_start(ap, msg);
  vsnprintf(j->description, DT_CONTROL_DESCRIPTION_LEN, msg, ap);
  va_end(ap);
#endif
  j->state = DT_JOB_STATE_INITIALIZED;
  dt_pthread_mutex_init (&j->state_mutex,NULL);
  dt_pthread_mutex_init (&j->wait_mutex,NULL);
}

void dt_control_job_set_state_callback(dt_job_t *j,dt_job_state_change_callback cb,void *user_data)
{
  j->state_changed_cb = cb;
  j->user_data = user_data;
}


void dt_control_job_print(dt_job_t *j)
{
#ifdef DT_CONTROL_JOB_DEBUG
  dt_print(DT_DEBUG_CONTROL, "%s", j->description);
#endif
}

void _control_job_set_state(dt_job_t *j,int state)
{
  dt_pthread_mutex_lock (&j->state_mutex);
  j->state = state;
  /* pass state change to callback */
  if (j->state_changed_cb)
    j->state_changed_cb (j,state);
  dt_pthread_mutex_unlock (&j->state_mutex);

}

int dt_control_job_get_state(dt_job_t *j)
{
  dt_pthread_mutex_lock (&j->state_mutex);
  int state = j->state;
  dt_pthread_mutex_unlock (&j->state_mutex);
  return state;
}

void dt_control_job_cancel(dt_job_t *j)
{
  _control_job_set_state (j,DT_JOB_STATE_CANCELLED);
}

void dt_control_job_wait(dt_job_t *j)
{
  int state = dt_control_job_get_state (j);

  /* if job execution not is finished let's wait for signal */
  if (state==DT_JOB_STATE_RUNNING || state==DT_JOB_STATE_CANCELLED)
  {
    dt_pthread_mutex_lock (&j->wait_mutex);
    dt_pthread_mutex_unlock (&j->wait_mutex);
  }

}

int32_t dt_control_run_job_res(dt_control_t *s, int32_t res)
{
  assert(res < DT_CTL_WORKER_RESERVED && res >= 0);
  dt_job_t *j = NULL;
  dt_pthread_mutex_lock(&s->queue_mutex);
  if(s->new_res[res]) j = s->job_res + res;
  s->new_res[res] = 0;
  dt_pthread_mutex_unlock(&s->queue_mutex);
  if(!j) return -1;

  /* change state to running */
  dt_pthread_mutex_lock (&j->wait_mutex);
  if (dt_control_job_get_state (j) == DT_JOB_STATE_QUEUED)
  {
    dt_print(DT_DEBUG_CONTROL, "[run_job+] %02d %f ", res, dt_get_wtime());
    dt_control_job_print(j);
    dt_print(DT_DEBUG_CONTROL, "\n");

    _control_job_set_state (j,DT_JOB_STATE_RUNNING);

    /* execute job */
    j->result = j->execute (j);

    _control_job_set_state (j,DT_JOB_STATE_FINISHED);
    dt_print(DT_DEBUG_CONTROL, "[run_job-] %02d %f ", res, dt_get_wtime());
    dt_control_job_print(j);
    dt_print(DT_DEBUG_CONTROL, "\n");

  }
  dt_pthread_mutex_unlock (&j->wait_mutex);
  return 0;
}

int32_t dt_control_run_job(dt_control_t *s)
{
  dt_job_t *j;
  int32_t i;
  dt_pthread_mutex_lock(&s->queue_mutex);
  // dt_print(DT_DEBUG_CONTROL, "[run_job] %d\n", s->queued_top);
  if(s->queued_top == 0)
  {
    dt_pthread_mutex_unlock(&s->queue_mutex);
    return -1;
  }
  i = s->queued[--s->queued_top];
  j = s->job + i;
  dt_pthread_mutex_unlock(&s->queue_mutex);

  /* change state to running */
  dt_pthread_mutex_lock (&j->wait_mutex);
  if (dt_control_job_get_state (j) == DT_JOB_STATE_QUEUED)
  {
    dt_print(DT_DEBUG_CONTROL, "[run_job+] %02d %f ", DT_CTL_WORKER_RESERVED+dt_control_get_threadid(), dt_get_wtime());
    dt_control_job_print(j);
    dt_print(DT_DEBUG_CONTROL, "\n");

    _control_job_set_state (j,DT_JOB_STATE_RUNNING);

    /* execute job */
    j->result = j->execute (j);

    _control_job_set_state (j,DT_JOB_STATE_FINISHED);

    dt_print(DT_DEBUG_CONTROL, "[run_job-] %02d %f ", DT_CTL_WORKER_RESERVED+dt_control_get_threadid(), dt_get_wtime());
    dt_control_job_print(j);
    dt_print(DT_DEBUG_CONTROL, "\n");
  }
  dt_pthread_mutex_unlock (&j->wait_mutex);

  dt_pthread_mutex_lock(&s->queue_mutex);
  assert(s->idle_top < DT_CONTROL_MAX_JOBS);
  s->idle[s->idle_top++] = i;
  dt_pthread_mutex_unlock(&s->queue_mutex);
  return 0;
}

int32_t dt_control_add_job_res(dt_control_t *s, dt_job_t *job, int32_t res)
{
  // TODO: pthread cancel and restart in tough cases?
  dt_pthread_mutex_lock(&s->queue_mutex);
  dt_print(DT_DEBUG_CONTROL, "[add_job_res] %d ", res);
  dt_control_job_print(job);
  dt_print(DT_DEBUG_CONTROL, "\n");
  _control_job_set_state (job,DT_JOB_STATE_QUEUED);
  s->job_res[res] = *job;
  s->new_res[res] = 1;
  dt_pthread_mutex_unlock(&s->queue_mutex);
  dt_pthread_mutex_lock(&s->cond_mutex);
  pthread_cond_broadcast(&s->cond);
  dt_pthread_mutex_unlock(&s->cond_mutex);
  return 0;
}

int32_t dt_control_add_job(dt_control_t *s, dt_job_t *job)
{
  int32_t i;
  dt_pthread_mutex_lock(&s->queue_mutex);
  for(i=0; i<s->queued_top; i++)
  {
    // find equivalent job and quit if already there
    const int j = s->queued[i];
    if(!memcmp(job, s->job + j, sizeof(dt_job_t)))
    {
      dt_print(DT_DEBUG_CONTROL, "[add_job] found job already in queue\n");
      dt_pthread_mutex_unlock(&s->queue_mutex);
      return -1;
    }
  }
  dt_print(DT_DEBUG_CONTROL, "[add_job] %d ", s->idle_top);
  dt_control_job_print(job);
  dt_print(DT_DEBUG_CONTROL, "\n");
  if(s->idle_top != 0)
  {
    i = --s->idle_top;
    _control_job_set_state (job,DT_JOB_STATE_QUEUED);
    s->job[s->idle[i]] = *job;
    s->queued[s->queued_top++] = s->idle[i];
    dt_pthread_mutex_unlock(&s->queue_mutex);
  }
  else
  {
    dt_print(DT_DEBUG_CONTROL, "[add_job] too many jobs in queue!\n");
    _control_job_set_state (job,DT_JOB_STATE_DISCARDED);
    dt_pthread_mutex_unlock(&s->queue_mutex);
    return -1;
  }

  // notify workers
  dt_pthread_mutex_lock(&s->cond_mutex);
  pthread_cond_broadcast(&s->cond);
  dt_pthread_mutex_unlock(&s->cond_mutex);
  return 0;
}

int32_t dt_control_revive_job(dt_control_t *s, dt_job_t *job)
{
  int32_t found_j = -1;
  dt_pthread_mutex_lock(&s->queue_mutex);
  dt_print(DT_DEBUG_CONTROL, "[revive_job] ");
  dt_control_job_print(job);
  dt_print(DT_DEBUG_CONTROL, "\n");
  for(int i=0; i<s->queued_top; i++)
  {
    // find equivalent job and push it up on top of the stack.
    const int j = s->queued[i];
    if(!memcmp(job, s->job + j, sizeof(dt_job_t)))
    {
      found_j = j;
      dt_print(DT_DEBUG_CONTROL, "[revive_job] found job in queue at position %d, moving to %d\n", i, s->queued_top);
      memmove(s->queued + i, s->queued + i + 1, sizeof(int32_t) * (s->queued_top - i - 1));
      s->queued[s->queued_top-1] = j;
    }
  }
  dt_pthread_mutex_unlock(&s->queue_mutex);

  // notify workers
  dt_pthread_mutex_lock(&s->cond_mutex);
  pthread_cond_broadcast(&s->cond);
  dt_pthread_mutex_unlock(&s->cond_mutex);
  return found_j;
}

int32_t dt_control_get_threadid()
{
  int32_t threadid = 0;
  while( !pthread_equal(darktable.control->thread[threadid],pthread_self()) && threadid < darktable.control->num_threads-1)
    threadid++;
  assert(pthread_equal(darktable.control->thread[threadid],pthread_self()));
  return threadid;
}

int32_t dt_control_get_threadid_res()
{
  int32_t threadid = 0;
  while(!pthread_equal(darktable.control->thread_res[threadid],pthread_self()) && threadid < DT_CTL_WORKER_RESERVED-1)
    threadid++;
  assert(pthread_equal(darktable.control->thread_res[threadid], pthread_self()));
  return threadid;
}

void *dt_control_work_res(void *ptr)
{
#ifdef _OPENMP // need to do this in every thread
  omp_set_num_threads(darktable.num_openmp_threads);
#endif
  dt_control_t *s = (dt_control_t *)ptr;
  int32_t threadid = dt_control_get_threadid_res();
  while(dt_control_running())
  {
    // dt_print(DT_DEBUG_CONTROL, "[control_work] %d\n", threadid);
    if(dt_control_run_job_res(s, threadid) < 0)
    {
      // wait for a new job.
      int old;
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old);
      dt_pthread_mutex_lock(&s->cond_mutex);
      dt_pthread_cond_wait(&s->cond, &s->cond_mutex);
      dt_pthread_mutex_unlock(&s->cond_mutex);
      pthread_setcancelstate(old, NULL);
    }
  }
  return NULL;
}

void *dt_control_work(void *ptr)
{
#ifdef _OPENMP // need to do this in every thread
  omp_set_num_threads(darktable.num_openmp_threads);
#endif
  dt_control_t *s = (dt_control_t *)ptr;
  // int32_t threadid = dt_control_get_threadid();
  while(dt_control_running())
  {
    // dt_print(DT_DEBUG_CONTROL, "[control_work] %d\n", threadid);
    if(dt_control_run_job(s) < 0)
    {
      // wait for a new job.
      dt_pthread_mutex_lock(&s->cond_mutex);
      dt_pthread_cond_wait(&s->cond, &s->cond_mutex);
      dt_pthread_mutex_unlock(&s->cond_mutex);
    }
  }
  return NULL;
}


// ================================================================================
//  gui functions:
// ================================================================================

gboolean dt_control_configure(GtkWidget *da, GdkEventConfigure *event, gpointer user_data)
{
  darktable.control->tabborder = 8;
  int tb = darktable.control->tabborder;
  // re-configure all components:
  dt_view_manager_configure(darktable.view_manager, event->width - 2*tb, event->height - 2*tb);
  return TRUE;
}

void *dt_control_expose(void *voidptr)
{
  int width, height, pointerx, pointery;
  if(!darktable.gui->pixmap) return NULL;
  gdk_drawable_get_size(darktable.gui->pixmap, &width, &height);
  GtkWidget *widget = dt_ui_center(darktable.gui->ui);
  gtk_widget_get_pointer(widget, &pointerx, &pointery);

  //create a gtk-independant surface to draw on
  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(cst);

  // TODO: control_expose: only redraw the part not overlapped by temporary control panel show!
  //
  float tb = 8;//fmaxf(10, width/100.0);
  darktable.control->tabborder = tb;
  darktable.control->width = width;
  darktable.control->height = height;

  GtkStyle *style = gtk_widget_get_style(widget);
  cairo_set_source_rgb (cr,
                        style->bg[GTK_STATE_NORMAL].red/65535.0,
                        style->bg[GTK_STATE_NORMAL].green/65535.0,
                        style->bg[GTK_STATE_NORMAL].blue/65535.0
                       );

  cairo_set_line_width(cr, tb);
  cairo_rectangle(cr, tb/2., tb/2., width-tb, height-tb);
  cairo_stroke(cr);
  cairo_set_line_width(cr, 1.5);
  cairo_set_source_rgb (cr, .1, .1, .1);
  cairo_rectangle(cr, tb, tb, width-2*tb, height-2*tb);
  cairo_stroke(cr);

  cairo_save(cr);
  cairo_translate(cr, tb, tb);
  cairo_rectangle(cr, 0, 0, width - 2*tb, height - 2*tb);
  cairo_clip(cr);
  cairo_new_path(cr);
  // draw view
  dt_view_manager_expose(darktable.view_manager, cr, width-2*tb, height-2*tb, pointerx-tb, pointery-tb);
  cairo_restore(cr);

  // draw status bar, if any
  if(darktable.control->progress < 100.0)
  {
    tb = fmaxf(20, width/40.0);
    char num[10];
    cairo_rectangle(cr, width*0.4, height*0.85, width*0.2*darktable.control->progress/100.0f, tb);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0., 0., 0.);
    cairo_rectangle(cr, width*0.4, height*0.85, width*0.2, tb);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, tb/3);
    cairo_move_to (cr, width/2.0-10, height*0.85+2.*tb/3.);
    snprintf(num, 10, "%d%%", (int)darktable.control->progress);
    cairo_show_text (cr, num);
  }
  // draw log message, if any
  dt_pthread_mutex_lock(&darktable.control->log_mutex);
  if(darktable.control->log_ack != darktable.control->log_pos)
  {
    cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    const float fontsize = 14;
    cairo_set_font_size (cr, fontsize);
    cairo_text_extents_t ext;
    cairo_text_extents (cr, darktable.control->log_message[darktable.control->log_ack], &ext);
    const float pad = 20.0f, xc = width/2.0, yc = height*0.85+10, wd = pad + ext.width*.5f;
    float rad = 14;
    cairo_set_line_width(cr, 1.);
    cairo_move_to( cr, xc-wd,yc+rad);
    for(int k=0; k<5; k++)
    {
      cairo_arc (cr, xc-wd, yc, rad, M_PI/2.0, 3.0/2.0*M_PI);
      cairo_line_to (cr, xc+wd, yc-rad);
      cairo_arc (cr, xc+wd, yc, rad, 3.0*M_PI/2.0, M_PI/2.0);
      cairo_line_to (cr, xc-wd, yc+rad);
      if(k == 0)
      {
        cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
        cairo_fill_preserve (cr);
      }
      cairo_set_source_rgba(cr, 0., 0., 0., 1.0/(1+k));
      cairo_stroke (cr);
      rad += .5f;
    }
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_move_to (cr, xc-wd+.5f*pad, yc + 1./3.*fontsize);
    cairo_show_text (cr, darktable.control->log_message[darktable.control->log_ack]);
  }
  // draw busy indicator
  if(darktable.control->log_busy > 0)
  {
    cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    const float fontsize = 14;
    cairo_set_font_size (cr, fontsize);
    cairo_text_extents_t ext;
    cairo_text_extents (cr, _("working.."), &ext);
    const float xc = width/2.0, yc = height*0.85-30, wd = ext.width*.5f;
    cairo_move_to (cr, xc-wd, yc + 1./3.*fontsize);
    cairo_text_path (cr, _("working.."));
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, 0.7);
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_stroke(cr);
  }
  dt_pthread_mutex_unlock(&darktable.control->log_mutex);

  cairo_destroy(cr);

  cairo_t *cr_pixmap = gdk_cairo_create(darktable.gui->pixmap);
  cairo_set_source_surface (cr_pixmap, cst, 0, 0);
  cairo_paint(cr_pixmap);
  cairo_destroy(cr_pixmap);

  cairo_surface_destroy(cst);
  return NULL;
}

gboolean dt_control_expose_endmarker(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  const int width = widget->allocation.width, height = widget->allocation.height;
  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(cst);
  dt_draw_endmarker(cr, width, height, (long int)user_data);
  cairo_destroy(cr);
  cairo_t *cr_pixmap = gdk_cairo_create(gtk_widget_get_window(widget));
  cairo_set_source_surface (cr_pixmap, cst, 0, 0);
  cairo_paint(cr_pixmap);
  cairo_destroy(cr_pixmap);
  cairo_surface_destroy(cst);
  return TRUE;
}

void dt_control_mouse_leave()
{
  dt_view_manager_mouse_leave(darktable.view_manager);
}

void dt_control_mouse_enter()
{
  dt_view_manager_mouse_enter(darktable.view_manager);
}

void dt_control_mouse_moved(double x, double y, int which)
{
  float tb = darktable.control->tabborder;
  float wd = darktable.control->width;
  float ht = darktable.control->height;

  if(x > tb && x < wd-tb && y > tb && y < ht-tb)
    dt_view_manager_mouse_moved(darktable.view_manager, x-tb, y-tb, which);
}

void dt_control_button_released(double x, double y, int which, uint32_t state)
{
  darktable.control->button_down = 0;
  darktable.control->button_down_which = 0;
  float tb = darktable.control->tabborder;
  // float wd = darktable.control->width;
  // float ht = darktable.control->height;

  // always do this, to avoid missing some events.
  // if(x > tb && x < wd-tb && y > tb && y < ht-tb)
  dt_view_manager_button_released(darktable.view_manager, x-tb, y-tb, which, state);
}

void dt_ctl_switch_mode_to(dt_ctl_gui_mode_t mode)
{
  dt_ctl_gui_mode_t oldmode = dt_conf_get_int("ui_last/view");
  if(oldmode == mode) return;

  darktable.control->button_down = 0;
  darktable.control->button_down_which = 0;
  darktable.gui->center_tooltip = 0;
  GtkWidget *widget = dt_ui_center(darktable.gui->ui);
  g_object_set(G_OBJECT(widget), "tooltip-text", "", (char *)NULL);

  char buf[512];
  snprintf(buf, 512, _("switch to %s mode"), dt_view_manager_name(darktable.view_manager));

  gboolean i_own_lock = dt_control_gdk_lock(); 

  int error = dt_view_manager_switch(darktable.view_manager, mode);

  if(i_own_lock) dt_control_gdk_unlock();

  if(error) return;

  dt_conf_set_int ("ui_last/view", mode);
}

void dt_ctl_switch_mode()
{
  dt_ctl_gui_mode_t mode = dt_conf_get_int("ui_last/view");
  if(mode == DT_LIBRARY) mode = DT_DEVELOP;
  else mode = DT_LIBRARY;
  dt_ctl_switch_mode_to(mode);
}

static gboolean _dt_ctl_log_message_timeout_callback (gpointer data)
{
  dt_pthread_mutex_lock(&darktable.control->log_mutex);
  if(darktable.control->log_ack != darktable.control->log_pos)
    darktable.control->log_ack = (darktable.control->log_ack+1)%DT_CTL_LOG_SIZE;
  darktable.control->log_message_timeout_id=0;
  dt_pthread_mutex_unlock(&darktable.control->log_mutex);
  dt_control_queue_redraw_center();
  return FALSE;
}


void dt_control_button_pressed(double x, double y, int which, int type, uint32_t state)
{
  float tb = darktable.control->tabborder;
  darktable.control->button_down = 1;
  darktable.control->button_down_which = which;
  darktable.control->button_type = type;
  darktable.control->button_x = x - tb;
  darktable.control->button_y = y - tb;
  float wd = darktable.control->width;
  float ht = darktable.control->height;

  // ack log message:
  dt_pthread_mutex_lock(&darktable.control->log_mutex);
  const float /*xc = wd/4.0-20,*/ yc = ht*0.85+10;
  if(darktable.control->log_ack != darktable.control->log_pos)
    if(which == 1 /*&& x > xc - 10 && x < xc + 10*/ && y > yc - 10 && y < yc + 10)
    {
      if(darktable.control->log_message_timeout_id) g_source_remove(darktable.control->log_message_timeout_id);
      darktable.control->log_ack = (darktable.control->log_ack+1)%DT_CTL_LOG_SIZE;
      dt_pthread_mutex_unlock(&darktable.control->log_mutex);
      return;
    }
  dt_pthread_mutex_unlock(&darktable.control->log_mutex);

  if(x > tb && x < wd-tb && y > tb && y < ht-tb)
  {
    if(!dt_view_manager_button_pressed(darktable.view_manager, x-tb, y-tb, which, type, state))
      if(type == GDK_2BUTTON_PRESS && which == 1) dt_ctl_switch_mode();
  }
}

void dt_control_log(const char* msg, ...)
{
  dt_pthread_mutex_lock(&darktable.control->log_mutex);
  va_list ap;
  va_start(ap, msg);
  vsnprintf(darktable.control->log_message[darktable.control->log_pos], DT_CTL_LOG_MSG_SIZE, msg, ap);
  va_end(ap);
  if(darktable.control->log_message_timeout_id) g_source_remove(darktable.control->log_message_timeout_id);
  darktable.control->log_ack = darktable.control->log_pos;
  darktable.control->log_pos = (darktable.control->log_pos+1)%DT_CTL_LOG_SIZE;
  darktable.control->log_message_timeout_id=g_timeout_add(DT_CTL_LOG_TIMEOUT,_dt_ctl_log_message_timeout_callback,NULL);
  dt_pthread_mutex_unlock(&darktable.control->log_mutex);
  dt_control_queue_redraw_center();
}

void dt_control_log_busy_enter()
{
  dt_pthread_mutex_lock(&darktable.control->log_mutex);
  darktable.control->log_busy++;
  dt_pthread_mutex_unlock(&darktable.control->log_mutex);
}

void dt_control_log_busy_leave()
{
  dt_pthread_mutex_lock(&darktable.control->log_mutex);
  darktable.control->log_busy--;
  dt_pthread_mutex_unlock(&darktable.control->log_mutex);
  /* lets redraw */
  dt_control_queue_redraw_center();
}

static GList *_control_gdk_lock_threads = NULL;
static GStaticMutex _control_gdk_lock_threads_mutex = G_STATIC_MUTEX_INIT;
gboolean dt_control_gdk_lock()
{
  /* if current thread equals gui thread do nothing */
  if(pthread_equal(darktable.control->gui_thread,pthread_self()) != 0) return FALSE;

  /* if we dont have any managed locks just lock and return */
  g_static_mutex_lock(&_control_gdk_lock_threads_mutex);
  if(!_control_gdk_lock_threads)
     goto lock_and_return;
 
  /* lets check if current thread has a managed lock */
  if(g_list_find(_control_gdk_lock_threads, (gpointer)pthread_self()))
  {
    /* current thread has a lock just do nothing */
    g_static_mutex_unlock(&_control_gdk_lock_threads_mutex);
    return FALSE;
  }

lock_and_return:
  /* lets add current thread to managed locks */
  _control_gdk_lock_threads = g_list_append(_control_gdk_lock_threads, (gpointer)pthread_self());
  g_static_mutex_unlock(&_control_gdk_lock_threads_mutex);

  /* enter gdk critical section */
  gdk_threads_enter();

  return TRUE;
}

void dt_control_gdk_unlock()
{
  /* check if current thread has a lock and remove if exists */
  GList *item=NULL;
  g_static_mutex_lock(&_control_gdk_lock_threads_mutex);
  if((item=g_list_find(_control_gdk_lock_threads, (gpointer)pthread_self()))) 
  {
    /* remove lock */
    _control_gdk_lock_threads = g_list_remove(_control_gdk_lock_threads,(gpointer)pthread_self());  
    
    /* leave critical section */
    gdk_threads_leave();
  }
  g_static_mutex_unlock(&_control_gdk_lock_threads_mutex);
}

/* redraw mutex to synchronize redraws */
G_LOCK_DEFINE(counter);
GStaticMutex _control_redraw_mutex = G_STATIC_MUTEX_INIT;

void _control_queue_redraw_wrapper(dt_signal_t signal)
{
  static uint32_t counter = 0;

  /* dont continue if control is not running */
  if (!dt_control_running())
    return;

  /* if we cant carry out an redraw, lets increment counter and bail out */
  if (!g_static_mutex_trylock(&_control_redraw_mutex))
  {
    G_LOCK(counter);
    counter++;
    G_UNLOCK(counter);
    return;
  }

  /* lock the gdk thread and carry out the redraw function */
  gboolean i_own_lock = dt_control_gdk_lock();
  dt_control_signal_raise(darktable.signals, signal);

  /* lets check if we got missing redraws from other threads */
  G_LOCK(counter);
  if (counter)
  {
    /* carry out an redraw due to missed redraws */
    counter = 0;
    G_UNLOCK(counter);
    dt_control_signal_raise(darktable.signals, DT_SIGNAL_CONTROL_REDRAW_ALL);
  }
  else
    G_UNLOCK(counter);

  /* unlock our locks */
  if (i_own_lock) 
    dt_control_gdk_unlock();
      
  g_static_mutex_unlock(&_control_redraw_mutex);

}

void dt_control_queue_redraw()
{
  _control_queue_redraw_wrapper(DT_SIGNAL_CONTROL_REDRAW_ALL);
}

void dt_control_queue_redraw_center() 
{  
  _control_queue_redraw_wrapper(DT_SIGNAL_CONTROL_REDRAW_CENTER);
}

void dt_control_queue_redraw_widget(GtkWidget *widget)
{
  if(dt_control_running())
  {
    gboolean i_own_lock = dt_control_gdk_lock();

    gtk_widget_queue_draw(widget);

    if (i_own_lock) dt_control_gdk_unlock();
  }
}


int dt_control_key_pressed_override(guint key, guint state)
{
  dt_control_accels_t* accels = &darktable.control->accels;

  /* check if key accelerators are enabled*/
  if (darktable.control->key_accelerators_on != 1) return 0;

  if(key == accels->global_sideborders.accel_key
     && state == accels->global_sideborders.accel_mods)
  {
    /* toggle panel viewstate */
    dt_ui_toggle_panels_visibility(darktable.gui->ui);
    
    /* trigger invalidation of centerview to reprocess pipe */
    dt_dev_invalidate(darktable.develop);

  } else if (key == accels->global_header.accel_key &&
	     state == accels->global_header.accel_mods)
  {
    char key[512];
    const dt_view_t *cv = dt_view_manager_get_current_view(darktable.view_manager);

    /* do nothing if in collaps panel state 
       TODO: reconsider adding this check to ui api */
    g_snprintf(key, 512, "%s/ui/panel_collaps_state",cv->module_name);
    if (dt_conf_get_int(key))
      return 0;

    /* toggle the header visibility state */
    g_snprintf(key, 512, "%s/ui/show_header", cv->module_name);
    gboolean header = !dt_conf_get_bool(key);
    dt_conf_set_bool(key, header);
    
    /* show/hide the actual header panel */
    dt_ui_panel_show(darktable.gui->ui, DT_UI_PANEL_TOP, header); 
  }
 
  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
  return 0;
}

int dt_control_key_pressed(guint key, guint state)
{
  int needRedraw;

  needRedraw = dt_view_manager_key_pressed(darktable.view_manager, key,
                                               state);
  if( needRedraw )
    gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
  
  return 0;
}

int dt_control_key_released(guint key, guint state)
{
  // this line is here to find the right key code on different platforms (mac).
  // printf("key code pressed: %d\n", which);

  switch (key)
  {
    default:
      // propagate to view modules.
      dt_view_manager_key_released(darktable.view_manager, key, state);
      break;
  }

  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
  return 0;
}

void dt_control_hinter_message(const struct dt_control_t *s, const char *message)
{
  if (s->proxy.hinter.module)
    return s->proxy.hinter.set_message(s->proxy.hinter.module, message);
}

const guint *dt_control_backgroundjobs_create(const struct dt_control_t *s, guint type,const gchar *message)
{
  if (s->proxy.backgroundjobs.module)
    return s->proxy.backgroundjobs.create(s->proxy.backgroundjobs.module, type, message);
  return 0;
}

void dt_control_backgroundjobs_destroy(const struct dt_control_t *s, const guint *key)
{
  if (s->proxy.backgroundjobs.module)
    s->proxy.backgroundjobs.destroy(s->proxy.backgroundjobs.module, key);
}

void dt_control_backgroundjobs_progress(const struct dt_control_t *s, const guint *key, double progress)
{
  if (s->proxy.backgroundjobs.module)
    s->proxy.backgroundjobs.progress(s->proxy.backgroundjobs.module, key, progress);
}

void dt_control_backgroundjobs_set_cancellable(const struct dt_control_t *s, const guint *key, dt_job_t *job)
{
  if (s->proxy.backgroundjobs.module)
    s->proxy.backgroundjobs.set_cancellable(s->proxy.backgroundjobs.module, key, job);
}
