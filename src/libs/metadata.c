/*
    This file is part of darktable,
    copyright (c) 2010-2011 tobias ellinghaus, Henrik Andersson.

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
#include "common/metadata.h"
#include "common/debug.h"
#include "control/control.h"
#include "control/signal.h"
#include "control/conf.h"
#include "libs/lib.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include "dtgtk/button.h"

DT_MODULE(1)

typedef struct dt_lib_metadata_t
{
  int imgsel;
  GtkComboBoxEntry * title;
  GtkComboBoxEntry * description;
  GtkComboBoxEntry * creator;
  GtkComboBoxEntry * publisher;
  GtkComboBoxEntry * rights;
  gboolean           multi_title;
  gboolean           multi_description;
  gboolean           multi_creator;
  gboolean           multi_publisher;
  gboolean           multi_rights;
  GtkWidget *clear_button;
  GtkWidget *apply_button;
}
dt_lib_metadata_t;

const char* name()
{
  return _("metadata editor");
}

uint32_t views()
{
  return DT_VIEW_LIGHTTABLE | DT_VIEW_TETHERING;
}

uint32_t container()
{
  return DT_UI_CONTAINER_PANEL_RIGHT_CENTER;
}

static void fill_combo_box_entry(GtkComboBoxEntry **box, uint32_t count, GList **items, gboolean *multi)
{
  GList *iter;

  // FIXME: use gtk_combo_box_text_remove_all() in future (gtk 3.0)
  // https://bugzilla.gnome.org/show_bug.cgi?id=324899
  gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(*box))));

  // FIXME: how to make a nice empty combo box without the append/remove?
  if(count == 0)
  {
    gtk_combo_box_append_text(GTK_COMBO_BOX(*box), "");
    gtk_combo_box_set_active(GTK_COMBO_BOX(*box), 0);
    gtk_combo_box_remove_text(GTK_COMBO_BOX(*box), 0);

    *multi = FALSE;
    return;
  }

  if(count>1)
  {
    gtk_combo_box_append_text(GTK_COMBO_BOX(*box), _("<leave unchanged>")); // FIXME: should be italic!
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(*box), GTK_SENSITIVITY_AUTO);
    *multi = TRUE;
  }
  else
  {
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(*box), GTK_SENSITIVITY_OFF);
    *multi = FALSE;
  }
  if((iter = g_list_first(*items)) != NULL)
  {
    do
    {
      gtk_combo_box_append_text(GTK_COMBO_BOX(*box), iter->data); // FIXME: dt segfaults when there are illegal characters in the string.
    }
    while((iter=g_list_next(iter)) != NULL);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(*box), 0);
}

static void update(dt_lib_module_t *user_data, gboolean early_bark_out)
{
// 	early_bark_out = FALSE; // FIXME: when barking out early we don't update on ctrl-a/ctrl-shift-a. but otherwise it's impossible to edit text
  dt_lib_module_t *self = (dt_lib_module_t *)user_data;
  dt_lib_metadata_t *d  = (dt_lib_metadata_t *)self->data;
  int imgsel = -1;
  DT_CTL_GET_GLOBAL(imgsel, lib_image_mouse_over_id);
  if(early_bark_out && imgsel == d->imgsel)
    return;

  d->imgsel = imgsel;

  sqlite3_stmt *stmt;

  GList *title       = NULL;
  uint32_t title_count       = 0;
  GList *description = NULL;
  uint32_t description_count = 0;
  GList *creator     = NULL;
  uint32_t creator_count     = 0;
  GList *publisher   = NULL;
  uint32_t publisher_count   = 0;
  GList *rights      = NULL;
  uint32_t rights_count     = 0;

  // using dt_metadata_get() is not possible here. we want to do all this in a single pass, everything else takes ages.
  if(imgsel < 0)  // selected images
  {
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select key, value from meta_data where id in (select imgid from selected_images) group by key, value order by value", -1, &stmt, NULL);
  }
  else     // single image under mouse cursor
  {
    char query[1024];
    snprintf(query, 1024, "select key, value from meta_data where id = %d group by key, value order by value", imgsel);
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), query, -1, &stmt, NULL);
  }
  while(sqlite3_step(stmt) == SQLITE_ROW)
  {
    char *value = g_strdup((char *)sqlite3_column_text(stmt, 1));
    if(sqlite3_column_bytes(stmt,1))
    {
      switch(sqlite3_column_int(stmt, 0))
      {
	case DT_METADATA_XMP_DC_CREATOR:
	  creator_count++;
	  creator = g_list_append(creator, value);
	  break;
        case DT_METADATA_XMP_DC_PUBLISHER:
	  publisher_count++;
	  publisher = g_list_append(publisher, value);
	  break;
        case DT_METADATA_XMP_DC_TITLE:
	  title_count++;
	  title = g_list_append(title, value);
	  break;
        case DT_METADATA_XMP_DC_DESCRIPTION:
	  description_count++;
	  description = g_list_append(description, value);
	  break;
        case DT_METADATA_XMP_DC_RIGHTS:
	  rights_count++;
	  rights = g_list_append(rights, value);
	  break;
      }
    }
  }
  sqlite3_finalize(stmt);

  fill_combo_box_entry(&(d->title), title_count, &title, &(d->multi_title));
  fill_combo_box_entry(&(d->description), description_count, &description, &(d->multi_description));
  fill_combo_box_entry(&(d->rights), rights_count, &rights, &(d->multi_rights));
  fill_combo_box_entry(&(d->creator), creator_count, &creator, &(d->multi_creator));
  fill_combo_box_entry(&(d->publisher), publisher_count, &publisher, &(d->multi_publisher));

  g_list_free(g_list_first(title));
  g_list_free(g_list_first(description));
  g_list_free(g_list_first(creator));
  g_list_free(g_list_first(publisher));
  g_list_free(g_list_first(rights));
}


static gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  if(!dt_control_running())
    return FALSE;
  update((dt_lib_module_t*)user_data, TRUE);
  return FALSE;
}

static void clear_button_clicked(GtkButton *button, gpointer user_data)
{
  dt_metadata_clear(-1);
  dt_image_synch_xmp(-1);
  update(user_data, FALSE);
}

static void write_metadata(dt_lib_module_t *self)
{
  dt_lib_metadata_t *d  = (dt_lib_metadata_t *)self->data;

  gchar *title       = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->title));
  gchar *description = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->description));
  gchar *rights      = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->rights));
  gchar *creator     = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->creator));
  gchar *publisher   = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->publisher));

  if(title != NULL && (d->multi_title == FALSE || gtk_combo_box_get_active(GTK_COMBO_BOX(d->title)) != 0))
    dt_metadata_set(-1, "Xmp.dc.title", title);
  if(description != NULL && (d->multi_description == FALSE || gtk_combo_box_get_active(GTK_COMBO_BOX(d->description)) != 0))
    dt_metadata_set(-1, "Xmp.dc.description", description);
  if(rights != NULL && (d->multi_rights == FALSE || gtk_combo_box_get_active(GTK_COMBO_BOX(d->rights)) != 0))
    dt_metadata_set(-1, "Xmp.dc.rights", rights);
  if(creator != NULL && (d->multi_creator == FALSE || gtk_combo_box_get_active(GTK_COMBO_BOX(d->creator)) != 0))
    dt_metadata_set(-1, "Xmp.dc.creator", creator);
  if(publisher != NULL && (d->multi_publisher == FALSE || gtk_combo_box_get_active(GTK_COMBO_BOX(d->publisher)) != 0))
    dt_metadata_set(-1, "Xmp.dc.publisher", publisher);

  if(title != NULL)
    g_free(title);
  if(description != NULL)
    g_free(description);
  if(rights != NULL)
    g_free(rights);
  if(creator != NULL)
    g_free(creator);
  if(publisher != NULL)
    g_free(publisher);

  dt_image_synch_xmp(-1);
  update(self, FALSE);
}

static void apply_button_clicked(GtkButton *button, gpointer user_data)
{
  write_metadata(user_data);
}

static void enter_pressed(GtkEntry *entry, gpointer user_data)
{
  write_metadata(user_data);
  gtk_window_set_focus(GTK_WINDOW(dt_ui_main_window(darktable.gui->ui)), NULL);
}

void gui_reset(dt_lib_module_t *self)
{
  update(self, FALSE);
}

int position()
{
  return 510;
}

static void _mouse_over_image_callback(gpointer instace,gpointer user_data) 
{
  dt_lib_module_t *self=(dt_lib_module_t *)user_data;
  /* lets trigger an expose for a redraw of widget */
  gtk_widget_queue_draw(GTK_WIDGET(self->widget));
} 

