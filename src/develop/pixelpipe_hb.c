/*
    This file is part of darktable,
    copyright (c) 2009--2011 johannes hanika.

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
#include "develop/pixelpipe.h"
#include "develop/blend.h"
#include "develop/tiling.h"
#include "gui/gtk.h"
#include "control/control.h"
#include "control/signal.h"
#include "common/opencl.h"
#include "libs/lib.h"
#include "libs/colorpicker.h"
#include "iop/colorout.h"

#include <assert.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

// this is to ensure compatibility with pixelpipe_gegl.c, which does not need to build the other module:
#include "develop/pixelpipe_cache.c"

#define max(a,b) ((a) > (b) ? (a) : (b))

int dt_dev_pixelpipe_init_export(dt_dev_pixelpipe_t *pipe, int32_t width, int32_t height)
{
  int res = dt_dev_pixelpipe_init_cached(pipe, 4*sizeof(float)*width*height, 2);
  pipe->type = DT_DEV_PIXELPIPE_EXPORT;
  return res;
}

int dt_dev_pixelpipe_init(dt_dev_pixelpipe_t *pipe)
{
  int res = dt_dev_pixelpipe_init_cached(pipe, 4*sizeof(float)*darktable.thumbnail_width*darktable.thumbnail_height, 5);
  pipe->type = DT_DEV_PIXELPIPE_FULL;
  return res;
}

int dt_dev_pixelpipe_init_cached(dt_dev_pixelpipe_t *pipe, int32_t size, int32_t entries)
{
  pipe->devid = -1;
  pipe->changed = DT_DEV_PIPE_UNCHANGED;
  pipe->processed_width  = pipe->backbuf_width  = pipe->iwidth = 0;
  pipe->processed_height = pipe->backbuf_height = pipe->iheight = 0;
  pipe->nodes = NULL;
  pipe->backbuf_size = size;
  if(!dt_dev_pixelpipe_cache_init(&(pipe->cache), entries, pipe->backbuf_size))
    return 0;
  pipe->backbuf = NULL;
  pipe->processing = 0;
  pipe->shutdown = 0;
  pipe->opencl_error = 0;
  pipe->input_timestamp = 0;
  dt_pthread_mutex_init(&(pipe->backbuf_mutex), NULL);
  dt_pthread_mutex_init(&(pipe->busy_mutex), NULL);
  return 1;
}

void dt_dev_pixelpipe_set_input(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, float *input, int width, int height, float iscale)
{
  pipe->iwidth  = width;
  pipe->iheight = height;
  pipe->iscale = iscale;
  pipe->input = input;
  pipe->image = dev->image_storage;
  if(width < pipe->image.width && height < pipe->image.height) pipe->type = DT_DEV_PIXELPIPE_PREVIEW;
}

void dt_dev_pixelpipe_cleanup(dt_dev_pixelpipe_t *pipe)
{
  dt_pthread_mutex_lock(&pipe->backbuf_mutex);
  pipe->backbuf = NULL;
  // blocks while busy and sets shutdown bit:
  dt_dev_pixelpipe_cleanup_nodes(pipe);
  // so now it's safe to clean up cache:
  dt_dev_pixelpipe_cache_cleanup(&(pipe->cache));
  dt_pthread_mutex_unlock(&pipe->backbuf_mutex);
  dt_pthread_mutex_destroy(&(pipe->backbuf_mutex));
  dt_pthread_mutex_destroy(&(pipe->busy_mutex));
}

void dt_dev_pixelpipe_cleanup_nodes(dt_dev_pixelpipe_t *pipe)
{
  // FIXME: either this or all process() -> gdk mutices have to be changed!
  //        (this is a circular dependency on busy_mutex and the gdk mutex)
  dt_pthread_mutex_lock(&pipe->busy_mutex);
  pipe->shutdown = 1;
  // destroy all nodes
  GList *nodes = pipe->nodes;
  while(nodes)
  {
    dt_dev_pixelpipe_iop_t *piece = (dt_dev_pixelpipe_iop_t *)nodes->data;
    // printf("cleanup module `%s'\n", piece->module->name());
    piece->module->cleanup_pipe(piece->module, pipe, piece);
    free(piece->blendop_data);
    free(piece);
    nodes = g_list_next(nodes);
  }
  g_list_free(pipe->nodes);
  pipe->nodes = NULL;
  dt_pthread_mutex_unlock(&pipe->busy_mutex);
}

void dt_dev_pixelpipe_create_nodes(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev)
{
  dt_pthread_mutex_lock(&pipe->busy_mutex);
  pipe->shutdown = 0;
  g_assert(pipe->nodes == NULL);
  // for all modules in dev:
  GList *modules = dev->iop;
  while(modules)
  {
    dt_iop_module_t *module = (dt_iop_module_t *)modules->data;
    // if(module->enabled) // no! always create nodes. just don't process.
    {
      dt_dev_pixelpipe_iop_t *piece = (dt_dev_pixelpipe_iop_t *)malloc(sizeof(dt_dev_pixelpipe_iop_t));
      piece->enabled = module->enabled;
      piece->colors  = 4;
      piece->iscale  = pipe->iscale;
      piece->iwidth  = pipe->iwidth;
      piece->iheight = pipe->iheight;
      piece->module  = module;
      piece->pipe    = pipe;
      piece->data = NULL;
      piece->hash = 0;
      dt_iop_init_pipe(piece->module, pipe,piece);
      pipe->nodes = g_list_append(pipe->nodes, piece);
    }
    modules = g_list_next(modules);
  }
  dt_pthread_mutex_unlock(&pipe->busy_mutex);
}

// helper
void dt_dev_pixelpipe_synch(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, GList *history)
{
  dt_dev_history_item_t *hist = (dt_dev_history_item_t *)history->data;
  // find piece in nodes list
  GList *nodes = pipe->nodes;
  dt_dev_pixelpipe_iop_t *piece = NULL;
  while(nodes)
  {
    piece = (dt_dev_pixelpipe_iop_t *)nodes->data;
    if(piece->module == hist->module)
    {
      piece->enabled = hist->enabled;
      dt_iop_commit_params(hist->module, hist->params, hist->blend_params, pipe, piece);
    }
    nodes = g_list_next(nodes);
  }
}

void dt_dev_pixelpipe_synch_all(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev)
{
  dt_pthread_mutex_lock(&pipe->busy_mutex);
  // call reset_params on all pieces first.
  GList *nodes = pipe->nodes;
  while(nodes)
  {
    dt_dev_pixelpipe_iop_t *piece = (dt_dev_pixelpipe_iop_t *)nodes->data;
    piece->hash = 0;
    piece->enabled = piece->module->default_enabled;
    dt_iop_commit_params(piece->module, piece->module->default_params, piece->module->default_blendop_params, pipe, piece);
    nodes = g_list_next(nodes);
  }
  // go through all history items and adjust params
  GList *history = dev->history;
  for(int k=0; k<dev->history_end; k++)
  {
    dt_dev_pixelpipe_synch(pipe, dev, history);
    history = g_list_next(history);
  }
  dt_pthread_mutex_unlock(&pipe->busy_mutex);
}

void dt_dev_pixelpipe_synch_top(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev)
{
  dt_pthread_mutex_lock(&pipe->busy_mutex);
  GList *history = g_list_nth(dev->history, dev->history_end - 1);
  if(history) dt_dev_pixelpipe_synch(pipe, dev, history);
  dt_pthread_mutex_unlock(&pipe->busy_mutex);
}

void dt_dev_pixelpipe_change(dt_dev_pixelpipe_t *pipe, struct dt_develop_t *dev)
{
  dt_pthread_mutex_lock(&dev->history_mutex);
  // case DT_DEV_PIPE_UNCHANGED: case DT_DEV_PIPE_ZOOMED:
  if(pipe->changed & DT_DEV_PIPE_TOP_CHANGED)
  {
    // only top history item changed.
    dt_dev_pixelpipe_synch_top(pipe, dev);
  }
  if(pipe->changed & DT_DEV_PIPE_SYNCH)
  {
    // pipeline topology remains intact, only change all params.
    dt_dev_pixelpipe_synch_all(pipe, dev);
  }
  if(pipe->changed & DT_DEV_PIPE_REMOVE)
  {
    // modules have been added in between or removed. need to rebuild the whole pipeline.
    dt_dev_pixelpipe_cleanup_nodes(pipe);
    dt_dev_pixelpipe_create_nodes(pipe, dev);
  }
  pipe->changed = DT_DEV_PIPE_UNCHANGED;
  dt_pthread_mutex_unlock(&dev->history_mutex);
  dt_dev_pixelpipe_get_dimensions(pipe, dev, pipe->iwidth, pipe->iheight, &pipe->processed_width, &pipe->processed_height);
}

// TODO:
void dt_dev_pixelpipe_add_node(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, int n)
{
}
// TODO:
void dt_dev_pixelpipe_remove_node(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, int n)
{
}

static int
get_output_bpp(dt_iop_module_t *module, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece, dt_develop_t *dev)
{
  if(!module)
  {
    // first input.
    // mipf and non-raw images have 4 floats per pixel
    if(pipe->type == DT_DEV_PIXELPIPE_PREVIEW || !(pipe->image.flags & DT_IMAGE_RAW)) return 4*sizeof(float);
    else return pipe->image.bpp;
  }
  return module->output_bpp(module, pipe, piece);
}


// recursive helper for process:
static int
dt_dev_pixelpipe_process_rec(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, void **output, void **cl_mem_output, int *out_bpp,
                             const dt_iop_roi_t *roi_out, GList *modules, GList *pieces, int pos)
{
  dt_iop_roi_t roi_in = *roi_out;

  void *input = NULL;
  void *cl_mem_input = NULL;
  *cl_mem_output = NULL;
  dt_iop_module_t *module = NULL;
  dt_dev_pixelpipe_iop_t *piece = NULL;
  if(modules)
  {
    module = (dt_iop_module_t *)modules->data;
    piece = (dt_dev_pixelpipe_iop_t *)pieces->data;
    // skip this module?
    if(!piece->enabled || (dev->gui_module && dev->gui_module->operation_tags_filter() &  module->operation_tags()))
      return dt_dev_pixelpipe_process_rec(pipe, dev, output, cl_mem_output, out_bpp, &roi_in, g_list_previous(modules), g_list_previous(pieces), pos-1);
  }

  const int bpp = get_output_bpp(module, pipe, piece, dev);
  *out_bpp = bpp;
  const size_t bufsize = bpp*roi_out->width*roi_out->height;


  // 1) if cached buffer is still available, return data
  dt_pthread_mutex_lock(&pipe->busy_mutex);
  if(pipe->shutdown)
  {
    dt_pthread_mutex_unlock(&pipe->busy_mutex);
    return 1;
  }
  uint64_t hash = dt_dev_pixelpipe_cache_hash(pipe->image.id, roi_out, pipe, pos);
  if(dt_dev_pixelpipe_cache_available(&(pipe->cache), hash))
  {
    // if(module) printf("found valid buf pos %d in cache for module %s %s %lu\n", pos, module->op, pipe == dev->preview_pipe ? "[preview]" : "", hash);
    // copy over cached processed max for clipping:
    if(piece) for(int k=0; k<3; k++) pipe->processed_maximum[k] = piece->processed_maximum[k];
    else      for(int k=0; k<3; k++) pipe->processed_maximum[k] = 1.0f;
    (void) dt_dev_pixelpipe_cache_get(&(pipe->cache), hash, bufsize, output);
    dt_pthread_mutex_unlock(&pipe->busy_mutex);
    if(!modules) return 0;
    // go to post-collect directly:
    goto post_process_collect_info;
  }
  else dt_pthread_mutex_unlock(&pipe->busy_mutex);

  // 2) if history changed or exit event, abort processing?
  // preview pipe: abort on all but zoom events (same buffer anyways)
  if(dt_iop_breakpoint(dev, pipe)) return 1;
  // if image has changed, stop now.
  if(pipe == dev->pipe && dev->image_force_reload) return 1;
  if(pipe == dev->preview_pipe && dev->preview_loading) return 1;
  if(dev->gui_leaving) return 1;


  // 3) input -> output
  if(!modules)
  {
    // 3a) import input array with given scale and roi
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    dt_times_t start;
    dt_get_times(&start);
    if(pipe->type != DT_DEV_PIXELPIPE_PREVIEW)
    {
      if(roi_out->scale == 1.0 && roi_out->x == 0 && roi_out->y == 0 && pipe->iwidth == roi_out->width && pipe->iheight == roi_out->height)
      {
        *output = pipe->input;
      }
      else if(dt_dev_pixelpipe_cache_get(&(pipe->cache), hash, bufsize, output))
      {
        memset(*output, 0, pipe->backbuf_size);
        if(roi_in.scale == 1.0f)
        {
          // fast branch for 1:1 pixel copies.
#ifdef _OPENMP
          #pragma omp parallel for schedule(static) default(none) shared(pipe,roi_out,roi_in,output)
#endif
          for(int j=0; j<MIN(roi_out->height, pipe->iheight-roi_in.y); j++)
            memcpy(((char *)*output) + bpp*j*roi_out->width, ((char *)pipe->input) + bpp*(roi_in.x + (roi_in.y + j)*pipe->iwidth), bpp*roi_out->width);
        }
        else
        {
          roi_in.x /= roi_out->scale;
          roi_in.y /= roi_out->scale;
          roi_in.width = pipe->iwidth;
          roi_in.height = pipe->iheight;
          roi_in.scale = 1.0f;
          dt_iop_clip_and_zoom(*output, pipe->input, roi_out, &roi_in, roi_out->width, pipe->iwidth);
        }
      }
      // else found in cache.
    }
    // optimized branch (for mipf-preview):
    else if(pipe->type == DT_DEV_PIXELPIPE_PREVIEW && roi_out->scale == 1.0 && roi_out->x == 0 && roi_out->y == 0 && pipe->iwidth == roi_out->width && pipe->iheight == roi_out->height) *output = pipe->input;
    else
    {
      // reserve new cache line: output
      if(dt_dev_pixelpipe_cache_get(&(pipe->cache), hash, bufsize, output))
      {
        roi_in.x /= roi_out->scale;
        roi_in.y /= roi_out->scale;
        roi_in.width = pipe->iwidth;
        roi_in.height = pipe->iheight;
        roi_in.scale = 1.0f;
        dt_iop_clip_and_zoom(*output, pipe->input, roi_out, &roi_in, roi_out->width, pipe->iwidth);
      }
    }
    dt_show_times(&start, "[dev_pixelpipe]", "initing base buffer [%s]", pipe->type == DT_DEV_PIXELPIPE_PREVIEW ? "preview" : (pipe->type == DT_DEV_PIXELPIPE_FULL ? "full" : "export"));
    dt_pthread_mutex_unlock(&pipe->busy_mutex);
  }
  else
  {
    // 3b) recurse and obtain output array in &input

    // get region of interest which is needed in input
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    module->modify_roi_in(module, piece, roi_out, &roi_in);
    dt_pthread_mutex_unlock(&pipe->busy_mutex);

    // recurse to get actual data of input buffer
    int in_bpp;
    if(dt_dev_pixelpipe_process_rec(pipe, dev, &input, &cl_mem_input, &in_bpp, &roi_in, g_list_previous(modules), g_list_previous(pieces), pos-1)) return 1;
    piece = (dt_dev_pixelpipe_iop_t *)pieces->data;

    // reserve new cache line: output
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    if(!strcmp(module->op, "gamma"))
      (void) dt_dev_pixelpipe_cache_get_important(&(pipe->cache), hash, bufsize, output);
    else
      (void) dt_dev_pixelpipe_cache_get(&(pipe->cache), hash, bufsize, output);
    dt_pthread_mutex_unlock(&pipe->busy_mutex);

    // if(module) printf("reserving new buf in cache for module %s %s: %ld buf %lX\n", module->op, pipe == dev->preview_pipe ? "[preview]" : "", hash, (long int)*output);

    // tonecurve/levels histogram (collect luminance only):
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    if(dev->gui_attached && pipe == dev->preview_pipe && (strcmp(module->op, "tonecurve") == 0 || strcmp(module->op, "levels") == 0))
    {
      float *pixel = (float *)input;
      float *histogram_pre = NULL;
      float *histogram_pre_max = NULL;

      if(!strcmp(module->op, "tonecurve"))
      {
        histogram_pre = dev->histogram_pre_tonecurve;
        histogram_pre_max = &(dev->histogram_pre_tonecurve_max);
      }
      else
      {
        histogram_pre = dev->histogram_pre_levels;
        histogram_pre_max = &(dev->histogram_pre_levels_max);
      }

      *histogram_pre_max = 0;
      memset (histogram_pre, 0, sizeof(float)*4*64);
      for(int j=0; j<roi_in.height; j+=4) for(int i=0; i<roi_in.width; i+=4)
        {
          uint8_t L = CLAMP(63/100.0*(pixel[4*j*roi_in.width+4*i]), 0, 63);
          histogram_pre[4*L+3] ++;
        }
      // don't count <= 0 pixels
      for(int k=3; k<4*64; k+=4) histogram_pre[k] = logf(1.0 +histogram_pre[k]);
      for(int k=19; k<4*64; k+=4) *histogram_pre_max = *histogram_pre_max > histogram_pre[k] ? *histogram_pre_max : histogram_pre[k];
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      if(module->widget) dt_control_queue_redraw_widget(module->widget);
    }
    else dt_pthread_mutex_unlock(&pipe->busy_mutex);

    // if module requested color statistics, get mean in box from preview pipe
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }

    // Lab color picking for module
    if(dev->gui_attached && pipe == dev->preview_pipe && // pick from preview pipe to get pixels outside the viewport
        (module == dev->gui_module || !strcmp(module->op, "colorout")) && // only modules with focus or colorout for bottom panel can pick
        module->request_color_pick) // and they need to want to pick ;)
    {
      for(int k=0; k<3; k++) module->picked_color_min[k] =  666.0f;
      for(int k=0; k<3; k++) module->picked_color_max[k] = -666.0f;
      int box[4];
      int point[2];
      float Lab[3], *in = (float *)input;
      for(int k=0; k<3; k++) Lab[k] = 0.0f;

      // Initializing bounds of colorpicker box
      for(int k=0; k<4; k+=2) box[k] = MIN(roi_in.width -1, MAX(0, module->color_picker_box[k]*roi_in.width));
      for(int k=1; k<4; k+=2) box[k] = MIN(roi_in.height-1, MAX(0, module->color_picker_box[k]*roi_in.height));

      // Initializing bounds of colorpicker point
      point[0] = MIN(roi_in.width - 1, MAX(0, module->color_picker_point[0] * roi_in.width));
      point[1] = MIN(roi_in.height - 1, MAX(0, module->color_picker_point[1] * roi_in.height));

      if(darktable.lib->proxy.colorpicker.size)
      {
        const float w = 1.0/((box[3]-box[1]+1)*(box[2]-box[0]+1));
        for(int j=box[1]; j<=box[3]; j++) for(int i=box[0]; i<=box[2]; i++)
        {
          const float L = in[4*(roi_in.width*j + i) + 0];
          const float a = in[4*(roi_in.width*j + i) + 1];
          const float b = in[4*(roi_in.width*j + i) + 2];
          Lab[0] += w*L;
          Lab[1] += w*a;
          Lab[2] += w*b;
          module->picked_color_min[0] = fminf(module->picked_color_min[0], L);
          module->picked_color_min[1] = fminf(module->picked_color_min[1], a);
          module->picked_color_min[2] = fminf(module->picked_color_min[2], b);
          module->picked_color_max[0] = fmaxf(module->picked_color_max[0], L);
          module->picked_color_max[1] = fmaxf(module->picked_color_max[1], a);
          module->picked_color_max[2] = fmaxf(module->picked_color_max[2], b);
          for(int k=0; k<3; k++) module->picked_color[k] = Lab[k];
        }
      }
      else
      {
        for(int i = 0; i < 3; i++)
          module->picked_color[i]
              = module->picked_color_min[i]
                = module->picked_color_max[i]
                  = in[4*(roi_in.width*point[1] + point[0]) + i];
      }

      dt_pthread_mutex_unlock(&pipe->busy_mutex);

      gboolean i_own_lock = dt_control_gdk_lock();      
      if(module->widget) gtk_widget_queue_draw(module->widget);
      if (i_own_lock) dt_control_gdk_unlock();
    }
    else dt_pthread_mutex_unlock(&pipe->busy_mutex);

    // actual pixel processing done by module
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    dt_times_t start;
    dt_get_times(&start);

    dt_develop_tiling_t tiling = { 0 };

    /* get tiling requirement of module */
    module->tiling_callback(module, piece, &roi_in, roi_out, &tiling);

    assert(tiling.factor > 0.0f && tiling.factor < 100.0f);      

