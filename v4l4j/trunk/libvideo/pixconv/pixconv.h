/*
 * pixconv.h
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

#ifndef PIXCONV_H_
#define PIXCONV_H_


struct pixconv {
	int source_fmt;
	int dest_fmt;
	int pixel_count;
	int w;
	int h;
	int use_sse2;

	int (*convert)(struct pixconv* conv, unsigned char* src, unsigned char* dst);
};

// This function allocate a struct pixconv and sets it up
// for a conversion from the src_fmt to the dest_fmt.
// returns 0 if OK, -1 if conv is not supported between the two formats,
int				create_pixconv(struct pixconv** pc, int src_fmt,int dest_fmt, int width, int height);
void			destroy_pixconv(struct pixconv*);

#endif /* PIXCONV_H_ */
