/*
    This file is part of darktable,
    copyright (c) 2009--2010 johannes hanika.
    copyright (c) 2011 henrik andersson.

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
#include "common/colorspaces.h"
#include "common/imageio_module.h"
#include "control/control.h"
#include "control/jobs.h"
#include "control/conf.h"
#include "gui/gtk.h"
#include "dtgtk/slider.h"
#include "dtgtk/label.h"
#include "libs/lib.h"
#include "gui/accelerators.h"
#include "gui/presets.h"
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "dtgtk/button.h"

DT_MODULE(1)

typedef struct dt_lib_export_t
{
  GtkSpinButton *width, *height;
  GtkComboBox *storage, *format;
  int format_lut[128];
  GtkContainer *storage_box, *format_box;
  GtkComboBox *profile, *intent;
  GList *profiles;
  GtkButton *export_button;
}
dt_lib_export_t;

typedef struct dt_lib_export_profile_t
{
  char filename[512]; // icc file name
  char name[512];     // product name
  int  pos;           // position in combo box
}
dt_lib_export_profile_t;

/** Updates the combo box and shows only the supported formats of current selected storage module */
static void _update_formats_combobox(dt_lib_export_t *d);
static gboolean _combo_box_set_active_text(GtkComboBox *cb, const gchar *text);
/** Sets the max dimensions based upon what storage and format supports */
static void _update_dimensions(dt_lib_export_t *d);
/** get the max output dimension supported by combination of storage and format.. */
static void _get_max_output_dimension(dt_lib_export_t *d,uint32_t *width, uint32_t *height);

const char*
name ()
{
  return _("export selected");
}

uint32_t views()
{
  return DT_VIEW_LIGHTTABLE;
}

uint32_t container()
{
  return DT_UI_CONTAINER_PANEL_RIGHT_CENTER;
}

static void
export_button_clicked (GtkWidget *widget, gpointer user_data)
{
  // Let's get the max dimension restriction if any...
  dt_control_export();
}

static void
width_changed (GtkSpinButton *spin, gpointer user_data)
{
  int value = gtk_spin_button_get_value(spin);
  dt_conf_set_int ("plugins/lighttable/export/width", value);
}

static void
height_changed (GtkSpinButton *spin, gpointer user_data)
{
  int value = gtk_spin_button_get_value(spin);
  dt_conf_set_int ("plugins/lighttable/export/height", value);
}

void
gui_reset (dt_lib_module_t *self)
{
  // make sure we don't do anything useless:
  if(!dt_control_running()) return;
  dt_lib_export_t *d = (dt_lib_export_t *)self->data;
  gtk_spin_button_set_value(d->width,   dt_conf_get_int("plugins/lighttable/export/width"));
  gtk_spin_button_set_value(d->height,  dt_conf_get_int("plugins/lighttable/export/height"));

  // Set storage
  int k = dt_conf_get_int ("plugins/lighttable/export/storage");
  gtk_combo_box_set_active(d->storage, k);

  gtk_combo_box_set_active(d->intent, (int)dt_conf_get_int("plugins/lighttable/export/iccintent") + 1);
  int iccfound = 0;
  gchar *iccprofile = dt_conf_get_string("plugins/lighttable/export/iccprofile");
  if(iccprofile)
  {
    GList *prof = d->profiles;
    while(prof)
    {
      dt_lib_export_profile_t *pp = (dt_lib_export_profile_t *)prof->data;
      if(!strcmp(pp->filename, iccprofile))
      {
        gtk_combo_box_set_active(d->profile, pp->pos);
        iccfound = 1;
      }
      if(iccfound) break;
      prof = g_list_next(prof);
    }
    g_free(iccprofile);
  }
  if(!iccfound) gtk_combo_box_set_active(d->profile, 0);
  dt_imageio_module_format_t *mformat = dt_imageio_get_format();
  if(mformat) mformat->gui_reset(mformat);
  dt_imageio_module_storage_t *mstorage = dt_imageio_get_storage();
  if(mstorage) mstorage->gui_reset(mstorage);
}

