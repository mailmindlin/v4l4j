/*
* Copyright (C) 2009 Gilles Gigan (gilles.gigan@gmail.com)
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

#ifndef V4L_H_
#define V4L_H_

#define CAPTURE_WIDTH			640					// capture width
#define CAPTURE_HEIGHT			480					// capture height
#define CAPTURE_PIX_FORMAT		V4L2_PIX_FMT_YUYV	// capture pixel format
#define NB_BUFFERS_REQUESTED 	5 // how many v4l2 buffers we will request
#define NB_CAPTURED_FRAMES		10 // how many frames will we capture
#define NB_TEST_RUNS			1 // how many time to run the test

#include <linux/videodev2.h>

struct mmap_buffer {
	void *start;		//start of the mmaped buffer
	int length;			//length of the mmaped buffer as given by v4l
};

struct video_buffers {
	int nb_buf;			// number of buffers
	struct mmap_buffer *mmap_buffers;
	struct v4l2_buffer *v4l2_buf;
};


int 					check_caps(int );
int 					set_image_format(int );
struct video_buffers* 	get_v4l2_buffers(int );
int 					start_capture(int , struct video_buffers *);
int 					get_latest_frame(int , struct video_buffers *);
void 					stop_capture(int , struct video_buffers *);
void 					put_v4l2_buffers(int, struct video_buffers *);

#endif /* V4L_H_ */
