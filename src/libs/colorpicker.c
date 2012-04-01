/*
    This file is part of darktable,
    copyright (c) 2011-2012 Robert Bieber, Johannes Hanika, Henrik Andersson.

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
#include "control/control.h"
#include "control/conf.h"
#include "develop/develop.h"
#include "develop/imageop.h"
#include "dtgtk/label.h"
#include "dtgtk/togglebutton.h"
#include "dtgtk/button.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include "libs/lib.h"
#include "libs/colorpicker.h"

DT_MODULE(1);

const char *name()
{
  return _("color picker");
}

uint32_t views()
{
  return DT_VIEW_DARKROOM;
}

uint32_t container()
{
  return DT_UI_CONTAINER_PANEL_LEFT_CENTER;
}

int expandable()
{
  return 1;
}

int position()
{
  return 800;
}

void init_key_accels(dt_lib_module_t *self)
{
  dt_accel_register_lib(self, NC_("accel", "add sample"), 0, 0);
}

void connect_key_accels(dt_lib_module_t *self)
{
  dt_lib_colorpicker_t *d = (dt_lib_colorpicker_t*)self->data;
  dt_accel_connect_button_lib(self, "add sample", d->add_sample_button);
}

// GUI callbacks

static void _update_picker_output(dt_lib_module_t *self)
{
  GdkColor c;
  dt_lib_colorpicker_t *data = self->data;
  char colstring[512];
  dt_iop_module_t *module = get_colorout_module();
  if(module)
  {
    darktable.gui->reset = 1;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->picker_button),
                                 module->request_color_pick);
    darktable.gui->reset = 0;

    int input_color = dt_conf_get_int("ui_last/colorpicker_model");

    // always adjust picked color:
    int m = dt_conf_get_int("ui_last/colorpicker_mode");
    float fallback_col[] = {0,0,0};
    uint8_t fallback_rgb[] = {0,0,0};
    uint8_t *rgb = fallback_rgb;
    float *lab = fallback_col;
    switch(m)
    {
    case 0: // mean
      rgb = darktable.lib->proxy.colorpicker.picked_color_rgb_mean;
      lab = darktable.lib->proxy.colorpicker.picked_color_lab_mean;
      break;
    case 1: //min
      rgb = darktable.lib->proxy.colorpicker.picked_color_rgb_min;
      lab = darktable.lib->proxy.colorpicker.picked_color_lab_min;
      break;
    default:
      rgb = darktable.lib->proxy.colorpicker.picked_color_rgb_max;
      lab = darktable.lib->proxy.colorpicker.picked_color_lab_max;
      break;
    }
    switch(input_color)
    {
    case 0: // rgb
      snprintf(colstring, 512, "(%d, %d, %d)", rgb[0], rgb[1], rgb[2]);
      break;
    case 1: // Lab
      snprintf(colstring, 512, "(%.03f, %.03f, %.03f)", lab[0], lab[1], lab[2]);
      break;
    }
    gtk_label_set_label(GTK_LABEL(data->output_label), colstring);

    // Setting the button color
    c.red = rgb[0] * 65535 / 255;
    c.green = rgb[1] * 65535 / 255;
    c.blue = rgb[2] * 65535 / 255;
    gtk_widget_modify_bg(data->output_button, GTK_STATE_INSENSITIVE, &c);
  }
}

static void _picker_button_toggled (GtkToggleButton *button, gpointer p)
{
  dt_lib_colorpicker_t *data = ((dt_lib_module_t*)p)->data;
  gtk_widget_set_sensitive(GTK_WIDGET(data->add_sample_button),
                           gtk_toggle_button_get_active(button));
  if(darktable.gui->reset) return;
  dt_iop_module_t *module = get_colorout_module();
  if(module)
  {
    dt_iop_request_focus(module);
    module->request_color_pick = gtk_toggle_button_get_active(button);
    dt_dev_invalidate_from_gui(darktable.develop);
  }
  else
  {
    dt_iop_request_focus(NULL);
  }
  dt_control_queue_redraw();
}

static void _statistic_changed (GtkComboBox *widget, gpointer p)
{
  dt_conf_set_int("ui_last/colorpicker_mode", gtk_combo_box_get_active(widget));
  _update_picker_output((dt_lib_module_t*)p);
}

static void _color_mode_changed(GtkComboBox *widget, gpointer p)
{
  dt_conf_set_int("ui_last/colorpicker_model",
                  gtk_combo_box_get_active(widget));
  _update_picker_output((dt_lib_module_t*)p);
}

static void _size_changed(GtkComboBox *widget, gpointer p)
{
  dt_lib_colorpicker_t *data = ((dt_lib_module_t*)p)->data;

  dt_conf_set_int("ui_last/colorpicker_size",
                  gtk_combo_box_get_active(widget));
  darktable.lib->proxy.colorpicker.size = gtk_combo_box_get_active(widget);
  gtk_widget_set_sensitive(data->statistic_selector,
                           dt_conf_get_int("ui_last/colorpicker_size"));
  dt_dev_invalidate_from_gui(darktable.develop);
  _update_picker_output(p);
}

static void _update_samples_output(dt_lib_module_t *self)
{
  float fallback[] = {0., 0., 0.};
  uint8_t fallback_rgb[] = {0,0,0};
  uint8_t *rgb = fallback_rgb;
  float *lab = fallback;
  char text[1024];
  GSList *samples = darktable.lib->proxy.colorpicker.live_samples;
  dt_colorpicker_sample_t *sample = NULL;
  GdkColor c;

  int model = dt_conf_get_int("ui_last/colorsamples_model");
  int statistic = dt_conf_get_int("ui_last/colorsamples_mode");

  while(samples)
  {
    sample = samples->data;

    switch(statistic)
    {
    case 0:
      rgb = sample->picked_color_rgb_mean;
      lab = sample->picked_color_lab_mean;
      break;

    case 1:
      rgb = sample->picked_color_rgb_min;
      lab = sample->picked_color_lab_min;
      break;

    case 2:
      rgb = sample->picked_color_rgb_max;
      lab = sample->picked_color_lab_max;
      break;
    }

    // Setting the output button
    c.red = rgb[0] * 65535 / 255;
    c.green = rgb[1] * 65535 / 255;
    c.blue = rgb[2] * 65535 / 255;
    gtk_widget_modify_bg(sample->output_button, GTK_STATE_NORMAL, &c);
    gtk_widget_modify_bg(sample->output_button, GTK_STATE_PRELIGHT, &c);
    gtk_widget_modify_bg(sample->output_button, GTK_STATE_ACTIVE, &c);

    // Setting the output label
    switch(model)
    {
    case 0:
      // RGB
      snprintf(text, 1024, "(%d, %d, %d)", rgb[0], rgb[1], rgb[2]);
      break;

    case 1:
      // Lab
      snprintf(text, 1024, "(%.03f, %.03f, %.03f)",
               lab[0], lab[1], lab[2]);
      break;
    }
    gtk_label_set_text(GTK_LABEL(sample->output_label), text);

    samples = g_slist_next(samples);
  }
}

static void _samples_statistic_changed (GtkComboBox *widget, gpointer p)
{
  dt_conf_set_int("ui_last/colorsamples_mode",
                  gtk_combo_box_get_active(widget));
  _update_samples_output((dt_lib_module_t*)p);
}

static void _samples_mode_changed(GtkComboBox *widget, gpointer p)
{
  dt_conf_set_int("ui_last/colorsamples_model",
                  gtk_combo_box_get_active(widget));
  _update_samples_output((dt_lib_module_t*)p);
}

static gboolean _live_sample_enter(GtkWidget *widget, GdkEvent *event,
                                   gpointer sample)
{
  darktable.lib->proxy.colorpicker.selected_sample =
      (dt_colorpicker_sample_t*)sample;
  if(darktable.lib->proxy.colorpicker.display_samples)
    dt_dev_invalidate_from_gui(darktable.develop);

  return FALSE;
}

static gboolean _live_sample_leave(GtkWidget *widget, GdkEvent *event,
                                   gpointer data)
{
  darktable.lib->proxy.colorpicker.selected_sample = NULL;
  if(darktable.lib->proxy.colorpicker.display_samples)
    dt_dev_invalidate_from_gui(darktable.develop);

  return FALSE;
}

static void _remove_sample(GtkButton *widget, gpointer data)
{
  dt_colorpicker_sample_t *sample = (dt_colorpicker_sample_t*)data;
  gtk_widget_hide_all(sample->container);
  gtk_widget_destroy(sample->output_button);
  gtk_widget_destroy(sample->output_label);
  gtk_widget_destroy(sample->delete_button);
  gtk_widget_destroy(sample->container);
  darktable.lib->proxy.colorpicker.live_samples =
      g_slist_remove(darktable.lib->proxy.colorpicker.live_samples, data);
  free(sample);
  dt_dev_invalidate_from_gui(darktable.develop);
}


static gboolean _sample_lock_toggle(GtkWidget *widget, GdkEvent *event,
                                    gpointer data)
{
  dt_colorpicker_sample_t *sample = (dt_colorpicker_sample_t*)data;
  sample->locked = !sample->locked;
  dtgtk_button_set_paint(DTGTK_BUTTON(sample->output_button),
                         sample->locked ? dtgtk_cairo_paint_lock : NULL,
                         CPF_STYLE_BOX);

  return FALSE;
}

static void _add_sample(GtkButton *widget, gpointer self)
{
  dt_lib_colorpicker_t *data = ((dt_lib_module_t*)self)->data;
  dt_colorpicker_sample_t *sample =
      (dt_colorpicker_sample_t*)malloc(sizeof(dt_colorpicker_sample_t));
  darktable.lib->proxy.colorpicker.live_samples =
      g_slist_append(darktable.lib->proxy.colorpicker.live_samples, sample);
  dt_iop_module_t *module = get_colorout_module();
  int i;

  // Initializing the UI
  sample->container = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(data->samples_container), sample->container,
                     TRUE, TRUE, 0);

  sample->output_button = dtgtk_button_new(NULL, CPF_STYLE_BOX);
  gtk_widget_set_size_request(sample->output_button, 40, -1);
  gtk_widget_set_events(sample->output_button,
                        GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
  gtk_widget_set_tooltip_text(sample->output_button,
                              _("hover to highlight sample on canvas, "
                                "click to lock sample"));
  gtk_box_pack_start(GTK_BOX(sample->container), sample->output_button,
                     FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(sample->output_button), "enter-notify-event",
                   G_CALLBACK(_live_sample_enter), sample);
  g_signal_connect(G_OBJECT(sample->output_button), "leave-notify-event",
                   G_CALLBACK(_live_sample_leave), sample);
  g_signal_connect(G_OBJECT(sample->output_button), "button-press-event",
                   G_CALLBACK(_sample_lock_toggle), sample);

  sample->output_label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(sample->container), sample->output_label,
                     TRUE, TRUE, 0);

  sample->delete_button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
  gtk_box_pack_start(GTK_BOX(sample->container), sample->delete_button,
                     FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(sample->delete_button), "clicked",
                   G_CALLBACK(_remove_sample), sample);

  gtk_widget_show_all(data->samples_container);

  // Setting the actual data
  if(dt_conf_get_int("ui_last/colorpicker_size"))
  {
    sample->size = DT_COLORPICKER_SIZE_BOX;
    for(i = 0; i < 4; i++)
      sample->box[i] = module->color_picker_box[i];
  }
  else
  {
    sample->size = DT_COLORPICKER_SIZE_POINT;
    for(i = 0; i < 2; i++)
      sample->point[i] = module->color_picker_point[i];
  }

  for(i = 0; i < 3; i++)
    sample->picked_color_lab_max[i] =
        darktable.lib->proxy.colorpicker.picked_color_lab_max[i];
  for(i = 0; i < 3; i++)
    sample->picked_color_lab_mean[i] =
        darktable.lib->proxy.colorpicker.picked_color_lab_mean[i];
  for(i = 0; i < 3; i++)
    sample->picked_color_lab_min[i] =
        darktable.lib->proxy.colorpicker.picked_color_lab_min[i];
  for(i = 0; i < 3; i++)
    sample->picked_color_rgb_max[i] =
        darktable.lib->proxy.colorpicker.picked_color_rgb_max[i];
  for(i = 0; i < 3; i++)
    sample->picked_color_rgb_mean[i] =
        darktable.lib->proxy.colorpicker.picked_color_rgb_mean[i];
  for(i = 0; i < 3; i++)
    sample->picked_color_rgb_min[i] =
        darktable.lib->proxy.colorpicker.picked_color_rgb_min[i];

  sample->locked = 0;

  // Updating the display
  _update_samples_output((dt_lib_module_t*)self);
}

static void _display_samples_changed(GtkToggleButton *button, gpointer data)
{
  dt_conf_set_int("ui_last/colorpicker_display_samples",
                  gtk_toggle_button_get_active(button));
  darktable.lib->proxy.colorpicker.display_samples =
      gtk_toggle_button_get_active(button);
  dt_dev_invalidate_from_gui(darktable.develop);
}

static void _restrict_histogram_changed(GtkToggleButton *button, gpointer data)
{
  dt_conf_set_int("ui_last/colorpicker_restrict_histogram",
                  gtk_toggle_button_get_active(button));
  darktable.lib->proxy.colorpicker.restrict_histogram =
      gtk_toggle_button_get_active(button);
  dt_dev_invalidate_from_gui(darktable.develop);
}

/* set sample area proxy impl */
static void _set_sample_area(dt_lib_module_t *self, float size)
{
  dt_lib_colorpicker_t *d = (dt_lib_colorpicker_t *)self->data;
  
  if (darktable.develop->gui_module)
  {
    darktable.develop->gui_module->color_picker_box[0] = 
      darktable.develop->gui_module->color_picker_box[1] = 1.0 - size;
    darktable.develop->gui_module->color_picker_box[2] =
      darktable.develop->gui_module->color_picker_box[3] = size;
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(d->size_selector), 1);
}

