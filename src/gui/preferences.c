/*
    This file is part of darktable,
    copyright (c) 2009--2011 johannes hanika, Tobias Ellinghaus.

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

#include <gdk/gdkkeysyms.h>

#include "common/darktable.h"
#include "common/debug.h"
#include "control/control.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include "gui/preferences.h"
#include "develop/imageop.h"
#include "libs/lib.h"
#include "preferences_gen.h"

#define ICON_SIZE 13

typedef struct dt_gui_presets_edit_dialog_t
{
  GtkTreeView *tree; // CHANGED!
  gint rowid; // CHANGED!
  GtkLabel *name;
  GtkEntry *description;
  GtkCheckButton *autoapply, *filter;
  GtkBox *details;
  GtkEntry *model, *maker, *lens;
  GtkSpinButton *iso_min, *iso_max;
  GtkComboBox *exposure_min, *exposure_max;
  GtkComboBox *aperture_min, *aperture_max;
  GtkSpinButton *focal_length_min, *focal_length_max;
}
dt_gui_presets_edit_dialog_t;

// FIXME: this is copypasta from gui/presets.c. better put these somewhere so that all places can access the same data.
static const int   dt_gui_presets_exposure_value_cnt = 24;
static const float dt_gui_presets_exposure_value[] =
{
  0., 1./8000, 1./4000, 1./2000, 1./1000, 1./1000, 1./500, 1./250, 1./125, 1./60, 1./30, 1./15,
  1./15, 1./8, 1./4, 1./2, 1, 2, 4, 8, 15, 30, 60, FLT_MAX
};
static const char* dt_gui_presets_exposure_value_str[] =
{
  "0", "1/8000", "1/4000", "1/2000", "1/1000", "1/1000", "1/500", "1/250", "1/125", "1/60", "1/30", "1/15",
  "1/15", "1/8", "1/4", "1/2", "1\"", "2\"", "4\"", "8\"", "15\"", "30\"", "60\"", "+"
};
static const int   dt_gui_presets_aperture_value_cnt = 19;
static const float dt_gui_presets_aperture_value[] = {0, 0.5, 0.7, 1.0, 1.4, 2.0, 2.8, 4.0, 5.6, 8.0, 11.0, 16.0,
    22.0, 32.0, 45.0, 64.0, 90.0, 128.0, FLT_MAX
                                                     };
static const char* dt_gui_presets_aperture_value_str[] = {"f/0", "f/0.5", "f/0.7", "f/1.0", "f/1.4", "f/2",
    "f/2.8", "f/4", "f/5.6", "f/8", "f/11", "f/16", "f/22", "f/32", "f/45", "f/64", "f/90", "f/128", "f/+"
                                                         };

// Values for the accelerators/presets treeview

enum {A_ACCEL_COLUMN, A_BINDING_COLUMN, A_TRANS_COLUMN, A_N_COLUMNS};
enum {P_ROWID_COLUMN, P_OPERATION_COLUMN, P_MODULE_COLUMN, P_EDITABLE_COLUMN, P_NAME_COLUMN, P_MODEL_COLUMN, P_MAKER_COLUMN, P_LENS_COLUMN, P_ISO_COLUMN, P_EXPOSURE_COLUMN, P_APERTURE_COLUMN, P_FOCAL_LENGTH_COLUMN, P_AUTOAPPLY_COLUMN, P_N_COLUMNS};

static void init_tab_presets(GtkWidget *book);
static void init_tab_accels(GtkWidget *book);
static void tree_insert_accel(gpointer accel_struct, gpointer model_link);
static void tree_insert_rec(GtkTreeStore *model, GtkTreeIter *parent,
                            const gchar *accel_path,
                            const gchar *translated_path,
                            guint accel_key, GdkModifierType accel_mods);
static void path_to_accel(GtkTreeModel *model, GtkTreePath *path, gchar *str);
static void update_accels_model(gpointer widget, gpointer data);
static void update_accels_model_rec(GtkTreeModel *model, GtkTreeIter *parent,
                                    gchar *path);
static void delete_matching_accels(gpointer path, gpointer key_event);
static void import_export(GtkButton *button, gpointer data);
static void restore_defaults(GtkButton *button, gpointer data);
static gint compare_rows_accels(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
                         gpointer data);
static gint compare_rows_presets(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
                         gpointer data);

// Signal handlers
static void tree_row_activated_accels(GtkTreeView *tree, GtkTreePath *path,
                               GtkTreeViewColumn *column, gpointer data);
static void tree_row_activated_presets(GtkTreeView *tree, GtkTreePath *path,
                               GtkTreeViewColumn *column, gpointer data);
static void tree_selection_changed(GtkTreeSelection *selection, gpointer data);
static gboolean tree_key_press(GtkWidget *widget, GdkEventKey *event,
                               gpointer data);
static gboolean prefix_search(GtkTreeModel *model, gint column,
                              const gchar *key, GtkTreeIter *iter, gpointer d);

static void edit_preset (GtkTreeView * tree, const gint rowid, const gchar * name, const gchar * module);
static void edit_preset_response(GtkDialog *dialog, gint response_id, dt_gui_presets_edit_dialog_t *g);

static GtkWidget *_preferences_dialog;

/*
static GList *language_codes = NULL;
static gint sys_default = -1;

static void language_callback(GtkWidget *widget, gpointer user_data)
{
  dt_conf_set_string("ui_last/gui_language", (gchar*)g_list_nth(language_codes, gtk_combo_box_get_active(GTK_COMBO_BOX(widget)))->data);
}

static gboolean reset_language_widget(GtkWidget *label, GdkEventButton *event, GtkWidget *widget)
{
  if(event->type == GDK_2BUTTON_PRESS)
  {
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), sys_default+1);
    return TRUE;
  }
  return FALSE;
}

// TODO: sort the list of languages
static void hardcoded_gui(GtkWidget *vbox1, GtkWidget *vbox2)
{
  GtkWidget *label, *widget, *labelev;
  GDir *dir;
  const gchar *filename;
  GList *languages = NULL, *iter;
  const gchar * const * names = g_get_language_names();
  gint selected = -1;
  sys_default = -1;
  gboolean store_codes = (language_codes == NULL);
  gchar *ui_lang = dt_conf_get_string("ui_last/gui_language");
  dir = g_dir_open(DARKTABLE_LOCALEDIR, 0, NULL);

  if(store_codes)
    language_codes = g_list_append(language_codes, g_strdup("C"));

  if(dir)
  {
    int i = -1;
    while((filename = g_dir_read_name(dir)))
    {
      gchar *testname =g_build_filename(DARKTABLE_LOCALEDIR, filename, NULL);
      if(g_file_test(testname, G_FILE_TEST_IS_DIR))
      {
        gchar *entry = NULL;
        i++;
        if(sys_default == -1)
        {
          // check if this is the system default
          const gchar * const * n = names;
          while(*n)
          {
            if(g_strcmp0(*n, filename) == 0)
            {
              sys_default = i;
              entry = g_strconcat(filename, " (", _("system default"), ")", NULL);
              break;
            }
            n++;
          }
        }
        if(g_strcmp0(ui_lang, filename) == 0)
          selected = i;
        languages = g_list_append(languages, entry?entry:g_strdup(filename));
        if(store_codes)
          language_codes = g_list_append(language_codes, g_strdup(filename));
      }
      g_free(testname);
    }
    g_dir_close(dir) ;
  }
  if(selected == -1 && g_strcmp0(ui_lang, "C") != 0)
    selected = sys_default;

  label = gtk_label_new(_("interface language"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  labelev = gtk_event_box_new();
  gtk_widget_add_events(labelev, GDK_BUTTON_PRESS_MASK);
  gtk_container_add(GTK_CONTAINER(labelev), label);
  widget = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), _("english"));

  if((iter = g_list_first(languages)) != NULL)
  {
    do
    {
      gtk_combo_box_append_text(GTK_COMBO_BOX(widget), iter->data);
      g_free(iter->data);
    }
    while((iter=g_list_next(iter)) != NULL);
    g_list_free(languages);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(widget), selected + 1);
  g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(language_callback), 0);
  gtk_object_set(GTK_OBJECT(labelev),  "tooltip-text", _("double click to reset to the system language"), (char *)NULL);
  gtk_event_box_set_visible_window(GTK_EVENT_BOX(labelev), FALSE);
  gtk_object_set(GTK_OBJECT(widget), "tooltip-text", _("set the language of the user interface (needs a restart)"), (char *)NULL);
  gtk_box_pack_start(GTK_BOX(vbox1), labelev, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(labelev), "button-press-event", G_CALLBACK(reset_language_widget), (gpointer)widget);
}
*/

