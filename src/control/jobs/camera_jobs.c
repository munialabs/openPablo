/*
    This file is part of darktable,
    copyright (c) 2010 - 2011 Henrik Andersson.

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
#include "common/camera_control.h"
#include "common/utility.h"
#include "views/view.h"
#include "control/conf.h"
#include "control/jobs/camera_jobs.h"
#include "gui/gtk.h"

#include <stdio.h>
#include <glib.h>


int32_t dt_captured_image_import_job_run(dt_job_t *job)
{
  dt_captured_image_import_t *t = (dt_captured_image_import_t *)job->param;

  char message[512]= {0};
  snprintf(message, 512, _("importing image %s"), t->filename);
  const guint *jid = dt_control_backgroundjobs_create(darktable.control, 0, message );

  int id = dt_image_import(t->film_id, t->filename, TRUE);
  if(id)
  {
    //dt_film_open(1);
    dt_view_filmstrip_set_active_image(darktable.view_manager,id);
    dt_control_queue_redraw();
    //dt_ctl_switch_mode_to(DT_DEVELOP);
  }
 
  dt_control_backgroundjobs_progress(darktable.control, jid, 1.0);
  dt_control_backgroundjobs_destroy(darktable.control, jid);
  return 0;
}

void dt_captured_image_import_job_init(dt_job_t *job,uint32_t filmid, const char *filename)
{
  dt_control_job_init(job, "import tethered image");
  job->execute = &dt_captured_image_import_job_run;
  dt_captured_image_import_t *t = (dt_captured_image_import_t *)job->param;
  t->filename = g_strdup(filename);
  t->film_id = filmid;
}

int32_t dt_camera_capture_job_run(dt_job_t *job)
{
  dt_camera_capture_t *t=(dt_camera_capture_t*)job->param;
  int total = t->brackets ? t->count * t->brackets : t->count;
  char message[512]= {0};
  double fraction=0;
  snprintf(message, 512, ngettext ("capturing %d image", "capturing %d images", total), total );
  
  /* try to get exp program mode for nikon */
  char *expprogram = (char *)dt_camctl_camera_get_property(darktable.camctl, NULL, "expprogram");
  
  /* if fail, lets try fetching mode for cannon */
  if(!expprogram) 
    expprogram = (char *)dt_camctl_camera_get_property(darktable.camctl, NULL, "autoexposuremode");

  /* Fetch all values for shutterspeed and initialize current value */
  GList *values=NULL;
  gconstpointer orginal_value=NULL;
  const char *cvalue = dt_camctl_camera_get_property(darktable.camctl, NULL, "shutterspeed");
  const char *value = dt_camctl_camera_property_get_first_choice(darktable.camctl, NULL, "shutterspeed");
  
  /* get values for bracketing */
  if (t->brackets && expprogram && expprogram[0]=='M' && value && cvalue)
  {
    do
    {
      // Add value to list
      values = g_list_append(values, g_strdup(value));
      // Check if current values is the same as orginal value, then lets store item ptr
      if (strcmp(value,cvalue) == 0)
        orginal_value = g_list_last(values)->data;
    }
    while ((value = dt_camctl_camera_property_get_next_choice(darktable.camctl, NULL, "shutterspeed")) != NULL);
  }
  else
  {
    /* if this was an itended bracket capture bail out */
    if(t->brackets)
    {
      dt_control_log(_("please set your camera to manual mode first!"));
      return 1;
    }
  }

  /* create the bgjob plate */
  const guint *jid  = dt_control_backgroundjobs_create(darktable.control, 0, message);

  GList *current_value = g_list_find(values,orginal_value);
  for(int i=0; i<t->count; i++)
  {
    for(int b=0; b<(t->brackets*2)+1; b++)
    {
      // If bracket capture, lets set change shutterspeed
      if (t->brackets)
      {
        if (b == 0)
        {
          // First bracket, step down time with (steps*brackets), also check so we never set the longest shuttertime which would be bulb mode
          for(int s=0; s<(t->steps*t->brackets); s++)
            if (g_list_next(current_value) && g_list_next(g_list_next(current_value)))
              current_value = g_list_next(current_value);
        }
        else
        {
          // Step up with (steps)
          for(int s=0; s<t->steps; s++)
            if(g_list_previous(current_value))
              current_value = g_list_previous(current_value);
        }
      }

      // set the time property for bracked capture
      if (t->brackets && current_value)
        dt_camctl_camera_set_property(darktable.camctl, NULL, "shutterspeed", current_value->data);

      // Capture image
      dt_camctl_camera_capture(darktable.camctl,NULL);

      fraction += 1.0/total;
      dt_control_backgroundjobs_progress(darktable.control, jid, fraction);
    }

    // lets reset to orginal value before continue
    if (t->brackets)
    {
      current_value = g_list_find(values,orginal_value);
      dt_camctl_camera_set_property(darktable.camctl, NULL, "shutterspeed", current_value->data);
    }

    // Delay if active
    if(t->delay)
      g_usleep(t->delay*G_USEC_PER_SEC);

  }

  dt_control_backgroundjobs_destroy(darktable.control, jid);


  // free values
  if(values)
  {
    for(int i=0; i<g_list_length(values); i++)
      g_free(g_list_nth_data(values,i));

    g_list_free(values);
  }

  return 0;
}

