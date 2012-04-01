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
#include "common/colorlabels.h"
#include "common/image.h"
#include "common/image_cache.h"
#include "common/metadata.h"
#include "common/variables.h"
#include "common/utility.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct dt_variables_data_t
{
  gchar *source;

  /** expanded result string */
  gchar *result;
  time_t time;
  guint sequence;
}
dt_variables_data_t;

gchar *_string_get_first_variable(gchar *string,gchar *variable)
{
  if (g_strrstr (string,"$("))
  {
    gchar *pend=string+strlen(string);
    gchar *p,*e;
    p=e=string;
    while( p < pend && e < pend)
    {
      while( *p!='$' && *(p+1)!='(' && p<pend) p++;
      if( *p=='$' && *(p+1)=='(' )
      {
        e=p;
        while( *e!=')' && e < pend) e++;
        if(e < pend && *e==')')
        {
          strncpy(variable,p,e-p+1);
          variable[e-p+1]='\0';
          return p+1;
        }
        else
          return NULL;
      }
      p++;
    }
    return p+1;
  }
  return NULL;
}

gchar *_string_get_next_variable(gchar *string,gchar *variable)
{
  gchar *pend=string+strlen(string);
  gchar *p,*e;
  p=e=string;
  while( p < pend && e < pend)
  {
    while( !(*p=='$' && *(p+1)=='(') && p<=pend) p++;
    if( *p=='$' && *(p+1)=='(' )
    {
      e=p;
      while( *e!=')' && e < pend) e++;
      if(e < pend && *e==')')
      {
        strncpy(variable,p,e-p+1);
        variable[e-p+1]='\0';
        return p+1;
      }
      else
        return NULL;
    }

  }
  return NULL;
}

