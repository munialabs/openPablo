/*
    This file is part of darktable,
    copyright (c) 2009--2010 johannes hanika.

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
#include "common/imageio_module.h"
#include "control/conf.h"
#include "control/control.h"
#include <stdlib.h>

static gint
dt_imageio_sort_modules_storage (gconstpointer a, gconstpointer b)
{
  const dt_imageio_module_storage_t *am = (const dt_imageio_module_storage_t *)a;
  const dt_imageio_module_storage_t *bm = (const dt_imageio_module_storage_t *)b;
  return strcmp(am->name(), bm->name());
}

static gint
dt_imageio_sort_modules_format (gconstpointer a, gconstpointer b)
{
  const dt_imageio_module_format_t *am = (const dt_imageio_module_format_t *)a;
  const dt_imageio_module_format_t *bm = (const dt_imageio_module_format_t *)b;
  return strcmp(am->name(), bm->name());
}

/** Default implementation of dimension module function, used if format modules does not implements dimension() */
static int
_default_format_dimension(dt_imageio_module_format_t *module, uint32_t *width, uint32_t *height)
{
  return 0;
}

static int
dt_imageio_load_module_format (dt_imageio_module_format_t *module, const char *libname, const char *plugin_name)
{
  module->widget = NULL;
  g_strlcpy(module->plugin_name, plugin_name, 20);
  module->module = g_module_open(libname, G_MODULE_BIND_LAZY);
  if(!module->module) goto error;
  int (*version)();
  if(!g_module_symbol(module->module, "dt_module_dt_version", (gpointer)&(version))) goto error;
  if(version() != dt_version())
  {
    fprintf(stderr, "[imageio_load_module] `%s' is compiled for another version of dt (module %d (%s) != dt %d (%s)) !\n", libname, abs(version()), version() < 0 ? "debug" : "opt", abs(dt_version()), dt_version() < 0 ? "debug" : "opt");
    goto error;
  }
  if(!g_module_symbol(module->module, "name",                         (gpointer)&(module->name)))                         goto error;
  if(!g_module_symbol(module->module, "init",                         (gpointer)&(module->init)))                         goto error;
  if(!g_module_symbol(module->module, "cleanup",                         (gpointer)&(module->cleanup)))                         goto error;
  if(!g_module_symbol(module->module, "gui_reset",                    (gpointer)&(module->gui_reset)))                    goto error;
  if(!g_module_symbol(module->module, "gui_init",                     (gpointer)&(module->gui_init)))                     goto error;
  if(!g_module_symbol(module->module, "gui_cleanup",                  (gpointer)&(module->gui_cleanup)))                  goto error;

  if(!g_module_symbol(module->module, "mime",                         (gpointer)&(module->mime)))                         goto error;
  if(!g_module_symbol(module->module, "extension",                    (gpointer)&(module->extension)))                    goto error;
  if(!g_module_symbol(module->module, "dimension",                   (gpointer)&(module->dimension)))                   module->dimension = _default_format_dimension;
  if(!g_module_symbol(module->module, "get_params",                   (gpointer)&(module->get_params)))                   goto error;
  if(!g_module_symbol(module->module, "free_params",                  (gpointer)&(module->free_params)))                  goto error;
  if(!g_module_symbol(module->module, "set_params",                   (gpointer)&(module->set_params)))                   goto error;
  if(!g_module_symbol(module->module, "write_image",                  (gpointer)&(module->write_image)))                  goto error;
  if(!g_module_symbol(module->module, "bpp",                          (gpointer)&(module->bpp)))                          goto error;

  if(!g_module_symbol(module->module, "decompress_header",            (gpointer)&(module->decompress_header)))            module->decompress_header = NULL;
  if(!g_module_symbol(module->module, "decompress",                   (gpointer)&(module->decompress)))                   module->decompress = NULL;
  if(!g_module_symbol(module->module, "compress",                     (gpointer)&(module->compress)))                     module->compress = NULL;

  if(!g_module_symbol(module->module, "read_header",                  (gpointer)&(module->read_header)))                  module->read_header = NULL;
  if(!g_module_symbol(module->module, "read_image",                   (gpointer)&(module->read_image)))                   module->read_image = NULL;

  module->init(module);

  return 0;
error:
  fprintf(stderr, "[imageio_load_module] failed to open format `%s': %s\n", plugin_name, g_module_error());
  if(module->module) g_module_close(module->module);
  return 1;
}


