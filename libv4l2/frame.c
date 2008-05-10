/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a   
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>		//for memcpy

#include "frame.h"
#include "libv4l2_log.h"

void copy_to_frame(struct frame *f, void *data, int size){
	assert((f->buf_size >=size) && (f->freebytes>=size));
	f->data = memcpy(f->data, data, (size_t) size);
	f->freebytes -= size;
}

struct frame *create_empty_frame(int w, int h, enum formats ft, int do_alloc, void *p) {
	struct frame *f = (struct frame *) malloc(sizeof(struct frame));
	f->width = w;
	f->height = h;
	if (p == NULL)
		dprint(LIBV4L2_LOG_SOURCE_FRAME, LIBV4L2_LOG_LEVEL_WARN, "FRAME: creating new frame with priv = NULL\n");
	f->priv = p;
	f->format = ft;
	f->buf_size = f->freebytes = 0;
	f->data = NULL;
	if (do_alloc) {
		switch((int) ft) {
		case JPEG:
		case RGB24:
			f->data = (void *) malloc( (size_t) (w * h * 3));
			if (f->data == NULL) {
				dprint(LIBV4L2_LOG_SOURCE_FRAME, LIBV4L2_LOG_LEVEL_WARN, "FRAME: Cant allocate memory for a new empty frame\n");
				free(f);
				return NULL;
			}
			f->buf_size = w * h * 3;
			f->freebytes = f->buf_size;
			break;
		case YUV420P:
			f->data = (void *) malloc( (size_t) (w * h * 1.5));
			if (f->data == NULL) {
				dprint(LIBV4L2_LOG_SOURCE_FRAME, LIBV4L2_LOG_LEVEL_WARN, "FRAME: Cant allocate memory for a new empty frame\n");
				free(f);
				return NULL;
			}
			f->buf_size = w * h * 1.5;
			f->freebytes = f->buf_size;
			break;
		case PPM3:
		case PPM6:
			dprint(LIBV4L2_LOG_SOURCE_FRAME, LIBV4L2_LOG_LEVEL_DEBUG ,"FRAME: Need a new PPMx frame but not going to allocate buffer memory\n");
			break;
		default:
			dprint(LIBV4L2_LOG_SOURCE_FRAME, LIBV4L2_LOG_LEVEL_DEBUG ,"FRAME: Need a new empty frame, but unknown format %d (w %d, h %d\n", ft,w,h);
			return NULL;
		}
	}
	dprint(LIBV4L2_LOG_SOURCE_FRAME, LIBV4L2_LOG_LEVEL_DEBUG1 ,"FRAME: created empty frame(w %d, h %d fmt %d, buf_size %d free %d\n",w,h, ft, f->buf_size, f->freebytes);
	return	f;
}

void delete_frame(struct frame *f){
	if (f->data)
		free(f->data);
	
	free(f);
}

