/*
    This file is part of darktable,
    copyright (c) 2011 Henrik Andersson.

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
#include "common/film.h"
#include "common/collection.h"
#include "common/image_cache.h"
#include "common/mipmap_cache.h"
#include "control/control.h"
#include "control/conf.h"
#include "control/jobs/camera_jobs.h"
#include "dtgtk/label.h"
#include "dtgtk/button.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include "gui/camera_import_dialog.h"
#include "libs/lib.h"

DT_MODULE(1)

// #ifdef HAVE_GPHOTO2


#ifdef HAVE_GPHOTO2
/** helper function to update ui with available cameras and ther actionbuttons */
static void _lib_import_ui_devices_update(dt_lib_module_t *self);
#endif


typedef struct dt_lib_import_t
{
  dt_camctl_listener_t camctl_listener;
  GtkButton *import_file;
  GtkButton *import_directory;
  GtkButton *import_camera;
  GtkButton *scan_devices;
  GtkButton *tethered_shoot;

  GtkBox *devices;
}
dt_lib_import_t;

const char* name()
{
  return _("import");
}


uint32_t views()
{
  return DT_VIEW_LIGHTTABLE;
}

uint32_t container()
{
  return DT_UI_CONTAINER_PANEL_LEFT_CENTER;
}

int position()
{
  return 999;
}

void init_key_accels(dt_lib_module_t *self)
{
  dt_accel_register_lib(self, NC_("accel", "scan for devices"), 0, 0);
  dt_accel_register_lib(self, NC_("accel", "import from camera"), 0, 0);
  dt_accel_register_lib(self, NC_("accel", "tethered shoot"), 0, 0);
  dt_accel_register_lib(self, NC_("accel", "import image"), 0, 0);
}

void connect_key_accels(dt_lib_module_t *self)
{
  dt_lib_import_t *d = (dt_lib_import_t*)self->data;

  dt_accel_connect_button_lib(self, "scan for devices",
                              GTK_WIDGET(d->scan_devices));
  dt_accel_connect_button_lib(self, "import image",
                              GTK_WIDGET(d->import_file));
  if(d->tethered_shoot)
    dt_accel_connect_button_lib(self, "tethered shoot",
                                GTK_WIDGET(d->tethered_shoot));
  if(d->import_camera)
    dt_accel_connect_button_lib(self, "import from camera",
                                GTK_WIDGET(d->import_camera));
}

#ifdef HAVE_GPHOTO2

/* scan for new devices button callback */
static void _lib_import_scan_devices_callback(GtkButton *button,gpointer data)
{
  /* detect cameras */
  dt_camctl_detect_cameras(darktable.camctl);
  /* update UI */
  _lib_import_ui_devices_update(data);
}

/* show import from camera dialog */
static void _lib_import_from_camera_callback(GtkButton *button,gpointer data)
{
  dt_camera_import_dialog_param_t *params=(dt_camera_import_dialog_param_t *)g_malloc(sizeof(dt_camera_import_dialog_param_t));
  memset( params, 0, sizeof(dt_camera_import_dialog_param_t));
  params->camera = (dt_camera_t*)data;

  dt_camera_import_dialog_new(params);
  if( params->result )
  {
    /* initialize a import job and put it on queue.... */
    gchar *path = g_build_path(G_DIR_SEPARATOR_S,params->basedirectory,params->subdirectory,(char *)NULL);
    dt_job_t j;
    dt_camera_import_job_init(&j,params->jobcode,path,params->filenamepattern,params->result,params->camera,params->time_override);
    dt_control_add_job(darktable.control, &j);
    g_free(path);
  }
  g_free(params);
}

/* enter tethering mode for camera */
static void _lib_import_tethered_callback(GtkToggleButton *button,gpointer data)
{
  /* select camera to work with before switching mode */
  dt_camctl_select_camera(darktable.camctl, (dt_camera_t *)data);
  dt_conf_set_int( "plugins/capture/mode", DT_CAPTURE_MODE_TETHERED);
  dt_conf_set_int("plugins/capture/current_filmroll",-1);
  dt_ctl_switch_mode_to(DT_CAPTURE);
}


