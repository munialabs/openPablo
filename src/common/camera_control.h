/*
    This file is part of darktable,
    copyright (c) 2010-2011 henrik andersson.

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

#ifndef DT_CAMERA_CONTROL_H
#define DT_CAMERA_CONTROL_H

#include "common/darktable.h"

#include <gphoto2/gphoto2.h>
#include <glib.h>
#include <gtk/gtk.h>


/** A camera object used for camera actions and callbacks */
typedef struct dt_camera_t
{
  /** A pointer to the model string of camera. */
  const char *model;
  /** A pointer to the port string of camera. */
  const char *port;
  /** Camera summary text */
  CameraText summary;

  /** Camera configuration cache */
  CameraWidget *configuration;

  /** Registered timeout func */  
  CameraTimeoutFunc timeout;

  gboolean config_changed;
  dt_pthread_mutex_t config_lock;
  /** This camera/device can import images. */
  gboolean can_import;
  /** This camera/device can do tethered shoots. */
  gboolean can_tether;
  /** This camera/device can be remote controlled. */
  gboolean can_config;

  /** Flag camera in tethering mode. \see dt_camera_tether_mode() */
  gboolean is_tethering;

  /** A mutex lock for jobqueue */
  dt_pthread_mutex_t jobqueue_lock;
  /** The jobqueue */
  GList *jobqueue;

  struct
  {
    CameraWidget *widget;
    uint32_t index;
  } current_choice;

  /** gphoto2 camera pointer */
  Camera *gpcam;

  /** gphoto2 context */
  GPContext *gpcontext;
}
dt_camera_t;

/** Camera control status.
  These enumerations are passed back to host application using
  listener interface function control_status().
*/
typedef enum dt_camctl_status_t
{
  /** Camera control is busy, operations will block . \remarks Technically this means that the dt_camctl_t.mutex is locked*/
  CAMERA_CONTROL_BUSY,
  /** Camera control is available. \remarks dt_camctl_t.mutex is freed */
  CAMERA_CONTROL_AVAILABLE
}
dt_camctl_status_t;

/** Camera control errors.
  These enumerations are passed to the host application using
  listener interface function camera_error().
*/
typedef enum dt_camera_error_t
{
  /** Locking camera failed. \remarks This means that camera control is busy and locking failed. */
  CAMERA_LOCK_FAILED,
  /**  Camera conenction is broken and unuseable.
  \remarks Beyond this message references to dt_camera_t pointer is invalid, which means that the host application should remove all references of camera pointer and disallow any opertations onto it.
   */
  CAMERA_CONNECTION_BROKEN
}
dt_camera_error_t;

/** Context of camera control */
typedef struct dt_camctl_t
{
  dt_pthread_mutex_t lock;
  /** Camera event thread. */
  pthread_t camera_event_thread;
  /** List of registered listeners of camera control. \see dt_camctl_register_listener() , dt_camctl_unregister_listener() */
  GList *listeners;
  /** List of cameras found and initialized by camera control.*/
  GList *cameras;

  /** The actual gphoto2 context */
  GPContext *gpcontext;
  /** List of gphoto2 port drivers */
  GPPortInfoList *gpports;
  /** List of gphoto2 camera drivers */
  CameraAbilitiesList *gpcams;

  /** The host application want to use this camera. \see dt_camctl_select_camera() */
  const dt_camera_t *wanted_camera;

  const dt_camera_t *active_camera;
}
dt_camctl_t;


