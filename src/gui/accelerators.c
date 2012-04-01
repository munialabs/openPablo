/*
    This file is part of darktable,
    copyright (c) 2011 robert bieber.

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
#include "gui/accelerators.h"
#include "control/control.h"
#include "dtgtk/slider.h"
#include "common/debug.h"
#include "develop/blend.h"

#include <gtk/gtk.h>


void dt_accel_path_global(char *s, size_t n, const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s",
           NC_("accel", "global"), path);
}

void dt_accel_path_view(char *s, size_t n, char *module,
                               const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           NC_("accel", "views"), module, path);
}

void dt_accel_path_iop(char *s, size_t n, char *module,
                              const char *path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           NC_("accel", "image operations"), module, path);
}

void dt_accel_path_lib(char *s, size_t n, char *module,
                              const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           NC_("accel", "plugins"), module, path);
}

void dt_accel_paths_slider_iop(char *s[], size_t n, char *module,
                               const char* path)
{
  snprintf(s[0], n, "<Darktable>/%s/%s/%s/%s",
           NC_("accel", "image operations"), module, path,
           NC_("accel", "increase"));
  snprintf(s[1], n, "<Darktable>/%s/%s/%s/%s",
           NC_("accel", "image operations"), module, path,
           NC_("accel", "decrease"));
  snprintf(s[2], n, "<Darktable>/%s/%s/%s/%s",
           NC_("accel", "image operations"), module, path,
           NC_("accel", "reset"));
  snprintf(s[3], n, "<Darktable>/%s/%s/%s/%s",
           NC_("accel", "image operations"), module, path,
           NC_("accel", "edit"));
}

static void dt_accel_path_global_translated(char *s, size_t n, const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s",
           C_("accel", "global"), g_dpgettext2(NULL, "accel", path));
}

static void dt_accel_path_view_translated(char *s, size_t n, dt_view_t *module,
                                          const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           C_("accel", "views"), module->name(module),
           g_dpgettext2(NULL, "accel", path));
}

static void dt_accel_path_iop_translated(char *s, size_t n,
                                         dt_iop_module_so_t *module,
                                         const char *path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           C_("accel", "image operations"), module->name(),
           g_dpgettext2(NULL, "accel", path));
}

static void dt_accel_path_lib_translated(char *s, size_t n,
                                         dt_lib_module_t *module,
                                         const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           C_("accel", "plugins"), module->name(),
           g_dpgettext2(NULL, "accel", path));
}

static void dt_accel_paths_slider_iop_translated(char *s[], size_t n,
                                                 dt_iop_module_so_t *module,
                                                 const char* path)
{
  snprintf(s[0], n, "<Darktable>/%s/%s/%s/%s",
           C_("accel", "image operations"),
           module->name(),
           g_dpgettext2(NULL, "accel", path),
           C_("accel", "increase"));
  snprintf(s[1], n, "<Darktable>/%s/%s/%s/%s",
           C_("accel", "image operations"),
           module->name(),
           g_dpgettext2(NULL, "accel", path),
           C_("accel", "decrease"));
  snprintf(s[2], n, "<Darktable>/%s/%s/%s/%s",
           C_("accel", "image operations"),
           module->name(),
           g_dpgettext2(NULL, "accel", path),
           C_("accel", "reset"));
  snprintf(s[3], n, "<Darktable>/%s/%s/%s/%s",
           C_("accel", "image operations"),
           module->name(),
           g_dpgettext2(NULL, "accel", path),
           C_("accel", "edit"));

}

void dt_accel_register_global(const gchar *path, guint accel_key,
                              GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_global(accel_path, 256, path);
  gtk_accel_map_add_entry(accel_path, accel_key, mods);

  strcpy(accel->path, accel_path);
  dt_accel_path_global_translated(accel_path, 256, path);
  strcpy(accel->translated_path, accel_path);

  *(accel->module) = '\0';
  accel->views = DT_VIEW_DARKROOM | DT_VIEW_LIGHTTABLE | DT_VIEW_TETHERING;
  accel->local = FALSE;
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);

}

void dt_accel_register_view(dt_view_t *self, const gchar *path, guint accel_key,
                            GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_view(accel_path, 256, self->module_name, path);
  gtk_accel_map_add_entry(accel_path, accel_key, mods);

  strcpy(accel->path, accel_path);
  dt_accel_path_view_translated(accel_path, 256, self, path);
  strcpy(accel->translated_path, accel_path);

  strcpy(accel->module, self->module_name);
  accel->views = self->view(self);
  accel->local = FALSE;
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);
}

void dt_accel_register_iop(dt_iop_module_so_t *so, gboolean local,
                           const gchar *path, guint accel_key,
                           GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_iop(accel_path, 256, so->op, path);
  gtk_accel_map_add_entry(accel_path, accel_key, mods);

  strcpy(accel->path, accel_path);
  dt_accel_path_iop_translated(accel_path, 256, so, path);
  strcpy(accel->translated_path, accel_path);

  strcpy(accel->module, so->op);
  accel->local = local;
  accel->views = DT_VIEW_DARKROOM;
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);
}

void dt_accel_register_lib(dt_lib_module_t *self, const gchar *path,
                           guint accel_key, GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_lib(accel_path, 256, self->plugin_name, path);
  gtk_accel_map_add_entry(accel_path, accel_key, mods);
  strcpy(accel->path, accel_path);
  dt_accel_path_lib_translated(accel_path, 256, self, path);
  strcpy(accel->translated_path, accel_path);

  strcpy(accel->module, self->plugin_name);
  accel->local = FALSE;
  accel->views = self->views();
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);
}

void dt_accel_register_slider_iop(dt_iop_module_so_t *so, gboolean local,
                           const gchar *path)
{
  gchar increase_path[256];
  gchar decrease_path[256];
  gchar reset_path[256];
  gchar edit_path[256];
  gchar increase_path_trans[256];
  gchar decrease_path_trans[256];
  gchar reset_path_trans[256];
  gchar edit_path_trans[256];

  char *paths[] = {increase_path, decrease_path, reset_path, edit_path};
  char *paths_trans[] = {increase_path_trans, decrease_path_trans,
                         reset_path_trans, edit_path_trans};

  int i = 0;
  dt_accel_t *accel = NULL;

  dt_accel_paths_slider_iop(paths, 256, so->op, path);
  dt_accel_paths_slider_iop_translated(paths_trans, 256, so, path);

  for(i = 0; i < 4; i++)
  {
    gtk_accel_map_add_entry(paths[i], 0, 0);
    accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

    strcpy(accel->path, paths[i]);
    strcpy(accel->translated_path, paths_trans[i]);
    strcpy(accel->module, so->op);
    accel->local = local;
    accel->views = DT_VIEW_DARKROOM;

    darktable.control->accelerator_list =
        g_slist_prepend(darktable.control->accelerator_list, accel);
  }
}


static dt_accel_t* _lookup_accel(gchar *path)
{
  GSList *l = darktable.control->accelerator_list;
  while(l)
  {
    dt_accel_t *accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, path))
      return accel;
    l = g_slist_next(l);
  }
  return NULL;
}

void dt_accel_connect_global(const gchar *path, GClosure *closure)
{
  gchar accel_path[256];
  dt_accel_path_global(accel_path, 256, path);
  dt_accel_t *laccel = _lookup_accel(accel_path);
  laccel->closure = closure;
  gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                  closure);
}

void dt_accel_connect_view(dt_view_t *self, const gchar *path,
                           GClosure *closure)
{
  gchar accel_path[256];
  dt_accel_path_view(accel_path, 256, self->module_name, path);
  gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                  closure);
  dt_accel_t *laccel = _lookup_accel(accel_path);
  laccel->closure = closure;

  self->accel_closures = g_slist_prepend(self->accel_closures, laccel);
}

static void _connect_local_accel(dt_iop_module_t *module, dt_accel_t *accel)
{
  module->accel_closures_local =
      g_slist_prepend(module->accel_closures_local, accel);
}

void dt_accel_connect_iop(dt_iop_module_t *module, const gchar *path,
                          GClosure *closure)
{
  dt_accel_t *accel = NULL;
  gchar accel_path[256];
  dt_accel_path_iop(accel_path, 256, module->op, path);
  // Looking up the entry in the global accelerators list
  accel = _lookup_accel(accel_path);

  accel->closure = closure;

  if(accel && accel->local)
  {
    // Local accelerators don't actually get connected, just added to the list
    // They will be connected if/when the module gains focus
    _connect_local_accel(module, accel);
  }
  else
  {
    gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                    closure);
    module->accel_closures = g_slist_prepend(module->accel_closures, accel);
  }
}

void dt_accel_connect_lib(dt_lib_module_t *module, const gchar *path,
                          GClosure *closure)
{
  gchar accel_path[256];
  dt_accel_path_lib(accel_path, 256, module->plugin_name, path);
  gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                  closure);

  dt_accel_t *accel =_lookup_accel(accel_path);

  accel->closure = closure;

  module->accel_closures = g_slist_prepend(module->accel_closures, accel);
}

static gboolean _press_button_callback(GtkAccelGroup *accel_group,
                                   GObject *acceleratable,
                                   guint keyval, GdkModifierType modifier,
                                   gpointer data)
{
  if(!(GTK_IS_BUTTON(data)))
    return FALSE;

  g_signal_emit_by_name(G_OBJECT(data),"activate");
  return TRUE;
}

void dt_accel_connect_button_iop(dt_iop_module_t *module, const gchar *path,
                                 GtkWidget *button)
{
  GClosure *closure = g_cclosure_new(G_CALLBACK(_press_button_callback),
                                     button, NULL);
  dt_accel_connect_iop(module, path, closure);
}

void dt_accel_connect_button_lib(dt_lib_module_t *module, const gchar *path,
                                 GtkWidget *button)
{
  GClosure *closure = g_cclosure_new(G_CALLBACK(_press_button_callback),
                                     (gpointer)button, NULL);
  dt_accel_connect_lib(module, path, closure);
}


static gboolean slider_edit_callback(GtkAccelGroup *accel_group,
                                    GObject *acceleratable, guint keyval,
                                    GdkModifierType modifier, gpointer data)
{
  GtkDarktableSlider *slider=DTGTK_SLIDER(data);
  char sv[32]= {0};
  slider->is_entry_active=TRUE;
  gdouble value = gtk_adjustment_get_value(slider->adjustment);
  sprintf(sv,"%.*f",slider->digits,value);
  gtk_entry_set_text (GTK_ENTRY(slider->entry),sv);
  gtk_widget_show (GTK_WIDGET(slider->entry));
  gtk_widget_grab_focus (GTK_WIDGET(slider->entry));
  gtk_widget_queue_draw (GTK_WIDGET(slider));
  return TRUE;
}
static gboolean slider_increase_callback(GtkAccelGroup *accel_group,
                                    GObject *acceleratable, guint keyval,
                                    GdkModifierType modifier, gpointer data)
{
  GtkDarktableSlider *slider=DTGTK_SLIDER(data);
  float value = gtk_adjustment_get_value(slider->adjustment);
  value += gtk_adjustment_get_step_increment(slider->adjustment);
  if(slider->snapsize) value = slider->snapsize * (((int)value)/slider->snapsize);

  gtk_adjustment_set_value(slider->adjustment, value);
  gtk_widget_draw(GTK_WIDGET(slider),NULL);
  g_signal_emit_by_name(G_OBJECT(slider),"value-changed");
  return TRUE;
}
static gboolean slider_decrease_callback(GtkAccelGroup *accel_group,
                                    GObject *acceleratable, guint keyval,
                                    GdkModifierType modifier, gpointer data)
{
  GtkDarktableSlider *slider=DTGTK_SLIDER(data);
  float value = gtk_adjustment_get_value(slider->adjustment);
  value -= gtk_adjustment_get_step_increment(slider->adjustment);
  if(slider->snapsize) value = slider->snapsize * (((int)value)/slider->snapsize);

  gtk_adjustment_set_value(slider->adjustment, value);
  gtk_widget_draw(GTK_WIDGET(slider),NULL);
  g_signal_emit_by_name(G_OBJECT(slider),"value-changed");
  return TRUE;
}

static gboolean slider_reset_callback(GtkAccelGroup *accel_group,
                                    GObject *acceleratable, guint keyval,
                                    GdkModifierType modifier, gpointer data)
{
  GtkDarktableSlider *slider=DTGTK_SLIDER(data);
  gtk_adjustment_set_value(slider->adjustment, slider->default_value);
  gtk_widget_draw(GTK_WIDGET(slider),NULL);
  g_signal_emit_by_name(G_OBJECT(slider),"value-changed");
  return TRUE;
}
void dt_accel_connect_slider_iop(dt_iop_module_t *module, const gchar *path,
                                 GtkWidget *slider)
{
  gchar increase_path[256];
  gchar decrease_path[256];
  gchar reset_path[256];
  gchar edit_path[256];
  dt_accel_t *accel = NULL;
  GClosure *closure;
  char *paths[] = {increase_path, decrease_path, reset_path, edit_path};
  dt_accel_paths_slider_iop(paths, 256, module->op, path);

  closure =  g_cclosure_new(G_CALLBACK(slider_increase_callback),
                            (gpointer)slider, NULL);
  accel = _lookup_accel(increase_path);

  accel->closure = closure;

  if(accel && accel->local)
  {
    _connect_local_accel(module, accel);
  }
  else
  {
    gtk_accel_group_connect_by_path(darktable.control->accelerators,
                                    increase_path, closure);
    module->accel_closures = g_slist_prepend(module->accel_closures, accel);
  }

  closure = g_cclosure_new(G_CALLBACK(slider_decrease_callback),
                           (gpointer)slider, NULL);
  accel = _lookup_accel(decrease_path);
  accel->closure = closure;
  if(accel && accel->local)
  {
    _connect_local_accel(module, accel);
  }
  else
  {
    gtk_accel_group_connect_by_path(darktable.control->accelerators,
                                    decrease_path, closure);
    module->accel_closures = g_slist_prepend(module->accel_closures, accel);
  }

  closure = g_cclosure_new(G_CALLBACK(slider_reset_callback),
                           (gpointer)slider, NULL);
  accel = _lookup_accel(reset_path);
  accel->closure = closure;
  if(accel && accel->local)
  {
    _connect_local_accel(module, accel);
  }
  else
  {
    gtk_accel_group_connect_by_path(darktable.control->accelerators,
                                    reset_path, closure);
    module->accel_closures = g_slist_prepend(module->accel_closures, accel);
  }

  closure = g_cclosure_new(G_CALLBACK(slider_edit_callback),
                           (gpointer)slider, NULL);
  accel = _lookup_accel(edit_path);
  accel->closure = closure;
  if(accel && accel->local)
  {
    _connect_local_accel(module, accel);
  }
  else
  {
    gtk_accel_group_connect_by_path(darktable.control->accelerators,
                                    edit_path, closure);
    module->accel_closures = g_slist_prepend(module->accel_closures, accel);
  }
}

void dt_accel_connect_locals_iop(dt_iop_module_t *module)
{
  dt_accel_t *accel;
  GSList *l = module->accel_closures_local;

  while(l)
  {
    accel = (dt_accel_t*)l->data;
    gtk_accel_group_connect_by_path(darktable.control->accelerators,
                                    accel->path, accel->closure);
    l = g_slist_next(l);
  }

  module->local_closures_connected = TRUE;
}

void dt_accel_disconnect_list(GSList *list)
{
  dt_accel_t *accel;
  while(list)
  {
    accel = (dt_accel_t*)list->data;
    gtk_accel_group_disconnect(darktable.control->accelerators, accel->closure);
    list = g_slist_delete_link(list, list);
  }
}

void dt_accel_disconnect_locals_iop(dt_iop_module_t *module)
{
  dt_accel_t *accel;
  GSList *l = module->accel_closures_local;

  if(!module->local_closures_connected)
    return;

  while(l)
  {
    accel = (dt_accel_t*)l->data;
    gtk_accel_group_disconnect(darktable.control->accelerators, accel->closure);
    l = g_slist_next(l);
  }

  module->accel_closures_local = NULL;
  module->local_closures_connected = FALSE;
}

void dt_accel_cleanup_locals_iop(dt_iop_module_t *module)
{
  dt_accel_t *accel;
  GSList *l = module->accel_closures_local;
  while(l)
  {
    accel = (dt_accel_t*)l->data;
    if(module->local_closures_connected)
      gtk_accel_group_disconnect(darktable.control->accelerators,
                                 accel->closure);
    l = g_slist_delete_link(l, l);
  }
  module->accel_closures_local = NULL;
}



typedef struct {
  dt_iop_module_t* module;
  char *name;
}preset_iop_module_callback_description;

static void preset_iop_module_callback_destroyer(gpointer data, GClosure *closure) {
  preset_iop_module_callback_description *callback_description = (preset_iop_module_callback_description*)data;
  g_free(callback_description->name);
}
static gboolean preset_iop_module_callback(GtkAccelGroup *accel_group,
                                   GObject *acceleratable,
                                   guint keyval, GdkModifierType modifier,
                                   gpointer data)

{
  preset_iop_module_callback_description *callback_description = (preset_iop_module_callback_description*)data;
  dt_iop_module_t *module = callback_description->module;
  const char* name = callback_description->name;

  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select op_params, enabled, blendop_params from presets where operation = ?1 and name = ?2", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, module->op, strlen(module->op), SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 2, name, strlen(name), SQLITE_TRANSIENT);

  if(sqlite3_step(stmt) == SQLITE_ROW)
  {
    const void *op_params = sqlite3_column_blob(stmt, 0);
    int op_length  = sqlite3_column_bytes(stmt, 0);
    int enabled = sqlite3_column_int(stmt, 1);
    const void *blendop_params = sqlite3_column_blob(stmt, 2);
    int bl_length = sqlite3_column_bytes(stmt, 2);
    if(op_params && (op_length == module->params_size))
    {
      memcpy(module->params, op_params, op_length);
      module->enabled = enabled;
    }
    if (blendop_params && (bl_length == sizeof(dt_develop_blend_params_t)))
    {
      memcpy(module->blend_params, blendop_params, sizeof(dt_develop_blend_params_t));
    }
    else
    {
      memcpy(module->blend_params, module->default_blendop_params, sizeof(dt_develop_blend_params_t));
    }
  }
  sqlite3_finalize(stmt);
  dt_iop_gui_update(module);
  dt_dev_add_history_item(darktable.develop, module, FALSE);
  gtk_widget_queue_draw(module->widget);
  return TRUE;
}

void dt_accel_connect_preset_iop(dt_iop_module_t *module,  const gchar *path)
{
  GClosure* closure = NULL;
  char build_path[1024];
  gchar* name =g_strdup(path);
  snprintf(build_path,1024,"%s/%s",_("preset"),name);
  preset_iop_module_callback_description *callback_description = malloc(sizeof(preset_iop_module_callback_description));
  callback_description->module = module;
  callback_description->name = name;

  closure = g_cclosure_new(G_CALLBACK(preset_iop_module_callback),
                         callback_description, preset_iop_module_callback_destroyer);
  dt_accel_connect_iop(module, build_path, closure);

}




typedef struct {
  dt_lib_module_t* module;
  char *name;
}preset_lib_module_callback_description;

static void preset_lib_module_callback_destroyer(gpointer data, GClosure *closure) {
  preset_lib_module_callback_description *callback_description = (preset_lib_module_callback_description*)data;
  g_free(callback_description->name);
}
static gboolean preset_lib_module_callback(GtkAccelGroup *accel_group,
                                   GObject *acceleratable,
                                   guint keyval, GdkModifierType modifier,
                                   gpointer data)

{
  preset_lib_module_callback_description *callback_description = (preset_lib_module_callback_description*)data;
  dt_lib_module_t *module = callback_description->module;
  const char* pn = callback_description->name;

  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select op_params from presets where operation = ?1 and op_version = ?2 and name = ?3", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, module->plugin_name, -1, SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, module->version());
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 3, pn, strlen(pn), SQLITE_TRANSIENT);

  int res = 0;
  if(sqlite3_step(stmt) == SQLITE_ROW)
  {
    const void *blob = sqlite3_column_blob(stmt, 0);
    int length  = sqlite3_column_bytes(stmt, 0);
    if(blob)
    {
      GList *it = darktable.lib->plugins;
      while(it)
      {
        dt_lib_module_t *search_module = (dt_lib_module_t *)it->data;
        if(!strncmp(search_module->plugin_name, module->plugin_name, 128))
        {
          res = module->set_params(module, blob, length);
          break;
        }
        it = g_list_next(it);
      }
    }
  }
  sqlite3_finalize(stmt);
  if(res)
  {
    dt_control_log(_("deleting preset for obsolete module"));
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "delete from presets where operation = ?1 and op_version = ?2 and name = ?3", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, module->plugin_name, strlen(module->plugin_name), SQLITE_TRANSIENT);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, module->version());
    DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 3, pn, strlen(pn), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
  return TRUE;
}

void dt_accel_connect_preset_lib(dt_lib_module_t *module,  const gchar *path)
{
  GClosure* closure = NULL;
  char build_path[1024];
  gchar* name =g_strdup(path);
  snprintf(build_path,1024,"%s/%s",_("preset"),name);
  preset_lib_module_callback_description *callback_description = malloc(sizeof(preset_lib_module_callback_description));
  callback_description->module = module;
  callback_description->name = name;

  closure = g_cclosure_new(G_CALLBACK(preset_lib_module_callback),
                         callback_description, preset_lib_module_callback_destroyer);
  dt_accel_connect_lib(module, build_path, closure);

}

void dt_accel_deregister_iop(dt_iop_module_t *module,const gchar *path)
{
  dt_accel_t *accel;
  GSList *l = module->accel_closures_local;
  char build_path[1024];
  dt_accel_path_iop(build_path, 1024, module->op, path);
  while(l)
  {
    accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path,build_path)) {
      module->accel_closures_local = g_slist_delete_link(module->accel_closures_local, l);
      l = NULL;
    }
  }
  l = darktable.control->accelerator_list;
  while(l)
  {
    dt_accel_t *accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      darktable.control->accelerator_list = g_slist_delete_link(darktable.control->accelerator_list, l);
      l = NULL;
    } else {
      l = g_slist_next(l);
    }
  }
  l = module->accel_closures;
  while(l)
  {
    accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      if(!accel->local || !module->local_closures_connected)
        gtk_accel_group_disconnect(darktable.control->accelerators, accel->closure);
      module->accel_closures = g_slist_delete_link(module->accel_closures, l);
      l = NULL;
      free(accel);
    } else {
      l = g_slist_next(l);
    }
  }
}

void dt_accel_deregister_lib(dt_lib_module_t *module,const gchar *path)
{
  dt_accel_t *accel;
  GSList *l;
  char build_path[1024];
  dt_accel_path_lib(build_path, 1024, module->plugin_name, path);
  l = darktable.control->accelerator_list;
  while(l)
  {
    dt_accel_t *accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      darktable.control->accelerator_list = g_slist_delete_link(darktable.control->accelerator_list, l);
      l = NULL;
    } else {
      l = g_slist_next(l);
    }
  }
  l = module->accel_closures;
  while(l)
  {
    accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      module->accel_closures = g_slist_delete_link(module->accel_closures, l);
      gtk_accel_group_disconnect(darktable.control->accelerators, accel->closure);
      l = NULL;
      free(accel);
    } else {
      l = g_slist_next(l);
    }
  }
}

void dt_accel_deregister_global(const gchar *path)
{
  GSList *l;
  char build_path[1024];
  dt_accel_path_global(build_path, 1024,  path);
  l = darktable.control->accelerator_list;
  while(l)
  {
    dt_accel_t *accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      darktable.control->accelerator_list = g_slist_delete_link(darktable.control->accelerator_list, l);
      gtk_accel_group_disconnect(darktable.control->accelerators, accel->closure);
      l = NULL;
      free(accel);
    } else {
      l = g_slist_next(l);
    }
  }
}

gboolean find_accel_internal  (GtkAccelKey *key, GClosure *closure, gpointer data) 
{
  return (closure == data);
}

void dt_accel_rename_preset_iop(dt_iop_module_t *module,const gchar *path,const gchar *new_path)
{
  dt_accel_t *accel;
  GSList * l = module->accel_closures;
  char build_path[1024];
  dt_accel_path_iop(build_path, 1024, module->op, path);
  while(l)
  {
    accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      GtkAccelKey tmp_key = *(gtk_accel_group_find(darktable.control->accelerators,find_accel_internal,accel->closure));
      gboolean local = accel->local;
      dt_accel_deregister_iop(module,path);
      snprintf(build_path,1024,"%s/%s",_("preset"),new_path);
      dt_accel_register_iop(module->so,local,build_path,tmp_key.accel_key,tmp_key.accel_mods);
      dt_accel_connect_preset_iop(module,new_path);
      l = NULL;
    } else {
      l = g_slist_next(l);
    }
  }
}

void dt_accel_rename_preset_lib(dt_lib_module_t *module,const gchar *path,const gchar *new_path)
{
  dt_accel_t *accel;
  GSList * l = module->accel_closures;
  char build_path[1024];
  dt_accel_path_lib(build_path, 1024, module->plugin_name, path);
  while(l)
  {
    accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      GtkAccelKey tmp_key = *(gtk_accel_group_find(darktable.control->accelerators,find_accel_internal,accel->closure));
      dt_accel_deregister_lib(module,path);
      snprintf(build_path,1024,"%s/%s",_("preset"),new_path);
      dt_accel_register_lib(module,build_path,tmp_key.accel_key,tmp_key.accel_mods);
      dt_accel_connect_preset_lib(module,new_path);
      l = NULL;
    } else {
      l = g_slist_next(l);
    }
  }
}

void dt_accel_rename_global(const gchar *path,const gchar *new_path)
{
  dt_accel_t *accel;
  GSList * l = darktable.control->accelerator_list;
  char build_path[1024];
  dt_accel_path_global(build_path, 1024,path);
  while(l)
  {
    accel = (dt_accel_t*)l->data;
    if(!strcmp(accel->path, build_path))
    {
      GtkAccelKey tmp_key = *(gtk_accel_group_find(darktable.control->accelerators,find_accel_internal,accel->closure));
      dt_accel_deregister_global(path);
      g_closure_ref(accel->closure);
      dt_accel_register_global(new_path,tmp_key.accel_key,tmp_key.accel_mods);
      dt_accel_connect_global(new_path,accel->closure);
      g_closure_unref(accel->closure);
      l = NULL;
    } else {
      l = g_slist_next(l);
    }
  }
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