void dt_gui_preferences_show()
{
  GtkWidget *win = dt_ui_main_window(darktable.gui->ui);
  _preferences_dialog = gtk_dialog_new_with_buttons(
      _("darktable preferences"), GTK_WINDOW (win),
      GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL,
      _("close"),
      GTK_RESPONSE_ACCEPT,
      NULL);
  gtk_window_set_position(GTK_WINDOW(_preferences_dialog), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_resize(GTK_WINDOW(_preferences_dialog), 600, 300);
  GtkWidget *content = gtk_dialog_get_content_area (GTK_DIALOG (_preferences_dialog));
  GtkWidget *notebook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(content), notebook, TRUE, TRUE, 0);

  // Make sure remap mode is off initially
  darktable.control->accel_remap_str = NULL;
  darktable.control->accel_remap_path = NULL;

//   init_tab_gui(notebook, &hardcoded_gui);
  init_tab_gui(notebook, NULL);
  init_tab_core(notebook, NULL);
  init_tab_accels(notebook);
  init_tab_presets(notebook);
  gtk_widget_show_all(_preferences_dialog);
  (void) gtk_dialog_run(GTK_DIALOG(_preferences_dialog));
  gtk_widget_destroy(_preferences_dialog);

  // Cleaning up any memory still allocated for remapping
  if(darktable.control->accel_remap_path)
  {
    gtk_tree_path_free(darktable.control->accel_remap_path);
    darktable.control->accel_remap_path = NULL;
  }

}

static void tree_insert_presets(GtkTreeStore *tree_model){
  GtkTreeIter iter, parent;
  sqlite3_stmt *stmt;
  gchar *last_module = NULL;

  // Create a GdkPixbuf with a cairo drawing.
  // WARNING: In general, this doesn't work as the pixel format of GDK and cairo differs. But in our case with a monochrome icon that doesn't matter. :-)
  // lock
  cairo_surface_t *lock_cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ICON_SIZE, ICON_SIZE);
  cairo_t *lock_cr = cairo_create(lock_cst);
  cairo_set_source_rgb(lock_cr, 0.7,0.7,0.7);
  dtgtk_cairo_paint_lock(lock_cr, 0, 0, ICON_SIZE, ICON_SIZE, 0);
  guchar* data = cairo_image_surface_get_data(lock_cst);
  GdkPixbuf* lock_pixbuf = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, TRUE, 8, ICON_SIZE,
                                                    ICON_SIZE, cairo_image_surface_get_stride(lock_cst), NULL, NULL);
  // check mark
  cairo_surface_t *check_cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ICON_SIZE, ICON_SIZE);
  cairo_t *check_cr = cairo_create(check_cst);
  cairo_set_source_rgb(check_cr, 0.7,0.7,0.7);
  dtgtk_cairo_paint_check_mark(check_cr, 0, 0, ICON_SIZE, ICON_SIZE, 0);
  data = cairo_image_surface_get_data(check_cst);
  GdkPixbuf* check_pixbuf = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, TRUE, 8, ICON_SIZE,
                                                    ICON_SIZE, cairo_image_surface_get_stride(check_cst), NULL, NULL);

  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select rowid, name, operation, autoapply, model, maker, lens, iso_min, iso_max, exposure_min, exposure_max, aperture_min, aperture_max, focal_length_min, focal_length_max, writeprotect from presets order by operation,name", -1, &stmt, NULL);
  while(sqlite3_step(stmt) == SQLITE_ROW)
  {
    gint rowid                  = sqlite3_column_int(stmt, 0);
    gchar* name                 = (gchar*) sqlite3_column_text(stmt, 1);
    gchar* operation            = (gchar*) sqlite3_column_text(stmt, 2);
    const gboolean autoapply    = (sqlite3_column_int(stmt, 3)==0 ? FALSE : TRUE);
    gchar* model                = (gchar*) sqlite3_column_text(stmt, 4);
    gchar* maker                = (gchar*) sqlite3_column_text(stmt, 5);
    gchar* lens                 = (gchar*) sqlite3_column_text(stmt, 6);
    int iso_min                 = sqlite3_column_double(stmt, 7);
    int iso_max                 = sqlite3_column_double(stmt, 8);
    float exposure_min          = sqlite3_column_double(stmt, 9);
    float exposure_max          = sqlite3_column_double(stmt, 10);
    float aperture_min          = sqlite3_column_double(stmt, 11);
    float aperture_max          = sqlite3_column_double(stmt, 12);
    int focal_length_min        = sqlite3_column_double(stmt, 13);
    int focal_length_max        = sqlite3_column_double(stmt, 14);
    const gboolean writeprotect = (sqlite3_column_int(stmt, 15)==0 ? FALSE : TRUE);

    gchar *iso, *exposure, *aperture, *focal_length;
    int min, max;

    gchar *module             = dt_iop_get_localized_name(operation);
    if(module == NULL) module = dt_lib_get_localized_name(operation);
    if(module == NULL) module = g_strdup(operation);

    if(iso_min == 0.0 && iso_max == 51200.0)
      iso = g_strdup("%");
    else
      iso = g_strdup_printf("%d – %d", iso_min, iso_max);

    min=0, max=0;
    for(; min<dt_gui_presets_exposure_value_cnt&&exposure_min>dt_gui_presets_exposure_value[min]; min++);
    for(; max<dt_gui_presets_exposure_value_cnt&&exposure_max>dt_gui_presets_exposure_value[max]; max++);
    if(min == 0 && max == dt_gui_presets_exposure_value_cnt-1)
      exposure = g_strdup("%");
    else
      exposure = g_strdup_printf("%s – %s", dt_gui_presets_exposure_value_str[min], dt_gui_presets_exposure_value_str[max]);

    min=0, max=0;
    for(; min<dt_gui_presets_aperture_value_cnt&&aperture_min>dt_gui_presets_aperture_value[min]; min++);
    for(; max<dt_gui_presets_aperture_value_cnt&&aperture_max>dt_gui_presets_aperture_value[max]; max++);
    if(min == 0 && max == dt_gui_presets_aperture_value_cnt-1)
      aperture = g_strdup("%");
    else
      aperture = g_strdup_printf("%s – %s", dt_gui_presets_aperture_value_str[min], dt_gui_presets_aperture_value_str[max]);

    if(focal_length_min == 0.0 && focal_length_max == 1000.0)
      focal_length = g_strdup("%");
    else
      focal_length = g_strdup_printf("%d – %d", focal_length_min, focal_length_max);

    if(g_strcmp0(last_module, operation) != 0)
    {
      gtk_tree_store_append(tree_model, &iter, NULL);
      gtk_tree_store_set(tree_model, &iter,
                         P_ROWID_COLUMN, 0,
                         P_OPERATION_COLUMN, "",
                         P_MODULE_COLUMN, _(module),
                         P_EDITABLE_COLUMN, NULL,
                         P_NAME_COLUMN, "",
                         P_MODEL_COLUMN, "",
                         P_MAKER_COLUMN, "",
                         P_LENS_COLUMN, "",
                         P_ISO_COLUMN, "",
                         P_EXPOSURE_COLUMN, "",
                         P_APERTURE_COLUMN, "",
                         P_FOCAL_LENGTH_COLUMN, "",
                         P_AUTOAPPLY_COLUMN, NULL,
                         -1);
      g_free(last_module);
      last_module = g_strdup(operation);
      parent = iter;
    }

    gtk_tree_store_append(tree_model, &iter, &parent);
    gtk_tree_store_set(tree_model, &iter,
                         P_ROWID_COLUMN, rowid,
                         P_OPERATION_COLUMN, operation,
                         P_MODULE_COLUMN, "",
                         P_EDITABLE_COLUMN, writeprotect?lock_pixbuf:NULL,
                         P_NAME_COLUMN, name,
                         P_MODEL_COLUMN, model,
                         P_MAKER_COLUMN, maker,
                         P_LENS_COLUMN, lens,
                         P_ISO_COLUMN, iso,
                         P_EXPOSURE_COLUMN, exposure,
                         P_APERTURE_COLUMN, aperture,
                         P_FOCAL_LENGTH_COLUMN, focal_length,
                         P_AUTOAPPLY_COLUMN, autoapply?check_pixbuf:NULL,
                      -1);

  }
  sqlite3_finalize(stmt);

  g_object_unref(lock_pixbuf);
  cairo_surface_destroy(lock_cst);
  g_object_unref(check_pixbuf);
  cairo_surface_destroy(check_cst);
}