static void
set_format_by_name (dt_lib_export_t *d, const char *name)
{
  // Find index of selected format plugin among all existing plugins
  int k=-1;
  GList *it = g_list_first(darktable.imageio->plugins_format);
  if( it != NULL )
    do
    {
      k++;
      if( strcmp(  ((dt_imageio_module_format_t *)it->data)->name(),name) == 0 ||
          strcmp(  ((dt_imageio_module_format_t *)it->data)->plugin_name,name) == 0) break;
    }
    while( ( it = g_list_next(it) ) );

  // Store the new format
  dt_conf_set_int ("plugins/lighttable/export/format", k);
  it = g_list_nth(darktable.imageio->plugins_format, k);
  if(it)
  {
    dt_imageio_module_format_t *module = (dt_imageio_module_format_t *)it->data;
    GtkWidget *old = gtk_bin_get_child(GTK_BIN(d->format_box));
    if(old != module->widget)
    {
      if(old) gtk_container_remove(d->format_box, old);
      if(module->widget) gtk_container_add(d->format_box, module->widget);
    }
    gtk_widget_show_all(GTK_WIDGET(d->format_box));
    if(!darktable.gui->reset && _combo_box_set_active_text( d->format, module->name() ) == FALSE )
      gtk_combo_box_set_active( d->format, 0);
  }

  // Let's also update combination of storage/format dimension restrictions
  _update_dimensions( d );
}

static void
format_changed (GtkComboBox *widget, dt_lib_export_t *d)
{
  gchar *name = gtk_combo_box_get_active_text(d->format);
  darktable.gui->reset = 1;
  if(name) set_format_by_name (d, name);
  darktable.gui->reset = 0;
}

void _get_max_output_dimension(dt_lib_export_t *d,uint32_t *width, uint32_t *height)
{
  int k = dt_conf_get_int("plugins/lighttable/export/storage");
  dt_imageio_module_storage_t *storage = (dt_imageio_module_storage_t *)g_list_nth_data(darktable.imageio->plugins_storage, k);
  k = dt_conf_get_int("plugins/lighttable/export/format");
  dt_imageio_module_format_t *format = (dt_imageio_module_format_t *)g_list_nth_data(darktable.imageio->plugins_format, k);
  if( storage && format )
  {
    uint32_t fw,fh,sw,sh;
    fw=fh=sw=sh=0; // We are all equals!!!
    storage->dimension(storage, &sw,&sh);
    format->dimension(format, &fw,&fh);

    if( sw==0 || fw==0) *width=sw>fw?sw:fw;
    else *width=sw<fw?sw:fw;

    if( sh==0 || fh==0) *height=sh>fh?sh:fh;
    else *height=sh<fh?sh:fh;
  }
}

void _update_dimensions(dt_lib_export_t *d)
{
  uint32_t w=0,h=0;
  _get_max_output_dimension(d,&w,&h);
  gtk_spin_button_set_range( d->width,0, (w>0?w:10000) );
  gtk_spin_button_set_range( d->height,0, (h>0?h:10000) );
}

static void
set_storage_by_name (dt_lib_export_t *d, const char *name)
{
  int k=-1;
  GList *it = g_list_first(darktable.imageio->plugins_storage);
  if( it != NULL )
    do
    {
      k++;
      if( strcmp(  ((dt_imageio_module_storage_t *)it->data)->name(),name) == 0 ||
          strcmp(  ((dt_imageio_module_storage_t *)it->data)->plugin_name,name) == 0) break;
    }
    while( ( it = g_list_next(it) ) );
  if(it)
  {
    if(!darktable.gui->reset) gtk_combo_box_set_active(d->storage, k);
    dt_conf_set_int ("plugins/lighttable/export/storage", k);
    dt_imageio_module_storage_t *module = (dt_imageio_module_storage_t *)it->data;
    GtkWidget *old = gtk_bin_get_child(GTK_BIN(d->storage_box));
    if(old != module->widget)
    {
      if(old) gtk_container_remove(d->storage_box, old);
      if(module->widget) gtk_container_add(d->storage_box, module->widget);
    }

    // Check if plugin recommends a max dimension and set
    // if not implemnted the stored conf values are used..
    uint32_t w=0,h=0;
    w = dt_conf_get_int("plugins/lighttable/export/width");
    h = dt_conf_get_int("plugins/lighttable/export/height");
    module->recommended_dimension( module, &w, &h );
    // Set the recommended dimension, prevent signal changed...
    g_signal_handlers_block_by_func( d->width, width_changed, (gpointer)0 );
    g_signal_handlers_block_by_func( d->height, height_changed, (gpointer)0 );
    gtk_spin_button_set_value( d->width, w );
    gtk_spin_button_set_value( d->height, h );
    g_signal_handlers_unblock_by_func( d->width, width_changed, (gpointer)0 );
    g_signal_handlers_unblock_by_func( d->height, height_changed, (gpointer)0 );

    // Let's update formats combobox with supported formats of selected storage module...
    _update_formats_combobox( d );

    // Lets try to set selected format if fail select first in list..
    k = dt_conf_get_int("plugins/lighttable/export/format");
    GList *it = g_list_nth(darktable.imageio->plugins_format, k);
    if( it==NULL || _combo_box_set_active_text( d->format, ((dt_imageio_module_format_t *)it->data)->name() ) == FALSE )
      gtk_combo_box_set_active( d->format, 0);

    gtk_widget_show_all(GTK_WIDGET(d->storage_box));
  }
}

