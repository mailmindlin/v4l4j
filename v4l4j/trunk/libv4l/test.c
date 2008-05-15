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
// Uncomment line 144 to write the captured frames to PPM files


#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include "libv4l.h"

#define CAPTURE_LENGTH  	2 // in seconds
#define OUTPUT_BLOCK_SIZE 	4096
#define PPM6_HEADER 		"P6 %d %d 255\n"

void write_frame(struct capture_device *c, void *d, int len) {
	int outfile, curr, bs;
	char filename[50];
	char ppm_header[40];
	struct timeval tv;
	

	//Construct the filename
	gettimeofday(&tv, NULL);
	sprintf(filename,"%s-%d.4-%d.%s", "file" ,(int) tv.tv_sec, (int) tv.tv_usec,  "ppm");


	//open file
	if ((outfile = open(filename, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
		printf( "FILE: can't open %s\n", filename);
		return;
	}

	curr = sprintf(ppm_header, PPM6_HEADER, c->width, c->height);
	printf( "FILE: PPM header %s (size %d)\n", ppm_header, curr);
	write(outfile, ppm_header, curr);
	curr=len;	
	while(curr > 0) {
		bs = (curr < OUTPUT_BLOCK_SIZE) ? curr: OUTPUT_BLOCK_SIZE;
		write(outfile, d, bs);
		curr -= bs;
		d += bs; 
	}
	printf("FILE: written %d bytes in file\n",len);
	close(outfile);
}

int main(int argc, char** argv) {
	struct capture_device *c;
	void *b, *d;
	struct timeval start, now;
	int size, count=0;
	
	if(argc!=2) {
		printf("This program requires the path to the video device file to be tested.\n");
		printf("Usage: %s /dev/video0\n", argv[0]);
		return -1;
	}
	
	printf("This program will try capturing frames from %s for"\
		" %d seconds and will print the FPS\n", argv[1], CAPTURE_LENGTH);
	printf("Make sure your webcam is connected, make sure it is the"\
		" only one, and press <Enter>, or Ctrl-C to abort now.");
	getchar();

	c = init_libv4l(argv[1], 320,240 ,0,0,2);

	if(c==NULL) {
		printf("Error initialising device.");
		return -1;
	}
	
	printf("Capturing from /dev/video0 at %dx%d...\n", c->width,c->height);
	
	if(set_cap_param(c, NULL, 0)){
		del_libv4l(c);
		printf("Cant set capture parameters");
		return -1;
	}

	if(init_capture(c)<0){
		del_libv4l(c);
		printf("Cant initialise capture ");
		return -1;
	}
	
	if(start_capture(c)<0){
		free_capture(c);
		del_libv4l(c);
		printf("Cant start capture");
		return -1;
	}
	
	gettimeofday(&start, NULL);
	gettimeofday(&now, NULL);
	while(now.tv_sec<start.tv_sec+CAPTURE_LENGTH) {
	
		//get frame from v4l2 
		if((b = dequeue_buffer(c)) != NULL) {
			//get address of frame
			d = get_frame_buffer(c, b, &size);
			//uncomment the following line to output captured frame 
			//to a file in PPM format
			//write_frame(c,d, size);
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
	del_libv4l(c);

	return 0;
}