static void init_tab_presets(GtkWidget *book)
{
  GtkWidget *alignment = gtk_alignment_new(0.5, 0.0, 0.9, 1.0);
  GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
  GtkWidget *tree = gtk_tree_view_new();
  GtkTreeStore *model = gtk_tree_store_new(P_N_COLUMNS, G_TYPE_INT /*rowid*/, G_TYPE_STRING /*operation*/,
                                           G_TYPE_STRING /*module*/, GDK_TYPE_PIXBUF /*editable*/, G_TYPE_STRING /*name*/,
                                           G_TYPE_STRING /*model*/, G_TYPE_STRING /*maker*/, G_TYPE_STRING /*lens*/,
                                           G_TYPE_STRING /*iso*/, G_TYPE_STRING /*exposure*/, G_TYPE_STRING /*aperture*/, G_TYPE_STRING /*focal length*/,
                                           GDK_TYPE_PIXBUF /*auto*/);
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  // Adding the outer container
  gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 20, 20, 20, 20);
  gtk_container_add(GTK_CONTAINER(alignment), scroll);
  gtk_notebook_append_page(GTK_NOTEBOOK(book), alignment,
                           gtk_label_new(_("presets")));

  tree_insert_presets(model);

  // Seting a custom sort functions so expandable groups rise to the top
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), P_MODULE_COLUMN, GTK_SORT_ASCENDING);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(model), P_MODULE_COLUMN,
                                  compare_rows_presets, NULL, NULL);

  // Setting up the cell renderers
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("module"), renderer,
      "text", P_MODULE_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_pixbuf_new();
  column = gtk_tree_view_column_new_with_attributes(
      "", renderer,
      "pixbuf", P_EDITABLE_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("name"), renderer,
      "text", P_NAME_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("model"), renderer,
      "text", P_MODEL_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("maker"), renderer,
      "text", P_MAKER_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("lens"), renderer,
      "text", P_LENS_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("iso"), renderer,
      "text", P_ISO_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("exposure"), renderer,
      "text", P_EXPOSURE_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("aperture"), renderer,
      "text", P_APERTURE_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("focal length"), renderer,
      "text", P_FOCAL_LENGTH_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_pixbuf_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("auto"), renderer,
      "pixbuf", P_AUTOAPPLY_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  // Attaching treeview signals

  // row-activated either expands/collapses a row or activates editing
  g_signal_connect(G_OBJECT(tree), "row-activated",
                   G_CALLBACK(tree_row_activated_presets), NULL);

  // Setting up the search functionality
  gtk_tree_view_set_search_column(GTK_TREE_VIEW(tree), P_NAME_COLUMN);
  gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree), TRUE);

  // Attaching the model to the treeview
  gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(model));

  // Adding the treeview to its containers
  gtk_container_add(GTK_CONTAINER(scroll), tree);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);

  g_object_unref(G_OBJECT(model));

}

