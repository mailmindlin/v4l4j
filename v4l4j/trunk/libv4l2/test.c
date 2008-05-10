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


// This program tries capturing frames from /dev/video0
// for CAPTURE_LENGTH seconds and prints the resulting fps


#include <stdio.h>
#include <sys/time.h>
#include "v4l2-input.h"

#define CAPTURE_LENGTH  5 // in seconds

int main(int argc, char** argv) {
	struct capture_device *c;
	struct v4l2_buffer *b;
	struct timeval start, now;
	int size, count=0;
	
	printf("This program will try capturing frames from /dev/video0 for"\
		" %d seconds and will print the FPS\n", CAPTURE_LENGTH);
	printf("Make sure your webcam is connected, make sure it is the"\
		" only one, and press <Enter>, or Ctrl-C to abort now.");
	getchar();

	c = init_libv4l2("/dev/video0", 640,480 ,0,0,2);
	printf("Capturing from /dev/video0 at %dx%d...\n", c->width,c->height);

	if(open_device(c)<0) {
		printf("Cant open device");
		return -1;
	}
	
	if(check_capture_capabilities(c)<0){
		close_device(c);
		printf("Device doesnt have capture/streaming capabilities");
		return -1;
	}
	
	if(set_cap_param(c)){
		close_device(c);
		printf("Cant set capture parameters");
		return -1;
	}

	if(init_capture(c)<0){
		close_device(c);
		printf("Cant initialise capture ");
		return -1;
	}
	
	if(start_capture(c)<0){
		close_device(c);
		printf("Cant start capture");
	}
	
	gettimeofday(&start, NULL);
	gettimeofday(&now, NULL);
	while(now.tv_sec<start.tv_sec+CAPTURE_LENGTH) {
	
		//get frame from v4l2 
		if((b = dequeue_buffer(c)) != NULL) {
			//get address of frame
			get_frame_buffer(c, b, &size);
			count++;
		} else
			printf("Cant get buffer ");
	
		//Put frame  
		if(b != NULL) {
			//return buffer to v4l2
			enqueue_buffer(c, b);
		} else
			printf("Cant put buffer back");
		
		gettimeofday(&now, NULL);
	}
	printf("fps: %.1f\n", (count/((now.tv_sec - start.tv_sec) + ((float) (now.tv_usec - start.tv_usec)/1000000))));

	if(stop_capture(c)<0)
		fprintf(stderr, "Error stopping capture\n");

	free_capture(c);
	close_device(c);
	del_libv4l2(c);
	return 0;
}
