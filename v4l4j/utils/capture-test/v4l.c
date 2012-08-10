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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// for memset
#include <sys/ioctl.h>		// for ioctl
#include <sys/mman.h>		//for mmap
#include <sys/time.h>
#include "v4l.h"

#define CLEAR(address,size) 	memset((address), 0x0, (size));
#define GET_FOURCC_NAME(fmt)	(char) ((fmt) & 0xff),\
								(char) ((fmt>>8) & 0xff),\
								(char) ((fmt>>16) & 0xff),\
								(char) ((fmt>>24) & 0xff)


// Check that the given device file is a V4L2 device file and supports
// streaming capture.
int check_caps(int fd){
	int result = -1;
	struct v4l2_capability cap;
	CLEAR(&cap, sizeof(cap));

	printf("Checking device...");

	if(0 == ioctl(fd, VIDIOC_QUERYCAP, &cap)){
		if ( (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
				(cap.capabilities & V4L2_CAP_STREAMING)){
			printf("Found V4L2 device '%s' - driver '%s' - version: %u \n",
					cap.card, cap.driver, cap.version);
			result = 0;
		} else
			printf("\nThe device is not a capture device or does not support "
					"streaming\n");
	} else
		perror("\nError checking capabilities");

	return result;
}

// set capture resolution and pixel format
int set_image_format(int fd){
	int result = -1;
	struct v4l2_format format;
	CLEAR(&format, sizeof(format));

	printf("Setting format to %dx%d - %c%c%c%c ...",
			CAPTURE_WIDTH, CAPTURE_HEIGHT, GET_FOURCC_NAME(CAPTURE_PIX_FORMAT));

	// setup struct
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = CAPTURE_WIDTH;
	format.fmt.pix.height = CAPTURE_HEIGHT;
	format.fmt.pix.field = V4L2_FIELD_ANY;
	format.fmt.pix.pixelformat = CAPTURE_PIX_FORMAT;

	if (0 == ioctl(fd, VIDIOC_S_FMT, &format)) {
		printf("OK\n");
		result = 0;
	} else
		perror("\nUnable to set capture parameters");

	return result;
}

// request v4l2 buffers and mmap them to our address space
struct video_buffers* get_v4l2_buffers(int fd){
	int i;
	struct video_buffers *buffers = NULL;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	CLEAR(&req, sizeof(req));

	// ask for V4L2 buffers
	req.count = NB_BUFFERS_REQUESTED;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	printf("Asking for %d V4L2 buffers...", req.count);

	if (-1 == ioctl (fd, VIDIOC_REQBUFS, &req)) {
		perror("\nUnable to request v4l2 buffers");
		goto bail;
	}

	printf("Got %d\n", req.count);

	// allocate and mmap all buffers
	buffers = (struct video_buffers *) malloc(sizeof(struct video_buffers));
	CLEAR(buffers, sizeof(struct video_buffers));

	buffers->nb_buf = req.count;

	buffers->mmap_buffers = (struct mmap_buffer *) malloc (req.count * sizeof(struct mmap_buffer));
	CLEAR(buffers->mmap_buffers, req.count * sizeof(struct mmap_buffer));

	for(i = 0 ; i < req.count; i++){
		CLEAR(&buf, sizeof(buf));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		printf("Querying and mmap'ing buffer %d...", i);

		if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)){
			perror("\nError querying buffer");
			goto bail;
		}

		buffers->mmap_buffers[i].length = buf.length;
		buffers->mmap_buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd,	(off_t) buf.m.offset);

		if (MAP_FAILED == buffers->mmap_buffers[i].start) {
			perror("\nError mmap'ing buffer");
			goto bail;
		}

		printf("OK\n");
	}

	return buffers;

