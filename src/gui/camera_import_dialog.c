/*
    This file is part of darktable,
    copyright (c) 2010 henrik andersson.

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
#include "develop/develop.h"
#include "control/control.h"
#include "control/jobs.h"
#include "control/conf.h"
#include "common/exif.h"
#include "common/variables.h"
#include "common/camera_control.h"
#include "common/utility.h"
#include "dtgtk/button.h"
#include "dtgtk/label.h"
#include "gui/camera_import_dialog.h"

#include <time.h>

/*

  g_object_ref(model); // Make sure the model stays with us after the tree view unrefs it

  gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); // Detach model from view

  ... insert a couple of thousand rows ...

  gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); // Re-attach model to view

  g_object_unref(model);

*/


typedef struct _camera_gconf_widget_t
{
  GtkWidget *widget;
  GtkWidget *entry;
  gchar *value;
  struct _camera_import_dialog_t *dialog;
}
_camera_gconf_widget_t;

typedef struct _camera_import_dialog_t
{
  GtkWidget *dialog;

  GtkWidget *notebook;

  struct
  {
    GtkWidget *page;
    _camera_gconf_widget_t *jobname;
    GtkWidget *treeview;
    GtkWidget *info;
  } import;

  struct
  {
    GtkWidget *page;

    /** Group of general import settings */
    struct
    {
      GtkWidget *ignore_jpeg;
      GtkWidget *delete_originals;
      GtkWidget *date_override;
      GtkWidget *date_entry;
    } general;

    /** Group of  backup settings */
    struct
    {
      GtkWidget *enable,*foldername,*warn;
    } backup;

    _camera_gconf_widget_t *basedirectory;
    _camera_gconf_widget_t *subdirectory;
    _camera_gconf_widget_t *namepattern;
    GtkWidget *example;

  } settings;

  GtkListStore *store;
  dt_job_t *preview_job;
  dt_camera_import_dialog_param_t *params;
  dt_variables_params_t *vp;
}
_camera_import_dialog_t;


static void
_check_button_callback(GtkWidget *cb, gpointer user_data)
{

  _camera_import_dialog_t *cid=(_camera_import_dialog_t*)user_data;

  if( cb == cid->settings.general.ignore_jpeg )
  {
    dt_conf_set_bool ("ui_last/import_ignore_jpegs", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cid->settings.general.ignore_jpeg)));
  }
  else if( cb == cid->settings.general.delete_originals )
  {
    dt_conf_set_bool ("plugins/capture/camera/import/delete_originals", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cid->settings.general.delete_originals)));
  }
  else if (cb==cid->settings.general.date_override )
  {
    // Enable/disable the date entry widget
    gtk_widget_set_sensitive( cid->settings.general.date_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cid->settings.general.date_override)));
  }
  else if (cb==cid->settings.backup.enable)
  {
    dt_conf_set_bool ("plugins/capture/backup/enable", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cid->settings.backup.enable)));
    // Enable/disable the date entry widget
    gtk_widget_set_sensitive( cid->settings.backup.warn, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cid->settings.backup.enable)));
    gtk_widget_set_sensitive( cid->settings.backup.foldername, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cid->settings.backup.enable)));
  }
  else if (cb==cid->settings.backup.warn)
  {
    dt_conf_set_bool ("plugins/capture/backup/warning", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cid->settings.backup.enable)));
  }
}

static void
_gcw_store_callback (GtkDarktableButton *button, gpointer user_data)
{
  _camera_gconf_widget_t *gcw=(_camera_gconf_widget_t*)user_data;
  gchar *configstring=g_object_get_data(G_OBJECT(gcw->widget),"gconf:string");
  const gchar *newvalue=gtk_entry_get_text( GTK_ENTRY( gcw->entry ));
  if(newvalue && strlen(newvalue) > 0 )
  {
    dt_conf_set_string(configstring,newvalue);
    if(gcw->value) g_free(gcw->value);
    gcw->value=g_strdup(newvalue);
  }
}

static void
_gcw_reset_callback (GtkDarktableButton *button, gpointer user_data)
{
  _camera_gconf_widget_t *gcw=(_camera_gconf_widget_t*)user_data;
  gchar *configstring=g_object_get_data(G_OBJECT(gcw->widget),"gconf:string");
  gchar *value=dt_conf_get_string(configstring);
  if(value)
  {
    gtk_entry_set_text( GTK_ENTRY( gcw->entry ),value);
    if(gcw->value) g_free(gcw->value);
    gcw->value=g_strdup(value);
  }
}

