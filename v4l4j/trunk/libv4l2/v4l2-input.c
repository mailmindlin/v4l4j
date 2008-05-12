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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>	//for open
#include <sys/ioctl.h>		//for ioctl
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>

//According to the V4L2 API docs, V4L2 projects should include a local copy of videodev2.h
//and not rely on the system's
//#include <linux/videodev2.h>  
#include "videodev2.h"
#include "v4l2-input.h"
#include "libv4l2_log.h"


#define XMALLOC(var, type, size)	\
		do { \
			var = (type) malloc(size); \
			if (!var) dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ERR, "MEMALLOC: Cant allocate %d bytes.\n", size); \
			else { CLEAR(*var); \
				dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ALL, "MEMALLOC: allocating %d bytes of type %s for var %s (%p).\n", size, #type, #var, var); } \
		} while (0)

#define XFREE(var)					\
		do { dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ALL, "MEMALLOC: freeing memory for var %s (%p).\n", #var, var); \
			if (var) { free(var); } \
			else { dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ALL, "MEMALLOC: Trying to free a NULL pointer.\n");} \
		} while (0)



void get_libv4l2_version(char * c) {
	sprintf(c, "%d.%d-%d", VER_MAJ, VER_MIN, VER_REL);
}

static void close_device(struct capture_device *c) {
	//Close device file
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: closing device file %s.\n", c->file);
	close(c->fd);
}


void del_libv4l2(struct capture_device *c) {
	free_control_list(c->ctrls);
	close_device(c);
	XFREE(c->mmap);
	XFREE(c);
}

static int open_device(struct capture_device *c) {
	int retval = 0;
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: Opening device file %s.\n", c->file);
	if ((strlen(c->file) == 0) || ((c->fd = open(c->file,O_RDWR )) < 0)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: unable to open device file %s.\n", c->file);
		retval = -1;
	}
	
	return retval;
}


static int get_capabilities(int fd, struct v4l2_capability *cap) {
	if (-1 == ioctl(fd, VIDIOC_QUERYCAP, cap)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Not a V4L2 device.\n");
		return -1;
	}
	return 0;
}

static int check_capture_capabilities(struct capture_device *c) {
	struct v4l2_capability cap;

	CLEAR(cap);
	
	if (get_capabilities(c->fd, &cap)!=0) return -1;

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: no capture capability.\n");
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: no streaming capability.\n");
		return -1;
	}

	return 0;
}


struct capture_device *init_libv4l2(const char *dev, int w, int h, int ch, int s, int buf_nb) {
	//create capture device
	struct capture_device *c;
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: Initialising libv4l2 and creating struct cdev\n");
	XMALLOC(c, struct capture_device *,sizeof(struct capture_device));
	XMALLOC(c->mmap, struct mmap *, sizeof(struct mmap));
	
	//fill in cdev struct
	c->mmap->req_buffer_nr = buf_nb;
	strcpy(c->file, dev);
	c->width = w;
	c->height = h;
	c->channel = ch;
	c->std = s;

	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: Opening device %s\n", c->file);
	if(open_device(c)!=0) {
		XFREE(c->mmap);
		XFREE(c);
		return NULL;
	}

	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: Checking capabilities on device %s\n", c->file);
	if(check_capture_capabilities(c)!=0){
		close_device(c);
		XFREE(c->mmap);
		XFREE(c);
		return NULL;
	}

	c->ctrls = list_control(c);

	return c;
}

struct v4l2_buffer *dequeue_buffer(struct capture_device *c) {
	struct v4l2_buffer *b;
	b = &c->mmap->tmp_b;
	CLEAR(*b);

	b->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b->memory = V4L2_MEMORY_MMAP;
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: dequeuing v4l2 buffer\n");
	if (-1 == ioctl(c->fd, VIDIOC_DQBUF, b)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: error dequeing buffer\n");
		return NULL;
	}
	
	return b;
}

void *get_frame_buffer(struct capture_device *c, struct v4l2_buffer *b, int *len){
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: getting buffer address\n");
	*len = b->bytesused;
	return c->mmap->buffers[b->index].start;
}

