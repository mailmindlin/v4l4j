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


// This program list capture parameters from /dev/video0


#include <stdio.h>
#include <stdlib.h>

#include "libv4l.h"

int main(int argc, char** argv) {
	struct capture_device *c;
	struct video_device *v;
	int std=0, channel=0;

	if(argc!=2 && argc!=4) {
		printf("This program requires the path to the video device file to be tested.\n");
		printf("The optional second and third arguments are a video standard and channel.\n");
		printf("Usage: %s <video_device_file> [standard channel]\n", argv[0]);
		printf("Video standards: webcam:0 - PAL:1 - SECAM:2 - NTSC:3\n");
		return -1;
	}


	if (argc==4){
		std = atoi(argv[2]);
		channel = atoi(argv[3]);
		printf("Using standard %d, channel %d\n",std, channel);
	}

	v = open_device(argv[1]);
	if(v==NULL){
		printf("Error opening device\n");
		return -1;
	}
	c = init_capture_device(v, MAX_WIDTH, MAX_HEIGHT ,std ,channel ,2);

	if(c==NULL) {
		printf("Error initialising device.");
		return -1;
	}
	c->actions->list_cap(v->fd);

	free_capture_device(v);
	close_device(v);

	return 0;
}
