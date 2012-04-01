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

#include "common/darktable.h"
#include "develop/develop.h"
#include "control/control.h"
#include "common/imageio.h"
#include "common/image_cache.h"
#include "common/styles.h"
#include "common/tags.h"
#include "common/debug.h"
#include "common/exif.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gui/accelerators.h"

#include <string.h>
#include <stdio.h>
#include <glib.h>

typedef struct
{
  GString 	*name;
  GString 	*description;
} StyleInfoData;

typedef struct
{
  int   num;
  int   module;
  GString   *operation;
  GString   *op_params;
  GString   *blendop_params;
  int   enabled;
} StylePluginData;

typedef struct
{
  StyleInfoData   *info;
  GList           *plugins;
  gboolean        in_plugin;
} StyleData;

static gboolean _apply_style_shortcut_callback(GtkAccelGroup *accel_group,
                                   GObject *acceleratable,
                                   guint keyval, GdkModifierType modifier,
                                   gpointer data)
{
    dt_styles_apply_to_selection (data,0);
    return TRUE;
}

static void _destroy_style_shortcut_callback(gpointer data,GClosure *closure)
{
	g_free(data);
}

static int32_t dt_styles_get_id_by_name (const char *name);

gboolean
dt_styles_exists (const char *name)
{
  return (dt_styles_get_id_by_name(name))!=0?TRUE:FALSE;
}

static gboolean
dt_styles_create_style_header(const char *name, const char *description)
{
  sqlite3_stmt *stmt;
  if (dt_styles_get_id_by_name(name)!=0)
  {
    dt_control_log(_("style with name '%s' already exists"),name);
    return FALSE;
  }
  /* first create the style header */
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "insert into styles (name,description) values (?1,?2)", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, name,strlen (name),SQLITE_STATIC);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 2, description,strlen (description),SQLITE_STATIC);
  sqlite3_step (stmt);
  sqlite3_finalize (stmt);
  return TRUE;
}

void
dt_styles_create_from_image (const char *name,const char *description,int32_t imgid,GList *filter)
{
  int id=0;
  sqlite3_stmt *stmt;

  /* first create the style header */
  if (!dt_styles_create_style_header(name,description) ) return;

  if ((id=dt_styles_get_id_by_name(name)) != 0)
  {
    /* create the style_items from source image history stack */
    if (filter)
    {
      GList *list=filter;
      char tmp[64];
      char include[2048]= {0};
      g_strlcat(include,"num in (", 2048);
      do
      {
        if(list!=g_list_first(list))
          g_strlcat(include,",", 2048);
        sprintf(tmp,"%ld",(long int)list->data);
        g_strlcat(include,tmp, 2048);
      }
      while ((list=g_list_next(list)));
      g_strlcat(include,")", 2048);
      char query[4096]= {0};
      sprintf(query,"insert into style_items (styleid,num,module,operation,op_params,enabled,blendop_params) select ?1, num,module,operation,op_params,enabled,blendop_params from history where imgid=?2 and %s",include);
      DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), query, -1, &stmt, NULL);
    }
    else
      DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "insert into style_items (styleid,num,module,operation,op_params,enabled,blendop_params) select ?1, num,module,operation,op_params,enabled,blendop_params from history where imgid=?2", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, id);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, imgid);
    sqlite3_step (stmt);
    sqlite3_finalize (stmt);

    /* backup style to disk */
    char stylesdir[1024];
    dt_util_get_user_config_dir(stylesdir, 1024);
    g_strlcat(stylesdir,"/styles",1024);
    g_mkdir_with_parents(stylesdir,00755);

    dt_styles_save_to_file(name,stylesdir);

      char tmp_accel[1024];
      gchar* tmp_name = g_strdup(name); // freed by _destro_style_shortcut_callback
      snprintf(tmp_accel,1024,"styles/Apply %s",name);
      dt_accel_register_global( tmp_accel, 0, 0);
      GClosure *closure;
      closure = g_cclosure_new(
          G_CALLBACK(_apply_style_shortcut_callback),
          tmp_name, _destroy_style_shortcut_callback);
      dt_accel_connect_global(tmp_accel, closure);
    dt_control_log(_("style named '%s' successfully created"),name);
  }
}