static void
storage_changed (GtkComboBox *widget, dt_lib_export_t *d)
{
  gchar *name = gtk_combo_box_get_active_text(d->storage);
  darktable.gui->reset = 1;
  if(name) set_storage_by_name (d, name);
  darktable.gui->reset = 0;
}

static void
profile_changed (GtkComboBox *widget, dt_lib_export_t *d)
{
  int pos = gtk_combo_box_get_active(widget);
  GList *prof = d->profiles;
  while(prof)
  {
    // could use g_list_nth. this seems safer?
    dt_lib_export_profile_t *pp = (dt_lib_export_profile_t *)prof->data;
    if(pp->pos == pos)
    {
      dt_conf_set_string("plugins/lighttable/export/iccprofile", pp->filename);
      return;
    }
    prof = g_list_next(prof);
  }
  dt_conf_set_string("plugins/lighttable/export/iccprofile", "image");
}

static void
intent_changed (GtkComboBox *widget, dt_lib_export_t *d)
{
  int pos = gtk_combo_box_get_active(widget);
  dt_conf_set_int("plugins/lighttable/export/iccintent", pos-1);
}

int
position ()
{
  return 0;
}

gboolean _combo_box_set_active_text(GtkComboBox *cb, const gchar *text)
{
  g_assert( text!=NULL );
  g_assert( cb!=NULL );
  GtkTreeModel *model=gtk_combo_box_get_model(cb);
  GtkTreeIter iter;
  if( gtk_tree_model_get_iter_first( model, &iter ) )
  {
    int k=-1;
    do
    {
      k++;
      GValue value = { 0, };
      gtk_tree_model_get_value(model,&iter,0,&value);
      gchar *v=NULL;
      if( G_VALUE_HOLDS_STRING(&value) && (  v=(gchar *)g_value_get_string(&value) ) != NULL
        )
      {
        if( strcmp( v, text) == 0 )
        {
          gtk_combo_box_set_active( cb, k);
          return TRUE;
        }
      }
    }
    while( gtk_tree_model_iter_next( model, &iter ) );
  }
  return FALSE;
}

void _update_formats_combobox(dt_lib_export_t *d)
{
  // Clear format combo box
  gtk_list_store_clear( GTK_LIST_STORE(gtk_combo_box_get_model(d->format) ) );

  // Get current selected storage
  int k = dt_conf_get_int("plugins/lighttable/export/storage");
  dt_imageio_module_storage_t *storage = (dt_imageio_module_storage_t *)g_list_nth_data(darktable.imageio->plugins_storage, k);

  // Add supported formats to combobox
  GList *it = darktable.imageio->plugins_format;
  while(it)
  {
    dt_imageio_module_format_t *format = (dt_imageio_module_format_t *)it->data;
    if( storage->supported( storage, format ) )
      gtk_combo_box_append_text(d->format, format->name());

    it = g_list_next(it);
  }
}

/*
static gboolean
focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
  dt_control_tab_shortcut_off(darktable.control);
  return FALSE;
}

static gboolean
focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
  dt_control_tab_shortcut_on(darktable.control);
  return FALSE;
}
*/