/** update the device list */
void _lib_import_ui_devices_update(dt_lib_module_t *self)
{

  dt_lib_import_t *d = (dt_lib_import_t*)self->data;

  GList *citem;

  /* cleanup of widgets in devices container*/
  GList *item;
  if((item=gtk_container_get_children(GTK_CONTAINER(d->devices)))!=NULL)
    do
    {
      gtk_container_remove(GTK_CONTAINER(d->devices),GTK_WIDGET(item->data));
    }
    while((item=g_list_next(item))!=NULL);

  /* add the rescan button */
  GtkButton *scan = GTK_BUTTON(gtk_button_new_with_label(_("scan for devices")));
  d->scan_devices = scan;
  gtk_button_set_alignment(scan, 0.05, 0.5);
  g_object_set(G_OBJECT(scan), "tooltip-text", _("scan for newly attached devices"), (char *)NULL);
  g_signal_connect (G_OBJECT(scan), "clicked",G_CALLBACK (_lib_import_scan_devices_callback), self);
  gtk_box_pack_start(GTK_BOX(d->devices),GTK_WIDGET(scan),TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(d->devices),GTK_WIDGET(gtk_label_new("")),TRUE,TRUE,0);

  uint32_t count=0;
  /* FIXME: Verify that it's safe to access camctl->cameras list here ? */
  if( (citem = g_list_first (darktable.camctl->cameras))!=NULL)
  {
    // Add detected supported devices
    char buffer[512]= {0};
    do
    {
      dt_camera_t *camera=(dt_camera_t *)citem->data;
      count++;

      /* add camera label */
      GtkWidget *label = GTK_WIDGET (dtgtk_label_new (camera->model,DARKTABLE_LABEL_TAB|DARKTABLE_LABEL_ALIGN_LEFT));
      gtk_box_pack_start (GTK_BOX (d->devices),label,TRUE,TRUE,0);

      /* set camera summary if available */
      if( camera->summary.text !=NULL && strlen(camera->summary.text) >0 )
      {
        g_object_set(G_OBJECT(label), "tooltip-text", camera->summary.text, (char *)NULL);
      }
      else
      {
        sprintf(buffer,_("device \"%s\" connected on port \"%s\"."),camera->model,camera->port);
        g_object_set(G_OBJECT(label), "tooltip-text", buffer, (char *)NULL);
      }

      /* add camera actions buttons */
      GtkWidget *ib=NULL,*tb=NULL;
      GtkWidget *vbx=gtk_vbox_new(FALSE,5);
      if( camera->can_import==TRUE ) {
        gtk_box_pack_start (GTK_BOX (vbx),(ib=gtk_button_new_with_label (_("import from camera"))),FALSE,FALSE,0);
        d->import_camera = GTK_BUTTON(ib);
      }
      if( camera->can_tether==TRUE ) {
        gtk_box_pack_start (GTK_BOX (vbx),(tb=gtk_button_new_with_label (_("tethered shoot"))),FALSE,FALSE,0);
        d->tethered_shoot = GTK_BUTTON(tb);
      }

      if( ib )
      {
        g_signal_connect (G_OBJECT (ib), "clicked",G_CALLBACK (_lib_import_from_camera_callback), camera);
        gtk_button_set_alignment(GTK_BUTTON(ib), 0.05, 0.5);
      }
      if( tb )
      {
        g_signal_connect (G_OBJECT (tb), "clicked",G_CALLBACK (_lib_import_tethered_callback), camera);
        gtk_button_set_alignment(GTK_BUTTON(tb), 0.05, 0.5);
      }
      gtk_box_pack_start (GTK_BOX (d->devices),vbx,FALSE,FALSE,0);
    }
    while ((citem=g_list_next (citem))!=NULL);
  }

  if( count == 0 )
  {
    // No supported devices is detected lets notice user..
    gtk_box_pack_start(GTK_BOX(d->devices),gtk_label_new(_("no supported devices found")),TRUE,TRUE,0);
  }
  gtk_widget_show_all(GTK_WIDGET(d->devices));
}

/** camctl camera disconnect callback */
static void _camctl_camera_disconnected_callback (const dt_camera_t *camera,void *data)
{
  dt_lib_module_t *self = (dt_lib_module_t *)data;


  /* rescan connected cameras */
  dt_camctl_detect_cameras(darktable.camctl);
  
  /* update gui with detected devices */
  gboolean i_own_lock = dt_control_gdk_lock();
  _lib_import_ui_devices_update(self);
  if(i_own_lock) dt_control_gdk_unlock();
}

/** camctl status listener callback */
static void _camctl_camera_control_status_callback(dt_camctl_status_t status,void *data)
{
  dt_lib_module_t *self = (dt_lib_module_t *)data;
  dt_lib_import_t *d = (dt_lib_import_t*)self->data;

  /* check if we need gdk locking */
  gboolean i_have_lock = dt_control_gdk_lock();

  /* handle camctl status */
  switch(status)
  {
    case CAMERA_CONTROL_BUSY:
    {
      /* set all devicese as inaccessible */
      GList *child = gtk_container_get_children(GTK_CONTAINER(d->devices));
      if(child)
        do
        {
          if( !(GTK_IS_TOGGLE_BUTTON(child->data)  && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(child->data))==TRUE) )
            gtk_widget_set_sensitive(GTK_WIDGET(child->data),FALSE);
        }
        while( (child=g_list_next(child)) );
    }
    break;

    case CAMERA_CONTROL_AVAILABLE:
    {
      /* set all devices as accessible */
      GList *child = gtk_container_get_children(GTK_CONTAINER(d->devices));
      if(child)
        do
        {
          gtk_widget_set_sensitive(GTK_WIDGET(child->data),TRUE);
        }
        while( (child=g_list_next(child)) );
    }
    break;
  }

  /* unlock */
  if(i_have_lock) dt_control_gdk_unlock();
}