void enqueue_buffer(struct capture_device *cdev, struct v4l2_buffer *b) {
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: queuing v4l2 buffer\n");
	if (-1 == ioctl(cdev->fd, VIDIOC_QBUF, b))
			dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: error queuing buffer\n");

}


int wait_for_frame(struct capture_device *c){
	fd_set rfds;
	int retval = 0;
	
	FD_ZERO(&rfds);
	FD_SET(c->fd, &rfds);
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: waiting for frame\n");		
	if ((retval = select(c->fd+1, &rfds, NULL, NULL, NULL))<0)
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: select error");
		
	return retval;
}

int set_cap_param(struct capture_device *c) {

	struct v4l2_format fmt;
	struct v4l2_fmtdesc fmtd;
	struct v4l2_cropcap cc;
	struct v4l2_crop crop;
	CLEAR(fmt);
	CLEAR(fmtd);
	CLEAR(cc);
	CLEAR(crop);
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: setting capture parameters\n");
	
	//Select the input
	if (-1 == ioctl(c->fd, VIDIOC_S_INPUT, &(c->channel))) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cannot select the desired input (%d)\n", c->channel);
		query_capture_intf(c);
		return -1;
	}
	
	//Set the video standard if not a webcam
	if (-1 == ioctl(c->fd, VIDIOC_S_STD, (v4l2_std_id *) &c->std)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cannot select the desired standard (%d)\n", c->std);
		query_capture_intf(c);
		return -1;
	}
	
	//query the current image format
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(c->fd, VIDIOC_G_FMT, &fmt)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cannot get the current image format\n");
		return -1;
	}
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG1, "V4L2: current values: width: %d - height: %d - bytes/line %d - image size: %d\n", \
			fmt.fmt.pix.width,fmt.fmt.pix.height, fmt.fmt.pix.bytesperline, fmt.fmt.pix.sizeimage);
	
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;
	
	while(ioctl(c->fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		if (fmt.fmt.pix.pixelformat == fmtd.pixelformat) {
			dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG1, "V4L2: pixel format: %s (compressed : %d) (%d)\n", fmtd.description, fmtd.flags, fmtd.pixelformat);
			break;
		}
		fmtd.index++;
	}
	if (fmtd.index == 0) { 
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cant find the current image format...\n");
		return -1;
	}
	
	//Set image format
	//memset(&fmt, 0 , sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = c->width;
	fmt.fmt.pix.height = c->height;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	
	//FIXME: selecting a pixelformat by negotiation would be better than hardcoding it here ...
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	//fmt.fmt.pix.pixelformat = 843274064;
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: wanted values: width: %d - height: %d - format: %d\n", c->width, c->height, fmt.fmt.pix.pixelformat);
	
	if (-1 == ioctl(c->fd, VIDIOC_S_FMT, &fmt)) { 
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cant set the image format...\n");
		return -1;
	}


	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_INFO, "V4L2: Using width: %d, height: %d, bytes/line %d, image size: %d\n", \
			fmt.fmt.pix.width,fmt.fmt.pix.height, fmt.fmt.pix.bytesperline, fmt.fmt.pix.sizeimage);
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;
	
	while(ioctl(c->fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		if (fmt.fmt.pix.pixelformat == fmtd.pixelformat) {
			dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_INFO, "V4L2: pixel format: %s (compressed : %d)\n", fmtd.description, fmtd.flags);
			break;
		}
		fmtd.index++;
	}
	
	//Store actual width & height back in conf
	c->width = fmt.fmt.pix.width;
	c->height= fmt.fmt.pix.height;
	c->imagesize = fmt.fmt.pix.sizeimage;
	c->bytesperline = fmt.fmt.pix.bytesperline;
	
	//Set crop format
	cc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl( c->fd, VIDIOC_CROPCAP, &cc ) >= 0 )
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cc.defrect;
		ioctl( c->fd, VIDIOC_S_CROP, &crop );
	}
	
	return 0;
}

