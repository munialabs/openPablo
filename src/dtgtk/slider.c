/*
    This file is part of darktable,
    copyright (c) 2010-2011 Henrik Andersson.

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

#include <control/control.h>
#include <common/darktable.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include "paint.h"
#include "slider.h"
#include "gui/gtk.h"

#define DTGTK_SLIDER_LABEL_KEY  "dtgtk_slider_label"
#define DTGTK_SLIDER_VALUE_UNIT_KEY  "dtgtk_slider_value_unit"

#define DTGTK_SLIDER_CONTROL_MIN_HEIGHT	22
#define DTGTK_SLIDER_ADJUST_BUTTON_WIDTH 10
#define DTGTK_SLIDER_BORDER_WIDTH 1
#define DTGTK_VALUE_SENSITIVITY 5.0
#define DTGTK_SLIDER_SENSIBILITY_KEY GDK_Control_L
// Delay before emitting value change while draggin value.. (prevent hogging hostapp)
#define DTGTK_SLIDER_VALUE_CHANGED_DELAY 250

static GtkEventBoxClass* _slider_parent_class;
void _slider_get_value_area(GtkWidget *widget,GdkRectangle *rect);
gdouble _slider_translate_value_to_pos(GtkAdjustment *adj,GdkRectangle *value_area,gdouble value);
gdouble _slider_translate_pos_to_value(GtkAdjustment *adj,GdkRectangle *value_area,gint x);

static void _slider_entry_abort(GtkDarktableSlider *slider);

static void _slider_class_init(GtkDarktableSliderClass *klass);
static void _slider_init(GtkDarktableSlider *scale);
static void _slider_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void _slider_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void _slider_realize(GtkWidget *widget);
static gboolean _slider_expose(GtkWidget *widget, GdkEventExpose *event);
//static void _slider_destroy(GtkObject *object);

// Slider Events
static gboolean _slider_button_press(GtkWidget *widget, GdkEventButton *event);
static gboolean _slider_button_release(GtkWidget *widget, GdkEventButton *event);
static gboolean _slider_scroll_event(GtkWidget *widget, GdkEventScroll *event);
static gboolean _slider_motion_notify(GtkWidget *widget, GdkEventMotion *event);
static gboolean _slider_enter_notify_event(GtkWidget *widget, GdkEventCrossing *event);

// Slider entry events
static gboolean _slider_entry_key_event(GtkWidget *widget,GdkEventKey *event, gpointer data);

static guint _slider_signals[SLIDER_LAST_SIGNAL] = { 0 };

void _slider_get_value_area(GtkWidget *widget,GdkRectangle *rect)
{
  rect->x = DTGTK_SLIDER_BORDER_WIDTH;
  rect->y = DTGTK_SLIDER_BORDER_WIDTH;
  rect->width=widget->allocation.width-DTGTK_SLIDER_ADJUST_BUTTON_WIDTH-DTGTK_SLIDER_BORDER_WIDTH-rect->x;
  rect->height= widget->allocation.height-(DTGTK_SLIDER_BORDER_WIDTH*2);
}

gdouble _slider_translate_pos_to_value(GtkAdjustment *adj,GdkRectangle *value_area,gint x)
{
  double value=0;
  double normrange = gtk_adjustment_get_upper(adj)-gtk_adjustment_get_lower(adj);
  gint barwidth=value_area->width;
  if( x > 0)
    value = (((double)x/(double)barwidth)*normrange);
  value+=gtk_adjustment_get_lower(adj);
  return value;
}

static void _slider_class_init (GtkDarktableSliderClass *klass)
{
  GtkWidgetClass *widget_class=(GtkWidgetClass *) klass;
  //GtkObjectClass *object_class=(GtkObjectClass *) klass;
  _slider_parent_class = gtk_type_class (gtk_event_box_get_type ());

  widget_class->realize = _slider_realize;
  widget_class->size_request = _slider_size_request;
  widget_class->size_allocate = _slider_size_allocate;
  widget_class->expose_event = _slider_expose;
  widget_class->button_press_event = _slider_button_press;
  widget_class->button_release_event = _slider_button_release;
  widget_class->scroll_event = _slider_scroll_event;
  widget_class->motion_notify_event = _slider_motion_notify;
  widget_class->enter_notify_event = _slider_enter_notify_event;
  widget_class->leave_notify_event = _slider_enter_notify_event;
  //object_class->destroy = _slider_destroy;
  _slider_signals[VALUE_CHANGED] = g_signal_new(
                              "value-changed",
                              G_TYPE_FROM_CLASS(klass), 
			      G_SIGNAL_RUN_LAST,
                              0,NULL,NULL,
                              g_cclosure_marshal_VOID__VOID,
                              GTK_TYPE_NONE,0);
}


static void _slider_init (GtkDarktableSlider *slider)
{
  slider->is_dragging=FALSE;
  slider->is_sensibility_key_pressed=FALSE;
  slider->entry=gtk_entry_new();

  gtk_widget_add_events (GTK_WIDGET (slider),
                         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                         GDK_ENTER_NOTIFY_MASK |  GDK_LEAVE_NOTIFY_MASK |
                         GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
                         GDK_POINTER_MOTION_MASK);

  GtkWidget *hbox=gtk_hbox_new(TRUE,0);
  slider->hbox = GTK_HBOX(hbox);

  GtkWidget *alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, DTGTK_SLIDER_BORDER_WIDTH*2, DTGTK_SLIDER_ADJUST_BUTTON_WIDTH+DTGTK_SLIDER_BORDER_WIDTH*2);
  gtk_container_add(GTK_CONTAINER(alignment), slider->entry);
  gtk_box_pack_start(GTK_BOX(hbox),alignment,TRUE,TRUE,0);

  gtk_container_add(GTK_CONTAINER(slider),hbox);

  gtk_entry_set_has_frame (GTK_ENTRY(slider->entry), FALSE);
  gtk_entry_set_alignment (GTK_ENTRY(slider->entry), 1.0);
  g_signal_connect (G_OBJECT (slider->entry), "key-press-event", G_CALLBACK(_slider_entry_key_event), (gpointer)slider);
  dt_gui_key_accel_block_on_focus (slider->entry);
}

#if 0 // Deprecated the delayed value thingy
static gboolean _slider_postponed_value_change(gpointer data)
{
  gboolean i_own_lock = dt_control_gdk_lock();

  if (DTGTK_SLIDER(data)->is_changed==TRUE)
  {
    g_signal_emit_by_name(G_OBJECT(data),"value-changed");
    if(DTGTK_SLIDER(data)->type==DARKTABLE_SLIDER_VALUE)
      DTGTK_SLIDER(data)->is_changed=FALSE;
  }

  if (i_own_lock) dt_control_gdk_unlock();

  return DTGTK_SLIDER(data)->is_dragging;	// This is called by the gtk mainloop and is threadsafe
}

#endif

static gboolean _slider_button_press(GtkWidget *widget, GdkEventButton *event)
{
  GtkDarktableSlider *slider=DTGTK_SLIDER(widget);
  if( event->button==3)
  {
    /* right mouse button, bring up the in place edit*/
    char sv[32]= {0};
    slider->is_entry_active=TRUE;
    gdouble value = gtk_adjustment_get_value(slider->adjustment);
    sprintf(sv,"%.*f",slider->digits,value);
    gtk_entry_set_text (GTK_ENTRY(slider->entry),sv);
    gtk_widget_show (GTK_WIDGET(slider->entry));
    gtk_widget_grab_focus (GTK_WIDGET(slider->entry));
    gtk_widget_queue_draw (widget);
  }
  else if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
  {
    /* handle single button press */
    if (event->x > (widget->allocation.width - DTGTK_SLIDER_ADJUST_BUTTON_WIDTH - DTGTK_SLIDER_BORDER_WIDTH))
    {
      /* press event in arrow up/down area of slider control*/
      float value = gtk_adjustment_get_value(slider->adjustment);
      if (event->y > (widget->allocation.height/2.0))
        value -= gtk_adjustment_get_step_increment(slider->adjustment);
      else
        value += gtk_adjustment_get_step_increment(slider->adjustment);

      if(slider->snapsize) value = slider->snapsize * (((int)value)/slider->snapsize);

      gtk_adjustment_set_value(slider->adjustment, value);
      gtk_widget_draw(widget,NULL);
      g_signal_emit_by_name(G_OBJECT(widget),"value-changed");
    }
    else
    {
      slider->is_dragging=TRUE;
      slider->prev_x_root=event->x_root;
      if( slider->type==DARKTABLE_SLIDER_BAR) slider->is_changed=TRUE;
#if 0 // Deprecate
      g_timeout_add(DTGTK_SLIDER_VALUE_CHANGED_DELAY, _slider_postponed_value_change, widget);
#endif
    }
  }
  else if(event->button == 1 && event->type == GDK_2BUTTON_PRESS)
  {
    if(event->x < slider->labelwidth && event->y < slider->labelheight)
    {
      /* left mouse second click of doubleclick event */
      slider->is_dragging=FALSE; // otherwise button_release will overwrite our changes
      gtk_adjustment_set_value(slider->adjustment, slider->default_value);
      gtk_widget_draw(widget,NULL);
      g_signal_emit_by_name(G_OBJECT(widget),"value-changed");
    }
  }
  return TRUE;
}

