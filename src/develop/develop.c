/*
    This file is part of darktable,
    copyright (c) 2009--2011 johannes hanika.
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
#include "develop/develop.h"
#include "develop/imageop.h"
#include "develop/blend.h"
#include "control/jobs.h"
#include "control/control.h"
#include "control/conf.h"
#include "common/image_cache.h"
#include "common/mipmap_cache.h"
#include "common/imageio.h"
#include "common/tags.h"
#include "common/debug.h"
#include "gui/gtk.h"

#include <glib/gprintf.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <assert.h>


void dt_dev_init(dt_develop_t *dev, int32_t gui_attached)
{
  memset(dev,0,sizeof(dt_develop_t));
  dev->preview_downsampling = 1.0f;
  dev->gui_module = NULL;
  dev->timestamp = 0;
  dev->gui_leaving = 0;
  dev->gui_synch = 0;
  dt_pthread_mutex_init(&dev->history_mutex, NULL);
  dev->history_end = 0;
  dev->history = NULL; // empty list

  dev->gui_attached = gui_attached;
  dev->width = -1;
  dev->height = -1;

  dt_image_init(&dev->image_storage);
  dev->image_dirty = dev->preview_dirty = 1;
  dev->image_loading = dev->preview_loading = 0;
  dev->image_force_reload = 0;
  dev->preview_input_changed = 0;

  dev->pipe = dev->preview_pipe = NULL;
  dev->histogram = NULL;
  dev->histogram_pre_tonecurve = NULL;
  dev->histogram_pre_levels = NULL;

  if(dev->gui_attached)
  {
    dev->pipe = (dt_dev_pixelpipe_t *)malloc(sizeof(dt_dev_pixelpipe_t));
    dev->preview_pipe = (dt_dev_pixelpipe_t *)malloc(sizeof(dt_dev_pixelpipe_t));
    dt_dev_pixelpipe_init(dev->pipe);
    dt_dev_pixelpipe_init(dev->preview_pipe);

    dev->histogram = (float *)malloc(sizeof(float)*4*256);
    dev->histogram_pre_tonecurve = (float *)malloc(sizeof(float)*4*256);
    dev->histogram_pre_levels = (float*)malloc(sizeof(float) * 4 * 256);
    memset(dev->histogram, 0, sizeof(float)*256*4);
    memset(dev->histogram_pre_tonecurve, 0, sizeof(float)*256*4);
    memset(dev->histogram_pre_levels, 0, sizeof(float)*256*4);
    dev->histogram_max = -1;
    dev->histogram_pre_tonecurve_max = -1;
    dev->histogram_pre_levels_max = -1;
  }

  dev->iop_instance = 0;
  dev->iop = NULL;
}

void dt_dev_cleanup(dt_develop_t *dev)
{
  if(!dev) return;
  // image_cache does not have to be unref'd, this is done outside develop module.
  if(dev->pipe)
  {
    dt_dev_pixelpipe_cleanup(dev->pipe);
    free(dev->pipe);
  }
  if(dev->preview_pipe)
  {
    dt_dev_pixelpipe_cleanup(dev->preview_pipe);
    free(dev->preview_pipe);
  }
  while(dev->history)
  {
    free(((dt_dev_history_item_t *)dev->history->data)->params);
    free(((dt_dev_history_item_t *)dev->history->data)->blend_params);
    free( (dt_dev_history_item_t *)dev->history->data);
    dev->history = g_list_delete_link(dev->history, dev->history);
  }
  while(dev->iop)
  {
    dt_iop_cleanup_module((dt_iop_module_t *)dev->iop->data);
    free(dev->iop->data);
    dev->iop = g_list_delete_link(dev->iop, dev->iop);
  }
  dt_pthread_mutex_destroy(&dev->history_mutex);
  free(dev->histogram);
  free(dev->histogram_pre_tonecurve);
  free(dev->histogram_pre_levels);
}

void dt_dev_process_image(dt_develop_t *dev)
{
  if(/*dev->image_loading ||*/ !dev->gui_attached || dev->pipe->processing) return;
  dt_job_t job;
  dt_dev_process_image_job_init(&job, dev);
  int err = dt_control_add_job_res(darktable.control, &job, DT_CTL_WORKER_2);
  if(err) fprintf(stderr, "[dev_process_image] job queue exceeded!\n");
}