int init_capture(struct capture_device *c) {
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	int i=0;
	
	CLEAR(req);
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: initialising capture\n");
	//Init mmap
	
	//allocates v4l2 buffers
	req.count = c->mmap->req_buffer_nr;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG1, "V4L2: asking for %d V4L2 buffers\n", req.count);
	
	if (-1 == ioctl (c->fd, VIDIOC_REQBUFS, &req)) {
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: device does not support memory mapping\n");
		return -1;
	}
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: allocated %d V4L2 buffers\n", req.count);
	c->mmap->buffer_nr = req.count;
	XMALLOC( c->mmap->buffers, struct mmap_buffer *, (c->mmap->buffer_nr * sizeof(struct mmap_buffer)) );
	
	for(i=0; i<c->mmap->buffer_nr ; i++) {
		CLEAR(buf);
		
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		
		if (-1 == ioctl (c->fd, VIDIOC_QUERYBUF, &buf)){
			dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cant query allocated V4L2 buffers\n");
			return -1;
		}
		
		c->mmap->buffers[i].length = buf.length;
		c->mmap->buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, c->fd, (off_t) buf.m.offset);
		if (MAP_FAILED == c->mmap->buffers[i].start) {
			dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cant mmap allocated V4L2 buffers\n");
			return -1;
		}
		
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG1, "V4L2: buffer %d created (length: %d - address: %p)\n",i, c->mmap->buffers[i].length, c->mmap->buffers[i].start);
	}
	
	return 0;
}


void free_capture(struct capture_device *c) {
	int i = 0;
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG1, "V4L2: freeing capture structures\n");
	for(i=0; i < c->mmap->buffer_nr; i++){
		if (-1 == munmap(c->mmap->buffers[i].start, (size_t) c->mmap->buffers[i].length))
			dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: error unmapping buffer %d\n",i);
	}
	XFREE(c->mmap->buffers);

}

int start_capture(struct capture_device *c) {
	int i;
	struct v4l2_buffer b;
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: starting capture\n");
	
	//Enqueue all buffers
	for(i=0; i< c->mmap->buffer_nr; i++) {
		CLEAR(b);
		b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		b.memory = V4L2_MEMORY_MMAP;
		b.index = i;
		if(-1 == ioctl(c->fd, VIDIOC_QBUF, &b))
		{
			dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cannot enqueue initial buffers\n");
			return -1;
		}
	}
	
	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(c->fd, VIDIOC_STREAMON, &i ) < 0 )
	{
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cannot start capture\n");
		return -1;
	}
	
	return 0;
}

int stop_capture(struct capture_device *c) {
	int i;
	
	dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_DEBUG2, "V4L2: stopping capture\n");
	
	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(c->fd, VIDIOC_STREAMOFF, &i ) < 0 )
	{
		dprint(LIBV4L2_LOG_SOURCE_V4L2, LIBV4L2_LOG_LEVEL_ERR, "V4L2: cannot stop capture\n");
		return -1;
	}
	
	return 0;
}


// ****************************************
// List caps functions
// ****************************************

void enum_image_fmt(struct capture_device *cdev) {
	struct v4l2_fmtdesc fmtd;
	int fd = cdev->fd;
	
	printf("============================================\nQuerying image format\n\n");
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;
	
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		printf("%d - %s (compressed : %d) (%d) \n", fmtd.index, fmtd.description, fmtd.flags, fmtd.pixelformat);
		fmtd.index++;
	}
	
	if (fmtd.index == 0) printf("Not supported ...\n");
}

