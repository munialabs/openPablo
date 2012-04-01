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
#ifndef DT_IMAGE_COMPRESSION
#include <inttypes.h>

/** K. Roimela, T. Aarnio and J. Itäranta. High Dynamic Range Texture Compression. Proceedings of SIGGRAPH 2006. */
void dt_image_compress(const float *in, uint8_t *out, const int32_t width, const int32_t height);
void dt_image_uncompress(const uint8_t *in, float *out, const int32_t width, const int32_t height);

#endif
