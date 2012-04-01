/*
    This file is part of darktable,
    copyright (c) 2010--2011 Henrik Andersson.

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
#ifndef DT_COLLECTION_H
#define DT_COLLECTION_H

#include <inttypes.h>
#include <glib.h>

#define COLLECTION_QUERY_SIMPLE                 0           // a query with only select and where statement
#define COLLECTION_QUERY_USE_SORT               1           // if query should include order by statement
#define COLLECTION_QUERY_USE_LIMIT              2           // if query should include "limit ?1,?2" part
#define COLLECTION_QUERY_USE_WHERE_EXT          4           // if query should include extended where part
#define COLLECTION_QUERY_USE_ONLY_WHERE_EXT     8           // if query should only use extended where part

#define COLLECTION_QUERY_FULL (COLLECTION_QUERY_USE_SORT|COLLECTION_QUERY_USE_LIMIT)


#define COLLECTION_FILTER_FILM_ID               1             // use film_id in filter
#define COLLECTION_FILTER_ATLEAST_RATING        2             // show all stars including and above selected star filter
#define COLLECTION_FILTER_EQUAL_RATING          4             // show only selected star filter
#define COLLECTION_FILTER_ALTERED               8             // show only altered images
#define COLLECTION_FILTER_UNALTERED            16             // show only unaltered images

typedef enum dt_collection_filter_t
{
  DT_COLLECTION_FILTER_ALL = 0,
  DT_COLLECTION_FILTER_STAR_NO = 1,
  DT_COLLECTION_FILTER_STAR_1 = 2,
  DT_COLLECTION_FILTER_STAR_2 = 3,
  DT_COLLECTION_FILTER_STAR_3 = 4,
  DT_COLLECTION_FILTER_STAR_4 = 5,
  DT_COLLECTION_FILTER_STAR_5 = 6,
  DT_COLLECTION_FILTER_REJECT = 7
}
dt_collection_filter_t;

typedef enum dt_collection_sort_t
{
  DT_COLLECTION_SORT_FILENAME = 0,
  DT_COLLECTION_SORT_DATETIME,
  DT_COLLECTION_SORT_RATING,
  DT_COLLECTION_SORT_ID,
  DT_COLLECTION_SORT_COLOR
}
dt_collection_sort_t;

typedef struct dt_collection_params_t
{
  /** flags for which query parts to use, see COLLECTION_QUERY_x defines... */
  uint32_t query_flags;

  /** flags for which filters to use, see COLLECTION_FILTER_x defines... */
  uint32_t filter_flags;

  /** current film id */
  uint32_t film_id;

  /** current  filter */
  uint32_t rating;

  /** sorting **/
  dt_collection_sort_t sort;  // Has to be changed to a dt_collection_sort struct
  gint descending;

} dt_collection_params_t;

typedef struct dt_collection_t
{
  int clone;
  gchar *query;
  gchar *where_ext;
  dt_collection_params_t params;
  dt_collection_params_t store;
}
dt_collection_t;


/** instansiates a collection context, if clone equals NULL default query is constructed. */
const dt_collection_t * dt_collection_new (const dt_collection_t *clone);
/** frees a collection context. */
void dt_collection_free (const dt_collection_t *collection);
/** fetch params for collection for storing. */
const dt_collection_params_t * dt_collection_params (const dt_collection_t *collection);
/** get the generated query for collection */
const gchar *dt_collection_get_query (const dt_collection_t *collection);
/** updates sql query for a collection. @return 1 if query changed. */
int dt_collection_update (const dt_collection_t *collection);
/** reset collection to default dummy selection */
void dt_collection_reset (const dt_collection_t *collection);
/** sets an extended where part */
void dt_collection_set_extended_where (const dt_collection_t *collection,gchar *extended_where);

/** get filter flags for collection */
uint32_t dt_collection_get_filter_flags (const dt_collection_t *collection);
/** set filter flags for collection */
void dt_collection_set_filter_flags (const dt_collection_t *collection, uint32_t flags);

/** get filter flags for collection */
uint32_t dt_collection_get_query_flags (const dt_collection_t *collection);
/** set filter flags for collection */
void dt_collection_set_query_flags (const dt_collection_t *collection, uint32_t flags);

/** set the film_id of collection */
void dt_collection_set_film_id (const dt_collection_t *collection, uint32_t film_id);
/** set the star level for filter */
void dt_collection_set_rating (const dt_collection_t *collection, uint32_t rating);
/** get the star level for filter. The value returned starts on 0 **/
uint32_t dt_collection_get_rating (const dt_collection_t *collection);

/** set the sort fields and flags used to show the collection **/
void dt_collection_set_sort(const dt_collection_t *collection, dt_collection_sort_t sort, gint reverse);
/** get the sort field used **/
dt_collection_sort_t dt_collection_get_sort_field(const dt_collection_t *collection);
/** get if the collection must be shown in descending order **/
gboolean dt_collection_get_sort_descending(const dt_collection_t *collection);
/** get the part of the query for sorting the collection **/
gchar *dt_collection_get_sort_query(const dt_collection_t *collection);

/** get the count of query */
uint32_t dt_collection_get_count (const dt_collection_t *collection);

/** get selected image ids order as current selection. */
GList *dt_collection_get_selected (const dt_collection_t *collection);
/** get the count of selected images */
uint32_t dt_collection_get_selected_count (const dt_collection_t *collection);

/** update query by conf vars */
void dt_collection_update_query(const dt_collection_t *collection);

/** updates the hint message for collection */
void dt_collection_hint_message(const dt_collection_t *collection);

#endif