static gboolean _slider_button_release(GtkWidget *widget, GdkEventButton *event)
{
  GtkDarktableSlider *slider=DTGTK_SLIDER(widget);

  if( event->button==1 )
  {
    /* if x is in slider bar */
    if (event->x < (widget->allocation.width - DTGTK_SLIDER_ADJUST_BUTTON_WIDTH - DTGTK_SLIDER_BORDER_WIDTH) && event->x >= 0)
    {
      if( slider->type == DARKTABLE_SLIDER_BAR && !slider->is_sensibility_key_pressed && slider->is_dragging)
      {
        // First get some dimension info
        GdkRectangle vr;
        _slider_get_value_area(widget,&vr);

        // Adjust rect to match dimensions for bar
        vr.x+=DTGTK_SLIDER_BORDER_WIDTH*2;
        vr.width-=(DTGTK_SLIDER_BORDER_WIDTH*4);
        gint vmx = event->x-vr.x;
        if( vmx >= 0 && vmx <= vr.width)
        {
          float value = _slider_translate_pos_to_value(slider->adjustment, &vr, vmx);
          if(slider->snapsize) value = slider->snapsize * (((int)value)/slider->snapsize);
          gtk_adjustment_set_value(slider->adjustment, value);
        }
        gtk_widget_draw(widget,NULL);
        slider->prev_x_root = event->x_root;
      }
    }
    slider->is_dragging=FALSE;
    g_signal_emit_by_name(G_OBJECT(widget),"value-changed");
  }
  return TRUE;
}