#ifdef HAVE_OPENCL
    /* do we have opencl at all? did user tell us to use it? did we get a resource? */
    if (dt_opencl_is_inited() && pipe->opencl_enabled && pipe->devid >= 0)
    {
      int success_opencl = TRUE;

      /* if input is on gpu memory only, remember this fact to later take appropriate action */
      int valid_input_on_gpu_only = (cl_mem_input != NULL);
      
      /* general remark: in case of opencl errors within modules or out-of-memory on GPU, we transparently
         fall back to the respective cpu module and continue in pixelpipe. If we encounter late errors(*), we set 
         pipe->opencl_error=1, return this function with value 1, and leave appropriate action to the calling function,
         which normally would restart pixelpipe without opencl.
         (*) late errors are sometimes detected when trying to get back data from device into host memory */

      /* try to enter opencl path after checking some module specific pre-requisites */
      if(module->process_cl && piece->process_cl_ready)
      {

        // fprintf(stderr, "[opencl_pixelpipe 0] factor %f, overhead %d, width %d, height %d, bpp %d\n", (double)tiling.factor, tiling.overhead, roi_in.width, roi_in.height, bpp);

        // fprintf(stderr, "[opencl_pixelpipe 1] for module `%s', have bufs %lX and %lX \n", module->op, (long int)cl_mem_input, (long int)*cl_mem_output);
        // fprintf(stderr, "[opencl_pixelpipe 1] module '%s'\n", module->op);

        if(dt_opencl_image_fits_device(pipe->devid, max(roi_in.width, roi_out->width), max(roi_in.height, roi_out->height), max(in_bpp, bpp), tiling.factor, tiling.overhead))
        {
          /* image is small enough -> try to directly process entire image with opencl */

          // fprintf(stderr, "[opencl_pixelpipe 2] module '%s' running directly with process_cl\n", module->op);

          /* input is not on gpu memory -> copy it there */
          if (cl_mem_input == NULL)
          {
            cl_mem_input = dt_opencl_copy_host_to_device(pipe->devid, input, roi_in.width, roi_in.height, in_bpp);
            if (cl_mem_input == NULL)
            {
              dt_print(DT_DEBUG_OPENCL, "[opencl_pixelpipe] couldn't generate input buffer for module %s\n", module->op);
              success_opencl = FALSE;
            }
          }

          /* try to allocate GPU memory for output */
          if (success_opencl)
          {
            *cl_mem_output = dt_opencl_alloc_device(pipe->devid, roi_out->width, roi_out->height, bpp);
            if (*cl_mem_output == NULL)
            {
              dt_print(DT_DEBUG_OPENCL, "[opencl_pixelpipe] couldn't allocate output buffer for module %s\n", module->op);
              success_opencl = FALSE;
            }
          }

          // fprintf(stderr, "[opencl_pixelpipe 2] for module `%s', have bufs %lX and %lX \n", module->op, (long int)cl_mem_input, (long int)*cl_mem_output);

          /* now call process_cl of module; module should emit meaningful messages in case of error */
          if (success_opencl)
            success_opencl = module->process_cl(module, piece, cl_mem_input, *cl_mem_output, &roi_in, roi_out);


          /* process blending */
          if (success_opencl)
            success_opencl = dt_develop_blend_process_cl(module, piece, cl_mem_input, *cl_mem_output, &roi_in, roi_out);
        }
        else if(module->flags() & IOP_FLAGS_ALLOW_TILING)
        {
          /* image is too big for direct opencl processing -> try to process image via tiling */

          // fprintf(stderr, "[opencl_pixelpipe 3] module '%s' tiling with process_tiling_cl\n", module->op);

          /* we might need to copy back valid image from device to host */
          if (cl_mem_input != NULL)
          {
            cl_int err;

            /* copy back to CPU buffer, then clean unneeded buffer */
            err = dt_opencl_copy_device_to_host(pipe->devid, input, cl_mem_input, roi_in.width, roi_in.height, in_bpp);
            if (err != CL_SUCCESS)
            {
              /* late opencl error */
              dt_print(DT_DEBUG_OPENCL, "[opencl_pixelpipe (a)] late opencl error detected while copying back to cpu buffer: %d\n", err);
              dt_opencl_release_mem_object(cl_mem_input);
              pipe->opencl_error = 1;
              dt_pthread_mutex_unlock(&pipe->busy_mutex);
              return 1;
            }
            dt_opencl_release_mem_object(cl_mem_input);
            cl_mem_input = NULL;
            valid_input_on_gpu_only = FALSE;
          }

          /* now call process_tiling_cl of module; module should emit meaningful messages in case of error */
          if (success_opencl)
            success_opencl = module->process_tiling_cl(module, piece, input, *output, &roi_in, roi_out, in_bpp);

          /* do process blending on cpu (this is anyhow fast enough) */
          if (success_opencl)
            dt_develop_blend_process(module, piece, input, *output, &roi_in, roi_out);
        }
        else
        {
          /* image is too big for direct opencl and tiling is not allowed -> no opencl processing for this module */
          success_opencl = FALSE;
        }

        
        // if (rand() % 20 == 0) success_opencl = FALSE; // Test code: simulate spurious failures

        /* finally check, if we were successful */
        if (success_opencl)
        {
          /* Nice, everything went fine */
        
          /* we can now release cl_mem_input */
          if (cl_mem_input) dt_opencl_release_mem_object(cl_mem_input);
          // we speculate on the next plug-in to possibly copy back cl_mem_output to output,
          // so we're not just yet invalidating the (empty) output cache line.
        }
        else
        {
          /* Bad luck, opencl failed. Let's clean up and fall back to cpu module */
          dt_print(DT_DEBUG_OPENCL, "[opencl_pixelpipe] failed to run module '%s'. fall back to cpu path\n", module->op);

          // fprintf(stderr, "[opencl_pixelpipe 4] module '%s' running on cpu\n", module->op);

          /* we might need to free unused output buffer */
          if (*cl_mem_output != NULL)
          {
            dt_opencl_release_mem_object(*cl_mem_output);
            *cl_mem_output = NULL;
          }
          
          /* check where our input buffer is located */
          if (cl_mem_input != NULL)
          {
            cl_int err;

            /* copy back to host memory, then clean no longer needed opencl buffer.
               important info: in order to make this possible, opencl modules must
               not spoil their input buffer, even in case of errors. */
            err = dt_opencl_copy_device_to_host(pipe->devid, input, cl_mem_input, roi_in.width, roi_in.height, in_bpp);
            if (err != CL_SUCCESS)
            {
              /* late opencl error */
              dt_print(DT_DEBUG_OPENCL, "[opencl_pixelpipe (b)] late opencl error detected while copying back to cpu buffer: %d\n", err);
              dt_opencl_release_mem_object(cl_mem_input);
              pipe->opencl_error = 1;
              dt_pthread_mutex_unlock(&pipe->busy_mutex);
              return 1;
            }
            dt_opencl_release_mem_object(cl_mem_input);
            valid_input_on_gpu_only = FALSE;
          }

          /* process module on cpu. use tiling if needed and possible. */
          if((module->flags() & IOP_FLAGS_ALLOW_TILING) && 
                !dt_tiling_piece_fits_host_memory(max(roi_in.width, roi_out->width), max(roi_in.height, roi_out->height), 
                max(in_bpp, bpp), tiling.factor, tiling.overhead))
            module->process_tiling(module, piece, input, *output, &roi_in, roi_out, in_bpp);
          else
            module->process(module, piece, input, *output, &roi_in, roi_out);

          /* process blending on cpu */
          dt_develop_blend_process(module, piece, input, *output, &roi_in, roi_out);
        }
      }
      else
      {
        /* we are not allowed to use opencl for this module */
  
        // fprintf(stderr, "[opencl_pixelpipe 3] for module `%s', have bufs %lX and %lX \n", module->op, (long int)cl_mem_input, (long int)*cl_mem_output);

        *cl_mem_output = NULL;

        /* cleanup unneeded opencl buffer, and copy back to CPU buffer */
        if(cl_mem_input != NULL)
        {
          cl_int err;

          err = dt_opencl_copy_device_to_host(pipe->devid, input, cl_mem_input, roi_in.width, roi_in.height, in_bpp);
          // if (rand() % 5 == 0) err = !CL_SUCCESS; // Test code: simulate spurious failures
          if (err != CL_SUCCESS)
          {
            /* late opencl error */
            dt_print(DT_DEBUG_OPENCL, "[opencl_pixelpipe (c)] late opencl error detected while copying back to cpu buffer: %d\n", err);
            dt_opencl_release_mem_object(cl_mem_input);
            pipe->opencl_error = 1;
            dt_pthread_mutex_unlock(&pipe->busy_mutex);
            return 1;
          }
          dt_opencl_release_mem_object(cl_mem_input);
          valid_input_on_gpu_only = FALSE;
        }

        /* process module on cpu. use tiling if needed and possible. */
        if((module->flags() & IOP_FLAGS_ALLOW_TILING) && 
              !dt_tiling_piece_fits_host_memory(max(roi_in.width, roi_out->width), max(roi_in.height, roi_out->height), 
              max(in_bpp, bpp), tiling.factor, tiling.overhead))
          module->process_tiling(module, piece, input, *output, &roi_in, roi_out, in_bpp);
        else
          module->process(module, piece, input, *output, &roi_in, roi_out);

        /* process blending */
        dt_develop_blend_process(module, piece, input, *output, &roi_in, roi_out);
      }

      /* input is still only on GPU? Let's invalidate CPU input buffer then */
      if (valid_input_on_gpu_only) dt_dev_pixelpipe_cache_invalidate(&(pipe->cache), input);
    }
    else
    {
      /* opencl is not inited or not enabled or we got no resource/device -> everything runs on cpu */

      /* process module on cpu. use tiling if needed and possible. */
      if((module->flags() & IOP_FLAGS_ALLOW_TILING) && 
            !dt_tiling_piece_fits_host_memory(max(roi_in.width, roi_out->width), max(roi_in.height, roi_out->height), 
            max(in_bpp, bpp), tiling.factor, tiling.overhead))
        module->process_tiling(module, piece, input, *output, &roi_in, roi_out, in_bpp);
      else
        module->process(module, piece, input, *output, &roi_in, roi_out);

      /* process blending */
      dt_develop_blend_process(module, piece, input, *output, &roi_in, roi_out);
    } 