void init_key_accels(dt_lib_module_t *self)
{
  dt_accel_register_lib(self, NC_("accel", "clear"), 0, 0);
  dt_accel_register_lib(self, NC_("accel", "apply"), 0, 0);
}

void connect_key_accels(dt_lib_module_t *self)
{
  dt_lib_metadata_t *d = (dt_lib_metadata_t*)self->data;

  dt_accel_connect_button_lib(self, "clear", d->clear_button);
  dt_accel_connect_button_lib(self, "apply", d->apply_button);
}

void gui_init(dt_lib_module_t *self)
{
  GtkBox *hbox;
  GtkWidget *button;
  GtkWidget *label;
  GtkEntryCompletion *completion;

  dt_lib_metadata_t *d = (dt_lib_metadata_t *)malloc(sizeof(dt_lib_metadata_t));
  self->data = (void *)d;

  d->imgsel = -1;

  self->widget = gtk_table_new(6, 2, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(self->widget), 5);

  g_signal_connect(self->widget, "expose-event", G_CALLBACK(expose), self);
 
  label = gtk_label_new(_("title"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->title = GTK_COMBO_BOX_ENTRY(gtk_combo_box_entry_new_text());
  dt_gui_key_accel_block_on_focus(GTK_WIDGET(gtk_bin_get_child(GTK_BIN(d->title))));
  completion = gtk_entry_completion_new();
  gtk_entry_completion_set_model(completion, gtk_combo_box_get_model(GTK_COMBO_BOX(d->title)));
  gtk_entry_completion_set_text_column(completion, 0);
  gtk_entry_completion_set_inline_completion(completion, TRUE);
  gtk_entry_set_completion(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->title))), completion);
  g_signal_connect (GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->title))), "activate", G_CALLBACK (enter_pressed), self);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->title), 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  label = gtk_label_new(_("description"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->description = GTK_COMBO_BOX_ENTRY(gtk_combo_box_entry_new_text());
  dt_gui_key_accel_block_on_focus(GTK_WIDGET(gtk_bin_get_child(GTK_BIN(d->description))));
  completion = gtk_entry_completion_new();
  gtk_entry_completion_set_model(completion, gtk_combo_box_get_model(GTK_COMBO_BOX(d->description)));
  gtk_entry_completion_set_text_column(completion, 0);
  gtk_entry_completion_set_inline_completion(completion, TRUE);
  gtk_entry_set_completion(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->description))), completion);
  g_signal_connect (GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->description))), "activate", G_CALLBACK (enter_pressed), self);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->description), 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  label = gtk_label_new(_("creator"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->creator = GTK_COMBO_BOX_ENTRY(gtk_combo_box_entry_new_text());
  dt_gui_key_accel_block_on_focus(GTK_WIDGET(gtk_bin_get_child(GTK_BIN(d->creator))));
  completion = gtk_entry_completion_new();
  gtk_entry_completion_set_model(completion, gtk_combo_box_get_model(GTK_COMBO_BOX(d->creator)));
  gtk_entry_completion_set_text_column(completion, 0);
  gtk_entry_completion_set_inline_completion(completion, TRUE);
  gtk_entry_set_completion(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->creator))), completion);
  g_signal_connect (GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->creator))), "activate", G_CALLBACK (enter_pressed), self);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->creator), 1, 2, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  label = gtk_label_new(_("publisher"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 3, 4, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->publisher = GTK_COMBO_BOX_ENTRY(gtk_combo_box_entry_new_text());
  dt_gui_key_accel_block_on_focus(GTK_WIDGET(gtk_bin_get_child(GTK_BIN(d->publisher))));
  completion = gtk_entry_completion_new();
  gtk_entry_completion_set_model(completion, gtk_combo_box_get_model(GTK_COMBO_BOX(d->publisher)));
  gtk_entry_completion_set_text_column(completion, 0);
  gtk_entry_completion_set_inline_completion(completion, TRUE);
  gtk_entry_set_completion(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->publisher))), completion);
  g_signal_connect (GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->publisher))), "activate", G_CALLBACK (enter_pressed), self);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->publisher), 1, 2, 3, 4, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  label = gtk_label_new(_("rights"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 4, 5, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->rights = GTK_COMBO_BOX_ENTRY(gtk_combo_box_entry_new_text());
  dt_gui_key_accel_block_on_focus(GTK_WIDGET(gtk_bin_get_child(GTK_BIN(d->rights))));
  completion = gtk_entry_completion_new();
  gtk_entry_completion_set_model(completion, gtk_combo_box_get_model(GTK_COMBO_BOX(d->rights)));
  gtk_entry_completion_set_text_column(completion, 0);
  gtk_entry_completion_set_inline_completion(completion, TRUE);
  gtk_entry_set_completion(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->rights))), completion);
  g_signal_connect (GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d->rights))), "activate", G_CALLBACK (enter_pressed), self);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->rights), 1, 2, 4, 5, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  g_object_unref(completion);

  // reset/apply buttons
  hbox = GTK_BOX(gtk_hbox_new(TRUE, 5));

  button = gtk_button_new_with_label(_("clear"));
  d->clear_button = button;
  g_object_set(G_OBJECT(button), "tooltip-text", _("remove metadata from selected images"), (char *)NULL);
  gtk_box_pack_start(hbox, button, FALSE, TRUE, 0);
  g_signal_connect(G_OBJECT (button), "clicked",
                   G_CALLBACK (clear_button_clicked), (gpointer)self);

  button = gtk_button_new_with_label(_("apply"));
  d->apply_button = button;
  g_object_set(G_OBJECT(button), "tooltip-text", _("write metadata for selected images"), (char *)NULL);
  g_signal_connect(G_OBJECT (button), "clicked",
                   G_CALLBACK (apply_button_clicked), (gpointer)self);
  gtk_box_pack_start(hbox, button, FALSE, TRUE, 0);

  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(hbox), 0, 2, 5, 6, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  /* lets signup for mouse over image change signals */
  dt_control_signal_connect(darktable.signals,DT_SIGNAL_MOUSE_OVER_IMAGE_CHANGE, 
			    G_CALLBACK(_mouse_over_image_callback), self);

}