void dt_dev_process_preview(dt_develop_t *dev)
{
  if(!dev->gui_attached) return;
  dt_job_t job;
  dt_dev_process_preview_job_init(&job, dev);
  int err = dt_control_add_job_res(darktable.control, &job, DT_CTL_WORKER_3);
  if(err) fprintf(stderr, "[dev_process_preview] job queue exceeded!\n");
}

void dt_dev_invalidate(dt_develop_t *dev)
{
  dev->image_dirty = 1;
  dev->timestamp++;
  if(dev->preview_pipe) dev->preview_pipe->input_timestamp = dev->timestamp;
}

void dt_dev_invalidate_all(dt_develop_t *dev)
{
  dev->preview_dirty = dev->image_dirty = 1;
  dev->timestamp++;
}

void dt_dev_process_preview_job(dt_develop_t *dev)
{
  dt_mipmap_buffer_t buf;
  if(dev->image_loading)
  {
    // raw is already loading, no use starting another file access, we wait.
    return;
  }

  dt_control_log_busy_enter();
  dev->preview_pipe->input_timestamp = dev->timestamp;
  dev->preview_dirty = 1;

  // lock if there, issue a background load, if not (best-effort for mip f). 
  dt_mipmap_cache_read_get(darktable.mipmap_cache, &buf, dev->image_storage.id, DT_MIPMAP_F, 0);
  if(!buf.buf)
  {
    dt_control_log_busy_leave();
    return; // not loaded yet. load will issue a gtk redraw on completion, which in turn will trigger us again later.
  }
  // init pixel pipeline for preview.
  dt_dev_pixelpipe_set_input(dev->preview_pipe, dev, (float *)buf.buf, buf.width, buf.height, dev->image_storage.width/(float)buf.width);

  if(dev->preview_loading)
  {
    dt_dev_pixelpipe_cleanup_nodes(dev->preview_pipe);
    dt_dev_pixelpipe_create_nodes(dev->preview_pipe, dev);
    dt_dev_pixelpipe_flush_caches(dev->preview_pipe);
    dev->preview_loading = 0;
  }

  // if raw loaded, get new mipf
  if(dev->preview_input_changed)
  {
    dt_dev_pixelpipe_flush_caches(dev->preview_pipe);
    dev->preview_input_changed = 0;
  }

  // always process the whole downsampled mipf buffer, to allow for fast scrolling and mip4 write-through.
restart:
  if(dev->gui_leaving)
  {
    dt_control_log_busy_leave();
    dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);
    return;
  }
  // adjust pipeline according to changed flag set by {add,pop}_history_item.
  // this locks dev->history_mutex.
  dt_times_t start;
  dt_get_times(&start);
  dt_dev_pixelpipe_change(dev->preview_pipe, dev);
  if(dt_dev_pixelpipe_process(dev->preview_pipe, dev, 0, 0, dev->preview_pipe->processed_width*dev->preview_downsampling, dev->preview_pipe->processed_height*dev->preview_downsampling, dev->preview_downsampling))
  {
    if(dev->preview_loading || dev->preview_input_changed)
    {
      dt_control_log_busy_leave();
      dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);
      return;
    }
    else goto restart;
  }
  dt_show_times(&start, "[dev_process_preview] pixel pipeline processing", NULL);

  dev->preview_dirty = 0;
  dt_control_log_busy_leave();
  dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);
}

