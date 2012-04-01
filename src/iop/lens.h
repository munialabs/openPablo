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
#ifndef DT_IOP_LENS_H
#define DT_IOP_LENS_H

#include "develop/imageop.h"
#include "dtgtk/slider.h"
#include <lensfun.h>
#include <gtk/gtk.h>
#include <inttypes.h>

typedef struct dt_iop_lensfun_params_t
{
  int modify_flags;
  int inverse;
  float scale;
  float crop;
  float focal;
  float aperture;
  float distance;
  lfLensType target_geom;
  char camera[52];
  char lens[52];
  int tca_override;
  float tca_r, tca_b;
}
dt_iop_lensfun_params_t;

typedef struct dt_iop_lensfun_gui_data_t
{
  const lfCamera *camera;
  GtkWidget *lens_param_box;
  GtkComboBoxEntry *cbe[3];
  GtkButton *camera_model;
  GtkMenu *camera_menu;
  GtkButton *lens_model;
  GtkMenu *lens_menu;
  GtkComboBox *target_geom;
  GtkCheckButton *reverse;
  GtkDarktableSlider *tca_r;
  GtkDarktableSlider *tca_b;
  GtkDarktableSlider *scale;
  GtkWidget *auto_scale_button;
  GtkWidget *find_lens_button;
  GtkWidget *find_camera_button;
}
dt_iop_lensfun_gui_data_t;

typedef struct dt_iop_lensfun_data_t
{
  lfLens *lens;
  float *tmpbuf;
  float *tmpbuf2;
  size_t tmpbuf_len;
  size_t tmpbuf2_len;
  int modify_flags;
  int inverse;
  float scale;
  float crop;
  float focal;
  float aperture;
  float distance;
  lfLensType target_geom;
}
dt_iop_lensfun_data_t;

void init(dt_iop_module_t *module);
void cleanup(dt_iop_module_t *module);

const char *name();
void gui_update    (struct dt_iop_module_t *self);
void commit_params (struct dt_iop_module_t *self, dt_iop_params_t *params, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece);
void init_pipe     (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece);
void reset_params  (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece);
void cleanup_pipe  (struct dt_iop_module_t *self, dt_dev_pixelpipe_t *pipe, dt_dev_pixelpipe_iop_t *piece);

void modify_roi_out(struct dt_iop_module_t *self, struct dt_dev_pixelpipe_iop_t *piece, dt_iop_roi_t *roi_out, const dt_iop_roi_t *roi_in);
void modify_roi_in(struct dt_iop_module_t *self, struct dt_dev_pixelpipe_iop_t *piece, const dt_iop_roi_t *roi_out, dt_iop_roi_t *roi_in);

void gui_init     (struct dt_iop_module_t *self);
void gui_cleanup  (struct dt_iop_module_t *self);

void process (struct dt_iop_module_t *self, dt_dev_pixelpipe_iop_t *piece, void *i, void *o, const dt_iop_roi_t *roi_in, const dt_iop_roi_t *roi_out);

#endif