static void _set_sample_point(dt_lib_module_t *self, float x, float y)
{
  dt_lib_colorpicker_t *d = (dt_lib_colorpicker_t *)self->data;
    
  if (darktable.develop->gui_module)
  {
    darktable.develop->gui_module->color_picker_point[0] = x;
    darktable.develop->gui_module->color_picker_point[1] = y;
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(d->size_selector), 0);
}

void gui_init(dt_lib_module_t *self)
{
  unsigned int i;

  GtkWidget *container = gtk_vbox_new(FALSE, 5);
  GtkWidget *output_row = gtk_hbox_new(FALSE, 2);
  GtkWidget *output_options = gtk_vbox_new(FALSE, 5);
  GtkWidget *picker_subrow = gtk_hbox_new(FALSE, 2);
  GtkWidget *restrict_button;
  GtkWidget *samples_label = dtgtk_label_new(_("live samples"),
                                             DARKTABLE_LABEL_TAB
                                             | DARKTABLE_LABEL_ALIGN_RIGHT);
  GtkWidget *samples_options_row = gtk_hbox_new(FALSE, 2);

  // Initializing self data structure
  dt_lib_colorpicker_t *data =
      (dt_lib_colorpicker_t*)malloc(sizeof(dt_lib_colorpicker_t));
  self->data = (void*)data;
  memset(data, 0, sizeof(dt_lib_colorpicker_t));

  // Initializing proxy functions and data
  darktable.lib->proxy.colorpicker.module = self;
  darktable.lib->proxy.colorpicker.size =
      dt_conf_get_int("ui_last/colorpicker_size");
  darktable.lib->proxy.colorpicker.display_samples =
      dt_conf_get_int("ui_last/colorpicker_display_samples");
  darktable.lib->proxy.colorpicker.live_samples = NULL;
  darktable.lib->proxy.colorpicker.picked_color_rgb_mean =
      (uint8_t*)malloc(sizeof(uint8_t) * 3);
  darktable.lib->proxy.colorpicker.picked_color_rgb_min =
      (uint8_t*)malloc(sizeof(uint8_t) * 3);
  darktable.lib->proxy.colorpicker.picked_color_rgb_max =
      (uint8_t*)malloc(sizeof(uint8_t) * 3);
  darktable.lib->proxy.colorpicker.picked_color_lab_mean =
      (float*)malloc(sizeof(float) * 3);
  darktable.lib->proxy.colorpicker.picked_color_lab_min =
      (float*)malloc(sizeof(float) * 3);
  darktable.lib->proxy.colorpicker.picked_color_lab_max =
      (float*)malloc(sizeof(float) * 3);
  for(i = 0; i < 3; i++)
    darktable.lib->proxy.colorpicker.picked_color_rgb_mean[i]
        = darktable.lib->proxy.colorpicker.picked_color_rgb_min[i]
          = darktable.lib->proxy.colorpicker.picked_color_rgb_max[i] = 0;
  for(i = 0; i < 3; i++)
    darktable.lib->proxy.colorpicker.picked_color_lab_mean[i]
        = darktable.lib->proxy.colorpicker.picked_color_lab_min[i]
          = darktable.lib->proxy.colorpicker.picked_color_lab_max[i] = 0;
  darktable.lib->proxy.colorpicker.update_panel =  _update_picker_output;
  darktable.lib->proxy.colorpicker.update_samples = _update_samples_output;
  darktable.lib->proxy.colorpicker.set_sample_area = _set_sample_area;
  darktable.lib->proxy.colorpicker.set_sample_point = _set_sample_point;

  // Setting up the GUI
  self->widget = container;
  gtk_box_pack_start(GTK_BOX(container), output_row, TRUE, TRUE, 0);

  // The output button
  data->output_button = dtgtk_button_new(NULL, CPF_STYLE_BOX);
  gtk_widget_set_size_request(data->output_button, 100, 100);
  gtk_widget_set_sensitive(data->output_button, FALSE);
  gtk_box_pack_start(GTK_BOX(output_row), data->output_button, FALSE, FALSE, 0);

  // The picker button, output selectors and label
  gtk_box_pack_start(GTK_BOX(output_row), output_options, TRUE, TRUE, 0);

  data->size_selector = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->size_selector),
                            _("point"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->size_selector),
                            _("area"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->size_selector),
                           dt_conf_get_int("ui_last/colorpicker_size"));
  gtk_widget_set_size_request(data->size_selector, 30, -1);
  gtk_box_pack_start(GTK_BOX(picker_subrow), data->size_selector,
                     TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(data->size_selector), "changed",
                   G_CALLBACK(_size_changed), (gpointer)self);

  data->picker_button = dtgtk_togglebutton_new(dtgtk_cairo_paint_colorpicker,
                                               CPF_STYLE_BOX);
  gtk_widget_set_size_request(data->picker_button, 50, -1);
  gtk_box_pack_start(GTK_BOX(picker_subrow), data->picker_button,
                     FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(data->picker_button), "toggled",
                   G_CALLBACK(_picker_button_toggled), self);

  gtk_box_pack_start(GTK_BOX(output_options), picker_subrow, TRUE, TRUE, 0);

  data->statistic_selector = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->statistic_selector),
                            _("mean"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->statistic_selector),
                            _("min"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->statistic_selector),
                            _("max"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->statistic_selector),
                           dt_conf_get_int("ui_last/colorpicker_mode"));
  gtk_widget_set_sensitive(data->statistic_selector,
                           dt_conf_get_int("ui_last/colorpicker_size"));
  gtk_box_pack_start(GTK_BOX(output_options), data->statistic_selector,
                     TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(data->statistic_selector), "changed",
                   G_CALLBACK(_statistic_changed), self);

  data->color_mode_selector = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->color_mode_selector),
                            _("rgb"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->color_mode_selector),
                            _("Lab"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->color_mode_selector),
                           dt_conf_get_int("ui_last/colorpicker_model"));
  gtk_box_pack_start(GTK_BOX(output_options), data->color_mode_selector,
                     TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(data->color_mode_selector), "changed",
                   G_CALLBACK(_color_mode_changed), self);

  data->output_label = gtk_label_new(_(""));
  gtk_label_set_justify(GTK_LABEL(data->output_label), GTK_JUSTIFY_CENTER);
  gtk_widget_set_size_request(data->output_label, 80, -1);
  gtk_box_pack_start(GTK_BOX(output_options), data->output_label,
                     FALSE, FALSE, 0);

  restrict_button = gtk_check_button_new_with_label(
      _("restrict histogram to selection"));
  gtk_toggle_button_set_active(
      GTK_TOGGLE_BUTTON(restrict_button),
      dt_conf_get_int("ui_last/colorpicker_restrict_histogram"));
  darktable.lib->proxy.colorpicker.restrict_histogram =
      dt_conf_get_int("ui_last/colorpicker_restrict_histogram");
  gtk_box_pack_start(GTK_BOX(container), restrict_button, TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(restrict_button), "toggled",
                   G_CALLBACK(_restrict_histogram_changed), NULL);

  // Adding the live samples section
  gtk_box_pack_start(GTK_BOX(container), samples_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(container), samples_options_row, TRUE, TRUE, 0);

  data->samples_statistic_selector = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->samples_statistic_selector),
                            _("mean"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->samples_statistic_selector),
                            _("min"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->samples_statistic_selector),
                            _("max"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->samples_statistic_selector),
                           dt_conf_get_int("ui_last/colorsamples_mode"));
  gtk_box_pack_start(GTK_BOX(samples_options_row),
                     data->samples_statistic_selector,
                     TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(data->samples_statistic_selector), "changed",
                   G_CALLBACK(_samples_statistic_changed), self);

  data->samples_mode_selector = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->samples_mode_selector),
                            _("rgb"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(data->samples_mode_selector),
                            _("Lab"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->samples_mode_selector),
                           dt_conf_get_int("ui_last/colorsamples_model"));
  gtk_box_pack_start(GTK_BOX(samples_options_row), data->samples_mode_selector,
                     TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(data->samples_mode_selector), "changed",
                   G_CALLBACK(_samples_mode_changed), self);

  data->add_sample_button = gtk_button_new_from_stock(GTK_STOCK_ADD);
  gtk_widget_set_sensitive(data->add_sample_button, FALSE);
  gtk_box_pack_start(GTK_BOX(samples_options_row), data->add_sample_button,
                     FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(data->add_sample_button), "clicked",
                   G_CALLBACK(_add_sample), self);

  data->samples_container = gtk_vbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(container), data->samples_container,
                     TRUE, TRUE, 0);

  data->display_samples_check_box =
      gtk_check_button_new_with_label(_("display sample areas on image"));
  gtk_toggle_button_set_active(
      GTK_TOGGLE_BUTTON(data->display_samples_check_box),
      dt_conf_get_int("ui_last/colorpicker_display_samples"));
  gtk_box_pack_start(GTK_BOX(container), data->display_samples_check_box,
                     TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(data->display_samples_check_box), "toggled",
                   G_CALLBACK(_display_samples_changed), NULL);

}