void dt_dev_process_image_job(dt_develop_t *dev)
{
  dt_control_log_busy_enter();
  // let gui know to draw preview instead of us, if it's there:
  dev->image_dirty = 1;

  dt_mipmap_buffer_t buf;
  dt_times_t start;
  dt_get_times(&start);
  dt_mipmap_cache_read_get(darktable.mipmap_cache, &buf, dev->image_storage.id, DT_MIPMAP_FULL, DT_MIPMAP_BLOCKING);
  dt_show_times(&start, "[dev]", "to load the image.");

  // copy over image now that width and height are sure to be correct:
  const dt_image_t *img = dt_image_cache_read_get(darktable.image_cache, dev->image_storage.id);
  dev->image_storage = *img;
  // but don't lock the real thing, as that would avoid any writers to change stuff.
  // (such as raw loading or star rating changes)
  dt_image_cache_read_release(darktable.image_cache, img);

  // failed to load raw?
  if(!buf.buf) return;
  
  dt_dev_pixelpipe_set_input(dev->pipe, dev, (float *)buf.buf, buf.width, buf.height, 1.0);

  if(dev->image_loading)
  {
    // init pixel pipeline
    dt_dev_pixelpipe_cleanup_nodes(dev->pipe);
    dt_dev_pixelpipe_create_nodes(dev->pipe, dev);
    if(dev->image_force_reload) dt_dev_pixelpipe_flush_caches(dev->pipe);
    dev->image_dirty = 1;
    dev->image_force_reload = 0;
    if(dev->gui_attached)
    {
      // during load, a mipf update could have been issued.
      dev->preview_input_changed = 1;
      dev->preview_dirty = 1;
      dev->gui_synch = 1; // notify gui thread we want to synch (call gui_update in the modules)
      dev->preview_pipe->changed |= DT_DEV_PIPE_SYNCH;
    }
    dev->pipe->changed |= DT_DEV_PIPE_SYNCH;
  }

  dt_dev_zoom_t zoom;
  float zoom_x, zoom_y, scale;
  int x, y;

  // adjust pipeline according to changed flag set by {add,pop}_history_item.
restart:
  if(dev->gui_leaving)
  {
    dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);
    dt_control_log_busy_leave();
    return;
  }
  dev->pipe->input_timestamp = dev->timestamp;
  // this locks dev->history_mutex.
  dt_dev_pixelpipe_change(dev->pipe, dev);
  // determine scale according to new dimensions
  DT_CTL_GET_GLOBAL(zoom, dev_zoom);
  DT_CTL_GET_GLOBAL(zoom_x, dev_zoom_x);
  DT_CTL_GET_GLOBAL(zoom_y, dev_zoom_y);

  scale = dt_dev_get_zoom_scale(dev, zoom, 1.0f, 0);
  dev->capwidth  = MIN(MIN(dev->width,  dev->pipe->processed_width *scale), darktable.thumbnail_width);
  dev->capheight = MIN(MIN(dev->height, dev->pipe->processed_height*scale), darktable.thumbnail_height);
  x = MAX(0, scale*dev->pipe->processed_width *(.5+zoom_x)-dev->capwidth/2);
  y = MAX(0, scale*dev->pipe->processed_height*(.5+zoom_y)-dev->capheight/2);

  dt_get_times(&start);
  if(dt_dev_pixelpipe_process(dev->pipe, dev, x, y, dev->capwidth, dev->capheight, scale))
  {
    // interrupted because image changed?
    if(dev->image_force_reload)
    {
      dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);
      dt_control_log_busy_leave();
      return;
    }
    // or because the pipeline changed?
    else goto restart;
  }
  dt_show_times(&start, "[dev_process_image] pixel pipeline processing", NULL);

  // maybe we got zoomed/panned in the meantime?
  if(dev->pipe->changed != DT_DEV_PIPE_UNCHANGED) goto restart;

  // cool, we got a new image!
  dev->image_dirty = 0;
  dev->image_loading = 0;

  dt_mipmap_cache_read_release(darktable.mipmap_cache, &buf);
  dt_control_queue_redraw_center();
  dt_control_log_busy_leave();
}

void dt_dev_reload_image(dt_develop_t *dev, const uint32_t imgid)
{
  const dt_image_t *image = dt_image_cache_read_get(darktable.image_cache, imgid);
  dev->image_storage = *image;
  dt_image_cache_read_release(darktable.image_cache, image);
  dev->image_force_reload = dev->image_loading = dev->preview_loading = 1;
  dev->pipe->changed |= DT_DEV_PIPE_SYNCH;
  dt_dev_invalidate(dev); // only invalidate image, preview will follow once it's loaded.
}

float dt_dev_get_zoom_scale(dt_develop_t *dev, dt_dev_zoom_t zoom, int closeup_factor, int preview)
{
  float zoom_scale;
  // set processed width to something useful while image is not there yet:
  int procw, proch;
  dt_dev_get_processed_size(dev, &procw, &proch);
  const float w = preview ? dev->preview_pipe->backbuf_width  : procw;
  const float h = preview ? dev->preview_pipe->backbuf_height : proch;
  switch(zoom)
  {
    case DT_ZOOM_FIT:
      zoom_scale = fminf(dev->width/w, dev->height/h);
      break;
    case DT_ZOOM_FILL:
      zoom_scale = fmaxf(dev->width/w, dev->height/h);
      break;
    case DT_ZOOM_1:
      zoom_scale = closeup_factor;
      if(preview) zoom_scale *= dev->preview_pipe->iscale / dev->preview_downsampling;
      break;
    default: // DT_ZOOM_FREE
      DT_CTL_GET_GLOBAL(zoom_scale, dev_zoom_scale);
      if(preview) zoom_scale *= dev->preview_pipe->iscale / dev->preview_downsampling;
      break;
  }
  return zoom_scale;
}

