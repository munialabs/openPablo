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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <xmmintrin.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "develop/develop.h"
#include "develop/imageop.h"
#include "develop/tiling.h"
#include "control/control.h"
#include "common/opencl.h"
#include "dtgtk/slider.h"
#include "dtgtk/resetlabel.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include <gtk/gtk.h>
#include <inttypes.h>

DT_MODULE(1)

#define MAXR 12
#define BLOCKSIZE 2048		/* maximum blocksize. must be a power of 2 and will be automatically reduced if needed */

#define ROUNDUP(a, n)		((a) % (n) == 0 ? (a) : ((a) / (n) + 1) * (n))

typedef struct dt_iop_sharpen_params_t
{
  float radius, amount, threshold;
}
dt_iop_sharpen_params_t;

typedef struct dt_iop_sharpen_gui_data_t
{
  GtkVBox   *vbox;
  GtkDarktableSlider *scale1, *scale2, *scale3;
}
dt_iop_sharpen_gui_data_t;

typedef struct dt_iop_sharpen_data_t
{
  float radius, amount, threshold;
}
dt_iop_sharpen_data_t;

typedef struct dt_iop_sharpen_global_data_t
{
  int kernel_sharpen_hblur;
  int kernel_sharpen_vblur;
  int kernel_sharpen_mix;
}
dt_iop_sharpen_global_data_t;


const char *name()
{
  return C_("sharpen", "sharpen");
}


int
groups ()
{
  return IOP_GROUP_CORRECT;
}

int
flags ()
{
  return IOP_FLAGS_SUPPORTS_BLENDING | IOP_FLAGS_ALLOW_TILING;
}


void init_key_accels(dt_iop_module_so_t *self)
{
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "radius"));
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "amount"));
  dt_accel_register_slider_iop(self, FALSE, NC_("accel", "threshold"));
}

void connect_key_accels(dt_iop_module_t *self)
{
  dt_iop_sharpen_gui_data_t *g = (dt_iop_sharpen_gui_data_t*)self->gui_data;

  dt_accel_connect_slider_iop(self, "radius", GTK_WIDGET(g->scale1));
  dt_accel_connect_slider_iop(self, "amount", GTK_WIDGET(g->scale2));
  dt_accel_connect_slider_iop(self, "threshold", GTK_WIDGET(g->scale3));
}