typedef struct dt_camctl_listener_t
{
  void *data;
  /** Invoked when a image is downloaded while in tethered mode or  by import. \see dt_camctl_status_t */
  void (*control_status)(dt_camctl_status_t status,void *data);

  /** Invoked before images are fetched from camera and when tethered capture fetching an image. \note That only one listener should implement this at time... */
  const char * (*request_image_path)(const dt_camera_t *camera,void *data);

  /** Invoked before images are fetched from camera and when tethered capture fetching an image. \note That only one listener should implement this at time... */
  const char * (*request_image_filename)(const dt_camera_t *camera,const char *filename,void *data);

  /** Invoked when a image is downloaded while in tethered mode or  by import */
  void (*image_downloaded)(const dt_camera_t *camera,const char *filename,void *data);

  /** Invoked when a image is found on storage.. such as from dt_camctl_get_previews(), if 0 is returned the recurse is stopped.. */
  int (*camera_storage_image_filename)(const dt_camera_t *camera,const char *filename,CameraFile *preview,CameraFile *exif,void *data);

  /** Invoked when a value of a property is changed. */
  void (*camera_property_value_changed)(const dt_camera_t *camera,const char *name,const char *value,void *data);
  /** Invoked when accesibility of a property is changed. */
  void (*camera_property_accessibility_changed)(const dt_camera_t *camera,const char *name,gboolean read_only,void *data);

  /** Invoked from dt_camctl_detect_cameras() when a new camera is connected */
  void (*camera_connected)(const dt_camera_t *camera,void *data);
  /** Invoked from dt_camctl_detect_cameras() when a new camera is disconnected, or when connection is broken and camera is unuseable */
  void (*camera_disconnected)(const dt_camera_t *camera,void *data);
  /** Invoked when a error occured \see dt_camera_error_t */
  void (*camera_error)(const dt_camera_t *camera,dt_camera_error_t error,void *data);
} dt_camctl_listener_t;


typedef enum dt_camera_preview_flags_t
{
  /** No image data */
  CAMCTL_IMAGE_NO_DATA=0,
  /**Get a image  preview. */
  CAMCTL_IMAGE_PREVIEW_DATA=1,
  /**Get the image exif */
  CAMCTL_IMAGE_EXIF_DATA=2
}
dt_camera_preview_flags_t;


/** Initializes the gphoto and cam control, returns NULL if failed */
dt_camctl_t *dt_camctl_new();
/** Destroys the came control */
void dt_camctl_destroy(const dt_camctl_t *c);
/** Registers a listener of camera control */
void dt_camctl_register_listener(const dt_camctl_t *c, dt_camctl_listener_t *listener);
/** Unregisters a listener of camera control */
void dt_camctl_unregister_listener(const dt_camctl_t *c, dt_camctl_listener_t *listener);
/** Detect cameras and update list of available cameras */
void dt_camctl_detect_cameras(const dt_camctl_t *c);
/** Check if there is any camera connected */
int dt_camctl_have_cameras(const dt_camctl_t *c);
/** Selects a camera to be used by cam control, this camera is selected if NULL is passed as camera*/
void dt_camctl_select_camera(const dt_camctl_t *c, const dt_camera_t *cam);
/** Can tether...*/
int dt_camctl_can_enter_tether_mode(const dt_camctl_t *c, const dt_camera_t *cam);
/** Enables/Disables the tether mode on camera. */
void dt_camctl_tether_mode(const dt_camctl_t *c,const dt_camera_t *cam,gboolean enable);
/** travers filesystem on camera an retreives previews of images */
void dt_camctl_get_previews(const dt_camctl_t *c,dt_camera_preview_flags_t flags,const dt_camera_t *cam);
/** Imports the images in list from specified camera */
void dt_camctl_import(const dt_camctl_t *c,const dt_camera_t *cam,GList *images,gboolean delete_orginals);

/** Execute remote capture of camera.*/
void dt_camctl_camera_capture(const dt_camctl_t *c,const dt_camera_t *cam);
/** Returns a model string of camera.*/
const char *dt_camctl_camera_get_model(const dt_camctl_t *c,const dt_camera_t *cam);

/** Set a property value \param cam Pointer to dt_camera_t if NULL the camctl->active_camera is used. */
void dt_camctl_camera_set_property(const dt_camctl_t *c,const dt_camera_t *cam,const char *property_name, const char *value);
/** Get a property value from chached configuration. \param cam Pointer to dt_camera_t if NULL the camctl->active_camera is used. */
const char*dt_camctl_camera_get_property(const dt_camctl_t *c,const dt_camera_t *cam,const char *property_name);
/** Check if property exists. */
int dt_camctl_camera_property_exists(const dt_camctl_t *c,const dt_camera_t *cam,const char *property_name);
/** Get first choice availble for named property. */
const char *dt_camctl_camera_property_get_first_choice(const dt_camctl_t *c,const dt_camera_t *cam,const char *property_name);
/** Get next choice availble for named property. */
const char *dt_camctl_camera_property_get_next_choice(const dt_camctl_t *c,const dt_camera_t *cam,const char *property_name);

/** build a popupmenu with all properties available */
void dt_camctl_camera_build_property_menu (const dt_camctl_t *c,const dt_camera_t *cam,GtkMenu **menu, GCallback item_activate, gpointer user_data);

#endif