void dt_dev_load_image(dt_develop_t *dev, const uint32_t imgid)
{
  const dt_image_t *image = dt_image_cache_read_get(darktable.image_cache, imgid);
  dev->image_storage = *image;
  dt_image_cache_read_release(darktable.image_cache, image);
  if(dev->pipe)
  {
    dev->pipe->processed_width  = 0;
    dev->pipe->processed_height = 0;
  }
  dev->image_loading = 1;
  dev->preview_loading = 1;
  dev->image_dirty = dev->preview_dirty = 1;

  dev->iop = dt_iop_load_modules(dev);
  dt_dev_read_history(dev);
}

void dt_dev_configure (dt_develop_t *dev, int wd, int ht)
{
  wd = MIN(darktable.thumbnail_width, wd);
  ht = MIN(darktable.thumbnail_height, ht);
  if(dev->width != wd || dev->height != ht)
  {
    dev->width  = wd;
    dev->height = ht;
    dev->preview_pipe->changed |= DT_DEV_PIPE_ZOOMED;
    dev->pipe->changed |= DT_DEV_PIPE_ZOOMED;
    dt_dev_invalidate(dev);
  }
}

// helper used to synch a single history item with db
int dt_dev_write_history_item(const dt_image_t *image, dt_dev_history_item_t *h, int32_t num)
{
  if(!image) return 1;
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select num from history where imgid = ?1 and num = ?2", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, image->id);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, num);
  if(sqlite3_step(stmt) != SQLITE_ROW)
  {
    sqlite3_finalize(stmt);
    DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "insert into history (imgid, num) values (?1, ?2)", -1, &stmt, NULL);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, image->id);
    DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, num);
    sqlite3_step (stmt);
  }
  // printf("[dev write history item] writing %d - %s params %f %f\n", h->module->instance, h->module->op, *(float *)h->params, *(((float *)h->params)+1));
  sqlite3_finalize (stmt);
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "update history set operation = ?1, op_params = ?2, module = ?3, enabled = ?4, blendop_params = ?7 where imgid = ?5 and num = ?6", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_TEXT(stmt, 1, h->module->op, strlen(h->module->op), SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_BLOB(stmt, 2, h->params, h->module->params_size, SQLITE_TRANSIENT);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 3, h->module->version());
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 4, h->enabled);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 5, image->id);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 6, num);
  DT_DEBUG_SQLITE3_BIND_BLOB(stmt, 7, h->blend_params, sizeof(dt_develop_blend_params_t), SQLITE_TRANSIENT);

  sqlite3_step (stmt);
  sqlite3_finalize (stmt);
  return 0;
}