#else
    /* process module on cpu. use tiling if needed and possible. */
    if((module->flags() & IOP_FLAGS_ALLOW_TILING) && 
          !dt_tiling_piece_fits_host_memory(max(roi_in.width, roi_out->width), max(roi_in.height, roi_out->height), 
          max(in_bpp, bpp), tiling.factor, tiling.overhead))
      module->process_tiling(module, piece, input, *output, &roi_in, roi_out, in_bpp);
    else
      module->process(module, piece, input, *output, &roi_in, roi_out);

    /* process blending */
    dt_develop_blend_process(module, piece, input, *output, &roi_in, roi_out);
#endif

    dt_show_times(&start, "[dev_pixelpipe]", "processing `%s' [%s]", module->name(),
                  pipe->type == DT_DEV_PIXELPIPE_PREVIEW ? "preview" : (pipe->type == DT_DEV_PIXELPIPE_FULL ? "full" : "export"));
    // in case we get this buffer from the cache, also get the processed max:
    for(int k=0; k<3; k++) piece->processed_maximum[k] = pipe->processed_maximum[k];
    dt_pthread_mutex_unlock(&pipe->busy_mutex);
    if(module == darktable.develop->gui_module)
    {
      // give the input buffer to the currently focussed plugin more weight.
      // the user is likely to change that one soon, so keep it in cache.
      dt_dev_pixelpipe_cache_reweight(&(pipe->cache), input);
    }