void gui_cleanup(dt_lib_module_t *self)
{
  // Clearing proxy functions
  darktable.lib->proxy.colorpicker.module = NULL;
  darktable.lib->proxy.colorpicker.update_panel = NULL;
  darktable.lib->proxy.colorpicker.update_samples = NULL;

  darktable.lib->proxy.colorpicker.set_sample_area = NULL;

  free(darktable.lib->proxy.colorpicker.picked_color_rgb_mean);
  free(darktable.lib->proxy.colorpicker.picked_color_rgb_min);
  free(darktable.lib->proxy.colorpicker.picked_color_rgb_max);
  free(darktable.lib->proxy.colorpicker.picked_color_lab_mean);
  free(darktable.lib->proxy.colorpicker.picked_color_lab_min);
  free(darktable.lib->proxy.colorpicker.picked_color_lab_max);
  darktable.lib->proxy.colorpicker.picked_color_rgb_mean
      = darktable.lib->proxy.colorpicker.picked_color_rgb_min
        = darktable.lib->proxy.colorpicker.picked_color_rgb_max
          = NULL;
  darktable.lib->proxy.colorpicker.picked_color_lab_mean
      = darktable.lib->proxy.colorpicker.picked_color_lab_min
        = darktable.lib->proxy.colorpicker.picked_color_lab_max
          = NULL;


  while(darktable.lib->proxy.colorpicker.live_samples)
    _remove_sample(NULL, darktable.lib->proxy.colorpicker.live_samples->data);

  free(self->data);
  self->data = NULL;
}