static int
dt_imageio_load_modules_format(dt_imageio_t *iio)
{
  iio->plugins_format = NULL;
  GList *res = NULL;
  dt_imageio_module_format_t *module;
  char plugindir[1024], plugin_name[256];
  const gchar *d_name;
  dt_util_get_plugindir(plugindir, 1024);
  g_strlcat(plugindir, "/plugins/imageio/format", 1024);
  GDir *dir = g_dir_open(plugindir, 0, NULL);
  if(!dir) return 1;
  while((d_name = g_dir_read_name(dir)))
  {
    // get lib*.so
    if(strncmp(d_name, "lib", 3)) continue;
    if(strncmp(d_name + strlen(d_name) - 3, ".so", 3)) continue;
    strncpy(plugin_name, d_name+3, strlen(d_name)-6);
    plugin_name[strlen(d_name)-6] = '\0';
    module = (dt_imageio_module_format_t *)malloc(sizeof(dt_imageio_module_format_t));
    gchar *libname = g_module_build_path(plugindir, (const gchar *)plugin_name);
    if(dt_imageio_load_module_format(module, libname, plugin_name))
    {
      free(module);
      continue;
    }
    module->gui_data = NULL;
    module->gui_init(module);
    if(module->widget) gtk_widget_ref(module->widget);
    g_free(libname);
    res = g_list_insert_sorted(res, module, dt_imageio_sort_modules_format);
  }
  g_dir_close(dir);
  iio->plugins_format = res;
  return 0;
}

/** Default implementation of supported function, used if storage modules not implements supported() */
static int
_default_supported(struct dt_imageio_module_storage_t *self, struct dt_imageio_module_format_t *format)
{
  return 1;
}
/** Default implementation of dimension module function, used if storage modules does not implements dimension() */
static int
_default_storage_dimension(struct dt_imageio_module_storage_t *self,uint32_t *width, uint32_t *height)
{
  return 0;
}

static int
dt_imageio_load_module_storage (dt_imageio_module_storage_t *module, const char *libname, const char *plugin_name)
{
  module->widget = NULL;
  g_strlcpy(module->plugin_name, plugin_name, 20);
  module->module = g_module_open(libname, G_MODULE_BIND_LAZY);
  if(!module->module) goto error;
  int (*version)();
  if(!g_module_symbol(module->module, "dt_module_dt_version", (gpointer)&(version))) goto error;
  if(version() != dt_version())
  {
    fprintf(stderr, "[imageio_load_module] `%s' is compiled for another version of dt (module %d (%s) != dt %d (%s)) !\n", libname, abs(version()), version() < 0 ? "debug" : "opt", abs(dt_version()), dt_version() < 0 ? "debug" : "opt");
    goto error;
  }
  if(!g_module_symbol(module->module, "name",                   (gpointer)&(module->name)))                   goto error;
  if(!g_module_symbol(module->module, "gui_reset",              (gpointer)&(module->gui_reset)))              goto error;
  if(!g_module_symbol(module->module, "gui_init",               (gpointer)&(module->gui_init)))               goto error;
  if(!g_module_symbol(module->module, "gui_cleanup",            (gpointer)&(module->gui_cleanup)))            goto error;

  if(!g_module_symbol(module->module, "store",                  (gpointer)&(module->store)))                  goto error;
  if(!g_module_symbol(module->module, "get_params",             (gpointer)&(module->get_params)))             goto error;
  if(!g_module_symbol(module->module, "free_params",            (gpointer)&(module->free_params)))            goto error;
  if(!g_module_symbol(module->module, "finalize_store",         (gpointer)&(module->finalize_store)))         module->finalize_store = NULL;
  if(!g_module_symbol(module->module, "set_params",             (gpointer)&(module->set_params)))             goto error;

  if(!g_module_symbol(module->module, "supported",              (gpointer)&(module->supported)))              module->supported = _default_supported;
  if(!g_module_symbol(module->module, "dimension",              (gpointer)&(module->dimension)))            	module->dimension = _default_storage_dimension;
  if(!g_module_symbol(module->module, "recommended_dimension",  (gpointer)&(module->recommended_dimension)))  module->recommended_dimension = _default_storage_dimension;

  return 0;
error:
  fprintf(stderr, "[imageio_load_module] failed to open storage `%s': %s\n", plugin_name, g_module_error());
  if(module->module) g_module_close(module->module);
  return 1;
}

