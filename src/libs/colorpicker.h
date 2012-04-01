/*
    This file is part of darktable,
    copyright (c) 2011 Robert Bieber.

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

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <inttypes.h>
#include <gtk/gtk.h>

#define DT_COLORPICKER_SIZE_POINT 0
#define DT_COLORPICKER_SIZE_BOX 1

typedef struct dt_lib_colorpicker_t
{
  GtkWidget *output_button;
  GtkWidget *output_label;
  GtkWidget *color_mode_selector;
  GtkWidget *statistic_selector;
  GtkWidget *size_selector;
  GtkWidget *picker_button;
  GtkWidget *samples_container;
  GtkWidget *samples_mode_selector;
  GtkWidget *samples_statistic_selector;
  GtkWidget *add_sample_button;
  GtkWidget *display_samples_check_box;

} dt_lib_colorpicker_t;

/** The struct for live color picker samples */
typedef struct dt_colorpicker_sample_t
{

  /** The sample area or point */
  float point[2];
  float box[4];
  int size;
  int locked;

  /** The actual picked colors */
  uint8_t picked_color_rgb_mean[3];
  uint8_t picked_color_rgb_min[3];
  uint8_t picked_color_rgb_max[3];

  float picked_color_lab_mean[3];
  float picked_color_lab_min[3];
  float picked_color_lab_max[3];

  /** The GUI elements */
  GtkWidget *container;
  GtkWidget *output_button;
  GtkWidget *output_label;
  GtkWidget *delete_button;

} dt_colorpicker_sample_t;

#endif
