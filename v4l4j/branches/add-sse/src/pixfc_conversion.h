/*
 * pixfc_conversion.h
 *
 * Copyright (C) 2011 Gilles Gigan (gilles.gigan@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public  License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */





#ifndef PIXFC_CONVERSION_H_
#define PIXFC_CONVERSION_H_

#include "pixfc-sse.h"

// return the input libvideo image format that can be converted to the provided
// frame grabber output format. Call in a loop with index = 0
// return -1 if the conversion is not supported by input format at given index
// return -2 if index has exceeded number of input formats
// return -3 if the output format is not supported
// otherwise return the input libvideo palette
int	is_output_format_supported_by_pixfc(int out_fmt, int width, int height, int index);

#endif /* PIXFC_CONVERSION_H_ */