static gboolean _slider_scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
  double inc=gtk_adjustment_get_step_increment(DTGTK_SLIDER(widget)->adjustment);
  DTGTK_SLIDER(widget)->is_sensibility_key_pressed = (event->state&GDK_CONTROL_MASK)?TRUE:FALSE;

  inc*= (DTGTK_SLIDER(widget)->is_sensibility_key_pressed==TRUE) ? 1.0 : DTGTK_VALUE_SENSITIVITY;
  float value = gtk_adjustment_get_value( DTGTK_SLIDER(widget)->adjustment ) + ((event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_RIGHT)?inc:-inc);
  if(DTGTK_SLIDER(widget)->snapsize) value = DTGTK_SLIDER(widget)->snapsize * (((int)value)/DTGTK_SLIDER(widget)->snapsize);
  gtk_adjustment_set_value(DTGTK_SLIDER(widget)->adjustment, value);
  gtk_widget_draw( widget, NULL);
  g_signal_emit_by_name(G_OBJECT(widget),"value-changed");
  return TRUE;
}

static void _slider_entry_commit(GtkDarktableSlider *slider)
{
  gtk_widget_hide( slider->entry );
  gdouble value=atof(gtk_entry_get_text(GTK_ENTRY(slider->entry)));
  slider->is_entry_active=FALSE;
  dtgtk_slider_set_value(slider,value);
  gtk_widget_queue_draw(GTK_WIDGET(slider));
}