void dt_camera_capture_job_init(dt_job_t *job,uint32_t filmid, uint32_t delay, uint32_t count, uint32_t brackets, uint32_t steps)
{
  dt_control_job_init(job, "remote capture of image(s)");
  job->execute = &dt_camera_capture_job_run;
  dt_camera_capture_t *t = (dt_camera_capture_t *)job->param;
  t->film_id=filmid;
  t->delay=delay;
  t->count=count;
  t->brackets=brackets;
  t->steps=steps;
}

void dt_camera_get_previews_job_init(dt_job_t *job,dt_camera_t *camera,dt_camctl_listener_t *listener,uint32_t flags)
{
  dt_control_job_init(job, "get camera previews job");
  job->execute = &dt_camera_get_previews_job_run;
  dt_camera_get_previews_t *t = (dt_camera_get_previews_t *)job->param;

  t->listener=g_malloc(sizeof(dt_camctl_listener_t));
  memcpy(t->listener,listener,sizeof(dt_camctl_listener_t));

  t->camera=camera;
  t->flags=flags;
}

int32_t dt_camera_get_previews_job_run(dt_job_t *job)
{
  dt_camera_get_previews_t *t=(dt_camera_get_previews_t*)job->param;

  dt_camctl_register_listener(darktable.camctl,t->listener);
  dt_camctl_get_previews(darktable.camctl,t->flags,t->camera);
  dt_camctl_unregister_listener(darktable.camctl,t->listener);
  g_free(t->listener);
  return 0;
}

void dt_camera_import_backup_job_init(dt_job_t *job,const char *sourcefile, const char *destinationfile)
{
  dt_control_job_init(job, "backup of imported image from camera");
  job->execute = &dt_camera_import_backup_job_run;
  dt_camera_import_backup_t *t = (dt_camera_import_backup_t *)job->param;
  t->sourcefile=g_strdup(sourcefile);
  t->destinationfile=g_strdup(destinationfile);
}

int32_t dt_camera_import_backup_job_run(dt_job_t *job)
{
  // copy sourcefile to each found destination
  dt_camera_import_backup_t *t = (dt_camera_import_backup_t *)job->param;
  GVolumeMonitor *vmgr= g_volume_monitor_get();
  GList *mounts=g_volume_monitor_get_mounts(vmgr);
  GMount *mount=NULL;
  GFile *root=NULL;
  if( mounts !=NULL )
    do
    {
      mount=G_MOUNT(mounts->data);
      if( ( root=g_mount_get_root( mount ) ) != NULL )
      {
        // Got the mount point lets check for backup folder
        gchar *backuppath=NULL;
        gchar *rootpath=g_file_get_path(root);
        backuppath=g_build_path(G_DIR_SEPARATOR_S,rootpath,dt_conf_get_string("plugins/capture/backup/foldername"),(char *)NULL);
        g_free(rootpath);

        if( g_file_test(backuppath,G_FILE_TEST_EXISTS)==TRUE)
        {
          // Found a backup storage, lets copy file here..
          gchar *destinationfile=g_build_filename(G_DIR_SEPARATOR_S,backuppath,t->destinationfile,(char *)NULL);
          if( g_mkdir_with_parents(g_path_get_dirname(destinationfile),0755) >= 0 )
          {
            gchar *content;
            gsize size;
            if( g_file_get_contents(t->sourcefile,&content,&size,NULL) == TRUE )
            {
              GError *err=NULL;
              if( g_file_set_contents(destinationfile,content,size,&err) != TRUE)
              {
                fprintf(stderr,"Failed to set content of file with reason: %s\n",err->message);
                g_error_free(err);
              }
              g_free(content);
            }
          }
          g_free(destinationfile);
        }

        g_free(backuppath);
      }
    }
    while( (mounts=g_list_next(mounts)) !=NULL);

  // Release volume manager
  g_object_unref(vmgr);
  return 0;
}


void dt_camera_import_job_init(dt_job_t *job,char *jobcode, char *path,char *filename,GList *images, struct dt_camera_t *camera, time_t time_override)
{
  dt_control_job_init(job, "import selected images from camera");
  job->execute = &dt_camera_import_job_run;
  dt_camera_import_t *t = (dt_camera_import_t *)job->param;
  dt_variables_params_init(&t->vp);
  if(time_override != 0)
    dt_variables_set_time(t->vp, time_override);
  t->fraction=0;
  t->images=g_list_copy(images);
  t->camera=camera;
  t->vp->jobcode=g_strdup(jobcode);
  t->path=g_strdup(path);
  t->filename=g_strdup(filename);
  t->import_count=0;
}

