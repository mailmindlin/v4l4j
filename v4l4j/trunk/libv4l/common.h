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

#ifndef H_COMMON
#define H_COMMON

#include <asm/types.h>		//for videodev2
#include "videodev2.h"
#include "videodev.h"

#define CLEAR(x) memset(&x, 0x0, sizeof(x));
		
struct mmap_buffer {
	void *start;					//start of the mmaped buffer
	int length;						//length of the mmaped buffer - does NOT indicate the length of the frame,
									//use struct capture_device->imagesize instead
};

struct mmap {
	int req_buffer_nr;				//requested number of buffers
	int buffer_nr;					//actual number of mmap buffers
	struct mmap_buffer *buffers;	//array of buffers
	void * tmp;						//temp buffer pointing to the latest dequeued buffer (V4L2) - last requested frame (V4L1)
	int v4l1_mmap_size;				//used by v4l1 only, to store the overall size of the mmaped area
};

struct control_list {
	int count;						//how many controls are available
	struct v4l2_queryctrl *ctrl;	//array of 'count' v4l2_queryctrl' controls (see videodev.h)
	void *probe_priv;				//pointer to driver probe code's private data, do not touch
};

//Supported standards
#define		WEBCAM					0
#define		PAL						1
#define		SECAM					2
#define		NTSC					3

#define NB_SUPPORTED_PALETTE		7
//palette formats
//YUV420 is the same as YUV420P - YUYV is the same as YUV422
#define 	YUV420					0
#define  	YUYV					1
#define 	RGB24					2
#define 	RGB32					3
#define	 	RGB555					4
#define 	RGB565					5
#define 	GREY					6
#define	DEFAULT_PALETTE_ORDER		{YUV420, RGB24, RGB32, YUYV, RGB555, RGB565, GREY}

//all the fields in the following structure are read only
struct capture_device {
	struct mmap *mmap;				//do not touch
	struct control_list *ctrls;		//video controls
	
#define V4L1_VERSION		1
#define V4L2_VERSION		2
	int v4l_version;				//
	int palette;					//which palette is used. see #define enum above
	int fd;							//do not touch
	int width;						//captured frame width
	int height;						//captured frame width
	int std;						//v4l standard - see #define enum above
	int channel;					//channel number (for video capture cards, not webcams)
	char file[100];					//device file name
	int bytesperline;				//number of bytes per line in the captured image
	int imagesize;					//in bytes
};

struct libv4l_palette{
	int libv4l_palette;
	int v4l1_palette;
	int v4l2_palette;
	int depth;
	char name[30];
};

static struct libv4l_palette libv4l_palettes[] = {
	{   
		YUV420,
		VIDEO_PALETTE_YUV420,
		V4L2_PIX_FMT_YUV420,
		12,		
		"YUV420"
	},
	{   
		YUYV,
		VIDEO_PALETTE_YUYV,
		V4L2_PIX_FMT_YUYV,
		8,
		"YUYV"
	},
		{   
		RGB24,
		VIDEO_PALETTE_RGB24,
		V4L2_PIX_FMT_RGB24,
		24,
		"RGB24"
	},
		{   
		RGB32,
		VIDEO_PALETTE_RGB32,
		V4L2_PIX_FMT_RGB32,
		32,
		"RGB32"
	},
		{   
		RGB555,
		VIDEO_PALETTE_RGB555,
		V4L2_PIX_FMT_RGB555,
		16,
		"RGB555"
	},
		{   
		RGB565,
		VIDEO_PALETTE_RGB565,
		V4L2_PIX_FMT_RGB565,
		16,
		"RGB565"
	},
		{   
		GREY,
		VIDEO_PALETTE_GREY,
		V4L2_PIX_FMT_GREY,
		8,
		"GREY"
	}	
};

#endif
