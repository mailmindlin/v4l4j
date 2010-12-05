/*
 * pixconv.c
 *
 * Copyright (C) 2010 Gilles Gigan (gilles.gigan@gmail.com)
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

#include "pixconv.h"
#include "common.h"
#include "libvideo.h"
#include "log.h"
#include "yuv_to_rgb.h"

static struct converter converters[] = {
		{		// YUYV to RGB32
				check_conversion_yuyv_to_rgb32,
		},
		{	// this array must be NULL ended
				NULL
		},
};

int				create_pixconv(struct pixconv** pc, int src_fmt,
		int dest_fmt, int width, int height){
	struct converter* 	converter;
	struct pixconv*		conv;
	int					index = 0;

	if (! pc)
		return -1;

	// allocate struct
	XMALLOC(conv, struct pixconv *, sizeof(struct pixconv));

	// setup the struct pixconv
	conv->source_fmt = src_fmt;
	conv->dest_fmt = dest_fmt;
	conv->w = width;
	conv->h = height;
	conv->pixel_count = width * height;

	// check if one of the converters can handle the conversion
	while((converter = &converters[index++]) != NULL) {
		if (converter->check_conversion(conv) != 0)
		{
			// return struct pixconv
			*pc = conv;
			return 0;
		}
	}

	// no converter found
	XFREE(*pc);
	return -1;
}

void			destroy_pixconv(struct pixconv*  pc) {
	XFREE(pc);
}