/** Listener interface for import job */
void _camera_image_downloaded(const dt_camera_t *camera,const char *filename,void *data)
{
  // Import downloaded image to import filmroll
  dt_camera_import_t *t = (dt_camera_import_t *)data;
  dt_film_image_import(t->film,filename, FALSE);
  dt_control_log(_("%d/%d imported to %s"), t->import_count+1,g_list_length(t->images), g_path_get_basename(filename));

  t->fraction+=1.0/g_list_length(t->images);
  
  dt_control_backgroundjobs_progress(darktable.control, t->bgj, t->fraction );

  if( dt_conf_get_bool("plugins/capture/camera/import/backup/enable") == TRUE )
  {
    // Backup is enable, let's initialize a backup job of imported image...
    char *base=dt_conf_get_string("plugins/capture/storage/basedirectory");
    char *fixed_base=dt_util_fix_path(base);
    dt_variables_expand( t->vp, fixed_base, FALSE );
    g_free(base);
    const char *sdpart=dt_variables_get_result(t->vp);
    if( sdpart )
    {
      // Initialize a image backup job of file
      dt_job_t j;
      dt_camera_import_backup_job_init(&j, filename,filename+strlen(sdpart));
      dt_control_add_job(darktable.control, &j);
    }
  }
  t->import_count++;
}

const char *_camera_import_request_image_filename(const dt_camera_t *camera,const char *filename,void *data)
{
  dt_camera_import_t *t = (dt_camera_import_t *)data;
  t->vp->filename=filename;

  gchar* fixed_path = dt_util_fix_path(t->path);
  g_free(t->path);
  t->path = fixed_path;
  dt_variables_expand( t->vp, t->path, FALSE );
  const gchar *storage=dt_variables_get_result(t->vp);

  dt_variables_expand( t->vp, t->filename, TRUE );
  const gchar *file = dt_variables_get_result(t->vp);

  // Start check if file exist if it does, increase sequence and check again til we know that file doesnt exists..
  gchar *fullfile=g_build_path(G_DIR_SEPARATOR_S,storage,file,(char *)NULL);
  if( g_file_test(fullfile, G_FILE_TEST_EXISTS) == TRUE )
  {
    do
    {
      g_free(fullfile);
      dt_variables_expand( t->vp, t->filename, TRUE );
      file = dt_variables_get_result(t->vp);
      fullfile=g_build_path(G_DIR_SEPARATOR_S,storage,file,(char *)NULL);
    }
    while( g_file_test(fullfile, G_FILE_TEST_EXISTS) == TRUE);
  }

  return file;
}

const char *_camera_import_request_image_path(const dt_camera_t *camera,void *data)
{
  // :) yeap this is kind of stupid yes..
  dt_camera_import_t *t = (dt_camera_import_t *)data;
  return t->film->dirname;
}

int32_t dt_camera_import_job_run(dt_job_t *job)
{
  dt_camera_import_t *t = (dt_camera_import_t *)job->param;
  dt_control_log(_("starting to import images from camera"));

  // Setup a new filmroll to import images to....
  t->film=(dt_film_t*)g_malloc(sizeof(dt_film_t));

  dt_film_init(t->film);

  gchar* fixed_path = dt_util_fix_path(t->path);
  g_free(t->path);
  t->path = fixed_path;
  dt_variables_expand( t->vp, t->path, FALSE );
  sprintf(t->film->dirname,"%s",dt_variables_get_result(t->vp));

  dt_pthread_mutex_lock(&t->film->images_mutex);
  t->film->ref++;
  dt_pthread_mutex_unlock(&t->film->images_mutex);

  // Create recursive directories, abort if no access
  if( g_mkdir_with_parents(t->film->dirname,0755) == -1 )
  {
    dt_control_log(_("failed to create import path `%s', import aborted."), t->film->dirname);
    return 1;
  }

  // Import path is ok, lets actually create the filmroll in database..
  if(dt_film_new(t->film,t->film->dirname) > 0)
  {
    int total = g_list_length( t->images );
    char message[512]= {0};
    sprintf(message, ngettext ("importing %d image from camera", "importing %d images from camera", total), total );
    t->bgj = dt_control_backgroundjobs_create(darktable.control, 0, message);

    // Switch to new filmroll
    dt_film_open(t->film->id);
    dt_ctl_switch_mode_to(DT_LIBRARY);

    // register listener
    dt_camctl_listener_t listener= {0};
    listener.data=t;
    listener.image_downloaded=_camera_image_downloaded;
    listener.request_image_path=_camera_import_request_image_path;
    listener.request_image_filename=_camera_import_request_image_filename;

    //  start download of images
    dt_camctl_register_listener(darktable.camctl,&listener);
    dt_camctl_import(darktable.camctl,t->camera,t->images,dt_conf_get_bool("plugins/capture/camera/import/delete_originals"));
    dt_camctl_unregister_listener(darktable.camctl,&listener);
    dt_control_backgroundjobs_destroy(darktable.control, t->bgj);
    dt_variables_params_destroy(t->vp);
  }
  else
    dt_control_log(_("failed to create filmroll for camera import, import aborted."));

  dt_pthread_mutex_lock(&t->film->images_mutex);
  t->film->ref--;
  dt_pthread_mutex_unlock(&t->film->images_mutex);
  return 0;
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