void
gui_init (dt_lib_module_t *self)
{
  dt_lib_export_t *d = (dt_lib_export_t *)malloc(sizeof(dt_lib_export_t));
  self->data = (void *)d;
  self->widget = gtk_table_new(8, 2, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(self->widget), 5);

  GtkWidget *label;

  label = dtgtk_label_new(_("target storage"), DARKTABLE_LABEL_TAB | DARKTABLE_LABEL_ALIGN_RIGHT);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 2, 0, 1, GTK_FILL|GTK_EXPAND, 0, 0, 0);
  d->storage = GTK_COMBO_BOX(gtk_combo_box_new_text());
  GList *it = darktable.imageio->plugins_storage;
  while(it)
  {
    dt_imageio_module_storage_t *module = (dt_imageio_module_storage_t *)it->data;
    gtk_combo_box_append_text(d->storage, module->name());
    it = g_list_next(it);
  }
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->storage), 0, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  g_signal_connect (G_OBJECT (d->storage), "changed",
                    G_CALLBACK (storage_changed),
                    (gpointer)d);

  d->storage_box = GTK_CONTAINER(gtk_alignment_new(1.0, 1.0, 1.0, 1.0));
  gtk_alignment_set_padding(GTK_ALIGNMENT(d->storage_box), 0, 0, 0, 0);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->storage_box), 0, 2, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  label = dtgtk_label_new(_("file format"), DARKTABLE_LABEL_TAB | DARKTABLE_LABEL_ALIGN_RIGHT);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_set_row_spacing(GTK_TABLE(self->widget), 2, 20);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 2, 3, 4, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->format = GTK_COMBO_BOX(gtk_combo_box_new_text());

  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->format), 0, 2, 4, 5, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  g_signal_connect (G_OBJECT (d->format), "changed",
                    G_CALLBACK (format_changed),
                    (gpointer)d);

  d->format_box = GTK_CONTAINER(gtk_alignment_new(1.0, 1.0, 1.0, 1.0));
  gtk_alignment_set_padding(GTK_ALIGNMENT(d->format_box), 0, 0, 0, 0);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->format_box), 0, 2, 5, 6, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  label = dtgtk_label_new(_("global options"), DARKTABLE_LABEL_TAB | DARKTABLE_LABEL_ALIGN_RIGHT);
  gtk_table_set_row_spacing(GTK_TABLE(self->widget), 5, 20);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 2, 6, 7, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->width  = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 10000, 1));
  g_object_set(G_OBJECT(d->width), "tooltip-text", _("maximum output width\nset to 0 for no scaling"), (char *)NULL);
  d->height = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 10000, 1));
  g_object_set(G_OBJECT(d->height), "tooltip-text", _("maximum output height\nset to 0 for no scaling"), (char *)NULL);

  dt_gui_key_accel_block_on_focus (GTK_WIDGET (d->width));
  dt_gui_key_accel_block_on_focus (GTK_WIDGET (d->height));
  /*
    gtk_widget_add_events(GTK_WIDGET(d->width), GDK_FOCUS_CHANGE_MASK);
    g_signal_connect (G_OBJECT (d->width), "focus-in-event",  G_CALLBACK(focus_in),  NULL);
    g_signal_connect (G_OBJECT (d->width), "focus-out-event", G_CALLBACK(focus_out), NULL);
    gtk_widget_add_events(GTK_WIDGET(d->height), GDK_FOCUS_CHANGE_MASK);
    g_signal_connect (G_OBJECT (d->height), "focus-in-event",  G_CALLBACK(focus_in),  NULL);
    g_signal_connect (G_OBJECT (d->height), "focus-out-event", G_CALLBACK(focus_out), NULL);
    */
  label = gtk_label_new(_("max size"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 7, 8, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  GtkBox *hbox = GTK_BOX(gtk_hbox_new(FALSE, 5));
  gtk_box_pack_start(hbox, GTK_WIDGET(d->width), TRUE, TRUE, 0);
  gtk_box_pack_start(hbox, gtk_label_new(_("x")), FALSE, FALSE, 0);
  gtk_box_pack_start(hbox, GTK_WIDGET(d->height), TRUE, TRUE, 0);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(hbox), 1, 2, 7, 8, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  label = gtk_label_new(_("intent"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 8, 9, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->intent = GTK_COMBO_BOX(gtk_combo_box_new_text());
  gtk_combo_box_append_text(d->intent, _("image settings"));
  gtk_combo_box_append_text(d->intent, _("perceptual"));
  gtk_combo_box_append_text(d->intent, _("relative colorimetric"));
  gtk_combo_box_append_text(d->intent, C_("rendering intent", "saturation"));
  gtk_combo_box_append_text(d->intent, _("absolute colorimetric"));
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->intent), 1, 2, 8, 9, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  d->profiles = NULL;

  dt_lib_export_profile_t *prof = (dt_lib_export_profile_t *)g_malloc0(sizeof(dt_lib_export_profile_t));
  g_strlcpy(prof->filename, "sRGB", sizeof(prof->filename));
  g_strlcpy(prof->name, _("sRGB (web-safe)"), sizeof(prof->name));
  int pos;
  prof->pos = 1;
  d->profiles = g_list_append(d->profiles, prof);

  prof = (dt_lib_export_profile_t *)g_malloc0(sizeof(dt_lib_export_profile_t));
  g_strlcpy(prof->filename, "adobergb", sizeof(prof->filename));
  g_strlcpy(prof->name, _("Adobe RGB"), sizeof(prof->name));
  prof->pos = 2;
  d->profiles = g_list_append(d->profiles, prof);

  prof = (dt_lib_export_profile_t *)g_malloc0(sizeof(dt_lib_export_profile_t));
  g_strlcpy(prof->filename, "X profile", sizeof(prof->filename));
  g_strlcpy(prof->name, "X profile", sizeof(prof->name));
  prof->pos = 3;
  d->profiles = g_list_append(d->profiles, prof);

  prof = (dt_lib_export_profile_t *)g_malloc0(sizeof(dt_lib_export_profile_t));
  g_strlcpy(prof->filename, "linear_rgb", sizeof(prof->filename));
  g_strlcpy(prof->name, _("linear RGB"), sizeof(prof->name));
  pos = prof->pos = 4;
  d->profiles = g_list_append(d->profiles, prof);

  // read datadir/color/out/*.icc
  char datadir[1024], confdir[1024], dirname[1024], filename[1024];
  dt_util_get_user_config_dir(confdir, 1024);
  dt_util_get_datadir(datadir, 1024);
  cmsHPROFILE tmpprof;
  const gchar *d_name;
  snprintf(dirname, 1024, "%s/color/out", confdir);
  if(!g_file_test(dirname, G_FILE_TEST_IS_DIR))
    snprintf(dirname, 1024, "%s/color/out", datadir);
  GDir *dir = g_dir_open(dirname, 0, NULL);
  if(dir)
  {
    while((d_name = g_dir_read_name(dir)))
    {
      snprintf(filename, 1024, "%s/%s", dirname, d_name);
      tmpprof = cmsOpenProfileFromFile(filename, "r");
      if(tmpprof)
      {
	char *lang = getenv("LANG");
	if (!lang) lang = "en_US";

        dt_lib_export_profile_t *prof = (dt_lib_export_profile_t *)g_malloc0(sizeof(dt_lib_export_profile_t));
        char name[1024];
        cmsGetProfileInfoASCII(tmpprof, cmsInfoDescription, lang, lang+3, name, 1024);
        g_strlcpy(prof->name, name, sizeof(prof->name));
        g_strlcpy(prof->filename, d_name, sizeof(prof->filename));
        prof->pos = ++pos;
        cmsCloseProfile(tmpprof);
        d->profiles = g_list_append(d->profiles, prof);
      }
    }
    g_dir_close(dir);
  }
  GList *l = d->profiles;
  label = gtk_label_new(_("profile"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(self->widget), label, 0, 1, 9, 10, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  d->profile = GTK_COMBO_BOX(gtk_combo_box_new_text());
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->profile), 1, 2, 9, 10, GTK_SHRINK|GTK_EXPAND|GTK_FILL, 0, 0, 0);
  // gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->profile), 1, 2, 9, 10, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  gtk_combo_box_append_text(d->profile, _("image settings"));
  while(l)
  {
    dt_lib_export_profile_t *prof = (dt_lib_export_profile_t *)l->data;
    if(!strcmp(prof->name, "X profile"))
      gtk_combo_box_append_text(d->profile, _("system display profile"));
    else
      gtk_combo_box_append_text(d->profile, prof->name);
    l = g_list_next(l);
  }

#if 0
  // set ellipsisation:
  GList *renderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(d->profile));
  it = renderers;
  while(it)
  {
    GtkCellRendererText *tr = GTK_CELL_RENDERER_TEXT(it->data);
    g_object_set(G_OBJECT(tr), "ellipsize", PANGO_ELLIPSIZE_MIDDLE, (char *)NULL);
    it = g_list_next(it);
  }
  g_list_free(renderers);
#endif
  gtk_widget_set_size_request(GTK_WIDGET(d->profile), 200, -1);

  gtk_combo_box_set_active(d->profile, 0);
  char tooltip[1024];
  snprintf(tooltip, 1024, _("output icc profiles in %s/color/out or %s/color/out"), confdir, datadir);
  g_object_set(G_OBJECT(d->profile), "tooltip-text", tooltip, (char *)NULL);
  g_signal_connect (G_OBJECT (d->intent), "changed",
                    G_CALLBACK (intent_changed),
                    (gpointer)d);
  g_signal_connect (G_OBJECT (d->profile), "changed",
                    G_CALLBACK (profile_changed),
                    (gpointer)d);

  GtkButton *button = GTK_BUTTON(gtk_button_new_with_label(_("export")));
  d->export_button = button;
  g_object_set(G_OBJECT(button), "tooltip-text", _("export with current settings (ctrl-e)"), (char *)NULL);
  gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(button), 1, 2, 10, 11, GTK_EXPAND|GTK_FILL, 0, 0, 0);

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (export_button_clicked),
                    (gpointer)self);
  g_signal_connect (G_OBJECT (d->width), "value-changed",
                    G_CALLBACK (width_changed),
                    (gpointer)0);
  g_signal_connect (G_OBJECT (d->height), "value-changed",
                    G_CALLBACK (height_changed),
                    (gpointer)0);

  self->gui_reset(self);
}

