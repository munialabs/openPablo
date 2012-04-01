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
#include "common/metadata.h"
#include "common/debug.h"
#include "control/control.h"
#include "control/conf.h"
#include "common/image_cache.h"
#include "libs/lib.h"
#include "gui/gtk.h"

DT_MODULE(1)

enum {
  /* internal */
  md_internal_filmroll=0,
  md_internal_imgid,
  md_internal_filename,

  /* exif */
  md_exif_model,
  md_exif_maker,
  md_exif_lens,
  md_exif_aperture,
  md_exif_exposure,
  md_exif_focal_length,
  md_exif_focus_distance,
  md_exif_iso,
  md_exif_datetime,
  md_exif_width,
  md_exif_height,

  /* xmp */
  md_xmp_title,
  md_xmp_creator,
  md_xmp_rights,

  /* entries, do not touch! */
  md_size
};

static gchar *_md_labels[md_size];

/* initialize the labels text */
static void _lib_metatdata_view_init_labels()
{
  /* internal */
  _md_labels[md_internal_filmroll] = _("filmroll");
  _md_labels[md_internal_imgid] = _("image id");
  _md_labels[md_internal_filename] = _("filename");

  /* exif */
  _md_labels[md_exif_model] = _("model");
  _md_labels[md_exif_maker] = _("maker");
  _md_labels[md_exif_lens] = _("lens");
  _md_labels[md_exif_aperture] = _("aperture");
  _md_labels[md_exif_exposure] = _("exposure");
  _md_labels[md_exif_focal_length] = _("focal length");
  _md_labels[md_exif_focus_distance] = _("focus distance");
  _md_labels[md_exif_iso] = _("iso");
  _md_labels[md_exif_datetime] = _("datetime");
  _md_labels[md_exif_width] = _("width");
  _md_labels[md_exif_height] = _("height");

  /* xmp */
  _md_labels[md_xmp_title] = _("title");
  _md_labels[md_xmp_creator] = _("creator");
  _md_labels[md_xmp_rights] = _("copyright");

}


typedef struct dt_lib_metadata_view_t
{
  GtkLabel *metadata[md_size];
}
dt_lib_metadata_view_t;

const char* name()
{
  return _("image information");
}

/* show module in left panel in all views */
uint32_t views()
{
  return DT_VIEW_LIGHTTABLE | DT_VIEW_TETHERING | DT_VIEW_DARKROOM;
}

uint32_t container()
{
  return DT_UI_CONTAINER_PANEL_LEFT_CENTER;
}

int position()
{
  return 299;
}

/* helper function for updating a metadata value */
static void _metadata_update_value(GtkLabel *label, const char *value)
{
    gtk_label_set_text(GTK_LABEL(label), value);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_MIDDLE);
    g_object_set(G_OBJECT(label), "tooltip-text", value, (char *)NULL);
}

static void _metadata_update_value_end(GtkLabel *label, const char *value)
{
    gtk_label_set_text(GTK_LABEL(label), value);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    g_object_set(G_OBJECT(label), "tooltip-text", value, (char *)NULL);
}

#define NODATA_STRING "-"

