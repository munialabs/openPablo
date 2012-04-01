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
#ifndef DT_CONTROL_JOBS_DEVELOP_H
#define DT_CONTROL_JOBS_DEVELOP_H

#include "control/control.h"
#include "develop/develop.h"

#include <inttypes.h>

typedef struct dt_dev_process_t
{
  dt_develop_t *dev;
}
dt_dev_process_t;

/** process preview */
int32_t dt_dev_process_preview_job_run(dt_job_t *job);
void dt_dev_process_preview_job_init(dt_job_t *job, dt_develop_t *dev);

/** process image */
int32_t dt_dev_process_image_job_run(dt_job_t *job);
void dt_dev_process_image_job_init(dt_job_t *job, dt_develop_t *dev);

void dt_dev_export_init(dt_job_t *job);

#endif
