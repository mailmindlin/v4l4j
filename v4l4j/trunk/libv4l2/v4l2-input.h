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

#ifndef H_V4L2_INPUT
#define H_V4L2_INPUT

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/types.h>		//for videodev2
#include "videodev2.h"
#include <string.h>

#define CLEAR(x) memset(&x, 0x0, sizeof(x));
		
enum STANDARDS {WEBCAM, PAL, SECAM, NTSC};

struct mmap_buffer {
	int length;
	void *start;
};

struct mmap {
	int req_buffer_nr;				//requested number of buffers
	int buffer_nr;					//actual number of mmap buffers
	struct mmap_buffer *buffers;	//array of buffers
	struct v4l2_buffer tmp_b;		//temp buffer pointing to the latest dequeued buffer
};

struct control_list {
	int count;
	struct v4l2_queryctrl *ctrl;
	void *probe_priv;	//pointer to driver probe code's private data
};

struct capture_device {
	int fd;
	int width;
	int height;
	int fps;			//not implemented yet
	int std;			//v4l2 standard
	int channel;		//channel number (for video capture cards, not webcams)
	char file[100];		//device file name
	int bytesperline;	//number of bytes per line in the captured image
	int imagesize;		//in bytes
	struct mmap *mmap;
	struct control_list *ctrls;
};

void get_libv4l2_version(char *);

//Init methods
struct capture_device *init_libv4l2(const char *, int, int, int, int, int);
int open_device(struct capture_device *);
int check_capture_capabilities(struct capture_device *);
int set_cap_param(struct capture_device *);
int init_capture(struct capture_device *);
int start_capture(struct capture_device *);

//capture methods
int wait_for_frame(struct capture_device *);
struct v4l2_buffer *dequeue_buffer(struct capture_device *);
void *get_frame_buffer(struct capture_device *, struct v4l2_buffer *, int *);
void enqueue_buffer(struct capture_device *, struct v4l2_buffer *);
 
//Freeing resources
int stop_capture(struct capture_device *);
void free_capture(struct capture_device *);
void close_device(struct capture_device *);
void del_libv4l2(struct capture_device *);

//Control methods
struct control_list *list_control(struct capture_device *);
int get_control_value(struct capture_device *, struct v4l2_queryctrl *);
void set_control_value(struct capture_device *, struct v4l2_queryctrl *,  int);
void free_control_list(struct control_list *);

//Query and list methods (printf to stdout)
void list_cap(struct capture_device *);			//prints results from query methods listed below
void enum_image_fmt(struct capture_device *);		//lists all supported image formats
void query_control(struct capture_device *);		//lists all supported controls
void query_frame_sizes(struct capture_device *);	// not implemented
void query_capture_intf(struct capture_device *);	//prints capabilities
void query_current_image_fmt(struct capture_device *);	//useless...


#endif
