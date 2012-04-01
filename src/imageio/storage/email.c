/*
    This file is part of darktable,
    copyright (c) 2009--2011 Henrik Andersson.

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
#include "common/image.h"
#include "common/image_cache.h"
#include "common/imageio_module.h"
#include "common/imageio.h"
#include "control/control.h"
#include "control/conf.h"
#include "gui/gtk.h"
#include "dtgtk/button.h"
#include "dtgtk/paint.h"
#include <stdio.h>
#include <stdlib.h>

DT_MODULE(1)

typedef struct _email_attachment_t
{
  uint32_t imgid;     // The image id of exported image
  gchar *file;            // Full filename of exported image
}
_email_attachment_t;

// saved params
typedef struct dt_imageio_email_t
{
  char filename[1024];
  GList *images;
}
dt_imageio_email_t;


const char*
name ()
{
  return _("send as email");
}

int recommended_dimension(struct dt_imageio_module_storage_t *self, uint32_t *width, uint32_t *height)
{
  *width=1280;
  *height=1280;
  return 1;
}


void
gui_init (dt_imageio_module_storage_t *self)
{

}

void
gui_cleanup (dt_imageio_module_storage_t *self)
{
  free(self->gui_data);
}

void
gui_reset (dt_imageio_module_storage_t *self)
{

}

int
store (dt_imageio_module_data_t *sdata, const int imgid, dt_imageio_module_format_t *format, dt_imageio_module_data_t *fdata, const int num, const int total)
{
  dt_imageio_email_t *d = (dt_imageio_email_t *)sdata;

  _email_attachment_t *attachment = ( _email_attachment_t *)malloc(sizeof(_email_attachment_t));
  attachment->imgid = imgid;

  /* construct a temporary file name */
  char tmpdir[4096]= {0};
  dt_util_get_user_local_dir (tmpdir,4096);
  g_strlcat (tmpdir,"/tmp",4096);
  g_mkdir_with_parents(tmpdir,0700);

  char dirname[4096];
  dt_image_full_path(imgid, dirname, 1024);
  const gchar * filename = g_path_get_basename( dirname );
  gchar * end = g_strrstr( filename,".")+1;
  g_strlcpy( end, format->extension(fdata), sizeof(dirname)-(end-dirname));

  attachment->file = g_build_filename( tmpdir, filename, (char *)NULL );

  dt_imageio_export(imgid, attachment->file, format, fdata);

  char *trunc = attachment->file + strlen(attachment->file) - 32;
  if(trunc < attachment->file) trunc = attachment->file;
  dt_control_log(_("%d/%d exported to `%s%s'"), num, total, trunc != filename ? ".." : "", trunc);

#ifdef _OPENMP // store can be called in parallel, so synch access to shared memory
  #pragma omp critical
#endif
  d->images = g_list_append( d->images, attachment );

  return 0;
}

void*
get_params(dt_imageio_module_storage_t *self, int *size)
{
  *size = sizeof(dt_imageio_email_t) - sizeof(GList *);
  dt_imageio_email_t *d = (dt_imageio_email_t *)g_malloc(sizeof(dt_imageio_email_t));
  memset( d,0,sizeof( dt_imageio_email_t));
  return d;
}

int
set_params(dt_imageio_module_format_t *self, void *params, int size)
{
  if(size != sizeof(dt_imageio_email_t) - sizeof(GList *)) return 1;
  return 0;
}

void
free_params(dt_imageio_module_storage_t *self, void *params)
{
  free(params);
}

void
finalize_store(dt_imageio_module_storage_t *self, void *params)
{
  dt_imageio_email_t *d = (dt_imageio_email_t *)params;

  // All images are exported, generate a mailto uri and startup default mail client
  gchar uri[4096]= {0};
  gchar body[4096]= {0};
  gchar attachments[4096]= {0};
  gchar *uriFormat=NULL;
  gchar *subject="images exported from darktable";
  gchar *imageBodyFormat="%s %s"; // filename, exif oneliner
  gchar *attachmentFormat=NULL;
  gchar *attachmentSeparator="";

  // If no default handler detected above, we use gtk_show_uri with mailto:// and hopes things goes right..
  uriFormat="xdg-email --subject \"%s\" --body \"%s\" %s &";   // subject, body, and list of attachments with format:
  attachmentFormat=" --attach \"%s\"";

  while( d->images )
  {
    gchar exif[256]= {0};
    _email_attachment_t *attachment=( _email_attachment_t *)d->images->data;
    const gchar *filename = g_path_get_basename( attachment->file );
    const dt_image_t *img = dt_image_cache_read_get(darktable.image_cache, attachment->imgid);
    dt_image_print_exif( img, exif, 256 );
    g_snprintf(body+strlen(body),4096-strlen(body), imageBodyFormat, filename, exif );

    if( strlen( attachments ) )
      g_snprintf(attachments+strlen(attachments),4096-strlen(attachments), "%s", attachmentSeparator );

    g_snprintf(attachments+strlen(attachments),4096-strlen(attachments), attachmentFormat, attachment->file );
    // Free attachment item and remove
    dt_image_cache_read_release(darktable.image_cache, img);
    g_free( d->images->data );
    d->images = g_list_remove( d->images, d->images->data );
  }

  // build uri and launch before we quit...
  g_snprintf( uri, 4096,  uriFormat, subject, body, attachments );

  fprintf(stderr, "[email] launching `%s'\n", uri);
  if(system( uri ) < 0)
  {
    // TODO: after string freeze is broken again, report to ui:
    // dt_control_log(_("could not launch email client!"));
    fprintf(stderr, "[email] could not launch subprocess!\n");
  }
}

int supported(struct dt_imageio_module_storage_t *storage, struct dt_imageio_module_format_t *format)
{
  const char *mime = format->mime(NULL);
  if(mime[0] == '\0') // this seems to be the copy format
    return 0;

  return 1;
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