void gui_cleanup(dt_lib_module_t *self)
{
  dt_control_signal_disconnect(darktable.signals,G_CALLBACK(_mouse_over_image_callback),self);
  free(self->data);
  self->data = NULL;
}

static void add_rights_preset(dt_lib_module_t *self, char *name, char *string)
{
  unsigned int params_size = strlen(string)+5;

  char *params = calloc(sizeof(char), params_size);
  memcpy(params+2, string, params_size-5);
  dt_lib_presets_add(name, self->plugin_name, self->version(), params, params_size);
  free(params);
}

void init_presets(dt_lib_module_t *self)
{

  // <title>\0<description>\0<rights>\0<creator>\0<publisher>

  add_rights_preset(self, _("CC-by"), _("Creative Commons Attribution (CC-BY)"));
  add_rights_preset(self, _("CC-by-sa"), _("Creative Commons Attribution-ShareAlike (CC-BY-SA)"));
  add_rights_preset(self, _("CC-by-nd"), _("Creative Commons Attribution-NoDerivs (CC-BY-ND)"));
  add_rights_preset(self, _("CC-by-nc"), _("Creative Commons Attribution-NonCommercial (CC-BY-NC)"));
  add_rights_preset(self, _("CC-by-nc-sa"), _("Creative Commons Attribution-NonCommercial-ShareAlike (CC-BY-NC-SA)"));
  add_rights_preset(self, _("CC-by-nc-nd"), _("Creative Commons Attribution-NonCommercial-NoDerivs (CC-BY-NC-ND)"));
  add_rights_preset(self, _("all rights reserved"), _("All rights reserved."));
}