#endif // HAVE_GPHOTO2

static void _lib_import_single_image_callback(GtkWidget *widget,gpointer user_data) 
{
  GtkWidget *win = dt_ui_main_window(darktable.gui->ui);
  GtkWidget *filechooser = gtk_file_chooser_dialog_new (_("import image"),
                           GTK_WINDOW (win),
                           GTK_FILE_CHOOSER_ACTION_OPEN,
                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                           GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                           (char *)NULL);

  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filechooser), TRUE);

  char *last_directory = dt_conf_get_string("ui_last/import_last_directory");
  if(last_directory != NULL)
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (filechooser), last_directory);

  char *cp, **extensions, ext[1024];
  GtkFileFilter *filter;
  filter = GTK_FILE_FILTER(gtk_file_filter_new());
  extensions = g_strsplit(dt_supported_extensions, ",", 100);
  for(char **i=extensions; *i!=NULL; i++)
  {
    snprintf(ext, 1024, "*.%s", *i);
    gtk_file_filter_add_pattern(filter, ext);
    gtk_file_filter_add_pattern(filter, cp=g_ascii_strup(ext, -1));
    g_free(cp);
  }
  g_strfreev(extensions);
  gtk_file_filter_set_name(filter, _("supported images"));
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filechooser), filter);

  filter = GTK_FILE_FILTER(gtk_file_filter_new());
  gtk_file_filter_add_pattern(filter, "*");
  gtk_file_filter_set_name(filter, _("all files"));
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filechooser), filter);

  if (gtk_dialog_run (GTK_DIALOG (filechooser)) == GTK_RESPONSE_ACCEPT)
  {
    dt_conf_set_string("ui_last/import_last_directory", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (filechooser)));

    char *filename = NULL;
    dt_film_t film;
    GSList *list = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (filechooser));
    GSList *it = list;
    int id = 0;
    int filmid = 0;
    while(it)
    {
      filename = (char *)it->data;
      gchar *directory = g_path_get_dirname((const gchar *)filename);
      filmid = dt_film_new(&film, directory);
      id = dt_image_import(filmid, filename, TRUE);
      if(!id) dt_control_log(_("error loading file `%s'"), filename);
      g_free (filename);
      g_free (directory);
      it = g_slist_next(it);
    }

    if(id)
    {
      dt_film_open(filmid);
      // make sure buffers are loaded (load full for testing)
      dt_mipmap_buffer_t buf;
      dt_mipmap_cache_read_get(darktable.mipmap_cache, &buf, id, DT_MIPMAP_FULL, DT_MIPMAP_BLOCKING);
      if(!buf.buf)
      {
        dt_control_log(_("file has unknown format!"));
      }
      else
      {
        dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);
        DT_CTL_SET_GLOBAL(lib_image_mouse_over_id, id);
        dt_ctl_switch_mode_to(DT_DEVELOP);
      }
    }
  }
  gtk_widget_destroy (filechooser);
  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
}

