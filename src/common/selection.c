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
#include "common/debug.h"
#include "common/collection.h"
#include "control/signal.h"

typedef struct dt_selection_t
{
  /* the collection clone used for selection */
  const dt_collection_t *collection;

  /* this stores the last single clicked image id indicating
     the start of a selection range */
  uint32_t last_single_id;
}
dt_selection_t;

/* updates the internal collection of an selection */
static void _selection_update_collection(gpointer instance, gpointer user_data);

void _selection_update_collection(gpointer instance, gpointer user_data)
{
  dt_selection_t *selection = (dt_selection_t *)user_data;

  /* free previous collection copy if any */
  if (selection->collection)
    dt_collection_free(selection->collection);

  /* create a new fresh copy of darktable collection */
  selection->collection = dt_collection_new(darktable.collection);

  /* remove limit part of local collection */
  dt_collection_set_query_flags(selection->collection, 
				(dt_collection_get_query_flags(selection->collection) & 
				 (~(COLLECTION_QUERY_USE_LIMIT))));
  dt_collection_update (selection->collection);
}

const dt_selection_t * dt_selection_new()
{
  dt_selection_t *s = g_malloc(sizeof(dt_selection_t));
  memset(s, 0, sizeof(dt_selection_t));
  
  /* initialize the collection copy */
  _selection_update_collection(NULL, (gpointer)s);
 

  /* setup singal handler for darktable collection update 
   to update the internal collection of the selection */
  dt_control_signal_connect(darktable.signals, 
			    DT_SIGNAL_COLLECTION_CHANGED, 
			    G_CALLBACK(_selection_update_collection),
			    (gpointer) s);


  return s;
}

void dt_selection_free (dt_selection_t *selection)
{
  g_free(selection);
}

void dt_selection_invert(dt_selection_t *selection)
{
  gchar *fullq = NULL;

  if (!selection->collection)
    return;

  fullq = dt_util_dstrcat(fullq, "%s", "insert into selected_images ");
  fullq = dt_util_dstrcat(fullq, "%s", dt_collection_get_query(selection->collection));

  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"insert into memory.tmp_selection select imgid from selected_images", 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"delete from selected_images", 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), fullq, 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"delete from selected_images where imgid in (select imgid from memory.tmp_selection)", 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"delete from memory.tmp_selection", 
			NULL, NULL, NULL);

  g_free(fullq);

  /* update hint message */
  dt_collection_hint_message(darktable.collection);
}

void dt_selection_clear(const dt_selection_t *selection)
{
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "delete from selected_images", NULL, NULL, NULL);

  /* update hint message */
  dt_collection_hint_message(darktable.collection);
}

void dt_selection_select_single(dt_selection_t *selection, uint32_t imgid)
{
  gchar *query = NULL;
  selection->last_single_id = imgid;
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "delete from selected_images", NULL, NULL, NULL);
  query = dt_util_dstrcat(query,"insert into selected_images values(%d)", imgid);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), query, NULL, NULL, NULL);
  g_free(query);

  /* update hint message */
  dt_collection_hint_message(darktable.collection);
}

void dt_selection_toggle(dt_selection_t *selection, uint32_t imgid)
{
  gchar *query = NULL;
  sqlite3_stmt *stmt;
  gboolean exists = FALSE;

  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), 
			       "select imgid from selected_images where imgid=?1",-1,&stmt,NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, imgid);

  if(sqlite3_step(stmt) == SQLITE_ROW)
    exists = TRUE;

  sqlite3_finalize(stmt);

  if (exists)
  {
    selection->last_single_id = -1;
    query = dt_util_dstrcat(query,"delete from selected_images where imgid = %d", imgid);
  }
  else
  {
    selection->last_single_id = imgid;
    query = dt_util_dstrcat(query,"insert into selected_images values(%d)", imgid);
  }
 
  sqlite3_exec(dt_database_get(darktable.db), query, NULL, NULL, NULL);

  g_free(query);

  /* update hint message */
  dt_collection_hint_message(darktable.collection);
}