void query_current_image_fmt(struct capture_device *cdev) {
	struct v4l2_format fmt;
	struct v4l2_fmtdesc fmtd; //to find a text description of the image format
	int fd = cdev->fd;
		
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.index = 0;
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	printf("============================================\nQuerying current image format settings\n\n");
	if (-1 == ioctl(fd, VIDIOC_G_FMT, &fmt)) {
		perror("Failed to get image format.");
	}
	else {
		printf("Current width: %d\n", fmt.fmt.pix.width);
		printf("Current height: %d\n", fmt.fmt.pix.height);
		printf("Current bytes per line: %d\n", fmt.fmt.pix.bytesperline);
		printf("Current image size: %d\n", fmt.fmt.pix.sizeimage);
		printf("Current color space: %d\n", fmt.fmt.pix.colorspace);
		printf("Current pixel format: ");
		while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0){
			if (fmt.fmt.pix.pixelformat == fmtd.pixelformat) {
				printf("%s\n", fmtd.description);
				break;
			}
			fmtd.index++;
		}
		if (fmtd.index == 0) printf("Not supported ... (%d)\n", fmt.fmt.pix.pixelformat);
	}
}

void query_capture_intf(struct capture_device *cdev) {
	struct v4l2_input vin;
	int fd = cdev->fd;

	memset(&vin, 0, sizeof(struct v4l2_input));
	vin.index = 0;

	printf("============================================\nQuerying capture capabilities\n");
	while(ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0) {
		printf("Input number: %d\n", vin.index);
		printf("Name: %s\n", vin.name);
		printf("Type: (%d) ", vin.type);
		if(vin.type & V4L2_INPUT_TYPE_TUNER) {
			printf("Tuner\n");
			printf("Tuner index: %d", vin.tuner);
		}
		if(vin.type & V4L2_INPUT_TYPE_CAMERA) printf("Camera");
		printf("\nSupported standards: (%d) ", (int)vin.std);
		if(vin.std & V4L2_STD_PAL) printf("PAL ");
		if(vin.std & V4L2_STD_NTSC) printf("NTSC ");
		if(vin.std & V4L2_STD_SECAM) printf("SECAM ");
		printf("\n");
		vin.index++;
	}

}

void query_frame_sizes(struct capture_device *cdev){
	struct v4l2_frmsizeenum frms;
	struct v4l2_fmtdesc fmtd;
	
	memset(&frms, 0, sizeof(struct v4l2_frmsizeenum));
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;
	
	printf("============================================\nQuerying supported frame sizes\n\n");
/*	No V4L2 DRIVERS (as of 04/05/07) implement the VIDIOC_ENUM_FRAMESIZES ioctl...
	Although, i ve seen patches on the v4l ML for pwc.... but nothing released yet.
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		printf("Image format: %s\n",fmtd.description);
		frms.index = 0;
		frms.pixel_format = fmtd.pixelformat;
		while(ioctl (fd, VIDIOC_ENUM_FRAMESIZES, &frms) >=0) {
			printf("index %d", frms.index);
			if (frms.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
				printf("\tHeight: %d - Width: %d\n",frms.discrete.height, frms.discrete.width);
				frms.index++;
			}
			else {
				printf("\tMin, max & step height: %d - %d - %d Min, max & step width: %d - %d - %d",\
					frms.stepwise.min_height, frms.stepwise.max_height, frms.stepwise.step_height, \
					frms.stepwise.min_width, frms.stepwise.max_width, frms.stepwise.step_width);
				break;
			}
		}
		fmtd.index++;
	}*/
	
}