void
dt_styles_apply_to_selection(const char *name,gboolean duplicate)
{
  /* for each selected image apply style */
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select * from selected_images", -1, &stmt, NULL);
  while(sqlite3_step(stmt) == SQLITE_ROW)
  {
    int imgid = sqlite3_column_int (stmt, 0);
    dt_styles_apply_to_image (name,duplicate,imgid);
  }
  sqlite3_finalize(stmt);
}

void
dt_styles_apply_to_image(const char *name,gboolean duplicate, int32_t imgid)
{
  int id=0;
  sqlite3_stmt *stmt;

  if ((id=dt_styles_get_id_by_name(name)) != 0)
  {
    /* check if we should make a duplicate before applying style */
    if (duplicate)
      imgid = dt_image_duplicate (imgid);

    /* if merge onto history stack, lets find history offest in destination image */
    int32_t offs = 0;
#if 1
    {
      /* apply on top of history stack */
      DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select count(num) from history where imgid = ?1", -1, &stmt, NULL);
      DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, imgid);
      if (sqlite3_step (stmt) == SQLITE_ROW) offs = sqlite3_column_int (stmt, 0);
    }
#else
    {
      /* replace history stack */
      DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "delete from history where imgid = ?1", -1, &stmt, NULL);
      DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, imgid);
      sqlite3_step (stmt);
    }
#endif
    sqlite3_finalize (stmt);

    /* copy history items from styles onto image */
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "insert into history (imgid,num,module,operation,op_params,enabled,blendop_params) select ?1, num+?2,module,operation,op_params,enabled,blendop_params from style_items where styleid=?3", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, imgid);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, offs);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 3, id);
    sqlite3_step (stmt);
    sqlite3_finalize (stmt);

    /* add tag */
    guint tagid=0;
    gchar ntag[512]={0};
    g_snprintf(ntag,512,"darktable|style|%s",name);
    if (dt_tag_new(ntag,&tagid))
      dt_tag_attach(tagid,imgid);

    /* if current image in develop reload history */
    if (dt_dev_is_current_image(darktable.develop, imgid))
      dt_dev_reload_history_items (darktable.develop);

    /* update xmp file */
    dt_image_synch_xmp(imgid);

    /* remove old obsolete thumbnails */
    dt_mipmap_cache_remove(darktable.mipmap_cache, imgid);
  }
}

void
dt_styles_delete_by_name(const char *name)
{
  int id=0;
  if ((id=dt_styles_get_id_by_name(name)) != 0)
  {
    /* delete the style */
    sqlite3_stmt *stmt;
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "delete from styles where rowid = ?1", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, id);
    sqlite3_step (stmt);
    sqlite3_finalize (stmt);

    /* delete style_items belonging to style */
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "delete from style_items where styleid = ?1", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, id);
    sqlite3_step (stmt);
    sqlite3_finalize (stmt);

    char tmp_accel[1024];
    snprintf(tmp_accel,1024,"styles/Apply %s",name);
    dt_accel_deregister_global(tmp_accel);
  }
}

GList *
dt_styles_get_item_list (const char *name)
{
  GList *result=NULL;
  sqlite3_stmt *stmt;
  int id=0;
  if ((id=dt_styles_get_id_by_name(name)) != 0)
  {
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select num, operation, enabled from style_items where styleid=?1 order by num desc", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, id);
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
      char name[512]= {0};
      dt_style_item_t *item=g_malloc (sizeof (dt_style_item_t));
      item->num = sqlite3_column_int (stmt, 0);
      g_snprintf(name,512,"%s (%s)",sqlite3_column_text (stmt, 1),(sqlite3_column_int (stmt, 2)!=0)?_("on"):_("off"));
      item->name = g_strdup (name);
      result = g_list_append (result,item);
    }
    sqlite3_finalize(stmt);
  }
  return result;
}

GList *
dt_styles_get_list (const char *filter)
{
  char filterstring[512]= {0};
  sqlite3_stmt *stmt;
  sprintf (filterstring,"%%%s%%",filter);
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select name, description from styles where name like ?1 or description like ?1 order by name", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, filterstring, strlen(filterstring), SQLITE_TRANSIENT);
  GList *result = NULL;
  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    const char *name = (const char *)sqlite3_column_text (stmt, 0);
    const char *description = (const char *)sqlite3_column_text (stmt, 1);
    dt_style_t *s=g_malloc (sizeof (dt_style_t));
    s->name = g_strdup (name);
    s->description= g_strdup (description);
    result = g_list_append(result,s);
  }
  sqlite3_finalize (stmt);
  return result;
}