static void init_tab_accels(GtkWidget *book)
{
  GtkWidget *alignment = gtk_alignment_new(0.5, 0.0, 0.9, 1.0);
  GtkWidget *container = gtk_vbox_new(FALSE, 5);
  GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
  GtkWidget *tree = gtk_tree_view_new();
  GtkWidget *button;
  GtkWidget *hbox;
  GtkTreeStore *model = gtk_tree_store_new(A_N_COLUMNS,
                                           G_TYPE_STRING, G_TYPE_STRING,
                                           G_TYPE_STRING);
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  // Adding the outer container
  gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 20, 20, 20, 20);
  gtk_container_add(GTK_CONTAINER(alignment), container);
  gtk_notebook_append_page(GTK_NOTEBOOK(book), alignment,
                           gtk_label_new(_("shortcuts")));

  // Building the accelerator tree
  g_slist_foreach(darktable.control->accelerator_list, tree_insert_accel,
                  (gpointer)model);

  // Seting a custom sort functions so expandable groups rise to the top
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), A_TRANS_COLUMN, GTK_SORT_ASCENDING);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(model), A_TRANS_COLUMN,
                                  compare_rows_accels, NULL, NULL);

  // Setting up the cell renderers
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("shortcut"), renderer,
      "text", A_TRANS_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
      _("binding"), renderer,
      "text", A_BINDING_COLUMN,
      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  // Attaching treeview signals

  // row-activated either expands/collapses a row or activates remapping
  g_signal_connect(G_OBJECT(tree), "row-activated",
                   G_CALLBACK(tree_row_activated_accels), NULL);

  // A selection change will cancel a currently active remapping
  g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(tree))),
                   "changed",
                   G_CALLBACK(tree_selection_changed), NULL);

  // A keypress may remap an accel or delete one
  g_signal_connect(G_OBJECT(tree), "key-press-event",
                   G_CALLBACK(tree_key_press), (gpointer)model);

  // Setting up the search functionality
  gtk_tree_view_set_search_column(GTK_TREE_VIEW(tree), A_TRANS_COLUMN);
  gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(tree), prefix_search,
                                      NULL, NULL);
  gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree), TRUE);

  // Attaching the model to the treeview
  gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(model));

  // Adding the treeview to its containers
  gtk_container_add(GTK_CONTAINER(scroll), tree);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(container), scroll, TRUE, TRUE, 0);

  hbox = gtk_hbox_new(FALSE, 5);

  // Adding the restore defaults button
  button = gtk_button_new_with_label(_("default"));
  gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",
                   G_CALLBACK(restore_defaults), NULL);
  g_signal_connect(G_OBJECT(button), "clicked",
                   G_CALLBACK(update_accels_model), (gpointer)model);

  // Adding the import/export buttons

  button = gtk_button_new_with_label(C_("preferences", "import"));
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",
                   G_CALLBACK(import_export), (gpointer)0);
  g_signal_connect(G_OBJECT(button), "clicked",
                   G_CALLBACK(update_accels_model), (gpointer)model);

  button = gtk_button_new_with_label(_("export"));
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",
                   G_CALLBACK(import_export), (gpointer)1);

  gtk_box_pack_start(GTK_BOX(container), hbox, FALSE, FALSE, 0);

  g_object_unref(G_OBJECT(model));

}

static void tree_insert_accel(gpointer accel_struct, gpointer model_link)
{
  GtkTreeStore *model = (GtkTreeStore*)model_link;
  dt_accel_t *accel = (dt_accel_t*)accel_struct;
  GtkAccelKey key;

  // Getting the first significant parts of the paths
  char *accel_path = accel->path;
  char *translated_path = accel->translated_path;
  
  /* if prefixed lets forward pointer */
  if (!strncmp(accel_path,"<Darktable>",strlen("<Darktable>")))
  {
    accel_path += strlen("<Darktable>") + 1;
    translated_path += strlen("<Darktable>") + 1;
  }

  // Getting the accelerator keys
  gtk_accel_map_lookup_entry(accel->path, &key);

  /* lets recurse path */
  tree_insert_rec(model, NULL, accel_path, translated_path,
                  key.accel_key,
                  key.accel_mods);
}

static void tree_insert_rec(GtkTreeStore *model, GtkTreeIter *parent,
                            const gchar *accel_path,
                            const gchar *translated_path, guint accel_key,
                            GdkModifierType accel_mods)
{

  int i;
  gboolean found = FALSE;
  gchar *val_str;
  GtkTreeIter iter;

  /* if we are on end of path lets bail out of recursive insert */
  if (*accel_path==0) return;

  /* check if we are on a leaf or a branch  */
  if (!g_strrstr(accel_path,"/")) 
  {
    /* we are on a leaf lets add */
    gchar *name = gtk_accelerator_name(accel_key, accel_mods);
    gtk_tree_store_append(model, &iter, parent);
    gtk_tree_store_set(model, &iter,
                       A_ACCEL_COLUMN, accel_path,
                       A_BINDING_COLUMN, g_dpgettext2("gtk20",
                                                    "keyboard label",
                                                    name),
                       A_TRANS_COLUMN, translated_path,
                       -1);
    g_free(name);    
  } 
  else
  {
    /* we are on a branch let's get the node name */
    gchar *end = g_strstr_len(accel_path,strlen(accel_path),"/");
    gchar *node = g_strndup(accel_path,end-accel_path);
    gchar *trans_end = g_strstr_len(translated_path,
                                    strlen(translated_path), "/");
    gchar *trans_node = g_strndup(translated_path, trans_end-translated_path);

    /* search the tree if we alread have an sibiling with node name */
    int sibilings = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),
                                                   parent);
    for (i = 0; i < sibilings; i++)
    {
      gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model), &iter, parent, i);
      gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
                         A_ACCEL_COLUMN, &val_str,
                         -1);
      
      /* do we match current sibiling */
      if (!strcmp(val_str, node)) found = TRUE;
      
      g_free(val_str);
      
      /* if we found a matching node let's break out */
      if(found) break;
    }

    /* if not found let's add a branch */
    if(!found)
    {      
      gtk_tree_store_append(model, &iter, parent);
      gtk_tree_store_set(model, &iter,
                         A_ACCEL_COLUMN, node,
                         A_BINDING_COLUMN, "",
                         A_TRANS_COLUMN, trans_node,
                         -1);
    }

    /* recurse further down the path */
    tree_insert_rec(model, &iter, accel_path + strlen(node) + 1,
                    translated_path + strlen(trans_node) + 1,
                    accel_key, accel_mods);
    
    /* free up data */
    g_free(node);
    g_free(trans_node);
  }
}

static void path_to_accel(GtkTreeModel *model, GtkTreePath *path, gchar *str)
{
  gint depth;
  gint *indices;
  GtkTreeIter parent;
  GtkTreeIter child;
  gint i;
  gchar *data_str;

  // Start out with the base <Darktable>
  strcpy(str, "<Darktable>");

  // For each index in the path, append a '/' and that section of the path
  depth = gtk_tree_path_get_depth(path);
  indices = gtk_tree_path_get_indices(path);
  for(i = 0; i < depth; i++)
  {
    strcat(str, "/");
    gtk_tree_model_iter_nth_child(model, &child,  i == 0 ? NULL : &parent,
                                  indices[i]);
    gtk_tree_model_get(model, &child,
                       A_ACCEL_COLUMN, &data_str,
                       -1);
    strcat(str, data_str);
    g_free(data_str);
    parent = child;
  }
}