gboolean _variable_get_value(dt_variables_params_t *params, gchar *variable,gchar *value)
{
  const gchar *file_ext=NULL;
  gboolean got_value=FALSE;
  struct tm *tim=localtime(&params->data->time);

  const gchar *homedir = dt_util_get_home_dir(NULL);

  gchar *pictures_folder=NULL;

  if(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES) == NULL)
    pictures_folder=g_build_path(G_DIR_SEPARATOR_S,homedir,"Pictures",(char *)NULL);
  else
    pictures_folder=g_strdup( g_get_user_special_dir(G_USER_DIRECTORY_PICTURES) );

  if(params->filename)
  {
    file_ext=(g_strrstr(params->filename,".")+1);
    if(file_ext == (gchar*)1) file_ext = params->filename + strlen(params->filename);
  }

  /* image exif time */
  gboolean have_exif_tm = FALSE;
  struct tm exif_tm= {0};
  if (params->imgid)
  {
    const dt_image_t *img = dt_image_cache_read_get(darktable.image_cache, params->imgid);
    if (sscanf (img->exif_datetime_taken,"%d:%d:%d %d:%d:%d",
                &exif_tm.tm_year,
                &exif_tm.tm_mon,
                &exif_tm.tm_mday,
                &exif_tm.tm_hour,
                &exif_tm.tm_min,
                &exif_tm.tm_sec
               ) == 6
       )
    {
      exif_tm.tm_year-=1900;
      exif_tm.tm_mon--;
      have_exif_tm = TRUE;
    }
    dt_image_cache_read_release(darktable.image_cache, img);
  }

  if( g_strcmp0(variable,"$(YEAR)") == 0 && (got_value=TRUE) )  sprintf(value,"%.4d",tim->tm_year+1900);
  else if( g_strcmp0(variable,"$(MONTH)") == 0&& (got_value=TRUE)  )   sprintf(value,"%.2d",tim->tm_mon+1);
  else if( g_strcmp0(variable,"$(DAY)") == 0 && (got_value=TRUE) )   sprintf(value,"%.2d",tim->tm_mday);
  else if( g_strcmp0(variable,"$(HOUR)") == 0 && (got_value=TRUE) )  sprintf(value,"%.2d",tim->tm_hour);
  else if( g_strcmp0(variable,"$(MINUTE)") == 0 && (got_value=TRUE) )   sprintf(value,"%.2d",tim->tm_min);
  else if( g_strcmp0(variable,"$(SECOND)") == 0 && (got_value=TRUE) )   sprintf(value,"%.2d",tim->tm_sec);

  else if( g_strcmp0(variable,"$(EXIF_YEAR)") == 0 && (got_value=TRUE)  )   			sprintf(value,"%.4d", (have_exif_tm?exif_tm.tm_year:tim->tm_year)+1900);
  else if( g_strcmp0(variable,"$(EXIF_MONTH)") == 0 && (got_value=TRUE)  )  		sprintf(value,"%.2d", (have_exif_tm?exif_tm.tm_mon:tim->tm_mon)+1);
  else if( g_strcmp0(variable,"$(EXIF_DAY)") == 0 && (got_value=TRUE) )  			sprintf(value,"%.2d", (have_exif_tm?exif_tm.tm_mday:tim->tm_mday));
  else if( g_strcmp0(variable,"$(EXIF_HOUR)") == 0 && (got_value=TRUE) )  			sprintf(value,"%.2d", (have_exif_tm?exif_tm.tm_hour:tim->tm_hour));
  else if( g_strcmp0(variable,"$(EXIF_MINUTE)") == 0 && (got_value=TRUE) )   		sprintf(value,"%.2d", (have_exif_tm?exif_tm.tm_min:tim->tm_min));
  else if( g_strcmp0(variable,"$(EXIF_SECOND)") == 0 && (got_value=TRUE) )   		sprintf(value,"%.2d", (have_exif_tm?exif_tm.tm_sec:tim->tm_sec));
  else if( g_strcmp0(variable,"$(ID)") == 0 && (got_value=TRUE) ) sprintf(value,"%d", params->imgid); 
  else if( g_strcmp0(variable,"$(JOBCODE)") == 0 && (got_value=TRUE) )   sprintf(value,"%s",params->jobcode);
  else if( g_strcmp0(variable,"$(ROLL_NAME)") == 0 && params->filename && (got_value=TRUE) )   sprintf(value,"%s",g_path_get_basename(g_path_get_dirname(params->filename)));
  else if( g_strcmp0(variable,"$(FILE_DIRECTORY)") == 0 && params->filename && (got_value=TRUE) )   sprintf(value,"%s",g_path_get_dirname(params->filename));
  else if( g_strcmp0(variable,"$(FILE_NAME)") == 0 && params->filename && (got_value=TRUE) )
  {
    sprintf(value,"%s",g_path_get_basename(params->filename));
    if (g_strrstr(value,".")) *(g_strrstr(value,"."))=0;
  }
  else if( g_strcmp0(variable,"$(FILE_EXTENSION)") == 0 && params->filename && (got_value=TRUE) )   sprintf(value,"%s",file_ext);
  else if( g_strcmp0(variable,"$(SEQUENCE)") == 0 && (got_value=TRUE) )   sprintf(value,"%.4d",params->sequence>=0?params->sequence:params->data->sequence);
  else if( g_strcmp0(variable,"$(USERNAME)") == 0 && (got_value=TRUE) )   sprintf(value,"%s",g_get_user_name());
  else if( g_strcmp0(variable,"$(HOME_FOLDER)") == 0 && (got_value=TRUE)  )    sprintf(value,"%s",homedir);
  else if( g_strcmp0(variable,"$(PICTURES_FOLDER)") == 0 && (got_value=TRUE) )   sprintf(value,"%s",pictures_folder);
  else if( g_strcmp0(variable,"$(DESKTOP_FOLDER)") == 0 && (got_value=TRUE) )   sprintf(value,"%s",g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP));