static void _slider_entry_abort(GtkDarktableSlider *slider)
{
  gtk_widget_hide( slider->entry );
  slider->is_entry_active=FALSE;
  gtk_widget_queue_draw(GTK_WIDGET(slider));
}


static gboolean _slider_entry_key_event(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
  if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
    _slider_entry_commit(DTGTK_SLIDER(data));
  if (event->keyval==GDK_Escape || event->keyval==GDK_Tab)
    _slider_entry_abort(DTGTK_SLIDER(data));
  else if( // Masking allowed keys...
    event->keyval == GDK_minus || event->keyval == GDK_KP_Subtract ||
    event->keyval == GDK_plus || event->keyval == GDK_KP_Add ||
    event->keyval == GDK_period || event->keyval == GDK_KP_Decimal ||
    event->keyval == GDK_Left  ||
    event->keyval == GDK_Right  ||
    event->keyval == GDK_Delete  ||
    event->keyval == GDK_BackSpace  ||
    event->keyval == GDK_0  || event->keyval == GDK_KP_0  ||
    event->keyval == GDK_1  || event->keyval == GDK_KP_1  ||
    event->keyval == GDK_2  || event->keyval == GDK_KP_2  ||
    event->keyval == GDK_3  || event->keyval == GDK_KP_3  ||
    event->keyval == GDK_4  || event->keyval == GDK_KP_4  ||
    event->keyval == GDK_5  || event->keyval == GDK_KP_5  ||
    event->keyval == GDK_6  || event->keyval == GDK_KP_6  ||
    event->keyval == GDK_7  || event->keyval == GDK_KP_7  ||
    event->keyval == GDK_8  || event->keyval == GDK_KP_8  ||
    event->keyval == GDK_9  || event->keyval == GDK_KP_9
  )
  {
    return FALSE;
  }
  // Prevent all other keys within entry
  return TRUE;
}

static gboolean _slider_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
  GtkDarktableSlider *slider=DTGTK_SLIDER(widget);
  slider->is_sensibility_key_pressed = (event->state&GDK_CONTROL_MASK)?TRUE:FALSE;
  if( slider->is_dragging==TRUE )
  {
    // First get some dimension info
    GdkRectangle vr;
    _slider_get_value_area(widget,&vr);

    if( (slider->prev_x_root < (gint)event->x_root) )
      slider->motion_direction=1;
    else if( slider->prev_x_root > (gint)event->x_root )
      slider->motion_direction=-1;

    // Adjust rect to match dimensions for bar
    vr.x+=DTGTK_SLIDER_BORDER_WIDTH*2;
    vr.width-=(DTGTK_SLIDER_BORDER_WIDTH*4);
    gint vmx = event->x-vr.x;

    if( slider->type==DARKTABLE_SLIDER_VALUE || ( slider->type==DARKTABLE_SLIDER_BAR && slider->is_sensibility_key_pressed==TRUE ) )
    {
      gdouble inc = gtk_adjustment_get_step_increment( slider->adjustment );
      if(DARKTABLE_SLIDER_VALUE &&  !slider->is_sensibility_key_pressed) inc*=DTGTK_VALUE_SENSITIVITY;
      gdouble value = gtk_adjustment_get_value(slider->adjustment) + ( ((slider->prev_x_root <= (gint)event->x_root) && slider->motion_direction==1 )?(inc):-(inc));
      if(slider->snapsize) value = slider->snapsize * (((int)value)/slider->snapsize);
      gtk_adjustment_set_value(slider->adjustment, value);
      slider->is_changed=TRUE;
    }
    else if( slider->type==DARKTABLE_SLIDER_BAR )
    {
      if( vmx >= 0 && vmx <= vr.width)
      {
        float value = _slider_translate_pos_to_value(slider->adjustment, &vr, vmx);
        if(slider->snapsize) value = slider->snapsize * (((int)value)/slider->snapsize);
        gtk_adjustment_set_value(slider->adjustment, value);
      }
    }

    g_signal_emit_by_name(G_OBJECT(widget),"value-changed");

    gtk_widget_draw(widget,NULL);
    slider->prev_x_root = event->x_root;
  }
  return FALSE;
}