static void
_update_example(_camera_import_dialog_t *dialog)
{
  // create path/filename and execute a expand..
  gchar *path=g_build_path(G_DIR_SEPARATOR_S,dialog->settings.basedirectory->value,dialog->settings.subdirectory->value,"/",(char *)NULL);
  gchar *fixed_path=dt_util_fix_path(path);
  dt_variables_expand( dialog->vp, fixed_path, FALSE);

  gchar *ep=g_strdup(dt_variables_get_result(dialog->vp));
  dt_variables_expand( dialog->vp, dialog->settings.namepattern->value, TRUE);
  gchar *ef=g_strdup(dt_variables_get_result(dialog->vp));

  gchar *str=g_strdup_printf("%s\n%s",ep,ef);

  // then set result set
  gtk_label_set_text(GTK_LABEL(dialog->settings.example),str);
  // Clenaup
  g_free(path);
  g_free(fixed_path);
  g_free(ep);
  g_free(ef);
  g_free(str);
}

static void
_entry_text_changed(_camera_gconf_widget_t *gcw,GtkEntryBuffer *entrybuffer)
{
  const gchar *value=gtk_entry_buffer_get_text(entrybuffer);
  if(gcw->value)
    g_free(gcw->value);
  gcw->value=g_strdup(value);

  _update_example(gcw->dialog);
}

static void
entry_dt_callback(GtkEntryBuffer *entrybuffer,guint a1, guint a2,gpointer user_data)
{
  _entry_text_changed((_camera_gconf_widget_t*)user_data,entrybuffer);
}

static void
entry_it_callback(GtkEntryBuffer *entrybuffer,guint a1, gchar *a2, guint a3,gpointer user_data)
{
  _entry_text_changed((_camera_gconf_widget_t*)user_data,entrybuffer);
}

/** Creates a gconf widget. */
_camera_gconf_widget_t *_camera_import_gconf_widget(_camera_import_dialog_t *dlg,gchar *label,gchar *confstring)
{
  _camera_gconf_widget_t *gcw=malloc(sizeof(_camera_gconf_widget_t));
  memset(gcw,0,sizeof(_camera_gconf_widget_t));
  GtkWidget *vbox,*hbox;
  gcw->widget=vbox=GTK_WIDGET(gtk_vbox_new(FALSE,0));
  hbox=GTK_WIDGET(gtk_hbox_new(FALSE,0));
  g_object_set_data(G_OBJECT(vbox),"gconf:string",confstring);
  gcw->dialog=dlg;

  gcw->entry=gtk_entry_new();
  char* value = NULL;
  if((value = dt_conf_get_string (confstring)))
  {
    gtk_entry_set_text( GTK_ENTRY( gcw->entry ),dt_conf_get_string (confstring));
    gcw->value=g_strdup(value);
    g_free(value);
  }

  gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(gcw->entry),TRUE,TRUE,0);

  GtkWidget *button=dtgtk_button_new(dtgtk_cairo_paint_store,CPF_STYLE_FLAT|CPF_DO_NOT_USE_BORDER);
  g_object_set(button,"tooltip-text",_("store value as default"),(char *)NULL);
  gtk_widget_set_size_request(button,13,13);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (_gcw_store_callback), gcw);

  button=dtgtk_button_new(dtgtk_cairo_paint_reset,CPF_STYLE_FLAT|CPF_DO_NOT_USE_BORDER);
  g_object_set(button,"tooltip-text",_("reset value to default"),(char *)NULL);
  gtk_widget_set_size_request(button,13,13);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (_gcw_reset_callback), gcw);

  GtkWidget *l=gtk_label_new(label);
  gtk_misc_set_alignment(GTK_MISC(l), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(vbox),l,FALSE,FALSE,0);

  gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(hbox),FALSE,FALSE,0);

  g_signal_connect (G_OBJECT(gtk_entry_get_buffer(GTK_ENTRY(gcw->entry))), "inserted-text",
                    G_CALLBACK (entry_it_callback), gcw);
  g_signal_connect (G_OBJECT(gtk_entry_get_buffer(GTK_ENTRY(gcw->entry))), "deleted-text",
                    G_CALLBACK (entry_dt_callback), gcw);

  return gcw;
}