void
gui_cleanup (dt_lib_module_t *self)
{
  dt_lib_export_t *d = (dt_lib_export_t *)self->data;
  GtkWidget *old = gtk_bin_get_child(GTK_BIN(d->format_box));
  if(old) gtk_container_remove(d->format_box, old);
  old = gtk_bin_get_child(GTK_BIN(d->storage_box));
  if(old) gtk_container_remove(d->storage_box, old);
  while(d->profiles)
  {
    g_free(d->profiles->data);
    d->profiles = g_list_delete_link(d->profiles, d->profiles);
  }
  free(self->data);
  self->data = NULL;
}

void
init_presets (dt_lib_module_t *self)
{
  // TODO: store presets in db:
  // dt_lib_presets_add(const char *name, const char *plugin_name, const void *params, const int32_t params_size)
}

void*
get_params (dt_lib_module_t *self, int *size)
{
  // concat storage and format, size is max + header
  dt_imageio_module_format_t  *mformat  = dt_imageio_get_format();
  dt_imageio_module_storage_t *mstorage = dt_imageio_get_storage();
  if(!mformat || !mstorage) return NULL;

  int32_t fsize = 0, ssize = 0;
  // size will be only as large as to remove random pointers from params (stored at the end).
  dt_imageio_module_data_t *fdata = mformat->get_params(mformat, &fsize);
  void *sdata = mstorage->get_params(mstorage, &ssize);
  // we allow null pointers (plugin not ready for export in current state), and just dont copy back the settings later:
  if(!sdata) ssize = 0;
  if(!fdata) fsize = 0;
  if(fdata)
  {
    // clean up format global params:
    fdata->max_width  = 0;
    fdata->max_height = 0;
    fdata->width  = 0;
    fdata->height = 0;
  }

  // FIXME: also the web preset has to be applied twice to be known as preset! (other dimension magic going on here?)
  // TODO: get this stuff from gui and not from conf, so it will be sanity-checked (you can never delete an insane preset)?
  // also store icc profile/intent here.
  int32_t iccintent = dt_conf_get_int("plugins/lighttable/export/iccintent");
  int32_t max_width  = dt_conf_get_int ("plugins/lighttable/export/width");
  int32_t max_height = dt_conf_get_int ("plugins/lighttable/export/height");
  gchar *iccprofile = dt_conf_get_string("plugins/lighttable/export/iccprofile");
  if(!iccprofile)
  {
    iccprofile = (char *)g_malloc(1);
    iccprofile[0] = '\0';
  }

  char *fname = mformat->plugin_name, *sname = mstorage->plugin_name;
  int32_t fname_len = strlen(fname), sname_len = strlen(sname);
  *size = fname_len + sname_len + 2 + 2*sizeof(int32_t) + fsize + ssize + 3*sizeof(int32_t) + strlen(iccprofile) + 1;

  char *params = (char *)malloc(*size);
  memset(params, 0, *size);
  int pos = 0;
  memcpy(params+pos, &max_width, sizeof(int32_t));
  pos += sizeof(int32_t);
  memcpy(params+pos, &max_height, sizeof(int32_t));
  pos += sizeof(int32_t);
  memcpy(params+pos, &iccintent, sizeof(int32_t));
  pos += sizeof(int32_t);
  memcpy(params+pos, iccprofile, strlen(iccprofile)+1);
  pos += strlen(iccprofile) + 1;
  memcpy(params+pos, fname, fname_len+1);
  pos += fname_len+1;
  memcpy(params+pos, sname, sname_len+1);
  pos += sname_len+1;
  memcpy(params+pos, &fsize, sizeof(int32_t));
  pos += sizeof(int32_t);
  memcpy(params+pos, &ssize, sizeof(int32_t));
  pos += sizeof(int32_t);
  if(fdata != NULL) // otherwise fsize == 0, but clang doesn't like it ...
  {
    memcpy(params+pos, fdata, fsize);
    pos += fsize;
  }
  if(sdata != NULL) // see above
  {
    memcpy(params+pos, sdata, ssize);
    pos += ssize;
  }
  g_assert(pos == *size);

  g_free(iccprofile);

  if(fdata) mformat->free_params(mformat, fdata);
  if(sdata) mstorage->free_params(mstorage, sdata);
  return params;
}

