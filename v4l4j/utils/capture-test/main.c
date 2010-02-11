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

/*
 * This program runs a series of video capture tests from a video device (whose
 * device file is given as sole argument). It is used to test driver behaviour
 * and conformance to the V4L2 API.
 *
 * A video capture test consists of the following:
 * - check the given device supports V4L2 and streaming capture,
 * - setting up capture resolution and pixel format,
 * - request v4l2 buffers and mmap them to this program's address space,
 * - enqueue v4l2 buffers and start streaming
 * - capture a few frames
 * - stop streaming
 * - unmmap  v4l2 buffers
 *
 * The capture resolution, pixel format, number of v4l2 buffers requested,
 * number of captured frames can be adjusted by changing their values in v4l.h
 *
 * The number of video capture tests executed is also configurable.
 */

#include <errno.h>
#include <fcntl.h>			// for open
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>			//for close
#include "v4l.h"

void print_usage(char **argv){
	printf("Usage: %s <video device>\n", argv[0]);
	printf("where <video device> is the full path to the video device file\n");
}

// runs a single capture test
int run_test(int fd){
	int result = -1, nb_captured_frames = NB_CAPTURED_FRAMES;
	struct video_buffers *buffers = NULL;

	printf("====== STARTING TEST ======\n");

	// check the device is a V4L2 capture device
	if (check_caps(fd) != 0) {
		printf("Unable to test this device\n");
		goto bail;
	}

	// set capture resolution and pixel format
	if (set_image_format(fd) != 0) {
		printf("Unable to set image format\n");
		goto bail;
	}

	// request v4l2 buffers
	if ((buffers = get_v4l2_buffers(fd)) == NULL) {
		printf("Error getting v4l2 buffers\n");
		goto bail;
	}

	// start capture
	if (start_capture(fd, buffers) != 0) {
		printf("Error starting capture\n");
		put_v4l2_buffers(fd, buffers);
		goto bail;
	}

	// get a few frames
	while (nb_captured_frames-- && (get_latest_frame(fd, buffers) != -1 ));

	// stop capture
	stop_capture(fd, buffers);

	// put v4l2 buffers back
	put_v4l2_buffers(fd, buffers);

	result = 0;

bail:
	printf("====== TEST END ======\n\n");
	return result;
}

int main(int argc, char **argv){
	int fd, nb_test_runs = NB_TEST_RUNS;

	if (argc!=2) {
		print_usage(argv);
		exit(1);
	}

	// open device file
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror("Error opening device file");
		exit(1);
	}

	// run the tests
	while (nb_test_runs-- && (run_test(fd) == 0) );

	// close device file
	close(fd);

	return 0;
}