static char *
dt_style_encode(sqlite3_stmt *stmt, int row)
{
  const int32_t len = sqlite3_column_bytes(stmt, row);
  char *vparams = (char *)malloc(2*len + 1);
  dt_exif_xmp_encode ((const unsigned char *)sqlite3_column_blob(stmt, row), vparams, len);
  return vparams;
}

void
dt_styles_save_to_file(const char *style_name,const char *filedir)
{
  int rc = 0;
  char stylename[520];
  sqlite3_stmt *stmt;

  snprintf(stylename,512,"%s/%s.dtstyle",filedir,style_name);

  // check if file exists
  if( g_file_test(stylename, G_FILE_TEST_EXISTS) == TRUE )
  {
    dt_control_log(_("style file for %s exists"),style_name);
    return;
  }

  if ( !dt_styles_exists (style_name) ) return;

  xmlTextWriterPtr writer = xmlNewTextWriterFilename(stylename, 0);
  if (writer == NULL)
  {
    fprintf(stderr,"[dt_styles_save_to_file] Error creating the xml writer\n, path: %s", stylename);
    return;
  }
  rc = xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);
  if (rc < 0)
  {
    fprintf(stderr,"[dt_styles_save_to_file]: Error on encoding setting");
    return;
  }
  xmlTextWriterStartElement(writer, BAD_CAST "darktable_style");
  xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST "1.0");

  xmlTextWriterStartElement(writer, BAD_CAST "info");
  xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", style_name);
  xmlTextWriterWriteFormatElement(writer, BAD_CAST "description", "%s", dt_styles_get_description(style_name));
  xmlTextWriterEndElement(writer);

  xmlTextWriterStartElement(writer, BAD_CAST "style");
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select num,module,operation,op_params,enabled,blendop_params from style_items where styleid =?1",-1, &stmt,NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt,1,dt_styles_get_id_by_name(style_name));
  while (sqlite3_step (stmt) == SQLITE_ROW)
  {
    xmlTextWriterStartElement(writer, BAD_CAST "plugin");
    xmlTextWriterWriteFormatElement(writer, BAD_CAST "num", "%d", sqlite3_column_int(stmt,0));
    xmlTextWriterWriteFormatElement(writer, BAD_CAST "module", "%d", sqlite3_column_int(stmt,1));
    xmlTextWriterWriteFormatElement(writer, BAD_CAST "operation", "%s", sqlite3_column_text(stmt,2));
    xmlTextWriterWriteFormatElement(writer, BAD_CAST "op_params", "%s", dt_style_encode(stmt,3));
    xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", sqlite3_column_int(stmt,4));
    xmlTextWriterWriteFormatElement(writer, BAD_CAST "blendop_params", "%s", dt_style_encode(stmt,5));
    xmlTextWriterEndElement(writer);
  }
  sqlite3_finalize(stmt);
  xmlTextWriterEndDocument(writer);
  xmlFreeTextWriter(writer);
  dt_control_log(_("style %s was sucessfully saved"),style_name);
}

static StyleData *
dt_styles_style_data_new ()
{
  StyleInfoData *info = g_new0(StyleInfoData,1);
  info->name = g_string_new("");
  info->description = g_string_new("");

  StyleData *data = g_new0(StyleData,1);
  data->info = info;
  data->in_plugin = FALSE;
  data->plugins = NULL;

  return data;
}

static StylePluginData *
dt_styles_style_plugin_new ()
{
  StylePluginData *plugin = g_new0(StylePluginData,1);
  plugin->operation = g_string_new("");
  plugin->op_params = g_string_new("");
  plugin->blendop_params = g_string_new("");
  return plugin;
}

static void
dt_styles_style_data_free (StyleData *style, gboolean free_segments)
{
  g_string_free (style->info->name, free_segments);
  g_string_free (style->info->description, free_segments);
  g_list_free(style->plugins);
  g_free(style);
}