void dt_selection_select_all(dt_selection_t *selection)
{
  gchar *fullq = NULL;

  if (!selection->collection)
    return;

  fullq = dt_util_dstrcat(fullq, "%s", "insert into selected_images ");
  fullq = dt_util_dstrcat(fullq, "%s", dt_collection_get_query(selection->collection));

  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), "delete from selected_images", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), fullq, NULL, NULL, NULL);

  selection->last_single_id = -1;

  g_free(fullq);

  /* update hint message */
  dt_collection_hint_message(darktable.collection);
}

void dt_selection_select_range(dt_selection_t *selection, uint32_t imgid)
{
  gchar *fullq = NULL;
  sqlite3_stmt *stmt;
  if (!selection->collection || selection->last_single_id == -1)
    return;

  /* get start and end rows for range selection */
  int rc=0;
  uint32_t sr=-1,er=-1;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db),
			      dt_collection_get_query(selection->collection), -1, &stmt, NULL);
  
  while(sqlite3_step(stmt)==SQLITE_ROW)
  {
    int id = sqlite3_column_int(stmt, 0);
    if (id == selection->last_single_id)
      sr = rc;
  
    if (id == imgid)
       er = rc;
 
    if (sr != -1 && er != -1 )
      break;

    rc++;
  }

  sqlite3_finalize(stmt);

  /* selece the images in range from start to end */
  uint32_t old_flags = dt_collection_get_query_flags(selection->collection);
  
  /* use the limit to select range of images */
  dt_collection_set_query_flags(selection->collection, 
				(old_flags |COLLECTION_QUERY_USE_LIMIT));

  dt_collection_update(selection->collection);

  fullq = dt_util_dstrcat(fullq, "%s", "insert into selected_images ");
  fullq = dt_util_dstrcat(fullq, "%s", dt_collection_get_query(selection->collection));

  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db),
			      fullq, -1, &stmt, NULL);
  
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, MIN(sr,er));
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, (MAX(sr,er)-MIN(sr,er))+1);
  
  sqlite3_step(stmt);

  /* reset filter */
  dt_collection_set_query_flags(selection->collection,
				old_flags);
  dt_collection_update(selection->collection);
  selection->last_single_id = -1;
}

void dt_selection_select_filmroll(dt_selection_t *selection)
{
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"create temp table memory.tmp_selection (imgid integer)", 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"insert into memory.tmp_selection select imgid from selected_images", 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"delete from selected_images", 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"insert into selected_images select id from images where film_id in (select film_id from images as a join memory.tmp_selection as b on a.id = b.imgid)", 
			NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"delete from memory.tmp_selection", 
			NULL, NULL, NULL);
  selection->last_single_id = -1;
}

void dt_selection_select_unaltered(dt_selection_t *selection)
{
  char *fullq = NULL;

  if (!selection->collection)
    return;

  /* set unaltered collection filter and update query */
  uint32_t old_filter_flags = dt_collection_get_filter_flags(selection->collection); 
  dt_collection_set_filter_flags (selection->collection, 
				  (dt_collection_get_filter_flags(selection->collection) | 
				   COLLECTION_FILTER_UNALTERED));
  dt_collection_update(selection->collection);


  fullq = dt_util_dstrcat(fullq, "%s", "insert into selected_images ");
  fullq = dt_util_dstrcat(fullq, "%s", dt_collection_get_query(selection->collection));


  /* clean current selection and select unaltered images */
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			"delete from selected_images", NULL, NULL, NULL);
  DT_DEBUG_SQLITE3_EXEC(dt_database_get(darktable.db), 
			fullq, NULL, NULL, NULL);

  /* restore collection filter and update query */
  dt_collection_set_filter_flags(selection->collection, old_filter_flags);
  dt_collection_update(selection->collection);

  g_free(fullq);

  selection->last_single_id = -1;
}