#ifdef _DEBUG
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    if(strcmp(module->op, "gamma") && bpp == sizeof(float)*4) for(int k=0; k<4*roi_out->width*roi_out->height; k++)
      {
        if((k&3)<3 && !isfinite(((float*)(*output))[k]))
        {
          fprintf(stderr, "[dev_pixelpipe] module `%s' outputs non-finite floats!\n", module->name());
          break;
        }
      }
    dt_pthread_mutex_unlock(&pipe->busy_mutex);
#endif

post_process_collect_info:

    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    // Picking RGB for the live samples and converting to Lab
    if(dev->gui_attached
       && pipe == dev->preview_pipe
       && (strcmp(module->op, "gamma") == 0)
       && darktable.lib->proxy.colorpicker.live_samples) // samples to pick
    {
      dt_colorpicker_sample_t *sample = NULL;
      GSList *samples = darktable.lib->proxy.colorpicker.live_samples;

      while(samples)
      {
        sample = samples->data;

        if(sample->locked)
        {
          samples = g_slist_next(samples);
          continue;
        }

        uint8_t *pixel = (uint8_t*)*output;

        for(int k=0; k<3; k++) sample->picked_color_rgb_min[k] =  255;
        for(int k=0; k<3; k++) sample->picked_color_rgb_max[k] = 0;
        int box[4];
        int point[2];
        float rgb[3];
        for(int k=0; k<3; k++) rgb[k] = 0.0f;
        for(int k=0; k<4; k+=2)
          box[k] = MIN(roi_out->width -1,
                       MAX(0, sample->box[k]*roi_out->width));
        for(int k=1; k<4; k+=2)
          box[k] = MIN(roi_out->height-1,
                       MAX(0, sample->box[k]*roi_out->height));
        point[0] = MIN(roi_out->width -1,
                       MAX(0, sample->point[0] * roi_out->width));
        point[1] = MIN(roi_out->height -1,
                       MAX(0, sample->point[1] * roi_out->height));
        const float w = 1.0/((box[3]-box[1]+1)*(box[2]-box[0]+1));
        if(sample->size == DT_COLORPICKER_SIZE_BOX)
        {
          for(int j=box[1]; j<=box[3]; j++) for(int i=box[0]; i<=box[2]; i++)
          {
            for(int k=0; k<3; k++)
            {
              sample->picked_color_rgb_min[k] =
                  MIN(sample->picked_color_rgb_min[k],
                      pixel[4*(roi_out->width*j + i) + 2-k]);
              sample->picked_color_rgb_max[k] =
                  MAX(sample->picked_color_rgb_max[k],
                      pixel[4*(roi_out->width*j + i) + 2-k]);
              rgb[k] += w*pixel[4*(roi_out->width*j + i) + 2-k];
            }
          }
          for(int k=0; k<3; k++) sample->picked_color_rgb_mean[k] = rgb[k];
        }
        else
        {
          for(int i = 0; i < 3; i++)
            sample->picked_color_rgb_mean[i]
                = sample->picked_color_rgb_min[i]
                  = sample->picked_color_rgb_max[i]
                    = pixel[4*(roi_out->width*point[1] + point[0]) + 2-i];
        }

        // Converting the RGB values to Lab

        GList *nodes = pipe->nodes;
        cmsHPROFILE out_profile = NULL;
        while(nodes)
        {
          dt_dev_pixelpipe_iop_t *piece = (dt_dev_pixelpipe_iop_t*)nodes->data;
          if(!strcmp(piece->module->op, "colorout"))
          {
            out_profile = ((dt_iop_colorout_data_t*)piece->data)->output;
            break;
          }
          nodes = g_list_next(nodes);
        }
        if(out_profile)
        {
          cmsHPROFILE Lab = dt_colorspaces_create_lab_profile();

          cmsHTRANSFORM xform = cmsCreateTransform(out_profile, TYPE_RGB_FLT,
                                                  Lab, TYPE_Lab_FLT,
                                                  INTENT_PERCEPTUAL, 0);


          // Preparing the data for transformation
          float rgb_data[9];
          for(int i = 0; i < 3; i++)
          {
            rgb_data[i] =
                sample->picked_color_rgb_mean[i] / 255.0;
            rgb_data[i + 3] =
                sample->picked_color_rgb_min[i] / 255.0;
            rgb_data[i + 6] =
                sample->picked_color_rgb_max[i] / 255.0;
          }

          float Lab_data[9];
          cmsDoTransform(xform, rgb_data, Lab_data, 3);

          for(int i = 0; i < 3; i++)
          {
            sample->picked_color_lab_mean[i] =
                Lab_data[i];
            sample->picked_color_lab_min[i] =
                Lab_data[i + 3];
            sample->picked_color_lab_max[i] =
                Lab_data[i + 6];
          }

          cmsDeleteTransform(xform);
          dt_colorspaces_cleanup_profile(Lab);
        }

        samples = g_slist_next(samples);
      }
    }
    //Picking RGB for primary colorpicker output and converting to Lab
    if(dev->gui_attached
       && pipe == dev->preview_pipe
       && (strcmp(module->op, "gamma") == 0) // only gamma provides meaningful RGB data
       && dev->gui_module
       && !strcmp(dev->gui_module->op, "colorout")
       && dev->gui_module->request_color_pick
       && darktable.lib->proxy.colorpicker.picked_color_rgb_mean) // colorpicker module active
    {
      uint8_t *pixel = (uint8_t*)*output;

      for(int k=0; k<3; k++)
        darktable.lib->proxy.colorpicker.picked_color_rgb_min[k] =  255;
      for(int k=0; k<3; k++)
        darktable.lib->proxy.colorpicker.picked_color_rgb_max[k] = 0;
      int box[4];
      int point[2];
      float rgb[3];
      for(int k=0; k<3; k++) rgb[k] = 0.0f;
      for(int k=0; k<4; k+=2)
        box[k] =
            MIN(roi_out->width -1,
                MAX(0, dev->gui_module->color_picker_box[k]
                    * roi_out->width));
      for(int k=1; k<4; k+=2)
        box[k] =
            MIN(roi_out->height-1,
                MAX(0, dev->gui_module->color_picker_box[k]
                    * roi_out->height));
      point[0] = MIN(roi_out->width -1,
                     MAX(0, dev->gui_module->color_picker_point[0]
                         * roi_out->width));
      point[1] = MIN(roi_out->height -1,
                     MAX(0, dev->gui_module->color_picker_point[1]
                         * roi_out->height));
      const float w = 1.0/((box[3]-box[1]+1)*(box[2]-box[0]+1));
      if(darktable.lib->proxy.colorpicker.size)
      {
        for(int j=box[1]; j<=box[3]; j++) for(int i=box[0]; i<=box[2]; i++)
        {
          for(int k=0; k<3; k++)
          {
            darktable.lib->proxy.colorpicker.picked_color_rgb_min[k] =
                MIN(darktable.lib->proxy.colorpicker.picked_color_rgb_min[2-k],
                    pixel[4*(roi_out->width*j + i) + 2-k]);
            darktable.lib->proxy.colorpicker.picked_color_rgb_max[k] =
                MAX(darktable.lib->proxy.colorpicker.picked_color_rgb_max[k],
                    pixel[4*(roi_out->width*j + i) + 2-k]);
            rgb[k] += w*pixel[4*(roi_out->width*j + i) + 2-k];
          }
        }
        for(int k=0; k<3; k++)
          darktable.lib->proxy.colorpicker.picked_color_rgb_mean[k] = rgb[k];
      }
      else
      {
        for(int i = 0; i < 3; i++)
          darktable.lib->proxy.colorpicker.picked_color_rgb_mean[i]
              = darktable.lib->proxy.colorpicker.picked_color_rgb_min[i]
                = darktable.lib->proxy.colorpicker.picked_color_rgb_max[i]
                  = pixel[4*(roi_out->width*point[1] + point[0]) + 2-i];
      }

      // Converting the RGB values to Lab

      GList *nodes = pipe->nodes;
      cmsHPROFILE out_profile = NULL;
      while(nodes)
      {
        dt_dev_pixelpipe_iop_t *piece = (dt_dev_pixelpipe_iop_t*)nodes->data;
        if(!strcmp(piece->module->op, "colorout"))
        {
          out_profile = ((dt_iop_colorout_data_t*)piece->data)->output;
          break;
        }
        nodes = g_list_next(nodes);
      }
      if(out_profile)
      {
        cmsHPROFILE Lab = dt_colorspaces_create_lab_profile();

        cmsHTRANSFORM xform = cmsCreateTransform(out_profile, TYPE_RGB_FLT,
                                                Lab, TYPE_Lab_FLT,
                                                INTENT_PERCEPTUAL, 0);


        // Preparing the data for transformation
        float rgb_data[9];
        for(int i = 0; i < 3; i++)
        {
          rgb_data[i] =
              darktable.lib->proxy.colorpicker.picked_color_rgb_mean[i] / 255.0;
          rgb_data[i + 3] =
              darktable.lib->proxy.colorpicker.picked_color_rgb_min[i] / 255.0;
          rgb_data[i + 6] =
              darktable.lib->proxy.colorpicker.picked_color_rgb_max[i] / 255.0;
        }

        float Lab_data[9];
        cmsDoTransform(xform, rgb_data, Lab_data, 3);

        for(int i = 0; i < 3; i++)
        {
          darktable.lib->proxy.colorpicker.picked_color_lab_mean[i] =
              Lab_data[i];
          darktable.lib->proxy.colorpicker.picked_color_lab_min[i] =
              Lab_data[i + 3];
          darktable.lib->proxy.colorpicker.picked_color_lab_max[i] =
              Lab_data[i + 6];
        }

        cmsDeleteTransform(xform);
        dt_colorspaces_cleanup_profile(Lab);
      }

      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      
      gboolean i_own_lock = dt_control_gdk_lock();
      if(module->widget) gtk_widget_queue_draw(module->widget);
      if (i_own_lock) dt_control_gdk_unlock();

    }
    else dt_pthread_mutex_unlock(&pipe->busy_mutex);


    // 4) final histogram:
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    if(pipe->shutdown)
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      return 1;
    }
    if(dev->gui_attached && !dev->gui_leaving && 
       pipe == dev->preview_pipe && (strcmp(module->op, "gamma") == 0))
    {
      uint8_t *pixel = (uint8_t *)*output;
      float box[4];
      // Constraining the area if the colorpicker is active in area mode
      if(dev->gui_module
         && !strcmp(dev->gui_module->op, "colorout")
         && dev->gui_module->request_color_pick
         && darktable.lib->proxy.colorpicker.restrict_histogram)
      {
        if(darktable.lib->proxy.colorpicker.size == DT_COLORPICKER_SIZE_BOX)
        {
          for(int k=0; k<4; k+=2)
            box[k] = MIN(roi_out->width,
                         MAX(0, dev->gui_module->color_picker_box[k]
                             * roi_out->width));
          for(int k=1; k<4; k+=2)
            box[k] = MIN(roi_out->height-1,
                         MAX(0, module->color_picker_box[k]
                             * roi_out->height));
        }
        else
        {
          for(int k=0; k<4; k+=2)
            box[k] = MIN(roi_out->width,
                         MAX(0, dev->gui_module->color_picker_point[0]
                             * roi_out->width));
          for(int k=1; k<4; k+=2)
            box[k] = MIN(roi_out->height-1,
                         MAX(0, module->color_picker_point[1]
                             * roi_out->height));
        }

      }
      else
      {
        box[0] = box[1] = 0;
        box[2] = roi_out->width;
        box[3] = roi_out->height;
      }
      dev->histogram_max = 0;
      memset(dev->histogram, 0, sizeof(float)*4*64);
      for(int j=box[1]; j<=box[3]; j+=4) for(int i=box[0]; i<=box[2]; i+=4)
      {
        uint8_t rgb[3];
        for(int k=0; k<3; k++)
          rgb[k] = pixel[4*j*roi_out->width+4*i+2-k]>>2;

        for(int k=0; k<3; k++)
          dev->histogram[4*rgb[k]+k] ++;
        uint8_t lum = MAX(MAX(rgb[0], rgb[1]), rgb[2]);
        dev->histogram[4*lum+3] ++;
      }
      for(int k=0; k<4*64; k++) dev->histogram[k] = logf(1.0 + dev->histogram[k]);
      // don't count <= 0 pixels
      for(int k=19; k<4*64; k+=4) dev->histogram_max = dev->histogram_max > dev->histogram[k] ? dev->histogram_max : dev->histogram[k];
      dt_pthread_mutex_unlock(&pipe->busy_mutex);
      
      /* raise preview pipe finised signal */
      dt_control_signal_raise(darktable.signals, DT_SIGNAL_DEVELOP_PREVIEW_PIPE_FINISHED);

    }
    else 
    {
      dt_pthread_mutex_unlock(&pipe->busy_mutex);

      /* if gui attached, lets raise pipe finish signal */
      if (dev->gui_attached && !dev->gui_leaving && strcmp(module->op, "gamma") == 0)
	dt_control_signal_raise(darktable.signals, DT_SIGNAL_DEVELOP_UI_PIPE_FINISHED);
	
    }
  } 
  
  return 0;
}