#ifdef HAVE_OPENCL
int
process_cl (struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, cl_mem dev_in, cl_mem dev_out, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out)
{
  dt_iop_sharpen_data_t *d = (dt_iop_sharpen_data_t *)piece->data;
  dt_iop_sharpen_global_data_t *gd = (dt_iop_sharpen_global_data_t *)self->data;
  cl_mem dev_m = NULL;
  cl_int err = -999;

  const int devid = piece->pipe->devid;
  const int width = roi_in->width;
  const int height = roi_in->height;
  const int rad = MIN(MAXR, ceilf(d->radius * roi_in->scale / piece->iscale));
  const int wd = 2*rad+1;
  float mat[wd];

  if(rad == 0)
  {
    size_t origin[] = {0, 0, 0};
    size_t region[] = {width, height, 1};
    err = dt_opencl_enqueue_copy_image(devid, dev_in, dev_out, origin, origin, region);
    if (err != CL_SUCCESS) goto error;
    return TRUE;
  }

  // init gaussian kernel
  float *m = mat + rad;
  const float sigma2 = (1.0f/(2.5*2.5))*(d->radius*roi_in->scale/piece->iscale)*(d->radius*roi_in->scale/piece->iscale);
  float weight = 0.0f;
  for(int l=-rad; l<=rad; l++) weight += m[l] = expf(- (l*l)/(2.f*sigma2));
  for(int l=-rad; l<=rad; l++) m[l] /= weight;


  // prepare local work group
  size_t maxsizes[3] = { 0 };        // the maximum dimensions for a work group
  size_t workgroupsize = 0;          // the maximum number of items in a work group
  unsigned long localmemsize = 0;    // the maximum amount of local memory we can use
  
  // make sure blocksize is not too large
  int blocksize = BLOCKSIZE;
  if(dt_opencl_get_work_group_limits(devid, maxsizes, &workgroupsize, &localmemsize) == CL_SUCCESS)
  {
    // reduce blocksize step by step until it fits to limits
    while(blocksize > maxsizes[0] || blocksize > maxsizes[1] 
          || blocksize > workgroupsize || (blocksize+2*rad)*sizeof(float) > localmemsize)
    {
      if(blocksize == 1) break;
      blocksize >>= 1;    
    }
  }
  else
  {
    blocksize = 1;   // slow but safe
  }

  // width and height of intermediate buffers. Need to be multiples of BLOCKSIZE
  const size_t bwidth = width % blocksize == 0 ? width : (width / blocksize + 1)*blocksize;
  const size_t bheight = height % blocksize == 0 ? height : (height / blocksize + 1)*blocksize;

  size_t sizes[3];
  size_t local[3];

  dev_m = dt_opencl_copy_host_to_device_constant(devid, sizeof(float)*wd, mat);
  if (dev_m == NULL) goto error;

  /* horizontal blur */
  sizes[0] = bwidth;
  sizes[1] = ROUNDUP(height, 4);
  sizes[2] = 1;
  local[0] = blocksize;
  local[1] = 1;
  local[2] = 1;
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 0, sizeof(cl_mem), (void *)&dev_in);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 1, sizeof(cl_mem), (void *)&dev_out);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 2, sizeof(cl_mem), (void *)&dev_m);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 3, sizeof(int), (void *)&rad);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 4, sizeof(int), (void *)&width);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 5, sizeof(int), (void *)&height);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 6, sizeof(int), (void *)&blocksize);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_hblur, 7, (blocksize+2*rad)*sizeof(float), NULL);
  err = dt_opencl_enqueue_kernel_2d_with_local(devid, gd->kernel_sharpen_hblur, sizes, local);
  if(err != CL_SUCCESS) goto error;

  /* vertical blur */
  sizes[0] = ROUNDUP(width, 4);
  sizes[1] = bheight;
  sizes[2] = 1;
  local[0] = 1;
  local[1] = blocksize;
  local[2] = 1;
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 0, sizeof(cl_mem), (void *)&dev_out);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 1, sizeof(cl_mem), (void *)&dev_out);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 2, sizeof(cl_mem), (void *)&dev_m);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 3, sizeof(int), (void *)&rad);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 4, sizeof(int), (void *)&width);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 5, sizeof(int), (void *)&height);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 6, sizeof(int), (void *)&blocksize);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_vblur, 7, (blocksize+2*rad)*sizeof(float), NULL);
  err = dt_opencl_enqueue_kernel_2d_with_local(devid, gd->kernel_sharpen_vblur, sizes, local);
  if(err != CL_SUCCESS) goto error;

  /* mixing out and in -> out */
  sizes[0] = ROUNDUP(width, 4);
  sizes[1] = ROUNDUP(height, 4);
  sizes[2] = 1;
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_mix, 0, sizeof(cl_mem), (void *)&dev_in);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_mix, 1, sizeof(cl_mem), (void *)&dev_out);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_mix, 2, sizeof(cl_mem), (void *)&dev_out);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_mix, 3, sizeof(int), (void *)&width);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_mix, 4, sizeof(int), (void *)&height);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_mix, 5, sizeof(float), (void *)&d->amount);
  dt_opencl_set_kernel_arg(devid, gd->kernel_sharpen_mix, 6, sizeof(float), (void *)&d->threshold);
  err = dt_opencl_enqueue_kernel_2d(devid, gd->kernel_sharpen_mix, sizes);
  if(err != CL_SUCCESS) goto error;

  if (dev_m != NULL) dt_opencl_release_mem_object(dev_m);
  return TRUE;

error:
  if (dev_m != NULL) dt_opencl_release_mem_object(dev_m);
  dt_print(DT_DEBUG_OPENCL, "[opencl_sharpen] couldn't enqueue kernel! %d\n", err);
  return FALSE;
}
#endif


void tiling_callback  (struct dt_iop_module_t *self, struct dt_dev_pixelpipe_iop_t *piece, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out, struct dt_develop_tiling_t *tiling)
{
  dt_iop_sharpen_data_t *d = (dt_iop_sharpen_data_t *)piece->data;
  const int rad = MIN(MAXR, ceilf(d->radius * roi_in->scale / piece->iscale));

  tiling->factor = 2;
  tiling->overhead = 0;
  tiling->overlap = rad;
  tiling->xalign = 1;
  tiling->yalign = 1;
  return;
}