void dt_dev_add_history_item(dt_develop_t *dev, dt_iop_module_t *module, gboolean enable)
{
  if(darktable.gui->reset) return;
  dt_pthread_mutex_lock(&dev->history_mutex);
  if(dev->gui_attached)
  {
    GList *history = g_list_nth (dev->history, dev->history_end);
    while(history)
    {
      GList *next = g_list_next(history);
      dt_dev_history_item_t *hist = (dt_dev_history_item_t *)(history->data);
      // printf("removing obsoleted history item: %s\n", hist->module->op);
      free(hist->params);
      free(hist->blend_params);
      free(history->data);
      dev->history = g_list_delete_link(dev->history, history);
      history = next;
    }
    history = g_list_nth(dev->history, dev->history_end-1);
    if(!history || module->instance != ((dt_dev_history_item_t *)history->data)->module->instance)
    {
      // new operation, push new item
      // printf("adding new history item %d - %s\n", dev->history_end, module->op);
      // if(history) printf("because item %d - %s is different operation.\n", dev->history_end-1, ((dt_dev_history_item_t *)history->data)->module->op);
      dev->history_end++;
      dt_dev_history_item_t *hist = (dt_dev_history_item_t *)malloc(sizeof(dt_dev_history_item_t));
      if (enable)
      {
        module->enabled = TRUE;
        if(module->off)
        {
          darktable.gui->reset = 1;
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(module->off), module->enabled);
          darktable.gui->reset = 0;
        }
      }
      hist->enabled = module->enabled;
      hist->module = module;
      hist->params = malloc(module->params_size);

      /* allocate and set hist blend_params */
      hist->blend_params = malloc(sizeof(dt_develop_blend_params_t));
      memset(hist->blend_params, 0, sizeof(dt_develop_blend_params_t));

      memcpy(hist->params, module->params, module->params_size);
      if(module->flags() & IOP_FLAGS_SUPPORTS_BLENDING)
        memcpy(hist->blend_params, module->blend_params, sizeof(dt_develop_blend_params_t));

      dev->history = g_list_append(dev->history, hist);
      dev->pipe->changed |= DT_DEV_PIPE_SYNCH;
      dev->preview_pipe->changed |= DT_DEV_PIPE_SYNCH; // topology remains, as modules are fixed for now.
    }
    else
    {
      // same operation, change params
      // printf("changing same history item %d - %s\n", dev->history_end-1, module->op);
      dt_dev_history_item_t *hist = (dt_dev_history_item_t *)history->data;
      memcpy(hist->params, module->params, module->params_size);

      if(module->flags() & IOP_FLAGS_SUPPORTS_BLENDING)
        memcpy(hist->blend_params, module->blend_params, sizeof(dt_develop_blend_params_t));

      // if the user changed stuff and the module is still not enabled, do it:
      if(!hist->enabled && !module->enabled)
      {
        module->enabled = 1;
        if(module->off)
        {
          darktable.gui->reset = 1;
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(module->off), module->enabled);
          darktable.gui->reset = 0;
        }
      }
      hist->enabled = module->enabled;
      dev->pipe->changed |= DT_DEV_PIPE_TOP_CHANGED;
      dev->preview_pipe->changed |= DT_DEV_PIPE_TOP_CHANGED;
    }
  }
#if 0
  {
    // debug:
    printf("remaining %d history items:\n", dev->history_end);
    GList *history = dev->history;
    int i = 0;
    while(history)
    {
      dt_dev_history_item_t *hist = (dt_dev_history_item_t *)(history->data);
      printf("%d %s\n", i, hist->module->op);
      history = g_list_next(history);
      i++;
    }
  }
#endif

  // invalidate buffers and force redraw of darkroom
  dt_dev_invalidate_all(dev);
  dt_pthread_mutex_unlock(&dev->history_mutex);

  if(dev->gui_attached)
  {
    /* signal that history has changed */
    dt_control_signal_raise(darktable.signals, DT_SIGNAL_DEVELOP_HISTORY_CHANGE);

    /* redraw */
    dt_control_queue_redraw_center();
  }
}

void dt_dev_reload_history_items(dt_develop_t *dev)
{
  dt_dev_pop_history_items(dev, 0);
  // remove unused history items:
  GList *history = g_list_nth(dev->history, dev->history_end);
  while(history)
  {
    GList *next = g_list_next(history);
    dt_dev_history_item_t *hist = (dt_dev_history_item_t *)(history->data);
    free(hist->params);
    free(hist->blend_params);
    free(history->data);
    dev->history = g_list_delete_link(dev->history, history);
    history = next;
  }
  dt_dev_read_history(dev);
  dt_dev_pop_history_items(dev, dev->history_end);
}

void dt_dev_pop_history_items(dt_develop_t *dev, int32_t cnt)
{
  // printf("dev popping all history items >= %d\n", cnt);
  dt_pthread_mutex_lock(&dev->history_mutex);
  darktable.gui->reset = 1;
  dev->history_end = cnt;
  // reset gui params for all modules
  GList *modules = dev->iop;
  while(modules)
  {
    dt_iop_module_t *module = (dt_iop_module_t *)(modules->data);
    memcpy(module->params, module->default_params, module->params_size);
    memcpy(module->blend_params, module->default_blendop_params,sizeof(dt_develop_blend_params_t));
    module->enabled = module->default_enabled;
    modules = g_list_next(modules);
  }
  // go through history and set gui params
  GList *history = dev->history;
  for(int i=0; i<cnt && history; i++)
  {
    dt_dev_history_item_t *hist = (dt_dev_history_item_t *)(history->data);
    memcpy(hist->module->params, hist->params, hist->module->params_size);
    memcpy(hist->module->blend_params, hist->blend_params, sizeof(dt_develop_blend_params_t));

    hist->module->enabled = hist->enabled;
    history = g_list_next(history);
  }
  // update all gui modules
  modules = dev->iop;
  while(modules)
  {
    dt_iop_module_t *module = (dt_iop_module_t *)(modules->data);
    dt_iop_gui_update(module);
    modules = g_list_next(modules);
  }
  dev->pipe->changed |= DT_DEV_PIPE_SYNCH;
  dev->preview_pipe->changed |= DT_DEV_PIPE_SYNCH; // again, fixed topology for now.
  darktable.gui->reset = 0;
  dt_dev_invalidate_all(dev);
  dt_pthread_mutex_unlock(&dev->history_mutex);
  dt_control_queue_redraw_center();
}

