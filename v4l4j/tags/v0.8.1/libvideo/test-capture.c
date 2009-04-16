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


// This program tries capturing frames from the video device given in argument
// for CAPTURE_LENGTH seconds and prints the resulting fps
// Uncomment line 144 to write the captured frames to raw files


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include "libvideo.h"
#include "palettes.h"

#define CAPTURE_LENGTH  	10 // in seconds

void write_frame(void *d, int size) {
	int outfile, len = 0;
	char filename[50];
	struct timeval tv;


	//Construct the filename
	gettimeofday(&tv, NULL);
	sprintf(filename,"raw_frame-%d-%d.raw", (int) tv.tv_sec, (int) tv.tv_usec);


	//open file
	if ((outfile = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
		printf( "FILE: can't open %s\n", filename);
		return;
	}

	//printf( "FILE: writing %d bytes to file\n", size);
	while((len+=write(outfile, (d+len), (size-len)))<size);

	close(outfile);
}

int main(int argc, char** argv) {
	struct capture_device *c;
	struct video_device *v;
	void *d;
	struct timeval start, now;
	int size, count=0, std=0, channel=0, width=0, height=0, cap_length = 0,
		fmt=-1;

	if(argc!=7 && argc!=8) {
		printf("Usage: %s <video_device_file> <single_frame> <standard> <input>"
				" <width> <height> [ format ]\n", argv[0]);
		printf("This program requires the following arguments in this order:\n");
		printf("The video device file to be tested.\n");
		printf("Single frame capture (1), or 10 second capture (0)\n");
		printf("The video standard and input number.\n");
		printf("Video standards: webcam:0 - PAL:1 - SECAM:2 - NTSC:3\n");
		printf("The capture resolution (width and height)\n");
		printf("The last argument is optional and is an image format index. "
				"To see what formats are supported by a video device, run "
				"'./list-caps /dev/videoXX' and check the "
				"'Printing device info' section at the bottom.\n");
		printf("Arguments must be in the specified order !!!\n");
		return -1;
	}

	if(atoi(argv[2])>=1){
			cap_length = 0;
			printf("This program will capture a single frame from %s\n",
					argv[1]);
	} else {
		cap_length = CAPTURE_LENGTH;
		printf("This program will capture frames from %s for %d seconds\n",
				argv[1], cap_length);
	}

	std = atoi(argv[3]);
	channel = atoi(argv[4]);
	width = atoi(argv[5]);
	height = atoi(argv[6]);
	printf("Trying %dx%d standard %d, channel %d\n",width,height,std, channel);

	if(argc==8){
		fmt = atoi(argv[7]);
		printf("Trying image format %s (%d)\n",libvideo_palettes[fmt].name, fmt);
	}

	printf("Make sure your video source is connected, and press <Enter>, or "
			"Ctrl-C to abort now.");

	getchar();

	v = open_device(argv[1]);
	if(v==NULL){
		printf("Error opening device %s", argv[1]);
		return -1;
	}
	c = init_capture_device(v, width, height ,channel, std,2);

	if(c==NULL) {
		printf("Error initialising device.\n");
		close_device(v);
		return -1;
	}

	if(fmt!=-1){
		if((*c->actions->set_cap_param)(v, &fmt, 1)){
			free_capture_device(v);
			close_device(v);
			printf("Cant set capture parameters\n");
			return -1;
		}
	}else {
		if((*c->actions->set_cap_param)(v, NULL, 0)){
			free_capture_device(v);
			close_device(v);
			printf("Cant set capture parameters\n");
			return -1;
		}
	}


	printf("Capturing from %s at %dx%d.\n", argv[1], c->width,c->height);
	width = c->width;
	height = c->height;
	printf("Image format %s, size: %d\n", libvideo_palettes[c->palette].name, c->imagesize);

	if((*c->actions->init_capture)(v)<0){
		free_capture_device(v);
		close_device(v);
		printf("Cant initialise capture ");
		return -1;
	}

	if((*c->actions->start_capture)(v)<0){
		(*c->actions->free_capture)(v);
		free_capture_device(v);
		close_device(v);
		printf("Cant start capture");
		return -1;
	}

	gettimeofday(&start, NULL);
	gettimeofday(&now, NULL);
	while(now.tv_sec<=start.tv_sec+cap_length) {

		//get frame from v4l2
		if((d = (*c->actions->dequeue_buffer)(v, &size)) != NULL) {
			//uncomment the following line to output raw captured frame
			//to a file
			//write_frame(d, size);
			count++;
			//Put frame
			(*c->actions->enqueue_buffer)(v);
		} else {
			printf("Cant get buffer ");
			break;
		}
		gettimeofday(&now, NULL);
		if(cap_length==0)
			break;
	}
	printf("fps: %.1f\n", (count/((now.tv_sec - start.tv_sec) + ((float) (now.tv_usec - start.tv_usec)/1000000))));

	if((*c->actions->stop_capture)(v)<0)
		fprintf(stderr, "Error stopping capture\n");

	(*c->actions->free_capture)(v);
	free_capture_device(v);
	close_device(v);

	return 0;
}