/* update all values to reflect mouse over image id or no data at all */
static void _metadata_view_update_values(dt_lib_module_t *self)
{
  dt_lib_metadata_view_t *d = (dt_lib_metadata_view_t *)self->data;
  int32_t mouse_over_id = -1;
  DT_CTL_GET_GLOBAL(mouse_over_id, lib_image_mouse_over_id);
  
  if(mouse_over_id >= 0)
  {
    const int vl = 512;
    char value[vl];
    const dt_image_t *img = dt_image_cache_read_get(darktable.image_cache, mouse_over_id);
    if(!img) goto fill_minuses;
    if(img->film_id == -1)
    {
      dt_image_cache_read_release(darktable.image_cache, img);
      goto fill_minuses;
    }

    /* update all metadata */
    
    dt_image_film_roll(img, value, vl);
    _metadata_update_value(d->metadata[md_internal_filmroll], value);

    snprintf(value,vl,"%d", img->id);
    _metadata_update_value(d->metadata[md_internal_imgid], value);

    _metadata_update_value(d->metadata[md_internal_filename], img->filename);

    /* EXIF */
    _metadata_update_value_end(d->metadata[md_exif_model], img->exif_model);
    _metadata_update_value_end(d->metadata[md_exif_lens], img->exif_lens);
    _metadata_update_value_end(d->metadata[md_exif_maker], img->exif_maker);

    snprintf(value, vl, "F/%.1f", img->exif_aperture);
    _metadata_update_value(d->metadata[md_exif_aperture], value);
   
    if(img->exif_exposure <= 0.5) snprintf(value, vl, "1/%.0f", 1.0/img->exif_exposure);
    else                          snprintf(value, vl, "%.1f''", img->exif_exposure);
    _metadata_update_value(d->metadata[md_exif_exposure], value);
   
    snprintf(value, vl, "%.0f", img->exif_focal_length);
    _metadata_update_value(d->metadata[md_exif_focal_length], value);
  
    snprintf(value, vl, "%.0f", img->exif_focus_distance);
    _metadata_update_value(d->metadata[md_exif_focus_distance], value);
 
    snprintf(value, vl, "%.0f", img->exif_iso);
    _metadata_update_value(d->metadata[md_exif_iso], value);
    
    _metadata_update_value(d->metadata[md_exif_datetime], img->exif_datetime_taken);

    snprintf(value, vl, "%d", img->height);
    _metadata_update_value(d->metadata[md_exif_height], value);
    snprintf(value, vl, "%d", img->width);
    _metadata_update_value(d->metadata[md_exif_width], value);
    
    /* XMP */
    GList *res;
    if((res = dt_metadata_get(img->id, "Xmp.dc.title", NULL))!=NULL) {
      snprintf(value, vl, "%s", (char*)res->data);
      g_free(res->data);
      g_list_free(res);
    } else
    snprintf(value, vl, NODATA_STRING);
    _metadata_update_value(d->metadata[md_xmp_title], value);
   
    if((res = dt_metadata_get(img->id, "Xmp.dc.creator", NULL))!=NULL) {
      snprintf(value, vl, "%s", (char*)res->data);
      g_free(res->data);
      g_list_free(res);
    } else
      snprintf(value, vl, NODATA_STRING);
    _metadata_update_value(d->metadata[md_xmp_creator], value);
   
    if((res = dt_metadata_get(img->id, "Xmp.dc.rights", NULL))!=NULL) {
      snprintf(value, vl, "%s", (char*)res->data);
      g_free(res->data);
      g_list_free(res);
    } else
    snprintf(value, vl, NODATA_STRING);
    _metadata_update_value(d->metadata[md_xmp_rights], value);
   
   
    /* release img */
    dt_image_cache_read_release(darktable.image_cache, img);
    
  }

  return;

/* reset */
fill_minuses:
  for(int k=0;k<md_size;k++)
    _metadata_update_value(d->metadata[k],NODATA_STRING);


}

/* calback for the mouse over image change signal */
static void _mouse_over_image_callback(gpointer instance,gpointer user_data) 
{
  dt_lib_module_t *self = (dt_lib_module_t *)user_data;
  if(dt_control_running())
    _metadata_view_update_values(self);
}

void gui_init(dt_lib_module_t *self)
{
  /* initialize ui widgets */
  dt_lib_metadata_view_t *d = (dt_lib_metadata_view_t *)g_malloc(sizeof(dt_lib_metadata_view_t));
  self->data = (void *)d;
  _lib_metatdata_view_init_labels();

  self->widget = gtk_table_new(md_size, 2, FALSE);
  

  /* intialize the metadata name/value labels */
  for (int k = 0;k < md_size;k++) 
  {
    GtkLabel *name = GTK_LABEL(gtk_label_new(_md_labels[k]));
    d->metadata[k] = GTK_LABEL(gtk_label_new("-"));
    gtk_misc_set_alignment(GTK_MISC(name), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(d->metadata[k]), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(name), 0, 1, k, k+1, GTK_FILL, 0, 5, 0);
    gtk_table_attach(GTK_TABLE(self->widget), GTK_WIDGET(d->metadata[k]), 1, 2, k, k+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
  }

  /* lets signup for mouse over image change signals */
  dt_control_signal_connect(darktable.signals,DT_SIGNAL_MOUSE_OVER_IMAGE_CHANGE, 
			    G_CALLBACK(_mouse_over_image_callback), self);

  /* signup for develop initialize to update info of current 
     image in darkroom when enter */
  dt_control_signal_connect(darktable.signals, DT_SIGNAL_DEVELOP_INITIALIZE,
			    G_CALLBACK(_mouse_over_image_callback), self);

}

void gui_cleanup(dt_lib_module_t *self)
{
  dt_control_signal_disconnect(darktable.signals,
			    G_CALLBACK(_mouse_over_image_callback), self);
  g_free(self->data);
  self->data = NULL;
}