void dt_dev_write_history(dt_develop_t *dev)
{
  sqlite3_stmt *stmt;

  gboolean changed = FALSE;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "delete from history where imgid = ?1", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, dev->image_storage.id);
  sqlite3_step(stmt);
  sqlite3_finalize (stmt);
  GList *history = dev->history;
  for(int i=0; i<dev->history_end && history; i++)
  {
    dt_dev_history_item_t *hist = (dt_dev_history_item_t *)(history->data);
    (void)dt_dev_write_history_item(&dev->image_storage, hist, i);
    history = g_list_next(history);
    changed = TRUE;
  }
  
  /* attach / detach changed tag reflecting actual change */
  guint tagid = 0;
  dt_tag_new("darktable|changed",&tagid); 
  if(changed)
    dt_tag_attach(tagid, dev->image_storage.id);
  else
    dt_tag_detach(tagid, dev->image_storage.id);

}

void dt_dev_read_history(dt_develop_t *dev)
{
  if(!dev->image_storage.id) return;
  sqlite3_stmt *stmt;
  DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db), "select * from history where imgid = ?1 order by num", -1, &stmt, NULL);
  DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, dev->image_storage.id);
  dev->history_end = 0;
  while(sqlite3_step(stmt) == SQLITE_ROW)
  {
    // db record:
    // 0-img, 1-num, 2-module_instance, 3-operation char, 4-params blob, 5-enabled, 6-blend_params
    dt_dev_history_item_t *hist = (dt_dev_history_item_t *)malloc(sizeof(dt_dev_history_item_t));
    hist->enabled = sqlite3_column_int(stmt, 5);

    GList *modules = dev->iop;
    const char *opname = (const char *)sqlite3_column_text(stmt, 3);
    if(!opname)
    {
      fprintf(stderr, "[dev_read_history] database history for image `%s' seems to be corrupted!\n", dev->image_storage.filename);
      free(hist);
      continue;
    }
   
    hist->module = NULL;
    while(modules)
    {
      dt_iop_module_t *module = (dt_iop_module_t *)modules->data;
      if(!strcmp(module->op, opname))
      {
        hist->module = module;
        break;
      }
      modules = g_list_next(modules);
    }
    if(!hist->module)
    {
      fprintf(stderr, "[dev_read_history] the module `%s' requested by image `%s' is not installed on this computer!\n", opname, dev->image_storage.filename);
      free(hist);
      continue;
    }
    int modversion = sqlite3_column_int(stmt, 2);
    assert(strcmp((char *)sqlite3_column_text(stmt, 3), hist->module->op) == 0);
    hist->params = malloc(hist->module->params_size);
    hist->blend_params = malloc(sizeof(dt_develop_blend_params_t));
    if(hist->module->version() != modversion || hist->module->params_size != sqlite3_column_bytes(stmt, 4) ||
        strcmp((char *)sqlite3_column_text(stmt, 3), hist->module->op))
    {
      if(!hist->module->legacy_params ||
          hist->module->legacy_params(hist->module, sqlite3_column_blob(stmt, 4), labs(modversion), hist->params, labs(hist->module->version())))
      {
        free(hist->params);
        free(hist->blend_params);
        fprintf(stderr, "[dev_read_history] module `%s' version mismatch: history is %d, dt %d.\n", hist->module->op, modversion, hist->module->version());
        const char *fname = dev->image_storage.filename + strlen(dev->image_storage.filename);
        while(fname > dev->image_storage.filename && *fname != '/') fname --;
        if(fname > dev->image_storage.filename) fname++;
        dt_control_log(_("%s: module `%s' version mismatch: %d != %d"), fname, hist->module->op, hist->module->version(), modversion);
        free(hist);
        continue;
      }
    }
    else
    {
      memcpy(hist->params, sqlite3_column_blob(stmt, 4), hist->module->params_size);
    }

    if(sqlite3_column_bytes(stmt, 6) == sizeof(dt_develop_blend_params_t))
      memcpy(hist->blend_params, sqlite3_column_blob(stmt, 6), sizeof(dt_develop_blend_params_t));
    else
      memset(hist->blend_params, 0, sizeof(dt_develop_blend_params_t));

    // memcpy(hist->module->params, hist->params, hist->module->params_size);
    // hist->module->enabled = hist->enabled;
    // printf("[dev read history] img %d number %d for operation %d - %s params %f %f\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), instance, hist->module->op, *(float *)hist->params, *(((float*)hist->params)+1));
    dev->history = g_list_append(dev->history, hist);
    dev->history_end ++;
  }

  if(dev->gui_attached)
  {
    dev->pipe->changed |= DT_DEV_PIPE_SYNCH;
    dev->preview_pipe->changed |= DT_DEV_PIPE_SYNCH; // again, fixed topology for now.
    dt_dev_invalidate_all(dev);

    /* signal history changed */
    dt_control_signal_raise(darktable.signals,DT_SIGNAL_DEVELOP_HISTORY_CHANGE);
  }
  sqlite3_finalize (stmt);
}