void query_control(struct capture_device *cdev) {
	struct v4l2_queryctrl qctrl;
	struct v4l2_querymenu qmenu;
	struct v4l2_control ctrl;
	int fd = cdev->fd;
	
	memset(&qctrl, 0, sizeof(struct v4l2_queryctrl));
	memset(&qmenu, 0, sizeof(struct v4l2_querymenu));
	memset(&ctrl, 0, sizeof(struct v4l2_control));
	qctrl.id = V4L2_CID_BASE;

	printf("============================================\nQuerying available contols\n\n");
	while(ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0) {
		if (qctrl.flags == V4L2_CTRL_FLAG_DISABLED) continue;
		if (qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS) { printf("Control class name: %s\n",qctrl.name); continue; }
		printf("Name: %s", qctrl.name);
		switch ((int)qctrl.type) {
			case V4L2_CTRL_TYPE_INTEGER:
				ctrl.id = qctrl.id;
				ioctl(fd, VIDIOC_G_CTRL, &ctrl);
				printf(" - Value: %d (Min: %d Max: %d Step: %d)\n", ctrl.value, qctrl.minimum, qctrl.maximum, qctrl.step);
				break;
			case V4L2_CTRL_TYPE_BOOLEAN:
				ctrl.id = qctrl.id;
				ioctl(fd, VIDIOC_G_CTRL, &ctrl);
				printf(" - Value: %d (On/Off)\n", ctrl.value);
				break;
			case V4L2_CTRL_TYPE_BUTTON:
				ctrl.id = qctrl.id;
				ioctl(fd, VIDIOC_G_CTRL, &ctrl);
				printf(" - Value: %d (button)\n", ctrl.value);
				break;
			case V4L2_CTRL_TYPE_MENU:
				qmenu.id = qctrl.id;
				for(qmenu.index=qctrl.minimum;qmenu.index<=qctrl.maximum;qmenu.index++) {
					if (0 == ioctl(fd,VIDIOC_QUERYMENU, &qmenu)) printf("Menu item (%d): %s\n", qmenu.index, qmenu.name);
				}
				memset(&qmenu, 0, sizeof(struct v4l2_querymenu));
				break;
		}
		qctrl.id++;
	}
}


void list_cap(struct capture_device *c) {
	struct v4l2_capability cap;
	
	printf("============================================\nQuerying general capabilities\n\n");
	if (ioctl(c->fd, VIDIOC_QUERYCAP, &cap) < 0) {
		perror("v4l2 not supported. Maybe a v4l1 device ...");
		close(c->fd);
		exit(1);
	}
	else {
		//print capabilities
		printf("Driver name: %s\n",cap.driver);
		printf("Device name: %s\n",cap.card);
		printf("bus_info: %s\n", cap.bus_info);
		printf("version: %u.%u.%u\n",(cap.version >> 16) & 0xFF,(cap.version >> 8) & 0xFF, cap.version & 0xFF);
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) printf("Has"); else printf("Does NOT have");
		printf(" capture capability\n");
		if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) printf("Has"); else printf("Does NOT have");
		printf(" output capability\n");
		if (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY) printf("Has"); else printf("Does NOT have");
		printf(" overlay capability\n");
		if (cap.capabilities & V4L2_CAP_VBI_CAPTURE) printf("Has"); else printf("Does NOT have");
		printf(" VBI capture capability\n");
		if (cap.capabilities & V4L2_CAP_VBI_OUTPUT) printf("Has"); else printf("Does NOT have");
		printf(" VBI output capability\n");
		if (cap.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE) printf("Has"); else printf("Does NOT have");
		printf(" SLICED VBI capture capability\n");
		if (cap.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT) printf("Has"); else printf("Does NOT have");
		printf(" SLICED VBI output capability\n");
		if (cap.capabilities & V4L2_CAP_RDS_CAPTURE) printf("Has"); else printf("Does NOT have");
		printf(" RDS capability\n");
		if (cap.capabilities & V4L2_CAP_TUNER) printf("Has"); else printf("Does NOT have");
		printf(" tuner capability\n");
		if (cap.capabilities & V4L2_CAP_AUDIO) printf("Has"); else printf("Does NOT have");
		printf(" audio capability\n");
		if (cap.capabilities & V4L2_CAP_RADIO) printf("Has"); else printf("Does NOT have");
		printf(" radio capability\n");
		if (cap.capabilities & V4L2_CAP_READWRITE) printf("Has"); else printf("Does NOT have");
		printf(" read/write capability\n");
		if (cap.capabilities & V4L2_CAP_ASYNCIO) printf("Has"); else printf("Does NOT have");
		printf(" asyncIO capability\n");
		if (cap.capabilities & V4L2_CAP_STREAMING) printf("Has"); else printf("Does NOT have");
		printf(" streaming capability\n");
		printf("\n");

		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) query_capture_intf(c);
		// FIXME Enumerate other capabilites (output, overlay,...

		query_control(c);
		enum_image_fmt(c);
		query_current_image_fmt(c);
		query_frame_sizes(c);
	}
}