static gboolean _slider_enter_notify_event(GtkWidget *widget, GdkEventCrossing *event)
{
  gtk_widget_set_state(widget,(event->type == GDK_ENTER_NOTIFY)?GTK_STATE_PRELIGHT:GTK_STATE_NORMAL);
  gtk_widget_draw(widget,NULL);
  DTGTK_SLIDER(widget)->prev_x_root=event->x_root;
  return FALSE;
}

static void  _slider_size_request(GtkWidget *widget,GtkRequisition *requisition)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(DTGTK_IS_SLIDER(widget));
  g_return_if_fail(requisition != NULL);

  GTK_WIDGET_CLASS(_slider_parent_class)->size_request(widget, requisition);

  requisition->width = 100;
  requisition->height = DTGTK_SLIDER_CONTROL_MIN_HEIGHT;
}

static void _slider_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(DTGTK_IS_SLIDER(widget));
  g_return_if_fail(allocation != NULL);

  widget->allocation = *allocation;
  GTK_WIDGET_CLASS(_slider_parent_class)->size_allocate(widget, allocation);

  if (GTK_WIDGET_REALIZED(widget))
  {
    gdk_window_move_resize(
      widget->window,
      allocation->x, allocation->y,
      allocation->width, allocation->height
    );

    if(DTGTK_SLIDER(widget)->is_entry_active == FALSE)
      gtk_widget_hide(DTGTK_SLIDER (widget)->entry);
  }
}

static void _slider_realize(GtkWidget *widget)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(DTGTK_IS_SLIDER(widget));

  GdkWindowAttr attributes;
  guint attributes_mask;
  GtkWidgetClass* klass = GTK_WIDGET_CLASS (_slider_parent_class);

  if (klass->realize)
    klass->realize (widget);

  GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = 100;
  attributes.height = DTGTK_SLIDER_CONTROL_MIN_HEIGHT;

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  widget->window = gdk_window_new( gtk_widget_get_parent_window (widget->parent),& attributes, attributes_mask);
  gdk_window_set_user_data(widget->window, widget);
  widget->style = gtk_style_attach(widget->style, widget->window);
  gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

}

// helper function to render a filled rectangle with rounded corners, type defined if its ctrl background or value
static void _slider_draw_rounded_rect(cairo_t *cr,gfloat x,gfloat y,gfloat width,gfloat height,gfloat radius,int type)
{
  double degrees = M_PI / 180.0;
  cairo_new_sub_path (cr);
  if(type==1)
  {
    cairo_move_to(cr,x+width,y);
    cairo_line_to(cr,x+width,y+height);
  }
  else
  {
    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
  }
  cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
  cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
  cairo_close_path (cr);
  cairo_fill (cr);
}