void _camera_import_dialog_new(_camera_import_dialog_t *data)
{
  data->dialog=gtk_dialog_new_with_buttons(_("import images from camera"),NULL,GTK_DIALOG_MODAL,_("cancel"),GTK_RESPONSE_NONE,C_("camera import", "import"),GTK_RESPONSE_ACCEPT,NULL);
  GtkWidget *content = gtk_dialog_get_content_area (GTK_DIALOG (data->dialog));

  // List - setup store
  data->store = gtk_list_store_new (2,GDK_TYPE_PIXBUF,G_TYPE_STRING);

  // Setup variables
  dt_variables_params_init(&data->vp);
  data->vp->jobcode=_("my jobcode");
  data->vp->filename="DSC_0235.JPG";

  // IMPORT PAGE
  data->import.page=gtk_vbox_new(FALSE,5);
  gtk_container_set_border_width(GTK_CONTAINER(data->import.page),5);

  // Top info
  data->import.info=gtk_label_new( _("please wait while prefetching thumbnails of images from camera...") );
  gtk_label_set_single_line_mode( GTK_LABEL(data->import.info) , FALSE );
  gtk_misc_set_alignment(GTK_MISC(data->import.info), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(data->import.page),data->import.info,FALSE,FALSE,0);

  // jobcode
  data->import.jobname=_camera_import_gconf_widget(data,_("jobcode"),"plugins/capture/camera/import/jobcode");
  gtk_box_pack_start(GTK_BOX(data->import.page),GTK_WIDGET(data->import.jobname->widget),FALSE,FALSE,0);


  // Create the treview with list model data store
  data->import.treeview=gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(data->import.treeview),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);

  gtk_container_add(GTK_CONTAINER(data->import.treeview), gtk_tree_view_new());
  GtkTreeView *treeview=GTK_TREE_VIEW(gtk_bin_get_child(GTK_BIN(data->import.treeview)));

  GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new( );
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes( _("thumbnail") , renderer,"pixbuf",0, (char *)NULL);
  gtk_tree_view_append_column( treeview , column);

  renderer = gtk_cell_renderer_text_new( );
  column = gtk_tree_view_column_new_with_attributes( _("storage file"), renderer, "text", 1, (char *)NULL);
  gtk_tree_view_append_column( treeview , column);
  gtk_tree_view_column_set_expand( column, TRUE);


  GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_MULTIPLE);

  gtk_tree_view_set_model(treeview,GTK_TREE_MODEL(data->store));
  gtk_tree_view_set_headers_visible(treeview,FALSE);

  gtk_box_pack_start(GTK_BOX(data->import.page),data->import.treeview,TRUE,TRUE,0);


  // SETTINGS PAGE
  data->settings.page=gtk_vbox_new(FALSE,5);
  gtk_container_set_border_width(GTK_CONTAINER(data->settings.page),5);

  // general settings
  gtk_box_pack_start(GTK_BOX(data->settings.page),dtgtk_label_new(_("general"),DARKTABLE_LABEL_TAB|DARKTABLE_LABEL_ALIGN_RIGHT),FALSE,FALSE,0);

  // ignoring of jpegs. hack while we don't handle raw+jpeg in the same directories.
  data->settings.general.ignore_jpeg = gtk_check_button_new_with_label (_("ignore jpeg files"));
  g_object_set(data->settings.general.ignore_jpeg, "tooltip-text", _("do not load files with an extension of .jpg or .jpeg. this can be useful when there are raw+jpeg in a directory."), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->settings.general.ignore_jpeg), dt_conf_get_bool("ui_last/import_ignore_jpegs"));
  gtk_box_pack_start(GTK_BOX(data->settings.page), data->settings.general.ignore_jpeg, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT(data->settings.general.ignore_jpeg), "clicked",G_CALLBACK (_check_button_callback),data);

  data->settings.general.delete_originals = gtk_check_button_new_with_label(_("delete originals after import"));
  gtk_box_pack_start(GTK_BOX(data->settings.page),data->settings.general.delete_originals ,FALSE,FALSE,0);
  if( dt_conf_get_bool("plugins/capture/camera/import/delete_originals") ) gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( data->settings.general.delete_originals ), TRUE);

  g_object_set(data->settings.general.delete_originals ,"tooltip-text",_("check this option if you want to delete images on camera after download to computer"),(char *)NULL);
  g_signal_connect (G_OBJECT(data->settings.general.delete_originals), "clicked",G_CALLBACK (_check_button_callback),data);

  GtkWidget *hbox=gtk_hbox_new(FALSE,5);
  data->settings.general.date_override=gtk_check_button_new_with_label(_("override today's date"));
  gtk_box_pack_start(GTK_BOX(hbox),data->settings.general.date_override,FALSE,FALSE,0);
  g_object_set(data->settings.general.date_override,"tooltip-text",_("check this, if you want to override the timestamp used when expanding variables:\n$(YEAR), $(MONTH), $(DAY),\n$(HOUR), $(MINUTE), $(SECONDS)"),(char *)NULL);

  data->settings.general.date_entry=gtk_entry_new();
  gtk_widget_set_sensitive( data->settings.general.date_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->settings.general.date_override)));
  gtk_box_pack_start(GTK_BOX(hbox),data->settings.general.date_entry,TRUE,TRUE,0);

  g_signal_connect (G_OBJECT (data->settings.general.date_override), "clicked",G_CALLBACK (_check_button_callback),data);

  gtk_box_pack_start(GTK_BOX(data->settings.page),hbox,FALSE,FALSE,0);


  // Storage structure
  gtk_box_pack_start(GTK_BOX(data->settings.page),dtgtk_label_new(_("storage structure"),DARKTABLE_LABEL_TAB|DARKTABLE_LABEL_ALIGN_RIGHT),FALSE,FALSE,0);
  GtkWidget *l=gtk_label_new(_("the following three settings describe the directory structure and file renaming for import storage and images; if you dont know how to use this, keep the default settings."));
  gtk_label_set_line_wrap(GTK_LABEL(l),TRUE);
  gtk_widget_set_size_request(l,400,-1);
  gtk_misc_set_alignment(GTK_MISC(l), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(data->settings.page),l,FALSE,FALSE,0);

  data->settings.basedirectory=_camera_import_gconf_widget(data,_("storage directory"),"plugins/capture/storage/basedirectory");
  gtk_box_pack_start(GTK_BOX(data->settings.page),GTK_WIDGET(data->settings.basedirectory->widget),FALSE,FALSE,0);

  data->settings.subdirectory=_camera_import_gconf_widget(data,_("directory structure"),"plugins/capture/storage/subpath");
  gtk_box_pack_start(GTK_BOX(data->settings.page),GTK_WIDGET(data->settings.subdirectory->widget),FALSE,FALSE,0);


  data->settings.namepattern=_camera_import_gconf_widget(data,_("filename structure"),"plugins/capture/storage/namepattern");
  gtk_box_pack_start(GTK_BOX(data->settings.page),GTK_WIDGET(data->settings.namepattern->widget),FALSE,FALSE,0);

  // Add example
  l=gtk_label_new(_("above settings expands to:"));
  gtk_misc_set_alignment(GTK_MISC(l), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(data->settings.page),l,FALSE,FALSE,0);

  data->settings.example=gtk_label_new("");
  gtk_label_set_line_wrap(GTK_LABEL(data->settings.example),TRUE);
  gtk_widget_set_size_request(data->settings.example,400,-1);
  gtk_misc_set_alignment(GTK_MISC(data->settings.example), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(data->settings.page),data->settings.example,FALSE,FALSE,0);

  // External backup
  gtk_box_pack_start(GTK_BOX(data->settings.page),dtgtk_label_new(_("external backup"),DARKTABLE_LABEL_TAB|DARKTABLE_LABEL_ALIGN_RIGHT),FALSE,FALSE,0);
  l=gtk_label_new(_("external backup is an option to automatic do a backup of the imported image(s) to another physical location, when activated it does looks for specified backup foldername of mounted devices on your system... each found folder is used as basedirectory in the above storage structure and when a image are downloaded from camera it is replicated to found backup destinations."));
  gtk_label_set_line_wrap(GTK_LABEL(l),TRUE);
  gtk_widget_set_size_request(l,400,-1);
  gtk_misc_set_alignment(GTK_MISC(l), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(data->settings.page),l,FALSE,FALSE,0);

  data->settings.backup.enable=gtk_check_button_new_with_label(_("enable backup"));
  gtk_box_pack_start(GTK_BOX(data->settings.page),data->settings.backup.enable,FALSE,FALSE,0);
  g_object_set(data->settings.backup.enable,"tooltip-text",_("check this option to enable automatic backup of imported images"),(char *)NULL);

  data->settings.backup.warn=gtk_check_button_new_with_label(_("warn if no backup destinations are present"));
  gtk_box_pack_start(GTK_BOX(data->settings.page),data->settings.backup.warn,FALSE,FALSE,0);
  g_object_set(data->settings.backup.warn,"tooltip-text",_("check this option to get an interactive warning if no backupdestinations are present"),(char *)NULL);

  data->settings.backup.foldername=(_camera_import_gconf_widget(data,_("backup foldername"),"plugins/capture/camera/backup/foldername"))->widget;
  gtk_box_pack_start(GTK_BOX(data->settings.page),data->settings.backup.foldername,FALSE,FALSE,0);
  g_object_set(data->settings.backup.foldername,"tooltip-text",_("this is the name of folder that indicates a backup destination,\nif such a folder is found in any mounter media it is used as a backup destination."),(char *)NULL);

  if( dt_conf_get_bool("plugins/capture/backup/enable") ) gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( data->settings.backup.enable ), TRUE);
  else
  {
    gtk_widget_set_sensitive( data->settings.backup.warn, FALSE);
    gtk_widget_set_sensitive( data->settings.backup.foldername, FALSE);
  }
  if( dt_conf_get_bool("plugins/capture/backup/warn") ) gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( data->settings.backup.warn ), TRUE);

  g_signal_connect (G_OBJECT (data->settings.backup.enable), "clicked",G_CALLBACK (_check_button_callback),data);
  g_signal_connect (G_OBJECT(data->settings.backup.warn), "clicked",G_CALLBACK (_check_button_callback),data);


  // THE NOTEBOOOK
  data->notebook=gtk_notebook_new();
  gtk_notebook_append_page(GTK_NOTEBOOK(data->notebook),data->import.page,gtk_label_new(_("images")));
  gtk_notebook_append_page(GTK_NOTEBOOK(data->notebook),data->settings.page,gtk_label_new(_("settings")));

  // end
  gtk_box_pack_start(GTK_BOX(content),data->notebook,TRUE,TRUE,0);
  //gtk_widget_set_size_request(content,400,400);
  _update_example(data);
}

