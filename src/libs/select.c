/*
    This file is part of darktable,
    copyright (c) 2009--2010 johannes hanika.
    copyright (c) 2010--2011 henrik andersson.

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
#include "common/collection.h"
#include "common/selection.h"
#include "common/debug.h"
#include "control/control.h"
#include "control/conf.h"
#include "libs/lib.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include <gdk/gdkkeysyms.h>
#include "dtgtk/button.h"

DT_MODULE(1)

const char*
name ()
{
  return _("select");
}

uint32_t views()
{
  return DT_VIEW_LIGHTTABLE;
}

uint32_t container()
{
  return DT_UI_CONTAINER_PANEL_RIGHT_CENTER;
}

typedef struct dt_lib_select_t
{
  GtkWidget
      *select_all_button, *select_none_button, *select_invert_button,
      *select_film_roll_button, *select_untouched_button;
} dt_lib_select_t;

static void
button_clicked(GtkWidget *widget, gpointer user_data)
{
  switch((long int)user_data)
  {
    case 0:  // all 
      dt_selection_select_all(darktable.selection);
      break;
    case 1: // none
      dt_selection_clear(darktable.selection);
      break;
    case 2: // invert
      dt_selection_invert(darktable.selection);
      break;
    case 4: // untouched
      dt_selection_select_unaltered(darktable.selection);
      break;
    default: // case 3: same film roll
      dt_selection_select_filmroll(darktable.selection);
  }

  dt_control_queue_redraw_center();
}

int
position ()
{
  return 800;
}

void
gui_init (dt_lib_module_t *self)
{
  dt_lib_select_t *d = (dt_lib_select_t*)malloc(sizeof(dt_lib_select_t));
  self->data = d;
  self->widget = gtk_vbox_new(TRUE, 5);
  GtkBox *hbox;
  GtkWidget *button;
  hbox = GTK_BOX(gtk_hbox_new(TRUE, 5));

  button = gtk_button_new_with_label(_("select all"));
  d->select_all_button = button;
  g_object_set(G_OBJECT(button), "tooltip-text", _("select all images in current collection (ctrl-a)"), (char *)NULL);
  gtk_box_pack_start(hbox, button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked), (gpointer)0);

  button = gtk_button_new_with_label(_("select none"));
  d->select_none_button = button;
  g_object_set(G_OBJECT(button), "tooltip-text", _("clear selection (ctrl-shift-a)"), (char *)NULL);
  gtk_box_pack_start(hbox, button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked), (gpointer)1);

  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(hbox), TRUE, TRUE, 0);
  hbox = GTK_BOX(gtk_hbox_new(TRUE, 5));

  button = gtk_button_new_with_label(_("invert selection"));
  g_object_set(G_OBJECT(button), "tooltip-text", _("select unselected images\nin current collection (ctrl-!)"), (char *)NULL);
  d->select_invert_button = button;
  gtk_box_pack_start(hbox, button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked), (gpointer)2);

  button = gtk_button_new_with_label(_("select film roll"));
  d->select_film_roll_button = button;
  g_object_set(G_OBJECT(button), "tooltip-text", _("select all images which are in the same\nfilm roll as the selected images"), (char *)NULL);
  gtk_box_pack_start(hbox, button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked), (gpointer)3);

  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(hbox), TRUE, TRUE, 0);
  hbox = GTK_BOX(gtk_hbox_new(TRUE, 5));

  button = gtk_button_new_with_label(_("select untouched"));
  d->select_untouched_button = button;
  g_object_set(G_OBJECT(button), "tooltip-text", _("select untouched images in\ncurrent collection"), (char *)NULL);
  gtk_box_pack_start(hbox, button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked), (gpointer)4);
  // Just a filler, remove if a new button is added
  gtk_box_pack_start(hbox,gtk_hbox_new(TRUE, 5),TRUE,TRUE,0);

  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(hbox), TRUE, TRUE, 0);
}

void
gui_cleanup (dt_lib_module_t *self)
{
}

void init_key_accels(dt_lib_module_t *self)
{
  dt_accel_register_lib(self, NC_("accel", "select all"),
                        GDK_a, GDK_CONTROL_MASK);
  dt_accel_register_lib(self, NC_("accel", "select none"),
                        GDK_a, GDK_CONTROL_MASK | GDK_SHIFT_MASK);
  dt_accel_register_lib(self, NC_("accel", "invert selection"),
                        GDK_i, GDK_CONTROL_MASK);
  dt_accel_register_lib(self, NC_("accel", "select film roll"), 0, 0);
  dt_accel_register_lib(self, NC_("accel", "select untouched"), 0, 0);
}

void connect_key_accels(dt_lib_module_t *self)
{
  dt_lib_select_t *d = (dt_lib_select_t*)self->data;

  dt_accel_connect_button_lib(self, "select all", d->select_all_button);
  dt_accel_connect_button_lib(self, "select none", d->select_none_button);
  dt_accel_connect_button_lib(self, "invert selection",
                              d->select_invert_button);
  dt_accel_connect_button_lib(self, "select film roll",
                              d->select_film_roll_button);
  dt_accel_connect_button_lib(self, "select untouched",
                              d->select_untouched_button);
}