int
set_params (dt_lib_module_t *self, const void *params, int size)
{
  dt_lib_export_t *d = (dt_lib_export_t *)self->data;
  // apply these stored presets again (parse blob)
  const char *buf = (const char* )params;

  const int max_width  = *(const int *)buf;
  buf += sizeof(int32_t);
  const int max_height = *(const int *)buf;
  buf += sizeof(int32_t);
  const int iccintent  = *(const int *)buf;
  buf += sizeof(int32_t);
  const char *iccprofile = buf;
  buf += strlen(iccprofile) + 1;

  // reverse these by setting the gui, not the conf vars!
  gtk_combo_box_set_active (d->intent, iccintent + 1);
  if(!strcmp(iccprofile, "image"))
  {
    gtk_combo_box_set_active(d->profile, 0);
  }
  else
  {
    GList *prof = d->profiles;
    while(prof)
    {
      dt_lib_export_profile_t *pp = (dt_lib_export_profile_t *)prof->data;
      if(!strcmp(pp->filename, iccprofile))
      {
        gtk_combo_box_set_active(d->profile, pp->pos);
        break;
      }
      prof = g_list_next(prof);
    }
  }

  // parse both names to '\0'
  const char *fname = buf;
  buf += strlen(fname) + 1;
  const char *sname = buf;
  buf += strlen(sname) + 1;

  // get module by name and fail if not there.
  dt_imageio_module_format_t *fmod = dt_imageio_get_format_by_name(fname);
  dt_imageio_module_storage_t *smod = dt_imageio_get_storage_by_name(sname);
  if(!fmod || !smod) return 1;

  const int fsize = *(const int *)buf;
  buf += sizeof(int32_t);
  const int ssize = *(const int *)buf;
  buf += sizeof(int32_t);

  if(size != strlen(fname) + strlen(sname) + 2 + 2*sizeof(int32_t) + fsize + ssize + 3*sizeof(int32_t) + strlen(iccprofile) + 1) return 1;

  const dt_imageio_module_data_t *fdata = (const dt_imageio_module_data_t *)buf;
  buf += fsize;
  const void *sdata = buf;

  // switch modules
  set_storage_by_name(d, sname);
  set_format_by_name(d, fname);

  // set dimensions after switching, to have new range ready.
  gtk_spin_button_set_value(d->width,  max_width);
  gtk_spin_button_set_value(d->height, max_height);

  // propagate to modules
  int res = 0;
  if(ssize) res += smod->set_params(smod, sdata, ssize);
  if(fsize) res += fmod->set_params(fmod, fdata, fsize);
  return res;
}

void init_key_accels(dt_lib_module_t *self)
{
  dt_accel_register_lib(self, NC_("accel", "export"),
                        GDK_e, GDK_CONTROL_MASK);
}

void connect_key_accels(dt_lib_module_t *self)
{
  dt_lib_export_t *d = (dt_lib_export_t*)self->data;

  dt_accel_connect_button_lib(self, "export", GTK_WIDGET(d->export_button));
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