int _camera_storage_image_filename(const dt_camera_t *camera,const char *filename,CameraFile *preview,CameraFile *exif,void *user_data)
{
  _camera_import_dialog_t *data=(_camera_import_dialog_t*)user_data;
  GtkTreeIter iter;
  const char *img;
  unsigned long size;
  GdkPixbuf *pixbuf=NULL;
  GdkPixbuf *thumb=NULL;

  /* stop fetching previews if job is cancelled */
  if (data->preview_job && dt_control_job_get_state(data->preview_job) == DT_JOB_STATE_CANCELLED )
    return 0;


  gboolean i_own_lock = dt_control_gdk_lock();
  char exif_info[1024]= {0};
  char file_info[4096]= {0};

  if( preview )
  {
    gp_file_get_data_and_size(preview, &img, &size);
    if( size > 0 )
    {
      // we got preview image data lets create a pixbuf from image blob
      GError *err=NULL;
      GInputStream *stream;
      if( (stream = g_memory_input_stream_new_from_data(img, size,NULL)) !=NULL)
        pixbuf = gdk_pixbuf_new_from_stream( stream, NULL, &err );
    }

    if(pixbuf)
    {
      // Scale pixbuf to a thumbnail
      double sw=gdk_pixbuf_get_width( pixbuf );
      double scale=75.0/gdk_pixbuf_get_height( pixbuf );
      thumb = gdk_pixbuf_scale_simple( pixbuf, sw*scale,75 , GDK_INTERP_BILINEAR );
    }
  }

#if 0
  // libgphoto only supports fetching exif in jpegs, not raw
  char buffer[1024]= {0};
  if ( exif )
  {
    const char *exif_data;
    char *value=NULL;
    gp_file_get_data_and_size(exif, &exif_data, &size);
    if( size > 0 )
    {
      void *exif=dt_exif_data_new((uint8_t *)exif_data,size);
      if( (value=g_strdup( dt_exif_data_get_value(exif,"Exif.Photo.ExposureTime",buffer,1024) ) ) != NULL);
      sprintf(exif_info,"exposure: %s\n", value);
    }
    else fprintf(stderr,"No exifdata read\n");
  }
#endif

  // filename\n 1/60 f/2.8 24mm iso 160
  sprintf(file_info,"%s%c%s",filename,strlen(exif_info)?'\n':'\0',strlen(exif_info)?exif_info:"");
  gtk_list_store_append(data->store,&iter);
  gtk_list_store_set(data->store,&iter,0,thumb,1,file_info,-1);
  if(pixbuf) g_object_unref(pixbuf);
  if(thumb) g_object_ref(thumb);

  if (i_own_lock) dt_control_gdk_unlock();

  return 1;
}