static int
dt_imageio_load_modules_storage (dt_imageio_t *iio)
{
  iio->plugins_storage = NULL;
  GList *res = NULL;
  dt_imageio_module_storage_t *module;
  char plugindir[1024], plugin_name[256];
  const gchar *d_name;
  dt_util_get_plugindir(plugindir, 1024);
  g_strlcat(plugindir, "/plugins/imageio/storage", 1024);
  GDir *dir = g_dir_open(plugindir, 0, NULL);
  if(!dir) return 1;
  while((d_name = g_dir_read_name(dir)))
  {
    // get lib*.so
    if(strncmp(d_name, "lib", 3)) continue;
    if(strncmp(d_name + strlen(d_name) - 3, ".so", 3)) continue;
    strncpy(plugin_name, d_name+3, strlen(d_name)-6);
    plugin_name[strlen(d_name)-6] = '\0';
    module = (dt_imageio_module_storage_t *)malloc(sizeof(dt_imageio_module_storage_t));
    gchar *libname = g_module_build_path(plugindir, (const gchar *)plugin_name);
    if(dt_imageio_load_module_storage(module, libname, plugin_name))
    {
      free(module);
      continue;
    }
    module->gui_data = NULL;
    module->gui_init(module);
    if(module->widget) gtk_widget_ref(module->widget);
    g_free(libname);
    res = g_list_insert_sorted(res, module, dt_imageio_sort_modules_storage);
  }
  g_dir_close(dir);
  iio->plugins_storage = res;
  return 0;
}

void
dt_imageio_init (dt_imageio_t *iio)
{
  iio->plugins_format  = NULL;
  iio->plugins_storage = NULL;

  dt_imageio_load_modules_format (iio);
  dt_imageio_load_modules_storage(iio);
}

void
dt_imageio_cleanup (dt_imageio_t *iio)
{
  while(iio->plugins_format)
  {
    dt_imageio_module_format_t *module = (dt_imageio_module_format_t *)(iio->plugins_format->data);
    module->cleanup(module);
    if(module->widget) gtk_widget_unref(module->widget);
    if(module->module) g_module_close(module->module);
    free(module);
    iio->plugins_format = g_list_delete_link(iio->plugins_format, iio->plugins_format);
  }
  while(iio->plugins_storage)
  {
    dt_imageio_module_storage_t *module = (dt_imageio_module_storage_t *)(iio->plugins_storage->data);
    if(module->widget) gtk_widget_unref(module->widget);
    if(module->module) g_module_close(module->module);
    free(module);
    iio->plugins_storage = g_list_delete_link(iio->plugins_storage, iio->plugins_storage);
  }
}

dt_imageio_module_format_t *dt_imageio_get_format()
{
  dt_imageio_t *iio = darktable.imageio;
  int k = dt_conf_get_int ("plugins/lighttable/export/format");
  GList *it = g_list_nth(iio->plugins_format, k);
  if(!it) it = iio->plugins_format;
  return (dt_imageio_module_format_t *)it->data;
}

dt_imageio_module_storage_t *dt_imageio_get_storage()
{
  dt_imageio_t *iio = darktable.imageio;
  int k = dt_conf_get_int ("plugins/lighttable/export/storage");
  GList *it = g_list_nth(iio->plugins_storage, k);
  if(!it) it = iio->plugins_storage;
  return (dt_imageio_module_storage_t *)it->data;
}

dt_imageio_module_format_t *dt_imageio_get_format_by_name(const char *name)
{
  dt_imageio_t *iio = darktable.imageio;
  GList *it = iio->plugins_format;
  while(it)
  {
    dt_imageio_module_format_t *module = (dt_imageio_module_format_t *)it->data;
    if(!strcmp(module->plugin_name, name)) return module;
    it = g_list_next(it);
  }
  return NULL;
}

dt_imageio_module_storage_t *dt_imageio_get_storage_by_name(const char *name)
{
  dt_imageio_t *iio = darktable.imageio;
  GList *it = iio->plugins_storage;
  while(it)
  {
    dt_imageio_module_storage_t *module = (dt_imageio_module_storage_t *)it->data;
    if(!strcmp(module->plugin_name, name)) return module;
    it = g_list_next(it);
  }
  return NULL;
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