static void _lib_import_folder_callback(GtkWidget *widget,gpointer user_data) 
{
  GtkWidget *win = dt_ui_main_window(darktable.gui->ui);
  GtkWidget *filechooser = gtk_file_chooser_dialog_new (_("import film"),
                           GTK_WINDOW (win),
                           GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                           GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                           (char *)NULL);

  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filechooser), TRUE);

  char *last_directory = dt_conf_get_string("ui_last/import_last_directory");
  if(last_directory != NULL)
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (filechooser), last_directory);

  // add extra lines to 'extra'. don't forget to destroy the widgets later.
  GtkWidget *extra;
  extra = gtk_vbox_new(FALSE, 0);

  // recursive opening.
  GtkWidget *recursive;
  recursive = gtk_check_button_new_with_label (_("import directories recursively"));
  g_object_set(recursive, "tooltip-text", _("recursively import subdirectories. each directory goes into a new film roll."), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (recursive), dt_conf_get_bool("ui_last/import_recursive"));
  gtk_widget_show (recursive);
  gtk_box_pack_start(GTK_BOX (extra), recursive, FALSE, FALSE, 0);

  // ignoring of jpegs. hack while we don't handle raw+jpeg in the same directories.
  GtkWidget *ignore_jpeg;
  ignore_jpeg = gtk_check_button_new_with_label (_("ignore jpeg files"));
  g_object_set(ignore_jpeg, "tooltip-text", _("do not load files with an extension of .jpg or .jpeg. this can be useful when there are raw+jpeg in a directory."), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (ignore_jpeg), dt_conf_get_bool("ui_last/import_ignore_jpegs"));
  gtk_widget_show (ignore_jpeg);
  gtk_box_pack_start(GTK_BOX (extra), ignore_jpeg, FALSE, FALSE, 0);

  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (filechooser), extra);

  if (gtk_dialog_run (GTK_DIALOG (filechooser)) == GTK_RESPONSE_ACCEPT)
  {
    dt_conf_set_bool("ui_last/import_recursive", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (recursive)));
    dt_conf_set_bool("ui_last/import_ignore_jpegs", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (ignore_jpeg)));
    dt_conf_set_string("ui_last/import_last_directory", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (filechooser)));

    char *filename = NULL, *first_filename = NULL;
    GSList *list = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (filechooser));
    GSList *it = list;

    /* for each selected folder add import job */
    while(it)
    {
      filename = (char *)it->data;
      dt_film_import(filename);
      if (!first_filename)
	first_filename = dt_util_dstrcat(g_strdup(filename), "%%");
      g_free (filename);
      it = g_slist_next(it);
    }

    /* update collection to view import */
    if (first_filename)
    {
	dt_conf_set_int("plugins/lighttable/collect/num_rules", 1);
	dt_conf_set_int("plugins/lighttable/collect/item0", 0);
	dt_conf_set_string("plugins/lighttable/collect/string0",first_filename);
	dt_collection_update_query(darktable.collection);
	g_free(first_filename);
    }


    g_slist_free (list);
  }
  gtk_widget_destroy(recursive);
  gtk_widget_destroy(ignore_jpeg);
  gtk_widget_destroy(extra);
  gtk_widget_destroy (filechooser);
  gtk_widget_queue_draw(dt_ui_center(darktable.gui->ui));
}

void gui_init(dt_lib_module_t *self)
{
  /* initialize ui widgets */
  dt_lib_import_t *d = (dt_lib_import_t *)g_malloc(sizeof(dt_lib_import_t));
  memset(d,0,sizeof(dt_lib_import_t));
  self->data = (void *)d;
  self->widget = gtk_vbox_new(FALSE, 5);

  /* add import singel image buttons */
  GtkWidget *widget = gtk_button_new_with_label(_("image"));
  d->import_file = GTK_BUTTON(widget);
  gtk_button_set_alignment(GTK_BUTTON(widget), 0.05, 5);
  gtk_widget_set_tooltip_text(widget, _("select one or more images to import"));
  gtk_widget_set_can_focus(widget, TRUE);
  gtk_widget_set_receives_default(widget, TRUE);
  gtk_box_pack_start(GTK_BOX(self->widget), widget, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (widget), "clicked",
                    G_CALLBACK (_lib_import_single_image_callback),
                    self);
 
  /* adding the import folder button */
  widget = gtk_button_new_with_label(_("folder"));
  d->import_directory = GTK_BUTTON(widget);
  gtk_button_set_alignment(GTK_BUTTON(widget), 0.05, 5);
  gtk_widget_set_tooltip_text(widget,
                              _("select a folder to import as film roll"));
  gtk_widget_set_can_focus(widget, TRUE);
  gtk_widget_set_receives_default(widget, TRUE);
  gtk_box_pack_start(GTK_BOX(self->widget), widget, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (widget), "clicked",
                    G_CALLBACK (_lib_import_folder_callback),
                    self);

#ifdef HAVE_GPHOTO2
  /* add devices container for cameras */
  d->devices = GTK_BOX(gtk_vbox_new(FALSE,5));
  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(d->devices), FALSE, FALSE, 0);

  /* initialize camctl listener and update devices */
  d->camctl_listener.data = self;
  d->camctl_listener.control_status = _camctl_camera_control_status_callback;
  d->camctl_listener.camera_disconnected = _camctl_camera_disconnected_callback;
  dt_camctl_register_listener(darktable.camctl, &d->camctl_listener );
  _lib_import_ui_devices_update(self);

#endif

}

void gui_cleanup(dt_lib_module_t *self)
{ 
#ifdef HAVE_GPHOTO2
  dt_lib_import_t *d = (dt_lib_import_t*)self->data;
  /* unregister camctl listener */
  dt_camctl_unregister_listener(darktable.camctl, &d->camctl_listener );
#endif

  /* cleanup mem */
  g_free(self->data);
  self->data = NULL;
}