void _camera_import_dialog_free(_camera_import_dialog_t *data)
{
  gtk_list_store_clear( data->store );
  g_object_unref( data->store );
  g_free( data->vp );
}

static void _control_status(dt_camctl_status_t status,void *user_data)
{
  _camera_import_dialog_t *data=(_camera_import_dialog_t*)user_data;
  switch( status )
  {
    case CAMERA_CONTROL_BUSY:
    {
      gtk_dialog_set_response_sensitive(GTK_DIALOG( data->dialog ), GTK_RESPONSE_ACCEPT, FALSE);
      gtk_dialog_set_response_sensitive(GTK_DIALOG( data->dialog ), GTK_RESPONSE_NONE, FALSE);

    }
    break;
    case CAMERA_CONTROL_AVAILABLE:
    {
      gtk_dialog_set_response_sensitive(GTK_DIALOG( data->dialog ), GTK_RESPONSE_ACCEPT, TRUE);
      gtk_dialog_set_response_sensitive(GTK_DIALOG( data->dialog ), GTK_RESPONSE_NONE, TRUE);

    }
    break;
  }
}

static void _preview_job_state_changed(dt_job_t *job,int state)
{
  _camera_import_dialog_t *data=(_camera_import_dialog_t*)job->user_data;
  /* store job reference if needed for cancelation */
  if(  state == DT_JOB_STATE_RUNNING )
    data->preview_job = job;
  else if (state == DT_JOB_STATE_FINISHED)
    data->preview_job = NULL;


}