static void update_accels_model(gpointer widget, gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel*)data;
  GtkTreeIter iter;
  gchar path[256];
  gchar *end;
  gint i;

  strcpy(path, "<Darktable>");
  end = path + strlen(path);

  for(i = 0; i < gtk_tree_model_iter_n_children(model, NULL); i++)
  {
    gtk_tree_model_iter_nth_child(model, &iter, NULL, i);
    update_accels_model_rec(model, &iter, path);
    *end = '\0'; // Trimming the string back to the base for the next iteration
  }

}

static void update_accels_model_rec(GtkTreeModel *model, GtkTreeIter *parent,
                                    gchar *path)
{
  GtkAccelKey key;
  GtkTreeIter iter;
  gchar *str_data;
  gchar *end;
  gint i;

  // First concatenating this part of the key
  strcat(path, "/");
  gtk_tree_model_get(model, parent, A_ACCEL_COLUMN, &str_data, -1);
  strcat(path, str_data);
  g_free(str_data);

  if(gtk_tree_model_iter_has_child(model, parent))
  {
    // Branch node, carry on with recursion
    end = path + strlen(path);

    for(i = 0; i < gtk_tree_model_iter_n_children(model, parent); i++)
    {
      gtk_tree_model_iter_nth_child(model, &iter, parent, i);
      update_accels_model_rec(model, &iter, path);
      *end = '\0';
    }
  }
  else
  {
    // Leaf node, update the text

    gtk_accel_map_lookup_entry(path, &key);
    gtk_tree_store_set(
        GTK_TREE_STORE(model), parent,
        A_BINDING_COLUMN, gtk_accelerator_name(key.accel_key, key.accel_mods),
        -1);
  }
}

static void delete_matching_accels(gpointer current, gpointer mapped)
{
  dt_accel_t *current_accel = (dt_accel_t*)current;
  dt_accel_t *mapped_accel = (dt_accel_t*)mapped;
  GtkAccelKey current_key;
  GtkAccelKey mapped_key;

  // Make sure we're not deleting the key we just remapped
  if(!strcmp(current_accel->path, mapped_accel->path))
    return;

  // Finding the relevant keyboard shortcuts
  gtk_accel_map_lookup_entry(current_accel->path, &current_key);
  gtk_accel_map_lookup_entry(mapped_accel->path, &mapped_key);

  if(current_key.accel_key == mapped_key.accel_key // Key code matches
     && current_key.accel_mods == mapped_key.accel_mods // Key state matches
     && current_accel->views & mapped_accel->views // Conflicting views
     && !(current_accel->local && mapped_accel->local  // Not both local to
          && strcmp(current_accel->module, mapped_accel->module))) // diff mods
    gtk_accel_map_change_entry(current_accel->path, 0, 0, TRUE);

}

static gint _accelcmp(gconstpointer a, gconstpointer b)
{
  return (gint)(strcmp(((dt_accel_t*)a)->path,
                       ((dt_accel_t*)b)->path));
}

//TODO: remember which sections were collapsed/expanded and where the view was scrolled to and restore that after editing is done
//      Alternative: change edit_preset_response to not clear+refill the tree, but to update the single row which changed.
static void tree_row_activated_presets(GtkTreeView *tree, GtkTreePath *path,
                               GtkTreeViewColumn *column, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(tree);

  gtk_tree_model_get_iter(model, &iter, path);

  if(gtk_tree_model_iter_has_child(model, &iter))
  {
    // For branch nodes, toggle expansion on activation
    if(gtk_tree_view_row_expanded(tree, path))
      gtk_tree_view_collapse_row(tree, path);
    else
      gtk_tree_view_expand_row(tree, path, FALSE);
  }
  else
  {
    // For leaf nodes, open editing window if the preset is not writeprotected
    gint rowid;
    gchar *name, *operation;
    GdkPixbuf *editable;
    gtk_tree_model_get(model, &iter,
                       P_ROWID_COLUMN, &rowid,
                       P_NAME_COLUMN, &name,
                       P_OPERATION_COLUMN, &operation,
                       P_EDITABLE_COLUMN, &editable,
                       -1);
    if(editable == NULL)
      edit_preset (tree, rowid, name, operation);
    else
      g_object_unref(editable);
    g_free(name);
    g_free(operation);
  }
}

static void tree_row_activated_accels(GtkTreeView *tree, GtkTreePath *path,
                               GtkTreeViewColumn *column, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(tree);

  static gchar accel_path[256];

  gtk_tree_model_get_iter(model, &iter, path);

  if(gtk_tree_model_iter_has_child(model, &iter))
  {
    // For branch nodes, toggle expansion on activation
    if(gtk_tree_view_row_expanded(tree, path))
      gtk_tree_view_collapse_row(tree, path);
    else
      gtk_tree_view_expand_row(tree, path, FALSE);
  }
  else
  {
    // For leaf nodes, enter remapping mode

    // Assembling the full accelerator path
    path_to_accel(model, path, accel_path);

    // Setting the notification text
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
                       A_BINDING_COLUMN, _("press key combination to remap..."),
                       -1);

    // Activating remapping
    darktable.control->accel_remap_str = accel_path;
    darktable.control->accel_remap_path = gtk_tree_path_copy(path);
  }
}

static void tree_selection_changed(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  GtkAccelKey key;

  // If remapping is currently activated, it needs to be deactivated
  if(!darktable.control->accel_remap_str)
    return;

  model = gtk_tree_view_get_model(gtk_tree_selection_get_tree_view(selection));
  gtk_tree_model_get_iter(model, &iter, darktable.control->accel_remap_path);

  // Restoring the A_BINDING_COLUMN text
  gtk_accel_map_lookup_entry(darktable.control->accel_remap_str, &key);
  gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
                     A_BINDING_COLUMN,
                     gtk_accelerator_name(key.accel_key, key.accel_mods), -1);

  // Cleaning up the darktable.gui info
  darktable.control->accel_remap_str = NULL;
  gtk_tree_path_free(darktable.control->accel_remap_path);
  darktable.control->accel_remap_path = NULL;
}