void dt_dev_check_zoom_bounds(dt_develop_t *dev, float *zoom_x, float *zoom_y, dt_dev_zoom_t zoom, int closeup, float *boxww, float *boxhh)
{
  int procw, proch;
  dt_dev_get_processed_size(dev, &procw, &proch);
  float boxw = 1, boxh = 1; // viewport in normalised space
//   if(zoom == DT_ZOOM_1)
//   {
//     const float imgw = (closeup ? 2 : 1)*procw;
//     const float imgh = (closeup ? 2 : 1)*proch;
//     const float devw = MIN(imgw, dev->width);
//     const float devh = MIN(imgh, dev->height);
//     boxw = fminf(1.0, devw/imgw);
//     boxh = fminf(1.0, devh/imgh);
//   }
  if(zoom == DT_ZOOM_FIT)
  {
    *zoom_x = *zoom_y = 0.0f;
    boxw = boxh = 1.0f;
  }
  else
  {
    const float scale = dt_dev_get_zoom_scale(dev, zoom, closeup ? 2 : 1, 0);
    const float imgw = procw;
    const float imgh = proch;
    const float devw = dev->width;
    const float devh = dev->height;
    boxw = devw/(imgw*scale);
    boxh = devh/(imgh*scale);
  }

  if(*zoom_x < boxw/2 - .5) *zoom_x = boxw/2 - .5;
  if(*zoom_x > .5 - boxw/2) *zoom_x = .5 - boxw/2;
  if(*zoom_y < boxh/2 - .5) *zoom_y = boxh/2 - .5;
  if(*zoom_y > .5 - boxh/2) *zoom_y = .5 - boxh/2;
  if(boxw > 1.0) *zoom_x = 0.f;
  if(boxh > 1.0) *zoom_y = 0.f;

  if(boxww) *boxww = boxw;
  if(boxhh) *boxhh = boxh;
}

void dt_dev_get_processed_size(const dt_develop_t *dev, int *procw, int *proch)
{
  // [jo] TODO: i hate this whole function. thread-safe? get rid of it!
  const float scale = dev->preview_pipe ? (dev->preview_pipe->iscale / dev->preview_downsampling) : 1.0f;
  *procw = dev->pipe && dev->pipe->processed_width  ? dev->pipe->processed_width  : scale * dev->preview_pipe->processed_width;
  *proch = dev->pipe && dev->pipe->processed_height ? dev->pipe->processed_height : scale * dev->preview_pipe->processed_height;
}

void dt_dev_get_pointer_zoom_pos(dt_develop_t *dev, const float px, const float py, float *zoom_x, float *zoom_y)
{
  dt_dev_zoom_t zoom;
  int closeup, procw, proch;
  float zoom2_x, zoom2_y;
  DT_CTL_GET_GLOBAL(zoom, dev_zoom);
  DT_CTL_GET_GLOBAL(closeup, dev_closeup);
  DT_CTL_GET_GLOBAL(zoom2_x, dev_zoom_x);
  DT_CTL_GET_GLOBAL(zoom2_y, dev_zoom_y);
  dt_dev_get_processed_size(dev, &procw, &proch);
  const float scale = dt_dev_get_zoom_scale(dev, zoom, closeup ? 2.0 : 1.0, 0);
  // offset from center now (current zoom_{x,y} points there)
  const float mouse_off_x = px - .5*dev->width, mouse_off_y = py - .5*dev->height;
  zoom2_x += mouse_off_x/(procw*scale);
  zoom2_y += mouse_off_y/(proch*scale);
  *zoom_x = zoom2_x;
  *zoom_y = zoom2_y;
}