bail:
	if (buffers) {
		// unmmap any buffer
		for(i = 0; i < buffers->nb_buf; i++)
			if ( (buffers->mmap_buffers[i].start) && (buffers->mmap_buffers[i].length) &&
					(-1 == munmap(buffers->mmap_buffers[i].start, buffers->mmap_buffers[i].length)))
				perror("\nError unmmap'ing buffer");

		// free buffers
		free(buffers->mmap_buffers);
		free(buffers);
	}
	return NULL;
}

// enqueue all v4l2 buffers obtained by the previous function and
// start video streaming
int start_capture(int fd, struct video_buffers *buffers){
	int i, result = -1;
	struct v4l2_buffer *buf;
	buf = (struct v4l2_buffer *) malloc(sizeof(struct v4l2_buffer));

	printf("Enqueuing all v4l2 buffers...");

	//Enqueue all buffers
	for(i=0; i< buffers->nb_buf; i++) {
		CLEAR(buf, sizeof(struct v4l2_buffer));
		buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf->memory = V4L2_MEMORY_MMAP;
		buf->index = i;
		if(-1 == ioctl(fd, VIDIOC_QBUF, buf)) {
			perror("\nError enqueuing buffer");
			goto bail;
		}
	}

	printf("Ok\nStarting video stream...");

	// start streaming
	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 != ioctl(fd, VIDIOC_STREAMON, &i)){
		perror("\nError starting stream");
		goto bail;
	}

	buffers->v4l2_buf = buf;
	result  = 0;

	printf("OK\n");

bail:
	if (result != 0)
		free(buf);

	return result;
}

// dequeue the next buffer, and enqueue it back
int get_latest_frame(int fd, struct video_buffers *buffers){
	int result = -1;
        struct timeval before_ioctl, after_ioctl;
	struct v4l2_buffer *b = buffers->v4l2_buf;
	CLEAR(b, sizeof(struct v4l2_buffer));

	b->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b->memory = V4L2_MEMORY_MMAP;

	printf("Getting last frame...");

	// get latest buffer
        gettimeofday(&before_ioctl, NULL);
	if (0 == ioctl(fd, VIDIOC_DQBUF, b)) {
                gettimeofday(&after_ioctl, NULL);
		// frame starts at buffers->mmap_buffers[b->index].start
		// and is b->bytesused bytes long

#define TV_TO_US(tv)    ((tv).tv_sec*1000000ULL + (tv).tv_usec)
                timersub(&after_ioctl, &before_ioctl, &after_ioctl);
                printf(" Timestamp: %llu us - ioctl blocked for %llu us ...", TV_TO_US(b->timestamp), TV_TO_US(after_ioctl));
		// put the buffer back
		if (0 == ioctl(fd, VIDIOC_QBUF, b)) {
			printf(" OK\n");
			result = 0;
		} else
			perror("\nError enqueuing buffer");
	} else
		perror("\nError dequeuing latest buffer");

	return result;
}

// stop video streaming
void stop_capture(int fd, struct video_buffers *buffers) {
	int i;

	printf("stopping capture...");

	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(0 != ioctl(fd, VIDIOC_STREAMOFF, &i ))
		perror("\nError");
	else
		printf("OK\n");

	free(buffers->v4l2_buf);
}

// unmmap v4l2 buffers
void put_v4l2_buffers(int fd, struct video_buffers *buffers){
	int i;
	struct v4l2_requestbuffers req;

	printf("Unmmaping v4l2 buffers...");
	// unmap v4l buffers
	if (buffers){
		for(i = 0; i < buffers->nb_buf; i++)
			if (-1 == munmap(buffers->mmap_buffers[i].start, buffers->mmap_buffers[i].length))
				perror("\nError unmmap'ing buffer");
	}

	// free structs
	free(buffers->mmap_buffers);
	free(buffers);

	printf("OK\n");

	// release v4l2 buffers
	CLEAR(&req, sizeof(req));

	req.count = 0;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	printf("Releasing V4L2 buffers...");

	if (-1 == ioctl (fd, VIDIOC_REQBUFS, &req))
		perror("\nUnable to release v4l2 buffers");
	else
		printf("OK\n");
}