static gboolean tree_key_press(GtkWidget *widget, GdkEventKey *event,
                               gpointer data)
{

  GtkTreeModel *model = (GtkTreeModel*)data;
  GtkTreeIter iter;
  GtkTreeSelection *selection =
      gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
  GtkTreePath *path;
  GSList *remapped;
  dt_accel_t query;

  gchar accel[256];
  gchar datadir[1024];
  gchar accelpath[1024];

  // We can just ignore mod key presses outright
  if(event->is_modifier)
    return FALSE;

  dt_util_get_user_config_dir(datadir, 1024);
  snprintf(accelpath, 1024, "%s/keyboardrc", datadir);

  // Otherwise, determine whether we're in remap mode or not
  if(darktable.control->accel_remap_str)
  {
    // Change the accel map entry
    if(gtk_accel_map_change_entry(darktable.control->accel_remap_str,
                                  event->keyval,
                                  event->state & KEY_STATE_MASK, TRUE))
    {
      // If it succeeded delete any conflicting accelerators
      // First locate the accel list entry
      strcpy(query.path, darktable.control->accel_remap_str);
      remapped = g_slist_find_custom(darktable.control->accelerator_list,
                                     (gpointer)&query, _accelcmp);

      // Then remove conflicts
      g_slist_foreach(darktable.control->accelerator_list,
                      delete_matching_accels,
                      (gpointer)(remapped->data));

    }



    // Then update the text in the A_BINDING_COLUMN of each row
    update_accels_model(NULL, model);

    // Finally clear the remap state
    darktable.control->accel_remap_str = NULL;
    gtk_tree_path_free(darktable.control->accel_remap_path);
    darktable.control->accel_remap_path = NULL;

    // Save the changed keybindings
    gtk_accel_map_save(accelpath);

    return TRUE;
  }
  else if(event->keyval == GDK_BackSpace)
  {
    // If a leaf node is selected, clear that accelerator

    // If nothing is selected, or branch node selected, just return
    if(!gtk_tree_selection_get_selected(selection, &model, &iter)
      || gtk_tree_model_iter_has_child(model, &iter))
      return FALSE;

    // Otherwise, construct the proper accelerator path and delete its entry
    strcpy(accel, "<Darktable>");
    path = gtk_tree_model_get_path(model, &iter);
    path_to_accel(model, path, accel);
    gtk_tree_path_free(path);

    gtk_accel_map_change_entry(accel, 0, 0, TRUE);
    update_accels_model(NULL, model);

    // Saving the changed bindings
    gtk_accel_map_save(accelpath);

    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static void import_export(GtkButton *button, gpointer data)
{
  GtkWidget *chooser;
  gchar confdir[1024];
  gchar accelpath[1024];

  if(data)
  {
    // Non-zero value indicates export
    chooser = gtk_file_chooser_dialog_new(
        _("select file to export"),
        NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
        NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(chooser),
                                                   TRUE);
    if(dt_conf_get_string("ui_last/export_path"))
      gtk_file_chooser_set_current_folder(
          GTK_FILE_CHOOSER(chooser),
          dt_conf_get_string("ui_last/export_path"));
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser), "keyboardrc");
    if(gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
    {
      gtk_accel_map_save(
          gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser)));
      dt_conf_set_string(
          "ui_last/export_path",
          gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser)));
    }
    gtk_widget_destroy(chooser);
  }
  else
  {
    // Zero value indicates import
    chooser = gtk_file_chooser_dialog_new(
        _("select file to import"),
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
        NULL);

    if(dt_conf_get_string("ui_last/import_path"))
      gtk_file_chooser_set_current_folder(
          GTK_FILE_CHOOSER(chooser),
          dt_conf_get_string("ui_last/import_path"));
    if(gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
    {
      if(g_file_test(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser)),
                     G_FILE_TEST_EXISTS))
      {
        // Loading the file
        gtk_accel_map_load(
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser)));

        // Saving to the permanent keyboardrc
        dt_util_get_user_config_dir(confdir, 1024);
        snprintf(accelpath, 1024, "%s/keyboardrc", confdir);
        gtk_accel_map_save(accelpath);

        dt_conf_set_string(
            "ui_last/import_path",
            gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser)));
      }
    }
    gtk_widget_destroy(chooser);
  }
}

static void restore_defaults(GtkButton *button, gpointer data)
{
  GList *ops;
  dt_iop_module_so_t *op;
  gchar accelpath[256];
  gchar dir[1024];
  gchar path[1024];

  GtkWidget *message = gtk_message_dialog_new(
      NULL, GTK_DIALOG_MODAL,
      GTK_MESSAGE_WARNING,
      GTK_BUTTONS_OK_CANCEL,
      _("Are you sure you want to restore the default keybindings?  This will "
        "erase any modifications you have made."));
  if(gtk_dialog_run(GTK_DIALOG(message)) == GTK_RESPONSE_OK)
  {
    // First load the default keybindings for immediate effect
    dt_util_get_user_config_dir(dir, 1024);
    snprintf(path, 1024, "%s/keyboardrc_default", dir);
    gtk_accel_map_load(path);

    // Now deleting any iop show shortcuts
    ops = darktable.iop;
    while(ops)
    {
      op = (dt_iop_module_so_t*)ops->data;
      snprintf(accelpath, 256, "<Darktable>/darkroom/plugins/%s/show", op->op);
      gtk_accel_map_change_entry(accelpath, 0, 0, TRUE);
      ops = g_list_next(ops);
    }

    // Then delete any changes to the user's keyboardrc so it gets reset
    // on next startup
    dt_util_get_user_config_dir(dir, 1024);
    snprintf(path, 1024, "%s/keyboardrc", dir);
    g_file_delete(g_file_new_for_path(path), NULL, NULL);
  }
  gtk_widget_destroy(message);
}

static gboolean prefix_search(GtkTreeModel *model, gint column,
                              const gchar *key, GtkTreeIter *iter, gpointer d)
{
  gchar *row_data;

  gtk_tree_model_get(model, iter, A_TRANS_COLUMN, &row_data, -1);
  while(*key != '\0')
  {
    if(*row_data != *key)
      return TRUE;
    key++;
    row_data++;
  }
  return FALSE;
}

// Custom sort function for TreeModel entries for accels list
static gint compare_rows_accels(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
                         gpointer data)
{
  gchar *a_text;
  gchar *b_text;

  // First prioritize branch nodes over leaves
  if(gtk_tree_model_iter_has_child(model, a)
    && !gtk_tree_model_iter_has_child(model, b))
    return -1;

  if(gtk_tree_model_iter_has_child(model, b)
    && !gtk_tree_model_iter_has_child(model, a))
    return 1;

  // Otherwise just return alphabetical order
  gtk_tree_model_get(model, a, A_TRANS_COLUMN, &a_text, -1);
  gtk_tree_model_get(model, b, A_TRANS_COLUMN, &b_text, -1);
  return strcasecmp(a_text, b_text);

}

// Custom sort function for TreeModel entries for presets list
static gint compare_rows_presets(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
                         gpointer data)
{
  gchar *a_text;
  gchar *b_text;

  gtk_tree_model_get(model, a, P_MODULE_COLUMN, &a_text, -1);
  gtk_tree_model_get(model, b, P_MODULE_COLUMN, &b_text, -1);
  if(*a_text == '\0' && *b_text == '\0')
  {
    gtk_tree_model_get(model, a, P_NAME_COLUMN, &a_text, -1);
    gtk_tree_model_get(model, b, P_NAME_COLUMN, &b_text, -1);
  }
  return strcasecmp(a_text, b_text);
}

