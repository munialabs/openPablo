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
#ifndef DTGTK_PAINT_H
#define DTGTK_PAINT_H

#include <gtk/gtk.h>
#include <cairo.h>

#define CPF_USER_DATA 0x1000

typedef enum dtgtk_cairo_paint_flags_t
{
  CPF_NONE=0,
  CPF_DIRECTION_UP=1,
  CPF_DIRECTION_DOWN=2,
  CPF_DIRECTION_LEFT=4,
  CPF_DIRECTION_RIGHT=8,
  CPF_ACTIVE=16,
  CPF_PRELIGHT=32,
  CPF_IGNORE_FG_STATE=64,	    // Ignore state when setting foregroundcolor
  CPF_BG_TRANSPARENT=128,     // transparent background
  CPF_STYLE_FLAT=256,         // flat style widget
  CPF_STYLE_BOX=512,          // boxed style widget
  CPF_DO_NOT_USE_BORDER=1024, // do not paint inner border
  CPF_SPECIAL_FLAG=2048
}
dtgtk_cairo_paint_flags_t;


typedef void (*DTGTKCairoPaintIconFunc)(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);

/** Paint a arrow left or right */
void dtgtk_cairo_paint_arrow(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a solid arrow left/right/up/down */
void dtgtk_cairo_paint_solid_arrow(cairo_t *cr, gint x,int y,gint w,gint h, gint flags);
/** Paint a store icon */
void dtgtk_cairo_paint_store(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a reset icon */
void dtgtk_cairo_paint_reset(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a reset icon */
void dtgtk_cairo_paint_presets(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a flip icon */
void dtgtk_cairo_paint_flip(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a switch icon */
void dtgtk_cairo_paint_switch(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a color rect icon */
void dtgtk_cairo_paint_color(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a eye icon */
void dtgtk_cairo_paint_eye(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a timer icon */
void dtgtk_cairo_paint_timer(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a filmstrip icon */
void dtgtk_cairo_paint_filmstrip(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a directory icon */
void dtgtk_cairo_paint_directory(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a refresh/reload icon */
void dtgtk_cairo_paint_refresh(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a cancel X icon */
void dtgtk_cairo_paint_cancel(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint two boxes indicating portrait/landscape flip */
void dtgtk_cairo_paint_aspectflip(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** Paint a color label icon */
void dtgtk_cairo_paint_label(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint a color picker icon - a pipette for bigger buttons */
void dtgtk_cairo_paint_colorpicker(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint alignment icon */
void dtgtk_cairo_paint_alignment(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint styles icon */
void dtgtk_cairo_paint_styles(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint the preferences wheel. */
void dtgtk_cairo_paint_preferences(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint and */
void dtgtk_cairo_paint_and(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint or */
void dtgtk_cairo_paint_or(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint and not */
void dtgtk_cairo_paint_andnot(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint dropdown arrow */
void dtgtk_cairo_paint_dropdown(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint bracket capture */
void dtgtk_cairo_paint_bracket(cairo_t *cr,gint x,gint y,gint w,gint h,gint flags);
/** paint lock icon */
void dtgtk_cairo_paint_lock(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint check mark icon */
void dtgtk_cairo_paint_check_mark(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint a over/under exposure icon */
void dtgtk_cairo_paint_overexposed(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);

/** paint active modulgroup icon */
void dtgtk_cairo_paint_modulegroup_active(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint favorites modulgroup icon */
void dtgtk_cairo_paint_modulegroup_favorites(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint basic modulgroup icon */
void dtgtk_cairo_paint_modulegroup_basic(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint tone modulgroup icon */
void dtgtk_cairo_paint_modulegroup_tone(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint color modulgroup icon */
void dtgtk_cairo_paint_modulegroup_color(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint correct modulgroup icon */
void dtgtk_cairo_paint_modulegroup_correct(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);
/** paint effect modulgroup icon */
void dtgtk_cairo_paint_modulegroup_effect(cairo_t *cr, gint x, gint y, gint w, gint h, gint flags);

#endif

// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;
