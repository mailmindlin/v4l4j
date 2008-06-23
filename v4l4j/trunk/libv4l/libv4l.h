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

//Put the version in string & return it. allocation and freeing must be done by caller
//passing a char[10] is enough.
char *get_libv4l_version(char *);

//init_libv4l initialises required struct, opens the device file, and check what
//version of v4l the device supports, and whether capture and streaming are supported.
//Then creates the V4L control list.
//Arguments: device file, width, height, channel, std, nb_buf
struct capture_device *init_libv4l(const char *, int, int, int, int, int);

struct capture_device;
struct capture_actions {
/*
 * Init methods
 */
//set the capture parameters
//int * point to an array of image formats (palettes) to try (see bottom of libv4l.h for a list of supported palettes)
//the last argument (int) tells how many formats there are in the previous argument
//arg2 can be set to NULL and arg3 to 0 to try the default order (again, see libv4l.h) 
	int (*set_cap_param)(struct capture_device *, int *, int);

//initialise streaming, request create mmap'ed buffers
	int (*init_capture)(struct capture_device *);

//tell V4L to start the capture
	int (*start_capture)(struct capture_device *);

/*
 * capture methods
 * these methods can be called if calls to ALL the init methods (above) were successful
 */

//dequeue the next buffer with available frame
	void * (*dequeue_buffer)(struct capture_device *);

//enqueue the buffer when done using the frame
	void (*enqueue_buffer)(struct capture_device *);
 

/*
 * Freeing resources
 * these methods free resources created by matching init methods. Note that
 * set_cap_param doesnt have a counterpart since it only sets values and doesnt
 * create additional resources.
 */

//counterpart of start_capture, must be called it start_capture was successful
	int (*stop_capture)(struct capture_device *);

//counterpart of init_capture, must be called it init_capture was successful
	void (*free_capture)(struct capture_device *);


/*
 * Query and list methods (printf to stdout, use to debug)
 * these methods can be called after init_libv4l and before del_libv4l
 */
	void (*list_cap)(struct capture_device *);				//prints results from query methods listed below
	void (*enum_image_fmt)(struct capture_device *);		//lists all supported image formats
	void (*query_control)(struct capture_device *);		//lists all supported controls
	void (*query_frame_sizes)(struct capture_device *);	// not implemented
	void (*query_capture_intf)(struct capture_device *);	//prints capabilities
	void (*query_current_image_fmt)(struct capture_device *);	//print max width max height for v4l1 and current settings for v4l2
	
};

//counterpart of init_libv4l2, must be called it init_libv4l2 was successful
void del_libv4l(struct capture_device *);




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
//the default order in which palettes are tried if "set_cap_param(c, NULL, 0)" is used
#define		DEFAULT_PALETTE_ORDER	{YUV420, RGB24, RGB32, YUYV, RGB555, RGB565, GREY}
//Dont use the following two, use YUV420 and YUYV instead !!
#define 	YUV420P					7
#define 	YUV422					8




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
	struct capture_actions capture;	//see below
	int real_v4l1_palette;			//v4l1 weirdness: v4l1 defines 2 distinct palettes YUV420 and YUV420P 
									//but they are the same (same goes for YUYV and YUV422). In this field
									//we store the real palette used by v4l1. In the palette field above,
									//we store what the application should know (YUYV instead of YUV422)
};

struct libv4l_palette{
	int libv4l_palette;
	int v4l1_palette;
	int v4l2_palette;
	int depth;
	char name[10];
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
	},
//Dont use the following two, use YUV420 and YUYV instead !!
	{   
		YUV420P,
		VIDEO_PALETTE_YUV420P,
		0,
		12,
		"YUV420"
	},
		{   
		YUV422,
		VIDEO_PALETTE_YUV422,
		0,
		8,
		"YUYV"
	}
};

#endif