static void
dt_styles_start_tag_handler (GMarkupParseContext *context,
                             const gchar         *element_name,
                             const gchar        **attribute_names,
                             const gchar        **attribute_values,
                             gpointer             user_data,
                             GError             **error)
{
  StyleData *style = user_data;
  const gchar *elt = g_markup_parse_context_get_element (context);

  // We need to append the contents of any subtags to the content field
  // for this we need to know when we are inside the note-content tag
  if (g_ascii_strcasecmp (elt, "plugin") == 0)
  {
    style->in_plugin = TRUE;
    style->plugins = g_list_prepend(style->plugins,dt_styles_style_plugin_new());
  }
}

static void
dt_styles_end_tag_handler (GMarkupParseContext *context,
                           const gchar         *element_name,
                           gpointer             user_data,
                           GError             **error)
{
  StyleData *style = user_data;
  const gchar *elt = g_markup_parse_context_get_element (context);

  // We need to append the contents of any subtags to the content field
  // for this we need to know when we are inside the note-content tag
  if (g_ascii_strcasecmp (elt, "plugin") == 0)
  {
    style->in_plugin = FALSE;
  }
}

static void
dt_styles_style_text_handler (GMarkupParseContext *context,
                              const gchar *text,
                              gsize text_len,
                              gpointer user_data,
                              GError **error)
{

  StyleData *style = user_data;
  const gchar *elt = g_markup_parse_context_get_element (context);

  if ( g_ascii_strcasecmp (elt, "name") == 0 )
  {
    g_string_append_len (style->info->name, text, text_len);
  }
  else if (g_ascii_strcasecmp (elt, "description") == 0 )
  {
    g_string_append_len (style->info->description, text, text_len);
  }
  else if ( style->in_plugin)
  {
    StylePluginData *plug = g_list_first(style->plugins)->data;
    if ( g_ascii_strcasecmp (elt, "operation") == 0 )
    {
      g_string_append_len (plug->operation, text, text_len);
    }
    else if (g_ascii_strcasecmp (elt, "op_params") == 0 )
    {
      g_string_append_len (plug->op_params, text, text_len);
    }
    else if (g_ascii_strcasecmp (elt, "blendop_params") == 0 )
    {
      g_string_append_len (plug->blendop_params, text, text_len);
    }
    else if (g_ascii_strcasecmp (elt, "num") == 0 )
    {
      plug->num = atoi(text);
    }
    else if (g_ascii_strcasecmp (elt, "module") == 0 )
    {
      plug->module =  atoi(text);
    }
    else if (g_ascii_strcasecmp (elt, "enabled") == 0 )
    {
      plug->enabled = atoi(text);
    }
  }

}

static GMarkupParser dt_style_parser =
{
  dt_styles_start_tag_handler,	// Start element handler
  dt_styles_end_tag_handler,  	// End element handler
  dt_styles_style_text_handler,	// Text element handler
  NULL,					 		// Passthrough handler
  NULL					 		// Error handler
};

static void
dt_style_plugin_save(StylePluginData *plugin,gpointer styleId)
{
  int id = GPOINTER_TO_INT(styleId);
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "insert into style_items (styleid,num,module,operation,op_params,enabled,blendop_params) values(?1,?2,?3,?4,?5,?6,?7)", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, id);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, plugin->num);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 3, plugin->module);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 4, plugin->operation->str, plugin->operation->len, SQLITE_TRANSIENT);
  //
  const char *param_c = plugin->op_params->str;
  const int param_c_len = strlen(param_c);
  const int params_len = param_c_len/2;
  unsigned char *params = (unsigned char *)g_malloc(params_len);
  dt_exif_xmp_decode(param_c, params, param_c_len);
  DT_DEBUG_SQLITE3_BIND_BLOB(stmt, 5, params, params_len, SQLITE_TRANSIENT);
  //
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 6, plugin->enabled);
  
  /* decode and store blendop params */
  unsigned char *blendop_params = (unsigned char *)g_malloc(strlen(plugin->blendop_params->str));
  dt_exif_xmp_decode(plugin->blendop_params->str, blendop_params, strlen(plugin->blendop_params->str));
  DT_DEBUG_SQLITE3_BIND_BLOB(stmt, 7, blendop_params, strlen(plugin->blendop_params->str)/2, SQLITE_TRANSIENT);
 
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  g_free(params);
}