void gui_reset(dt_lib_module_t *self)
{
  dt_lib_colorpicker_t *data = self->data;

  int i;

  // First turning off any active picking
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->picker_button), FALSE);

  // Resetting the picked colors
  for(i = 0; i < 3; i++)
  {
    darktable.lib->proxy.colorpicker.picked_color_rgb_mean[i]
        = darktable.lib->proxy.colorpicker.picked_color_rgb_min[i]
          = darktable.lib->proxy.colorpicker.picked_color_rgb_max[i]
            = 0;

    darktable.lib->proxy.colorpicker.picked_color_lab_mean[i]
        = darktable.lib->proxy.colorpicker.picked_color_lab_min[i]
          = darktable.lib->proxy.colorpicker.picked_color_lab_max[i]
            = 0;
  }

  _update_picker_output(self);

  // Removing any live samples
  while(darktable.lib->proxy.colorpicker.live_samples)
    _remove_sample(NULL, darktable.lib->proxy.colorpicker.live_samples->data);

  // Resetting GUI elements
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->size_selector), 0);
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->statistic_selector), 0);
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->color_mode_selector), 0);
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->samples_mode_selector), 0);
  gtk_combo_box_set_active(GTK_COMBO_BOX(data->samples_statistic_selector), 0);
  gtk_toggle_button_set_active(
      GTK_TOGGLE_BUTTON(data->display_samples_check_box), FALSE);
}
