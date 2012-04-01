/*
    This file is part of darktable,
    copyright (c) 2010 Henrik Andersson.

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
#ifndef DT_CONTROL_JOBS_IMAGE_H
#define DT_CONTROL_JOBS_IMAGE_H

#include <inttypes.h>
#include "common/image.h"
#include "common/mipmap_cache.h"
#include "control/control.h"

typedef struct dt_image_load_t
{
  int32_t imgid;
  dt_mipmap_size_t mip;
}
dt_image_load_t;

int32_t dt_image_load_job_run(dt_job_t *job);
void dt_image_load_job_init(dt_job_t *job, int32_t imgid, dt_mipmap_size_t mip);


#endif