int dt_dev_pixelpipe_process_no_gamma(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, int x, int y, int width, int height, float scale)
{
  // temporarily disable gamma mapping.
  GList *gammap = g_list_last(pipe->nodes);
  dt_dev_pixelpipe_iop_t *gamma = (dt_dev_pixelpipe_iop_t *)gammap->data;
  while(strcmp(gamma->module->op, "gamma"))
  {
    gamma = NULL;
    gammap = g_list_previous(gammap);
    if(!gammap) break;
    gamma = (dt_dev_pixelpipe_iop_t *)gammap->data;
  }
  if(gamma) gamma->enabled = 0;
  int ret = dt_dev_pixelpipe_process(pipe, dev, x, y, width, height, scale);
  if(gamma) gamma->enabled = 1;
  return ret;
}


static int
dt_dev_pixelpipe_process_rec_and_backcopy(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, void **output, void **cl_mem_output, int *out_bpp,
                             const dt_iop_roi_t *roi_out, GList *modules, GList *pieces, int pos)
{
#ifdef HAVE_OPENCL
  int ret = dt_dev_pixelpipe_process_rec(pipe, dev, output, cl_mem_output, out_bpp, roi_out, modules, pieces, pos);

  // copy back final opencl buffer (if any) to CPU
  dt_pthread_mutex_lock(&pipe->busy_mutex);
  if (ret)
  {
    if (*cl_mem_output != 0) dt_opencl_release_mem_object(*cl_mem_output);
    *cl_mem_output = NULL;
  }
  else
  {
    if (*cl_mem_output != NULL)
    {
      cl_int err;

      err = dt_opencl_copy_device_to_host(pipe->devid, *output, *cl_mem_output, roi_out->width, roi_out->height, *out_bpp);
      dt_opencl_release_mem_object(*cl_mem_output);
      *cl_mem_output = NULL;

      if (err != CL_SUCCESS)
      {
        /* this indicates a opencl problem earlier in the pipeline */
        dt_print(DT_DEBUG_OPENCL, "[opencl_pixelpipe (d)] late opencl error detected while copying back to cpu buffer: %d\n", err);
        pipe->opencl_error = 1;
        ret = 1;
      }
    }
  }
  dt_pthread_mutex_unlock(&pipe->busy_mutex);

  return ret;
#else
  return dt_dev_pixelpipe_process_rec(pipe, dev, output, cl_mem_output, out_bpp, roi_out, modules, pieces, pos);
#endif
}