void dt_dev_get_history_item_label(dt_dev_history_item_t *hist, char *label, const int cnt)
{
  g_snprintf(label, cnt, "%s (%s)", hist->module->name(), hist->enabled ? _("on") : _("off"));
}

int
dt_dev_is_current_image (dt_develop_t *dev, uint32_t imgid)
{
  return (dev->image_storage.id==imgid)?1:0;
}

gboolean dt_dev_exposure_hooks_available(dt_develop_t *dev)
{
  /* check if exposure iop module has registered its hooks */
  if( dev->proxy.exposure.module && 
      dev->proxy.exposure.set_black && dev->proxy.exposure.get_black &&
      dev->proxy.exposure.set_white && dev->proxy.exposure.get_white)
    return TRUE;

  return FALSE;
}

void dt_dev_exposure_reset_defaults(dt_develop_t *dev)
{
  if (!dev->proxy.exposure.module) return;

  dt_iop_module_t *exposure = dev->proxy.exposure.module;
  memcpy(exposure->params, exposure->default_params, exposure->params_size);
  exposure->gui_update(exposure);
  dt_dev_add_history_item(exposure->dev, exposure, TRUE);
}

void dt_dev_exposure_set_white(dt_develop_t *dev, const float white)
{
   if (dev->proxy.exposure.module && dev->proxy.exposure.set_white)
     dev->proxy.exposure.set_white(dev->proxy.exposure.module, white);
}

float dt_dev_exposure_get_white(dt_develop_t *dev)
{
  if (dev->proxy.exposure.module && dev->proxy.exposure.set_white)
     return dev->proxy.exposure.get_white(dev->proxy.exposure.module);

  return 0.0;
}

void dt_dev_exposure_set_black(dt_develop_t *dev, const float black)
{
  if (dev->proxy.exposure.module && dev->proxy.exposure.set_black)
     dev->proxy.exposure.set_black(dev->proxy.exposure.module, black);
}

float dt_dev_exposure_get_black(dt_develop_t *dev)
{
  if (dev->proxy.exposure.module && dev->proxy.exposure.set_black)
     return dev->proxy.exposure.get_black(dev->proxy.exposure.module);

  return 0.0;
}

gboolean dt_dev_modulegroups_available(dt_develop_t *dev) 
{
  if(dev->proxy.modulegroups.module && dev->proxy.modulegroups.set && dev->proxy.modulegroups.get)
    return TRUE;

  return FALSE;
}

void dt_dev_modulegroups_set(dt_develop_t *dev, uint32_t group)
{
  if(dev->proxy.modulegroups.module && dev->proxy.modulegroups.set)
    dev->proxy.modulegroups.set(dev->proxy.modulegroups.module, group);
}

uint32_t dt_dev_modulegroups_get(dt_develop_t *dev)
{
  if(dev->proxy.modulegroups.module && dev->proxy.modulegroups.get)
    return dev->proxy.modulegroups.get(dev->proxy.modulegroups.module);

  return 0;
}

gboolean dt_dev_modulegroups_test(dt_develop_t *dev, uint32_t group, uint32_t iop_group)
{
  if(dev->proxy.modulegroups.module && dev->proxy.modulegroups.test)
    return dev->proxy.modulegroups.test(dev->proxy.modulegroups.module, group, iop_group); 
  return FALSE;
}

void dt_dev_modulegroups_switch(dt_develop_t *dev, dt_iop_module_t *module)
{
  if (dev->proxy.modulegroups.module && dev->proxy.modulegroups.switch_group)
    dev->proxy.modulegroups.switch_group(dev->proxy.modulegroups.module, module);
}


void dt_dev_snapshot_request(dt_develop_t *dev, const char *filename)
{
  dev->proxy.snapshot.filename = filename;
  dev->proxy.snapshot.request = TRUE;
  dt_control_queue_redraw_center();
}

void dt_dev_invalidate_from_gui (dt_develop_t *dev)
{
  dt_dev_pop_history_items(darktable.develop, darktable.develop->history_end);
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