void process (struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, void *ivoid, void *ovoid, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out)
{
  dt_iop_sharpen_data_t *data = (dt_iop_sharpen_data_t *)piece->data;
  const int ch = piece->colors;
  const int rad = MIN(MAXR, ceilf(data->radius * roi_in->scale / piece->iscale));
  if(rad == 0)
  {
    memcpy(ovoid, ivoid, sizeof(float)*ch*roi_out->width*roi_out->height);
    return;
  }

  float *const tmp = dt_alloc_align(16, sizeof(float)*roi_out->width*roi_out->height);
  if (tmp == NULL)
  {
    fprintf(stderr,"[sharpen] failed to allocate temporary buffer\n");
    return;
  }

  const int wd = 2*rad+1;
  const int wd4 = (wd & 3) ? (wd >> 2) + 1 : wd >> 2;
  __attribute__((aligned(16))) float mat[wd4*4];

  memset(mat, 0, sizeof(mat));

  const float sigma2 = (1.0f/(2.5*2.5))*(data->radius*roi_in->scale/piece->iscale)*(data->radius*roi_in->scale/piece->iscale);
  float weight = 0.0f;

  // init gaussian kernel
  for(int l=-rad; l<=rad; l++)
    weight += mat[l+rad] = expf(- l*l/(2.f*sigma2));
  for(int l=0; l<wd; l++)
    mat[l] /= weight;

  // gauss blur the image horizontally
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(mat, ivoid, ovoid, roi_out, roi_in) schedule(static)
#endif
  for(int j=0; j<roi_out->height; j++)
  {
    const float *in = ((float *)ivoid) + ch*(j*roi_in->width + rad);
    float *out = tmp + j*roi_out->width + rad;
    int i;
    for(i=rad; i<roi_out->width-wd4*4+rad; i++)
    {
      const float *inp = in - ch*rad;
      __attribute__((aligned(16))) float sum[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
      __m128 msum = _mm_setzero_ps();

      for(int k=0; k < wd4*4; k+=4,inp+=4*ch)
      {
        msum = _mm_add_ps(msum,_mm_mul_ps(_mm_load_ps(mat+k),_mm_set_ps(inp[3*ch],inp[2*ch],inp[ch],inp[0])));
      }
      _mm_store_ps(sum,msum);
      *out = sum[0]+sum[1]+sum[2]+sum[3];
      out++;
      in += ch;
    }
    for(; i<roi_out->width-rad; i++)
    {
      const float *inp = in - ch*rad;
      const float *m = mat;
      float sum = 0.0f;
      for(int k=-rad; k <=rad; k++,m++,inp+=ch)
      {
        sum += *m * *inp;
      }
      *out = sum;
      out++;
      in += ch;
    }
  }
  _mm_sfence();

// gauss blur the image vertically
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(mat, ivoid, ovoid, roi_out, roi_in) schedule(static)
#endif
  for(int j=rad; j<roi_out->height-wd4*4+rad; j++)
  {
    const float *in = tmp + j*roi_in->width;
    float *out = ((float *)ovoid) + ch*j*roi_out->width;

    const int step = roi_in->width;

    __attribute__((aligned(16))) float sum[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    for(int i = 0; i<roi_out->width; i++)
    {
      const float *inp = in - step*rad;
      __m128 msum = _mm_setzero_ps();

      for(int k=0; k < wd4*4; k+=4,inp+=step*4)
      {
        msum = _mm_add_ps(msum,_mm_mul_ps(_mm_load_ps(mat+k),_mm_set_ps(inp[3*step],inp[2*step],inp[step],inp[0])));
      }
      _mm_store_ps(sum,msum);
      *out = sum[0]+sum[1]+sum[2]+sum[3];
      out += ch;
      in ++;
    }
  }
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(mat, ivoid, ovoid, roi_out, roi_in) schedule(static)
#endif
  for(int j=roi_out->height-wd4*4+rad; j<roi_out->height-rad; j++)
  {
    const float *in = tmp + j*roi_in->width;
    float *out = ((float *)ovoid) + ch*j*roi_out->width;
    const int step = roi_in->width;

    for(int i = 0; i<roi_out->width; i++)
    {
      const float *inp = in - step*rad;
      const float *m = mat;
      float sum = 0.0f;
      for(int k=-rad; k<=rad; k++,m++,inp+=step)
        sum += *m * *inp;
      *out = sum;
      out += ch;
      in ++;
    }
  }

  _mm_sfence();

  // fill unsharpened border
  for(int j=0; j<rad; j++)
    memcpy(((float*)ovoid) + ch*j*roi_out->width, ((float*)ivoid) + ch*j*roi_in->width, ch*sizeof(float)*roi_out->width);
  for(int j=roi_out->height-rad; j<roi_out->height; j++)
    memcpy(((float*)ovoid) + ch*j*roi_out->width, ((float*)ivoid) + ch*j*roi_in->width, ch*sizeof(float)*roi_out->width);

  free(tmp);

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(ivoid, ovoid, roi_out, roi_in) schedule(static)
#endif
  for(int j=rad; j<roi_out->height-rad; j++)
  {
    float *in = ((float*)ivoid) + ch*roi_out->width*j;
    float *out = ((float*)ovoid) + ch*roi_out->width*j;
    for(int i=0; i<rad; i++)
      out[ch*i] = in[ch*i];
    for(int i=roi_out->width-rad; i<roi_out->width; i++)
      out[ch*i] = in[ch*i];
  }

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(data, ivoid, ovoid, roi_out, roi_in) schedule(static)
#endif
  // subtract blurred image, if diff > thrs, add *amount to orginal image
  for(int j=0; j<roi_out->height; j++)
  {
    float *in  = (float *)ivoid + j*ch*roi_out->width;
    float *out = (float *)ovoid + j*ch*roi_out->width;

    for(int i=0; i<roi_out->width; i++)
    {
      out[1] = in[1];
      out[2] = in[2];
      const float diff = in[0] - out[0];
      if (fabsf(diff) > data->threshold)
      {
        const float detail = copysignf(fmaxf(fabsf(diff) - data->threshold, 0.0), diff);
        out[0] = fmaxf(0.0, in[0] + detail*data->amount);
      }
      else out[0] = in[0];
      out += ch;
      in += ch;
    }
  }
}
    
static void
radius_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_sharpen_params_t *p = (dt_iop_sharpen_params_t *)self->params;
  p->radius = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

static void
amount_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_sharpen_params_t *p = (dt_iop_sharpen_params_t *)self->params;
  p->amount = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

static void
threshold_callback (GtkDarktableSlider *slider, gpointer user_data)
{
  dt_iop_module_t *self = (dt_iop_module_t *)user_data;
  if(self->dt->gui->reset) return;
  dt_iop_sharpen_params_t *p = (dt_iop_sharpen_params_t *)self->params;
  p->threshold = dtgtk_slider_get_value(slider);
  dt_dev_add_history_item(darktable.develop, self, TRUE);
}

void commit_params (struct dt_iop_module_t *self, dt_iop_params_t *p1, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
  dt_iop_sharpen_params_t *p = (dt_iop_sharpen_params_t *)p1;
#ifdef HAVE_GEGL
  fprintf(stderr, "[sharpen] TODO: implement gegl version!\n");
  // pull in new params to gegl
#else
  dt_iop_sharpen_data_t *d = (dt_iop_sharpen_data_t *)piece->data;
  // actually need to increase the mask to fit 2.5 sigma inside
  d->radius = 2.5f*p->radius;
  d->amount = p->amount;
  d->threshold = p->threshold;
#endif
}

void init_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
#ifdef HAVE_GEGL
  // create part of the gegl pipeline
  piece->data = NULL;
#else
  piece->data = malloc(sizeof(dt_iop_sharpen_data_t));
  self->commit_params(self, self->default_params, pipe, piece);
#endif
}

void cleanup_pipe (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece)
{
#ifdef HAVE_GEGL
  // clean up everything again.
  (void)gegl_node_remove_child(pipe->gegl, piece->input);
  // no free necessary, no data is alloc'ed
#else
  free(piece->data);
#endif
}

void gui_update(struct dt_iop_module_t *self)
{
  dt_iop_module_t *module = (dt_iop_module_t *)self;
  dt_iop_sharpen_gui_data_t *g = (dt_iop_sharpen_gui_data_t *)self->gui_data;
  dt_iop_sharpen_params_t *p = (dt_iop_sharpen_params_t *)module->params;
  dtgtk_slider_set_value(g->scale1, p->radius);
  dtgtk_slider_set_value(g->scale2, p->amount);
  dtgtk_slider_set_value(g->scale3, p->threshold);
}

void init(dt_iop_module_t *module)
{
  // module->data = malloc(sizeof(dt_iop_sharpen_data_t));
  module->params = malloc(sizeof(dt_iop_sharpen_params_t));
  module->default_params = malloc(sizeof(dt_iop_sharpen_params_t));
  module->default_enabled = 1;
  module->priority = 686; // module order created by iop_dependencies.py, do not edit!
  module->params_size = sizeof(dt_iop_sharpen_params_t);
  module->gui_data = NULL;
  dt_iop_sharpen_params_t tmp = (dt_iop_sharpen_params_t)
  {
    2.0, 0.5, 0.5
  };
  memcpy(module->params, &tmp, sizeof(dt_iop_sharpen_params_t));
  memcpy(module->default_params, &tmp, sizeof(dt_iop_sharpen_params_t));
}

void init_global(dt_iop_module_so_t *module)
{
  const int program = 7; // sharpen.cl, from programs.conf
  dt_iop_sharpen_global_data_t *gd = (dt_iop_sharpen_global_data_t *)malloc(sizeof(dt_iop_sharpen_global_data_t));
  module->data = gd;
  gd->kernel_sharpen_hblur = dt_opencl_create_kernel(program, "sharpen_hblur");
  gd->kernel_sharpen_vblur = dt_opencl_create_kernel(program, "sharpen_vblur");
  gd->kernel_sharpen_mix = dt_opencl_create_kernel(program, "sharpen_mix");
}

void cleanup(dt_iop_module_t *module)
{
  free(module->gui_data);
  module->gui_data = NULL;
  free(module->params);
  module->params = NULL;
}

void cleanup_global(dt_iop_module_so_t *module)
{
  dt_iop_sharpen_global_data_t *gd = (dt_iop_sharpen_global_data_t *)module->data;
  dt_opencl_free_kernel(gd->kernel_sharpen_hblur);
  dt_opencl_free_kernel(gd->kernel_sharpen_vblur);
  dt_opencl_free_kernel(gd->kernel_sharpen_mix);
  free(module->data);
  module->data = NULL;
}

void gui_init(struct dt_iop_module_t *self)
{
  self->gui_data = malloc(sizeof(dt_iop_sharpen_gui_data_t));
  dt_iop_sharpen_gui_data_t *g = (dt_iop_sharpen_gui_data_t *)self->gui_data;
  dt_iop_sharpen_params_t *p = (dt_iop_sharpen_params_t *)self->params;

  self->widget = GTK_WIDGET(gtk_hbox_new(FALSE, 0));
  g->vbox = GTK_VBOX(gtk_vbox_new(FALSE, DT_GUI_IOP_MODULE_CONTROL_SPACING));
  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(g->vbox), TRUE, TRUE, 5);

  g->scale1 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR,0.0, 8.0000, 0.100, p->radius, 3));
  g_object_set (GTK_OBJECT(g->scale1), "tooltip-text", _("spatial extent of the unblurring"), (char *)NULL);
  dtgtk_slider_set_label(g->scale1,_("radius"));
  g->scale2 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR,0.0, 2.0000, 0.010, p->amount, 3));
  g_object_set (GTK_OBJECT(g->scale2), "tooltip-text", _("strength of the sharpen"), (char *)NULL);
  dtgtk_slider_set_label(g->scale2,_("amount"));
  g->scale3 = DTGTK_SLIDER(dtgtk_slider_new_with_range(DARKTABLE_SLIDER_BAR,0.0, 100.00, 0.100, p->threshold, 3));
  g_object_set (GTK_OBJECT(g->scale3), "tooltip-text", _("threshold to activate sharpen"), (char *)NULL);
  dtgtk_slider_set_label(g->scale3,_("threshold"));
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale1), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale2), TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(g->vbox), GTK_WIDGET(g->scale3), TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (g->scale1), "value-changed",
                    G_CALLBACK (radius_callback), self);
  g_signal_connect (G_OBJECT (g->scale2), "value-changed",
                    G_CALLBACK (amount_callback), self);
  g_signal_connect (G_OBJECT (g->scale3), "value-changed",
                    G_CALLBACK (threshold_callback), self);
}

void gui_cleanup(struct dt_iop_module_t *self)
{
  free(self->gui_data);
  self->gui_data = NULL;
}

#undef MAXR

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