#if 0
  else if( g_strcmp0(variable,"$(VC)") == 0 && (got_value=TRUE) )
  {
    sqlite3_stmt *stmt;
    DT_DEBUG_SQLITE3_PREPARE_V2(darktable.db, "select id from images where filename=?1", &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, params->filename);
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
      if(sqlite3_column_int(stmt) == )
      {
      }
    }
    sqlite3_finalize(stmt);
    sprintf(value,"%s",g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP));
  }
#endif
  else if( g_strcmp0(variable,"$(STARS)") == 0 && (got_value=TRUE) )
  {
    const dt_image_t *img = dt_image_cache_read_get(darktable.image_cache, params->imgid);
    int stars = (img->flags & 0x7);
    dt_image_cache_read_release(darktable.image_cache, img);
    if(stars == 6) stars = -1;
    sprintf(value,"%d",stars);
  }
  else if( g_strcmp0(variable,"$(LABELS)") == 0 && (got_value=TRUE) )
  {
    //TODO: currently we concatenate all the color labels with a ',' as a seeparator. Maybe it's better to only use the first/last label?
    unsigned int count = 0;
    GList *res = dt_metadata_get(params->imgid, "Xmp.darktable.colorlabels", &count);
    res = g_list_first(res);
    if(res != NULL)
    {
      GList *labels = NULL;
      do
      {
        labels = g_list_append(labels, (char *)(_(dt_colorlabels_to_string(GPOINTER_TO_INT(res->data)))));
      }
      while((res=g_list_next(res)) != NULL);
      char* str = dt_util_glist_to_str(",", labels, count);
      sprintf(value, "%s", str);
      g_free(str);
    }
    else
    {
      sprintf(value, _("none"));
    }
    g_list_free(res);
  }
  

  g_free(pictures_folder);

  return got_value;
}

void dt_variables_params_init(dt_variables_params_t **params)
{
  *params=g_malloc(sizeof(dt_variables_params_t));
  memset(*params ,0,sizeof(dt_variables_params_t));
  (*params)->data = g_malloc(sizeof(dt_variables_data_t));
  memset((*params)->data ,0,sizeof(dt_variables_data_t));
  (*params)->data->time=time(NULL);
  (*params)->sequence = -1;
}

void dt_variables_params_destroy(dt_variables_params_t *params)
{
  g_free(params->data);
  g_free(params);
}

void dt_variables_set_time(dt_variables_params_t *params, time_t time)
{
  params->data->time = time;
}

const gchar *dt_variables_get_result(dt_variables_params_t *params)
{
  return params->data->result;
}

gboolean dt_variables_expand(dt_variables_params_t *params, gchar *string, gboolean iterate)
{
  gchar *variable=g_malloc(128);
  gchar *value=g_malloc(1024);
  gchar *token=NULL;

  // Let's free previous expanded result if any...
  if( params->data->result )
    g_free(  params->data->result );

  if(iterate)
    params->data->sequence++;

  // Lets expand string
  gchar *result=NULL;
  params->data->result = params->data->source = string;
  if( (token=_string_get_first_variable(params->data->source,variable)) != NULL)
  {
    do
    {
      //fprintf(stderr,"var: %s\n",variable);
      if( _variable_get_value(params,variable,value) )
      {
        //fprintf(stderr,"Substitute variable '%s' with value '%s'\n",variable,value);
        if( (result=dt_util_str_replace(params->data->result,variable,value)) != params->data->result && result != params->data->source)
        {
          // we got a result
          if( params->data->result != params->data->source)
            g_free(params->data->result);
          params->data->result=result;
        }
      }
    }
    while( (token=_string_get_next_variable(token,variable)) !=NULL );
  }
  else
    params->data->result = g_strdup (string);

  g_free(variable);
  g_free(value);
  return TRUE;
}

void dt_variables_reset_sequence(dt_variables_params_t *params)
{
  params->data->sequence = 0;
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