// FIXME: Mostly c&p from gui/presets.c
static void
check_buttons_activated (GtkCheckButton *button, dt_gui_presets_edit_dialog_t *g)
{
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g->autoapply)) || gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g->filter)))
  {
    gtk_widget_set_visible(GTK_WIDGET(g->details), TRUE);
    gtk_widget_set_no_show_all(GTK_WIDGET(g->details), FALSE);
    gtk_widget_show_all (GTK_WIDGET(g->details));
    gtk_widget_set_no_show_all(GTK_WIDGET(g->details), TRUE);
  }
  else
    gtk_widget_set_visible(GTK_WIDGET(g->details), FALSE);
}

static void
edit_preset (GtkTreeView * tree, const gint rowid, const gchar * name, const gchar * module)
{
  GtkWidget *dialog;
  /* Create the widgets */
  char title[1024];
  snprintf(title, 1024, _("edit `%s' for module `%s'"), name, module);
  dialog = gtk_dialog_new_with_buttons (title,
                                        GTK_WINDOW(_preferences_dialog),
                                        GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_MODAL,
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_NONE,
                                        NULL);
  GtkContainer *content_area = GTK_CONTAINER(gtk_dialog_get_content_area (GTK_DIALOG (dialog)));
  GtkWidget *alignment = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
  gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 5, 5, 5);
  gtk_container_add (content_area, alignment);
  GtkBox *box = GTK_BOX(gtk_vbox_new(FALSE, 5));
  gtk_container_add (GTK_CONTAINER(alignment), GTK_WIDGET(box));
  GtkWidget *label;
  // GtkBox *vbox1 = GTK_BOX(gtk_vbox_new(TRUE, 5));

  GtkBox *vbox2 = GTK_BOX(gtk_vbox_new(TRUE, 5));
  GtkBox *vbox3 = GTK_BOX(gtk_vbox_new(TRUE, 5));
  GtkBox *vbox4 = GTK_BOX(gtk_vbox_new(TRUE, 5));

  dt_gui_presets_edit_dialog_t *g = (dt_gui_presets_edit_dialog_t *)malloc(sizeof(dt_gui_presets_edit_dialog_t));