static gboolean _slider_expose(GtkWidget *widget, GdkEventExpose *event)
{
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(DTGTK_IS_SLIDER(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  if (widget->allocation.width<=1) return FALSE;

  GtkStyle *style=gtk_rc_get_style_by_paths(gtk_settings_get_default(), NULL,"GtkButton", GTK_TYPE_BUTTON);
  if(!style) style = gtk_rc_get_style(widget);
  GtkDarktableSlider *slider=DTGTK_SLIDER(widget);
  int state = gtk_widget_get_state(widget);
  int width = widget->allocation.width;
  int height = widget->allocation.height;

  /* get value fill rectangle constraints*/
  GdkRectangle vr;
  _slider_get_value_area(widget,&vr);

  /* create cairo context */
  cairo_t *cr;
  cr = gdk_cairo_create(widget->window);

  /* hardcode state for the rest of control */
  state = GTK_STATE_NORMAL;

  /* fill value rect */
  gfloat value = gtk_adjustment_get_value(slider->adjustment);

  /* convert scale value to range 0.0-1.0*/
  gfloat vscale = (value - gtk_adjustment_get_lower(slider->adjustment)) /
                  (gtk_adjustment_get_upper(slider->adjustment) -
                   gtk_adjustment_get_lower(slider->adjustment)
                  );


  cairo_set_source_rgba(cr,
                        (style->fg[state].red/65535.0)*1.7,
                        (style->fg[state].green/65535.0)*1.7,
                        (style->fg[state].blue/65535.0)*1.7,
                        0.2
                       );

  _slider_draw_rounded_rect(cr,vr.x,vr.y,vr.width*vscale,vr.height,3,1);

  /* setup font using family from style */
  const gchar *font_family = pango_font_description_get_family(style->font_desc);

  cairo_select_font_face (cr, font_family, CAIRO_FONT_SLANT_NORMAL,
                          CAIRO_FONT_WEIGHT_NORMAL);
  cairo_text_extents_t ext;

  /* Draw label */
  cairo_set_source_rgba(cr,
                        (style->text[state].red/65535.0)*1.7,
                        (style->text[state].green/65535.0)*1.7,
                        (style->text[state].blue/65535.0)*1.7,
                        0.8);

  gchar *label = (gchar *)g_object_get_data(G_OBJECT(widget),DTGTK_SLIDER_LABEL_KEY);
  if (label)
  {
    cairo_set_font_size(cr,vr.height*0.5);
    cairo_text_extents(cr, "j`", &ext);
    cairo_move_to(cr, vr.x+(DTGTK_SLIDER_BORDER_WIDTH*2),vr.y+ext.height);
    cairo_show_text(cr, label);
    /* store the size of the label. doing it in dtgtk_slider_set_label doesn't work as the widget isn't created yet. */
    if(slider->labelwidth == 0 && slider->labelheight == 0)
    {
      cairo_text_extents(cr, label, &ext);
      slider->labelwidth  = vr.x+(DTGTK_SLIDER_BORDER_WIDTH*2)+ext.width+(DTGTK_SLIDER_BORDER_WIDTH*2);
      slider->labelheight = vr.y+ext.height+(DTGTK_SLIDER_BORDER_WIDTH*2);
    }
  }

  /* Draw value unit */
  gchar *unit = (gchar *)g_object_get_data (G_OBJECT(slider),DTGTK_SLIDER_VALUE_UNIT_KEY);
  cairo_set_font_size(cr,vr.height*0.45);
  cairo_text_extents(cr, "%%", &ext);
  int unitwidth = ext.width;
  if(unit)
  {
    cairo_move_to(cr, vr.x+vr.width-ext.width - (DTGTK_SLIDER_BORDER_WIDTH) ,vr.y+vr.height-(DTGTK_SLIDER_BORDER_WIDTH*2));
    cairo_show_text(cr, unit);
  }


  /* Draw text value */
  cairo_select_font_face (cr, font_family, CAIRO_FONT_SLANT_NORMAL,
                          CAIRO_FONT_WEIGHT_BOLD);
  char sv[32]= {0};

  if (slider->force_sign)
    sprintf(sv,"%+.*f",slider->digits,value);
  else
    sprintf(sv,"%.*f",slider->digits,value);

  cairo_set_font_size(cr,vr.height*0.5);
  cairo_text_extents(cr, sv, &ext);
  cairo_move_to(cr, vr.x+vr.width-ext.width - unitwidth -(DTGTK_SLIDER_BORDER_WIDTH*3) ,vr.y+vr.height-(DTGTK_SLIDER_BORDER_WIDTH*2));
  cairo_show_text(cr, sv);


  /* draw up/down arrows */
  dtgtk_cairo_paint_arrow(cr,
                          width-DTGTK_SLIDER_ADJUST_BUTTON_WIDTH-DTGTK_SLIDER_BORDER_WIDTH, DTGTK_SLIDER_BORDER_WIDTH*2,
                          DTGTK_SLIDER_ADJUST_BUTTON_WIDTH, DTGTK_SLIDER_ADJUST_BUTTON_WIDTH-4,
                          CPF_DIRECTION_UP);

  dtgtk_cairo_paint_arrow(cr,
                          width-DTGTK_SLIDER_ADJUST_BUTTON_WIDTH-DTGTK_SLIDER_BORDER_WIDTH, height-DTGTK_SLIDER_ADJUST_BUTTON_WIDTH+4-DTGTK_SLIDER_BORDER_WIDTH*2,
                          DTGTK_SLIDER_ADJUST_BUTTON_WIDTH, DTGTK_SLIDER_ADJUST_BUTTON_WIDTH-4,
                          CPF_DIRECTION_DOWN);


  cairo_destroy(cr);
  return TRUE;
}

// Public functions
GtkWidget *dtgtk_slider_new(GtkAdjustment *adjustment)
{
  GtkDarktableSlider *slider;
  g_return_val_if_fail(adjustment == NULL || GTK_IS_ADJUSTMENT (adjustment), NULL);
  slider = gtk_type_new(dtgtk_slider_get_type());
  slider->adjustment = adjustment;
  slider->labelwidth = slider->labelheight = 0;
  return (GtkWidget *)slider;
}

GtkWidget *dtgtk_slider_new_with_range (darktable_slider_type_t type,gdouble min,gdouble max,gdouble step,gdouble value, gint digits)
{
  GtkAdjustment *adj = (GtkAdjustment *)gtk_adjustment_new (value, min, max, step, step, 0);
  GtkDarktableSlider *slider=DTGTK_SLIDER(dtgtk_slider_new(adj));
  slider->default_value=value;
  slider->type=type;
  slider->digits=digits;
  slider->is_entry_active=FALSE;
  slider->snapsize = 0;
  slider->labelwidth = slider->labelheight = 0;
  return GTK_WIDGET(slider);
}

void dtgtk_slider_set_digits(GtkDarktableSlider *slider, gint digits)
{
  slider->digits = digits;
}

void dtgtk_slider_set_snap(GtkDarktableSlider *slider, gint snapsize)
{
  slider->snapsize = snapsize;
}

void dtgtk_slider_set_format_type(GtkDarktableSlider *slider, darktable_slider_format_type_t type)
{
  slider->fmt_type=type;
}

void dtgtk_slider_set_label(GtkDarktableSlider *slider,gchar *label)
{
  slider->labelwidth = slider->labelheight = 0;
  g_object_set_data (G_OBJECT(slider),DTGTK_SLIDER_LABEL_KEY,label);
}

void dtgtk_slider_set_unit(GtkDarktableSlider *slider,gchar *unit)
{
  g_object_set_data (G_OBJECT(slider),DTGTK_SLIDER_VALUE_UNIT_KEY,unit);
}

void dtgtk_slider_set_force_sign(GtkDarktableSlider *slider,gboolean force)
{
  slider->force_sign = force;
}

void dtgtk_slider_set_default_value(GtkDarktableSlider *slider,gdouble val)
{
  slider->default_value = val;
}


gdouble dtgtk_slider_get_value(GtkDarktableSlider *slider)
{
  return gtk_adjustment_get_value( slider->adjustment );
}

void dtgtk_slider_set_value(GtkDarktableSlider *slider, gdouble value)
{
  if(slider->snapsize) value = slider->snapsize * (((int)value)/slider->snapsize);
  gtk_adjustment_set_value( slider->adjustment, value );
  g_signal_emit_by_name(G_OBJECT(slider),"value-changed");
  gtk_widget_queue_draw(GTK_WIDGET(slider));
}

void dtgtk_slider_set_type(GtkDarktableSlider *slider,darktable_slider_type_t type)
{
  slider->type = type;
  gtk_widget_draw( GTK_WIDGET(slider), NULL);
}

GtkType dtgtk_slider_get_type()
{
  static GtkType dtgtk_slider_type = 0;
  if (!dtgtk_slider_type)
  {
    static const GtkTypeInfo dtgtk_slider_info =
    {
      "GtkDarktableSlider",
      sizeof(GtkDarktableSlider),
      sizeof(GtkDarktableSliderClass),
      (GtkClassInitFunc) _slider_class_init,
      (GtkObjectInitFunc) _slider_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };
    dtgtk_slider_type = gtk_type_unique(GTK_TYPE_EVENT_BOX, &dtgtk_slider_info);
  }
  return dtgtk_slider_type;
}

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