void* get_params(dt_lib_module_t *self, int *size)
{
  dt_lib_metadata_t *d = (dt_lib_metadata_t *)self->data;

  char *title       = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->title));
  char *description = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->description));
  char *rights     = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->rights));
  char *creator     = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->creator));
  char *publisher   = gtk_combo_box_get_active_text(GTK_COMBO_BOX(d->publisher));

  int32_t title_len       = strlen(title);
  int32_t description_len = strlen(description);
  int32_t rights_len      = strlen(rights);
  int32_t creator_len     = strlen(creator);
  int32_t publisher_len   = strlen(publisher);

  *size = title_len + description_len + rights_len + creator_len + publisher_len + 5;

  char *params = (char *)malloc(*size);

  int pos = 0;
  memcpy(params+pos, title, title_len+1);
  pos += title_len+1;
  memcpy(params+pos, description, description_len+1);
  pos += description_len+1;
  memcpy(params+pos, rights, rights_len+1);
  pos += rights_len+1;
  memcpy(params+pos, creator, creator_len+1);
  pos += creator_len+1;
  memcpy(params+pos, publisher, publisher_len+1);
  pos += publisher_len+1;

  g_assert(pos == *size);

  return params;
}

int set_params(dt_lib_module_t *self, const void *params, int size)
{
  char *buf         = (char* )params;
  char *title       = buf;
  buf += strlen(title) + 1;
  char *description = buf;
  buf += strlen(description) + 1;
  char *rights     = buf;
  buf += strlen(rights) + 1;
  char *creator     = buf;
  buf += strlen(creator) + 1;
  char *publisher   = buf;

  if(size != strlen(title) + strlen(description) + strlen(rights) + strlen(creator) + strlen(publisher) + 5) return 1;

  if(title != NULL && title[0] != '\0')
    dt_metadata_set(-1, "Xmp.dc.title", title);
  if(description != NULL && description[0] != '\0')
    dt_metadata_set(-1, "Xmp.dc.description", description);
  if(rights != NULL && rights[0] != '\0')
    dt_metadata_set(-1, "Xmp.dc.rights", rights);
  if(creator != NULL && creator[0] != '\0')
    dt_metadata_set(-1, "Xmp.dc.creator", creator);
  if(publisher != NULL && publisher[0] != '\0')
    dt_metadata_set(-1, "Xmp.dc.publisher", publisher);

  dt_image_synch_xmp(-1);
  update(self, FALSE);
  return 0;
}