static void
dt_style_save(StyleData *style)
{
  int id = 0;
  if ( style == NULL) return;

  /* first create the style header */
  if (!dt_styles_create_style_header(style->info->name->str,style->info->description->str) ) return;

  if ((id=dt_styles_get_id_by_name(style->info->name->str)) != 0)
  {
    g_list_foreach(style->plugins,(GFunc)dt_style_plugin_save,GINT_TO_POINTER(id));
    dt_control_log(_("style %s was sucessfully imported"),style->info->name->str);
  }
}

void
dt_styles_import_from_file(const char *style_path)
{

  FILE    			*style_file;
  StyleData           *style;
  GMarkupParseContext	*parser;
  gchar				buf[1024];
  int					num_read;

  style = dt_styles_style_data_new();
  parser = g_markup_parse_context_new (&dt_style_parser, 0, style, NULL);

  if ((style_file = fopen (style_path, "r")))
  {

    while (!feof (style_file))
    {
      num_read = fread (buf, sizeof(gchar), 1024, style_file);

      if (num_read == 0)
      {
        break;
      }
      else if (num_read < 0)
      {
        // ERROR !
        break;
      }

      if (!g_markup_parse_context_parse(parser, buf, num_read, NULL))
      {
        g_markup_parse_context_free(parser);
        dt_styles_style_data_free(style,TRUE);
        fclose (style_file);
        return;
      }
    }
  }
  else
  {
    // Failed to open file, clean up.
    g_markup_parse_context_free(parser);
    dt_styles_style_data_free(style,TRUE);
    return;
  }

  if (!g_markup_parse_context_end_parse (parser, NULL))
  {
    g_markup_parse_context_free(parser);
    dt_styles_style_data_free(style,TRUE);
    fclose (style_file);
    return;
  }
  g_markup_parse_context_free (parser);
  // save data
  dt_style_save(style);
  //
  dt_styles_style_data_free(style,TRUE);
  fclose (style_file);
}

gchar *
dt_styles_get_description (const char *name)
{
  sqlite3_stmt *stmt;
  int id=0;
  gchar *description = NULL;
  if ((id=dt_styles_get_id_by_name(name)) != 0)
  {
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select description from styles where rowid=?1", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, id);
    sqlite3_step(stmt);
    description = (char *)sqlite3_column_text (stmt, 0);
    if (description)
      description = g_strdup (description);
    sqlite3_finalize (stmt);
  }
  return description;
}

static int32_t
dt_styles_get_id_by_name (const char *name)
{
  int id=0;
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select rowid from styles where name=?1 order by rowid desc limit 1", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, name,strlen (name),SQLITE_TRANSIENT);
  if(sqlite3_step(stmt) == SQLITE_ROW)
  {
    id = sqlite3_column_int (stmt, 0);
  }
  sqlite3_finalize (stmt);
  return id;
}

void init_styles_key_accels()
{
  GList *result = dt_styles_get_list("");
  if (result)
  {
    do
    {
      dt_style_t *style = (dt_style_t *)result->data;
      char tmp_accel[1024];
      snprintf(tmp_accel,1024,"styles/Apply %s",style->name);
      dt_accel_register_global( tmp_accel, 0, 0);

      g_free(style->name);
      g_free(style->description);
      g_free(style);
    }
    while ((result=g_list_next(result))!=NULL);
  }
}

void connect_styles_key_accels()
{
  GList *result = dt_styles_get_list("");
  if (result)
  {
    do
    {
      GClosure *closure;
      dt_style_t *style = (dt_style_t *)result->data;
      closure = g_cclosure_new(
          G_CALLBACK(_apply_style_shortcut_callback),
          style->name, _destroy_style_shortcut_callback);
      char tmp_accel[1024];
      snprintf(tmp_accel,1024,"styles/Apply %s",style->name);
      dt_accel_connect_global(tmp_accel, closure);

      //g_free(style->name); freed at closure destruction
      g_free(style->description);
      g_free(style);
    }
    while ((result=g_list_next(result))!=NULL);
  }
}
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