int dt_dev_pixelpipe_process(dt_dev_pixelpipe_t *pipe, dt_develop_t *dev, int x, int y, int width, int height, float scale)
{
  pipe->processing = 1;
  pipe->opencl_enabled = dt_opencl_update_enabled(); // update enabled flag from preferences
  pipe->devid = (pipe->opencl_enabled && (pipe->type != DT_DEV_PIXELPIPE_PREVIEW)) ? dt_opencl_lock_device(-1) : -1;  // try to get/lock opencl resource but not for preview pipe

  dt_print(DT_DEBUG_OPENCL, "[pixelpipe_process] [%s] using device %d\n", pipe->type == DT_DEV_PIXELPIPE_PREVIEW ? "preview" : (pipe->type == DT_DEV_PIXELPIPE_FULL ? "full" : "export"), pipe->devid);

  if(darktable.unmuted & DT_DEBUG_MEMORY)
  {
    fprintf(stderr, "[memory] before pixelpipe process\n");
    dt_print_mem_usage();
  }

  if(pipe->devid >= 0) dt_opencl_events_reset(pipe->devid);

  dt_iop_roi_t roi = (dt_iop_roi_t)
  {
    x, y, width, height, scale
  };
  // printf("pixelpipe homebrew process start\n");
  if(darktable.unmuted & DT_DEBUG_DEV)
    dt_dev_pixelpipe_cache_print(&pipe->cache);

  //  go through list of modules from the end:
  int pos = g_list_length(dev->iop);
  GList *modules = g_list_last(dev->iop);
  GList *pieces = g_list_last(pipe->nodes);

  // re-entry point: in case of late opencl errors we start all over again with opencl-support disabled
restart:

  // image max is normalized before
  for(int k=0; k<3; k++) pipe->processed_maximum[k] = 1.0f; // dev->image->maximum;

  void *buf = NULL;
  void *cl_mem_out = NULL;
  int out_bpp;
  int oclerr = 0;

  // run pixelpipe recursively and get error status
  int err = dt_dev_pixelpipe_process_rec_and_backcopy(pipe, dev, &buf, &cl_mem_out, &out_bpp, &roi, modules, pieces, pos);

  // get status summary of opencl queue by checking the eventlist
  if(pipe->devid >= 0) oclerr = (dt_opencl_events_flush(pipe->devid, 1) != 0);

  // Check if we had opencl errors ....
  // remark: opencl errors can come in two ways: pipe->opencl_error is TRUE (and err is TRUE) OR oclerr is TRUE
  if (oclerr || (err && pipe->opencl_error))
  {
    // Well, there were error -> we might need to free an invalid opencl memory object
    if (cl_mem_out != NULL) dt_opencl_release_mem_object(cl_mem_out);
    dt_opencl_unlock_device(pipe->devid); // release opencl resource
    dt_pthread_mutex_lock(&pipe->busy_mutex);
    pipe->opencl_enabled = 0;             // disable opencl for this pipe
    pipe->opencl_error = 0;               // reset error status
    pipe->devid = -1;
    dt_pthread_mutex_unlock(&pipe->busy_mutex);
    dt_dev_pixelpipe_flush_caches(pipe);
    dt_dev_pixelpipe_change(pipe, dev);
    dt_print(DT_DEBUG_OPENCL, "[pixelpipe_process] [%s] falling back to cpu path\n", pipe->type == DT_DEV_PIXELPIPE_PREVIEW ? "preview" : (pipe->type == DT_DEV_PIXELPIPE_FULL ? "full" : "export"));
    goto restart;  // try again (this time without opencl)
  }

  // release resources:
  if(pipe->devid >= 0)
  {
    dt_opencl_unlock_device(pipe->devid);
    pipe->devid = -1;
  }
  // ... and in case of other errors ...
  if (err)
  {
    pipe->processing = 0;
    return 1;
  }

  // terminate
  dt_pthread_mutex_lock(&pipe->backbuf_mutex);
  pipe->backbuf_hash = dt_dev_pixelpipe_cache_hash(pipe->image.id, &roi, pipe, 0);
  pipe->backbuf = buf;
  pipe->backbuf_width  = width;
  pipe->backbuf_height = height;
  dt_pthread_mutex_unlock(&pipe->backbuf_mutex);

  // printf("pixelpipe homebrew process end\n");
  pipe->processing = 0;
  return 0;
}