//   g->module = module;
  g->rowid = rowid;
  g->tree = tree;
  g->name = GTK_LABEL(gtk_label_new(name));
  gtk_box_pack_start(box, GTK_WIDGET(g->name), FALSE, FALSE, 0);

  g->description = GTK_ENTRY(gtk_entry_new());
  gtk_box_pack_start(box, GTK_WIDGET(g->description), FALSE, FALSE, 0);
  g_object_set(G_OBJECT(g->description), "tooltip-text", _("description or further information"), (char *)NULL);

  g->autoapply = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("auto apply this preset to matching images")));
  gtk_box_pack_start(box, GTK_WIDGET(g->autoapply), FALSE, FALSE, 0);
  g->filter = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("only show this preset for matching images")));
  g_object_set(G_OBJECT(g->filter), "tooltip-text", _("be very careful with this option. this might be the last time you see your preset."), (char *)NULL);
  gtk_box_pack_start(box, GTK_WIDGET(g->filter), FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(g->autoapply), "toggled", G_CALLBACK(check_buttons_activated), g);
  g_signal_connect(G_OBJECT(g->filter),    "toggled", G_CALLBACK(check_buttons_activated), g);

  g->details   = GTK_BOX(gtk_hbox_new(FALSE, 0));
  GtkBox *hbox = GTK_BOX(gtk_hbox_new(TRUE, 5));
  gtk_box_pack_start(box,  GTK_WIDGET(g->details),  FALSE, FALSE, 0);
  gtk_box_pack_start(g->details, GTK_WIDGET(hbox),  FALSE, FALSE, 0);
  gtk_box_pack_start(hbox, GTK_WIDGET(vbox2), TRUE, TRUE, 0);
  gtk_box_pack_start(hbox, GTK_WIDGET(vbox3), TRUE, TRUE, 0);
  gtk_box_pack_start(hbox, GTK_WIDGET(vbox4), TRUE, TRUE, 0);

  // model, maker, lens
  g->model = GTK_ENTRY(gtk_entry_new());
  g_object_set(G_OBJECT(g->model), "tooltip-text", _("string to match model (use % as wildcard)"), (char *)NULL);
  label = gtk_label_new(_("model"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(vbox2, label, FALSE, FALSE, 0);
  gtk_box_pack_start(vbox3, GTK_WIDGET(g->model), FALSE, FALSE, 0);
  gtk_box_pack_start(vbox4, gtk_label_new(""), FALSE, FALSE, 0);
  g->maker = GTK_ENTRY(gtk_entry_new());
  g_object_set(G_OBJECT(g->maker), "tooltip-text", _("string to match maker (use % as wildcard)"), (char *)NULL);
  label = gtk_label_new(_("maker"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(vbox2, label, FALSE, FALSE, 0);
  gtk_box_pack_start(vbox3, GTK_WIDGET(g->maker), FALSE, FALSE, 0);
  gtk_box_pack_start(vbox4, gtk_label_new(""), FALSE, FALSE, 0);
  g->lens  = GTK_ENTRY(gtk_entry_new());
  g_object_set(G_OBJECT(g->lens), "tooltip-text", _("string to match lens (use % as wildcard)"), (char *)NULL);
  label = gtk_label_new(_("lens"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(vbox2, label, FALSE, FALSE, 0);
  gtk_box_pack_start(vbox3, GTK_WIDGET(g->lens), FALSE, FALSE, 0);
  gtk_box_pack_start(vbox4, gtk_label_new(""), FALSE, FALSE, 0);

  // iso
  label = gtk_label_new(_("iso"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(vbox2, label, FALSE, FALSE, 0);
  g->iso_min = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 51200, 100));
  g_object_set(G_OBJECT(g->iso_min), "tooltip-text", _("minimum iso value"), (char *)NULL);
  gtk_spin_button_set_digits(g->iso_min, 0);
  gtk_box_pack_start(vbox3, GTK_WIDGET(g->iso_min), FALSE, FALSE, 0);
  g->iso_max = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 51200, 100));
  g_object_set(G_OBJECT(g->iso_max), "tooltip-text", _("maximum iso value"), (char *)NULL);
  gtk_spin_button_set_digits(g->iso_max, 0);
  gtk_box_pack_start(vbox4, GTK_WIDGET(g->iso_max), FALSE, FALSE, 0);

  // exposure
  label = gtk_label_new(_("exposure"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(vbox2, label, FALSE, FALSE, 0);
  g->exposure_min = GTK_COMBO_BOX(gtk_combo_box_new_text());
  g->exposure_max = GTK_COMBO_BOX(gtk_combo_box_new_text());
  g_object_set(G_OBJECT(g->exposure_min), "tooltip-text", _("minimum exposure time"), (char *)NULL);
  g_object_set(G_OBJECT(g->exposure_max), "tooltip-text", _("maximum exposure time"), (char *)NULL);
  for(int k=0; k<dt_gui_presets_exposure_value_cnt; k++)
    gtk_combo_box_append_text(g->exposure_min, dt_gui_presets_exposure_value_str[k]);
  for(int k=0; k<dt_gui_presets_exposure_value_cnt; k++)
    gtk_combo_box_append_text(g->exposure_max, dt_gui_presets_exposure_value_str[k]);
  gtk_box_pack_start(vbox3, GTK_WIDGET(g->exposure_min), FALSE, FALSE, 0);
  gtk_box_pack_start(vbox4, GTK_WIDGET(g->exposure_max), FALSE, FALSE, 0);

  // aperture
  label = gtk_label_new(_("aperture"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(vbox2, label, FALSE, FALSE, 0);
  g->aperture_min = GTK_COMBO_BOX(gtk_combo_box_new_text());
  g->aperture_max = GTK_COMBO_BOX(gtk_combo_box_new_text());
  g_object_set(G_OBJECT(g->aperture_min), "tooltip-text", _("minimum aperture value"), (char *)NULL);
  g_object_set(G_OBJECT(g->aperture_max), "tooltip-text", _("maximum aperture value"), (char *)NULL);
  for(int k=0; k<dt_gui_presets_aperture_value_cnt; k++)
    gtk_combo_box_append_text(g->aperture_min, dt_gui_presets_aperture_value_str[k]);
  for(int k=0; k<dt_gui_presets_aperture_value_cnt; k++)
    gtk_combo_box_append_text(g->aperture_max, dt_gui_presets_aperture_value_str[k]);
  gtk_box_pack_start(vbox3, GTK_WIDGET(g->aperture_min), FALSE, FALSE, 0);
  gtk_box_pack_start(vbox4, GTK_WIDGET(g->aperture_max), FALSE, FALSE, 0);

  // focal length
  label = gtk_label_new(_("focal length"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(vbox2, label, FALSE, FALSE, 0);
  g->focal_length_min = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 1000, 10));
  gtk_spin_button_set_digits(g->focal_length_min, 0);
  gtk_box_pack_start(vbox3, GTK_WIDGET(g->focal_length_min), FALSE, FALSE, 0);
  g->focal_length_max = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 1000, 10));
  g_object_set(G_OBJECT(g->focal_length_min), "tooltip-text", _("minimum focal length"), (char *)NULL);
  g_object_set(G_OBJECT(g->focal_length_max), "tooltip-text", _("maximum focal length"), (char *)NULL);
  gtk_spin_button_set_digits(g->focal_length_max, 0);
  gtk_box_pack_start(vbox4, GTK_WIDGET(g->focal_length_max), FALSE, FALSE, 0);

  gtk_widget_set_no_show_all(GTK_WIDGET(g->details), TRUE);

  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select description, model, maker, lens, iso_min, iso_max, exposure_min, exposure_max, aperture_min, aperture_max, focal_length_min, focal_length_max, autoapply, filter from presets where rowid = ?1", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, rowid );
  if(sqlite3_step(stmt) == SQLITE_ROW)
  {
    gtk_entry_set_text(g->description, (const char *)sqlite3_column_text(stmt, 0));
    gtk_entry_set_text(g->model, (const char *)sqlite3_column_text(stmt, 1));
    gtk_entry_set_text(g->maker, (const char *)sqlite3_column_text(stmt, 2));
    gtk_entry_set_text(g->lens,  (const char *)sqlite3_column_text(stmt, 3));
    gtk_spin_button_set_value(g->iso_min, sqlite3_column_double(stmt, 4));
    gtk_spin_button_set_value(g->iso_max, sqlite3_column_double(stmt, 5));

    float val = sqlite3_column_double(stmt, 6);
    int k=0;
    for(; k<dt_gui_presets_exposure_value_cnt&&val>dt_gui_presets_exposure_value[k]; k++);
    gtk_combo_box_set_active(g->exposure_min, k);
    val = sqlite3_column_double(stmt, 7);
    for(k=0; k<dt_gui_presets_exposure_value_cnt&&val>dt_gui_presets_exposure_value[k]; k++);
    gtk_combo_box_set_active(g->exposure_max, k);
    val = sqlite3_column_double(stmt, 8);
    for(k=0; k<dt_gui_presets_aperture_value_cnt&&val>dt_gui_presets_aperture_value[k]; k++);
    gtk_combo_box_set_active(g->aperture_min, k);
    val = sqlite3_column_double(stmt, 9);
    for(k=0; k<dt_gui_presets_aperture_value_cnt&&val>dt_gui_presets_aperture_value[k]; k++);
    gtk_combo_box_set_active(g->aperture_max, k);
    gtk_spin_button_set_value(g->focal_length_min, sqlite3_column_double(stmt, 10));
    gtk_spin_button_set_value(g->focal_length_max, sqlite3_column_double(stmt, 11));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->autoapply), sqlite3_column_int(stmt, 12));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->filter),    sqlite3_column_int(stmt, 13));
  }
  sqlite3_finalize(stmt);

  g_signal_connect (dialog, "response", G_CALLBACK (edit_preset_response), g);
  gtk_widget_show_all (dialog);
}

static void
edit_preset_response(GtkDialog *dialog, gint response_id, dt_gui_presets_edit_dialog_t *g)
{
  // commit all the user input fields
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "update presets set description = ?1, "
          "model = ?2, maker = ?3, lens = ?4, iso_min = ?5, iso_max = ?6, exposure_min = ?7, exposure_max = ?8, aperture_min = ?9, "
          "aperture_max = ?10, focal_length_min = ?11, focal_length_max = ?12, autoapply = ?13, filter = ?14, def = 0 "
          "where rowid = ?15", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, gtk_entry_get_text(g->description), -1, SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 2, gtk_entry_get_text(g->model), -1, SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 3, gtk_entry_get_text(g->maker), -1, SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 4, gtk_entry_get_text(g->lens), -1, SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt,  5, gtk_spin_button_get_value(g->iso_min));
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 6, gtk_spin_button_get_value(g->iso_max));
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 7, dt_gui_presets_exposure_value[gtk_combo_box_get_active(g->exposure_min)]);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 8, dt_gui_presets_exposure_value[gtk_combo_box_get_active(g->exposure_max)]);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 9, dt_gui_presets_aperture_value[gtk_combo_box_get_active(g->aperture_min)]);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 10, dt_gui_presets_aperture_value[gtk_combo_box_get_active(g->aperture_max)]);
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 11, gtk_spin_button_get_value(g->focal_length_min));
  DT_DEBUG_SQLITE3_BIND_DOUBLE(stmt, 12, gtk_spin_button_get_value(g->focal_length_max));
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 13, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g->autoapply)));
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 14, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g->filter)));
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 15, g->rowid);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  GtkTreeStore *tree_store = GTK_TREE_STORE(gtk_tree_view_get_model(g->tree));
  gtk_tree_store_clear(tree_store);
  tree_insert_presets(tree_store);

  gtk_widget_destroy(GTK_WIDGET(dialog));
  free(g);
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
