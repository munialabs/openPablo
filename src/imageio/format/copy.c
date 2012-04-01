/*
    This file is part of darktable,
    copyright (c) 2010 Tobias Ellinghaus.
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
#include <glib/gstdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "common/darktable.h"
#include "common/imageio_module.h"
#include "common/exif.h"
#include "common/debug.h"

DT_MODULE(1)

// FIXME: we can't rely on darktable to avoid file overwriting -- it doesn't know the filename (extension).
int write_image (dt_imageio_module_data_t *ppm, const char *filename, const uint16_t *in, void *exif, int exif_len, int imgid)
{
  int status = 1;
  char *sourcefile = NULL;
  char *targetfile = NULL;
  char *xmpfile = NULL;
  char *content = NULL;
  FILE *fin = NULL;
  FILE *fout = NULL;
  sqlite3_stmt *stmt;

  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select folder, filename from images, film_rolls where images.id = ?1 and film_id = film_rolls.id;", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, imgid);

  if(sqlite3_step(stmt) != SQLITE_ROW)
    goto END;

  char *sfolder    = (char*)sqlite3_column_text(stmt, 0);
  char *sfilename  = (char*)sqlite3_column_text(stmt, 1);
  sourcefile = g_build_filename(sfolder, sfilename, NULL);
  char *extension  = g_strrstr(sourcefile, ".");
  if(extension == NULL)
    goto END;
  targetfile = g_strconcat(filename, ++extension, NULL);

  if(!strcmp(sourcefile, targetfile))
    goto END;

  fin = fopen(sourcefile, "rb");
  fout = fopen(targetfile, "wb");
  if(fin == NULL || fout == NULL)
    goto END;

  fseek(fin,0,SEEK_END);
  int end = ftell(fin);
  rewind(fin);

  content = (char*)g_malloc(sizeof(char)*end);
  if(content == NULL)
    goto END;
  if(fread(content,sizeof(char),end,fin) != end)
    goto END;
  if(fwrite(content,sizeof(char),end,fout) != end)
    goto END;

  // we got a copy of the file, now write the xmp data
  xmpfile = g_strconcat(targetfile, ".xmp", NULL);
  if(dt_exif_xmp_write (imgid, xmpfile) != 0)
  {
    // something went wrong, unlink the copied image.
    g_unlink(targetfile);
    goto END;
  }

  status = 0;
END:
  if(sourcefile)
    g_free(sourcefile);
  if(targetfile)
    g_free(targetfile);
  if(xmpfile)
    g_free(xmpfile);
  if(content)
    g_free(content);
  if(fin)
    fclose(fin);
  if(fout)
    fclose(fout);
  return status;
}

void*
get_params(dt_imageio_module_format_t *self, int *size)
{
  *size = sizeof(dt_imageio_module_data_t);
  dt_imageio_module_data_t *d = (dt_imageio_module_data_t *)malloc(sizeof(dt_imageio_module_data_t));
  return d;
}

void
free_params(dt_imageio_module_format_t *self, void *params)
{
  free(params);
}

int
set_params(dt_imageio_module_format_t *self, void *params, int size)
{
  if(size != sizeof(dt_imageio_module_data_t)) return 1;
  return 0;
}

int bpp(dt_imageio_module_data_t *p)
{
  return 0;
}

const char*
mime(dt_imageio_module_data_t *data)
{
  return "";
}

const char*
extension(dt_imageio_module_data_t *data)
{
  return "";
}

const char*
name ()
{
  return _("copy");
}

void init(dt_imageio_module_format_t *self) {}
void cleanup(dt_imageio_module_format_t *self) {}

void gui_init    (dt_imageio_module_format_t *self)
{
  GtkWidget *box = gtk_hbox_new(FALSE, 20);
  self->widget = box;

  GtkWidget *label = gtk_label_new(_("do a 1:1 copy of the selected files.\nthe global options below do not apply!"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

}
void gui_cleanup (dt_imageio_module_format_t *self) {}
void gui_reset   (dt_imageio_module_format_t *self) {}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