static gboolean _dialog_close(GtkWidget *window, GdkEvent  *event,gpointer   user_data )
{
  _camera_import_dialog_t *data=(_camera_import_dialog_t*)user_data;

  if (data->preview_job)
  {
    /* cancel preview fetching job */
    dt_control_job_cancel (data->preview_job);

    /* wait for job execution to signal finished */
    dt_control_job_wait (data->preview_job);
  }
  return FALSE;
}

static time_t parse_date_time(const char* date_time_text)
{
  struct tm t;
  memset(&t, 0, sizeof(t));

  const char* end = NULL;
  if((end = strptime(date_time_text, "%Y-%m-%dT%T", &t)) && *end == 0)
    return mktime(&t);
  if((end = strptime(date_time_text, "%Y-%m-%d", &t)) && *end == 0)
    return mktime(&t);

  return 0;
}

void _camera_import_dialog_run(_camera_import_dialog_t *data)
{
  gtk_widget_show_all(data->dialog);

  // Populate store

  // Setup a listener for previews of all files on camera
  // then initiate fetch of all previews from camera
  if(data->params->camera!=NULL)
  {
    /* setup a camctl listener */
    dt_camctl_listener_t listener= {0};
    listener.data=data;
    listener.control_status=_control_status;
    listener.camera_storage_image_filename=_camera_storage_image_filename;

    dt_job_t job;
    dt_camera_get_previews_job_init(&job,data->params->camera, &listener, CAMCTL_IMAGE_PREVIEW_DATA);
    dt_control_job_set_state_callback(&job,_preview_job_state_changed,data);
    dt_control_add_job(darktable.control, &job);
  }
  else
    return;

  // Lets run dialog
  gtk_label_set_text(GTK_LABEL(data->import.info),_("select the images from the list below that you want to import into a new filmroll"));
  gboolean all_good=FALSE;
  g_signal_connect(G_OBJECT(data->dialog),"delete-event",G_CALLBACK(_dialog_close),data);
  while(!all_good)
  {
    gint result = gtk_dialog_run (GTK_DIALOG (data->dialog));
    if( result == GTK_RESPONSE_ACCEPT)
    {
      GtkTreeIter iter;
      all_good=TRUE;
      GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gtk_bin_get_child(GTK_BIN(data->import.treeview))));
      // Now build up result from store into GList **result
      if(data->params->result)
        g_list_free(data->params->result);
      data->params->result=NULL;
      GtkTreeModel *model=GTK_TREE_MODEL(data->store);
      GList *sp= gtk_tree_selection_get_selected_rows(selection,&model);
      if( sp )
      {
        do
        {
          GValue value = { 0, };
          gtk_tree_model_get_iter(GTK_TREE_MODEL (data->store),&iter,(GtkTreePath*)sp->data);
          gtk_tree_model_get_value(GTK_TREE_MODEL (data->store),&iter,1,&value);
          if (G_VALUE_HOLDS_STRING (&value))
            data->params->result = g_list_append(data->params->result,g_strdup(g_value_get_string(&value)) );
        }
        while( (sp=g_list_next(sp)) );
      }

      // Lets check jobcode, basedir etc..
      data->params->jobcode = data->import.jobname->value;
      data->params->basedirectory = data->settings.basedirectory->value;
      data->params->subdirectory = data->settings.subdirectory->value;
      data->params->filenamepattern = data->settings.namepattern->value;

      data->params->time_override = 0;
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->settings.general.date_override)))
        data->params->time_override = parse_date_time(gtk_entry_get_text(GTK_ENTRY(data->settings.general.date_entry)));

      if( data->params->jobcode == NULL || strlen(data->params->jobcode) <=0 )
        data->params->jobcode = dt_conf_get_string("plugins/capture/camera/import/jobcode");

      if( data->params->basedirectory == NULL || strlen( data->params->basedirectory ) <= 0 )
      {
        GtkWidget *dialog=gtk_message_dialog_new(NULL,GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("please set the basedirectory settings before importing"));
        g_signal_connect_swapped (dialog, "response",G_CALLBACK (gtk_widget_destroy),dialog);
        gtk_dialog_run (GTK_DIALOG (dialog));
        all_good=FALSE;
      }
      else if( data->params->subdirectory == NULL || strlen( data->params->subdirectory ) <= 0 )
      {
        GtkWidget *dialog=gtk_message_dialog_new(NULL,GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("please set the subdirectory settings before importing"));
        g_signal_connect_swapped (dialog, "response",G_CALLBACK (gtk_widget_destroy),dialog);
        gtk_dialog_run (GTK_DIALOG (dialog));
        all_good=FALSE;
      }
      else if( data->params->filenamepattern == NULL || strlen( data->params->filenamepattern ) <= 0 )
      {
        GtkWidget *dialog=gtk_message_dialog_new(NULL,GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("please set the filenamepattern settings before importing"));
        g_signal_connect_swapped (dialog, "response",G_CALLBACK (gtk_widget_destroy),dialog);
        gtk_dialog_run (GTK_DIALOG (dialog));
        all_good=FALSE;
      }
      else if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->settings.general.date_override)) && data->params->time_override == 0)
      {
        GtkWidget *dialog=gtk_message_dialog_new(NULL,GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("please use YYYY-MM-DD format for date override"));
        g_signal_connect_swapped (dialog, "response",G_CALLBACK (gtk_widget_destroy),dialog);
        gtk_dialog_run (GTK_DIALOG (dialog));
        all_good=FALSE;
      }
    }
    else
    {
      data->params->result=NULL;
      all_good=TRUE;
    }
  }

  // Destory and quit
  gtk_widget_destroy (data->dialog);
}

void dt_camera_import_dialog_new(dt_camera_import_dialog_param_t *params)
{
  _camera_import_dialog_t data;
  data.params=params;
  _camera_import_dialog_new(&data);
  _camera_import_dialog_run(&data);
  _camera_import_dialog_free(&data);
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
