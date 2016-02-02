/*
* Copyright (C) 2010 Gilles Gigan (gilles.gigan@gmail.com)
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

#ifndef LIBVIDEOLINUX_H_
#define LIBVIDEOLINUX_H_

// Video4Linux-specific structures

/*
 * C A P T U R E   S T R U C T U R E S
 *
 */
// represent a single mmap buffer
struct mmap_buffer {
	void *start;		//start of the mmaped buffer
	int length;			//length of the mmaped buffer as given by v4l
						//does NOT indicate the length of the frame,
						//use struct capture_device->imagesize instead
};

struct mmap {
	struct mmap_buffer *buffers;	//array of mmap 'buffer_nb' buffers
									//(buffer_nb) is in struct capture_device
	void * tmp;						//temp buffer pointing to the latest
									//dequeued buffer (V4L2) - last
									//requested frame (V4L1)
	int req_buffer_nr;				//number of buffers requested
	int v4l1_mmap_size;				//used by v4l1 only, to store the overall
									//mmap size
};


struct convert_data {
	struct v4lconvert_data *priv;//the libv4l convert struct (used only if V4L2)
	struct v4l2_format *src_fmt; //the source pixel format used for capture
	struct v4l2_format *dst_fmt; //the dest format, after conversion
	int src_palette;			//the source libvideo palette index
	void *frame;				//the last captured frame buffer after conv
								//the length of the buffer is set to
								//dst_fmt->fmt.pix.sizeimage
};

struct capture_backend {
	struct mmap *mmap;				//do not touch
	int real_v4l1_palette;			//v4l1 weirdness: v4l1 defines 2 distinct
									//palettes YUV420 and YUV420P but they are
									//the same (same goes for YUYV and YUV422).
									//In this field we store the real palette
									//used by v4l1. In the palette field above,
									//we store what the application should know
									//(YUYV instead of YUV422)
};


/*
 * C O N T R O L   S T R U C TU R E S
 *
 */
 //forward declarations
struct video_device;
struct control;
struct v4l_driver_probe {
	int (*probe) (struct video_device *, void **);
	int (*list_ctrl)(struct video_device *, struct control *, void *);
	int (*get_ctrl)(struct video_device *, struct control *,
			void *, int *);
	int (*set_ctrl)(struct video_device *,  struct control *,
			int *, void *);
	void *priv;
};
/*
 * element in linked list of driver probe
 */
typedef struct struct_elem {
	struct v4l_driver_probe *probe;
 	struct struct_elem *next;
} driver_probe;

struct control_backend {
	driver_probe *probes; 			//linked list of driver probes, allocated in
									//libvideo.c:get_control_list()
	struct v4lconvert_data *convert;//the libv4l convert struct (used only if V4L2)
								//DO NOT TOUCH
};


#endif /*LIBVIDEOLINUX_H_*/
