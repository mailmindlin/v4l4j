/*
 * picfc_conversion.c
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

#include "common.h"
#include "pixfc-sse.h"
#include "libvideo.h"

static int	pixfc_formats[][3] = {
		{PixFcYUYV, YUYV, -1},
		{PixFcUYVY, UYVY, -1},
		{PixFcARGB, RGB32, -1},
		{PixFcBGRA, BGR32, -1},
		{PixFcRGB24, RGB24, OUTPUT_RGB24},
		{PixFcBGR24, BGR24, OUTPUT_BGR24},
};

static int	get_pixfc_pix_fmt(int framegrabber_output_fmt) {
	int index = 0;
	while (index < ARRAY_SIZE(pixfc_formats)) {
		if (pixfc_formats[index][3] == framegrabber_output_fmt)
			return pixfc_formats[index][0];

		index++;
	}

	return -1;
}

int	get_intput_format_for_output_format(int out_fmt, int width, int height, int index) {
	struct PixFcSSE *pixfc = NULL;
	PixFcPixelFormat output;
	int i = 0;

	if (index >= ARRAY_SIZE(pixfc_formats))
		return -2;

	// Get the pixfc pixel format from the frame grabber output format
	output = get_pixfc_pix_fmt(out_fmt);
	if (output == -1)
		return -3;

	// try and create a struct pixfc
	if (create_pixfc(&pixfc, pixfc_formats[index][0],  output, width, height) != PIXFC_OK)
		return -1;

	// success, release struct
	destroy_pixfc(pixfc);

	return pixfc_formats[index][1];
}



