/*
    This file is part of darktable,
    copyright (c) 2011-2012 Henrik Andersson.

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

/* the defined modules groups, the specific order here sets the order
   of buttons in modulegroup buttonrow
*/
typedef enum dt_lib_modulegroup_t
{
  DT_MODULEGROUP_ACTIVE_PIPE,
  DT_MODULEGROUP_FAVORITES,

  DT_MODULEGROUP_BASIC,
  DT_MODULEGROUP_TONE,
  DT_MODULEGROUP_COLOR,
  DT_MODULEGROUP_CORRECT,
  DT_MODULEGROUP_EFFECT,

  /* dont touch the following */
  DT_MODULEGROUP_SIZE,

  DT_MODULEGROUP_NONE

} dt_lib_modulegroup_t;