void dt_dev_pixelpipe_flush_caches(dt_dev_pixelpipe_t *pipe)
{
  dt_dev_pixelpipe_cache_flush(&pipe->cache);
}

void dt_dev_pixelpipe_get_dimensions(dt_dev_pixelpipe_t *pipe, struct dt_develop_t *dev, int width_in, int height_in, int *width, int *height)
{
  dt_pthread_mutex_lock(&pipe->busy_mutex);
  dt_iop_roi_t roi_in = (dt_iop_roi_t)
  {
    0, 0, width_in, height_in, 1.0
  };
  dt_iop_roi_t roi_out;
  GList *modules = dev->iop;
  GList *pieces  = pipe->nodes;
  while(modules)
  {
    dt_iop_module_t *module = (dt_iop_module_t *)modules->data;
    dt_dev_pixelpipe_iop_t *piece = (dt_dev_pixelpipe_iop_t *)pieces->data;
    // skip this module?
    if(piece->enabled && !(dev->gui_module && dev->gui_module->operation_tags_filter() &  module->operation_tags()))
    {
      piece->buf_in = roi_in;
      module->modify_roi_out(module, piece, &roi_out, &roi_in);
      piece->buf_out = roi_out;
      roi_in = roi_out;
    }
    modules = g_list_next(modules);
    pieces = g_list_next(pieces);
  }
  *width  = roi_out.width;
  *height = roi_out.height;
  dt_pthread_mutex_unlock(&pipe->busy_mutex);
}

