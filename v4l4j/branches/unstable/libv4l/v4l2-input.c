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

#include <sys/ioctl.h>		//for ioctl
#include <sys/mman.h>		//for mmap
#include <errno.h>			//for errno
#include <string.h>			//for memcpy
#include "libv4l.h"
#include "log.h"
#include "libv4l-err.h"
#include "palettes.h"
#include "v4l2-input.h"

//Arbitrary values that hopefully will never be reached
//v4l2 will adjust them to the closest available
#define V4L2_MAX_WIDTH 			4096
#define V4L2_MAX_HEIGHT 		4096

int check_v4l2(int fd, struct v4l2_capability* caps){
	return ioctl(fd, VIDIOC_QUERYCAP, caps);
}

int check_capture_capabilities_v4l2(int fd, char *file) {
	struct v4l2_capability cap;
	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: Checking capture device\n");

	CLEAR(cap);

	if (-1 == check_v4l2(fd, &cap)) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: Not a V4L2 device.\n");
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		info("The device %s seems to be a valid V4L2 device but without capture capability.\n", file);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libv4l README file.\n");
		info("Listing the reported capabilities:\n");
		list_cap_v4l2(fd);
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		info("The device %s seems to be a valid V4L2 device with capture capability, but\n", file);
		info("the device does NOT support streaming. Please let the author know about this error.\n");
		info("See the ISSUES section in the libv4l README file.\n");
		info("Listing the reported capabilities:\n");
		list_cap_v4l2(fd);
		return -1;
	}

	return 0;
}

static int try_std(int fd, int s){
	v4l2_std_id std;
	CLEAR(std);

	switch (s) {
		case WEBCAM:
			std = V4L2_STD_UNKNOWN;
			break;
		case PAL:
			std = V4L2_STD_PAL;
			break;
		case SECAM:
			std = V4L2_STD_SECAM;
			break;
		case NTSC:
			std = V4L2_STD_NTSC;
			break;
	}
	return ioctl(fd, VIDIOC_S_STD, &std);
}

static int detect_standard(struct capture_device *c, int fd){
	int found=0, i=0;

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "Trying to autodetect standard\n");

	while(found==0 && i<4) {
		if (try_std(fd, i)==0){
			found=1;
			if(i==WEBCAM) {
				info("Adjusted standard to WEBCAM\n");
				c->std = WEBCAM;
			} else if (i==PAL) {
				info("Adjusted standard to PAL\n");
				c->std = PAL;
			} else if (i==NTSC) {
				info("Adjusted standard to NTSC\n");
				c->std = NTSC;
			} else  {
				info("Adjusted standard to SECAM\n");
				c->std = SECAM;
			}
		}
		i++;
	}
	return found==0 ? -1 : 0;
}

static int set_std(struct capture_device *c, int fd){
	struct v4l2_standard s;
	CLEAR(s);


	//Linux UVC doesnt like to be ioctl'ed(VIDIOC_S_STD) even though v4l2 specs say nothing
	//about usb cam drivers returning EINVAL...
	//so we first try VIDIOC_ENUMSTD. if it returns EINVAL, then we assume it is a webcam
	//otherwise, we need to set a standard
	if(-1 != ioctl(fd, VIDIOC_ENUMSTD, &s)){
		//driver says "I use standards" - check if it is the UNKNOWN one, only used by webcams
		if(s.id != V4L2_STD_UNKNOWN) {
			//driver says "not webcam", check what we want...
			if(c->std == WEBCAM){
				//we want webcam... try to autodetect
				info("The specified standard (%d) is invalid.\n", c->std);
				if(detect_standard(c, fd)!=0) {
					//autodetect failed, so do we
					info("libv4l could not autodetect a standard for this input.\n");
					return -1;
				}
				//autodetect suceeded keep going
			} else {
				//we want !WEBCAM, so try that standard
				dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG,"Trying standard (%d).\n", c->std);
				if (-1 == try_std(fd, c->std)) {
					//failed, try autodetect
					info("The specified standard (%d) cannot be selected\n", c->std);
					if(detect_standard(c, fd)!=0) {
						//failed, exit
						info("libv4l couldnt autodetect a standard for this input.\n");
						return -1;
					}
					//autodetect succeeded, keep going
				}
				//given standard succeeded, keep going
			}
		} else {
			//driver says webcam, check what we want
			if(c->std != WEBCAM){
				//we want !WEBCAM, so try that standard
				if (-1 == try_std(fd, c->std)) {
					//failed, try autodetect
					info("The specified standard (%d) cannot be selected\n", c->std);
					if(detect_standard(c, fd)!=0) {
						//failed, exit
						dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR,"CAP: Couldnt autodetect a standard for this input.\n");
						return -1;
					}
					//autodetect succeeded, keep going
				}
			} else {
				//driver sais WEBCAM, so did we, keep going
			}
		}
	} else {
		//driver doesnt use standards - most likely a webcam
		if(c->std!=WEBCAM){
			info("The standard has been autodetected and set to WEBCAM \n");
			c->std = WEBCAM;
		}
	}

	return 0;
}

static int set_input(struct capture_device *c, int fd){
	struct v4l2_input vi;
	//Linux UVC doesnt like to be ioctl'ed (VIDIOC_S_INPUT)
	//so we only execute them if std!="webcam"

	//TODO: Add autodetection here so if the given input channel is invalid
	//a valid one is selected
	if (c->std!=WEBCAM) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: Setting input.\n");
		if(-1 == ioctl(fd, VIDIOC_S_INPUT, &(c->channel))) {
			info("The desired input (%d) cannot be selected.\n", c->channel);
			return -1;
		}
		vi.index = c->channel;
		if (-1 == ioctl(fd, VIDIOC_ENUMINPUT, &vi)) {
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: Failed to get details of input %d, errno(%d)\n", c->channel, errno);
			return -1;
		}
		if(vi.type == V4L2_INPUT_TYPE_TUNER)
			c->tuner_nb = vi.tuner;
		else
			c->tuner_nb = -1;
	}
	return 0;
}

static int try_image_format(struct v4l2_format *fmt, int width, int height, int fd, int palette){
	CLEAR(*fmt);
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_G_FMT, fmt)) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: cannot get the current image format\n");
		return LIBV4L_ERR_IOCTL;
	}
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->fmt.pix.width = width;
	fmt->fmt.pix.height = height;
	fmt->fmt.pix.field = V4L2_FIELD_ANY;
	fmt->fmt.pix.pixelformat = palette;

	if (0 == ioctl(fd, VIDIOC_S_FMT, fmt) && fmt->fmt.pix.pixelformat == palette) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_INFO, "CAP: palette %#x  - accepted at %dx%d\n", palette, fmt->fmt.pix.width, fmt->fmt.pix.height);
		return 0;
	}

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: palette %#x rejected\n", palette);
	return -1;
}


static int set_image_format(struct capture_device *c, int *palettes, int nb, int fd){
	int found=0, i, best_palette=-1, best_width =-1, best_height=-1;
	struct v4l2_format fmt;

	if(c->width==MAX_WIDTH)
		c->width=V4L2_MAX_WIDTH;
	if(c->height==MAX_HEIGHT)
		c->height=V4L2_MAX_HEIGHT;

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: trying palettes (%d to try in total)\n", nb);

	//we try all the supplied palettes and find the best one that give us a resolution closes to the desired one
	for(i=0; i<nb; i++) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG1, "CAP: trying palette %#x (%s) %dx%d - ...\n",\
			libv4l_palettes[palettes[i]].v4l2_palette, libv4l_palettes[palettes[i]].name, c->width, c->height);

		if( (try_image_format(&fmt, c->width, c->height, fd, libv4l_palettes[palettes[i]].v4l2_palette)==0) && ((best_palette == -1) || \
			 (abs(c->width*c->height - fmt.fmt.pix.width*fmt.fmt.pix.height) < abs(c->width*c->height - best_width*best_height))) ){
			best_palette = i;
			best_width = fmt.fmt.pix.width;
			best_height = fmt.fmt.pix.height;
			found = 1;
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: palette (%s) is best palette so far\n", 	libv4l_palettes[palettes[i]].name);
		}
	}
	if(!found) {
		info("libv4l was unable to find a suitable palette. The following palettes have been tried and failed:\n");
		for(i=0; i<nb;i++)
			info("%s\n",libv4l_palettes[palettes[i]].name);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libv4l README file.\n");
		return -1;
	} else {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: Setting to best palette %s...\n",\
				libv4l_palettes[palettes[best_palette]].name);

		if (0 == try_image_format(&fmt, c->width, c->height, fd, libv4l_palettes[palettes[best_palette]].v4l2_palette)) {
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG1, "CAP: palette (%s) accepted\n", 	libv4l_palettes[palettes[best_palette]].name);
			c->palette = palettes[best_palette];
		} else {
			info("Unable to set the best detected palette: %s\n", libv4l_palettes[palettes[best_palette]].name);
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libv4l README file.\n");
			return -1;
		}
	}

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: Using width: %d, height: %d, bytes/line %d, image size: %d\n", \
			fmt.fmt.pix.width,fmt.fmt.pix.height, fmt.fmt.pix.bytesperline, fmt.fmt.pix.sizeimage);

	//Store actual width & height
	c->width = fmt.fmt.pix.width;
	c->height= fmt.fmt.pix.height;
	if (COMPRESSED_FORMAT_DEPTH == libv4l_palettes[palettes[best_palette]].depth)
		c->imagesize = fmt.fmt.pix.sizeimage;
	else {
		c->imagesize  = c->width*c->height*libv4l_palettes[palettes[best_palette]].depth / 8;
		if(c->imagesize != fmt.fmt.pix.sizeimage) {
			info("The image size (%d) is not the same as what the driver returned (%d)\n",c->imagesize, fmt.fmt.pix.sizeimage);
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libv4l README file.\n");
			c->imagesize = fmt.fmt.pix.sizeimage;
		}
	}

	return 0;
}

static int set_crop(struct capture_device *c, int fd) {
	struct v4l2_cropcap cc;
	struct v4l2_crop crop;

	CLEAR(cc);
	CLEAR(crop);
	cc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//dont set cropping for webcams, Linux UVC doesnt like it
	if(c->std!=WEBCAM && ioctl( fd, VIDIOC_CROPCAP, &cc ) >= 0 ) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cc.defrect;
		if(ioctl( fd, VIDIOC_S_CROP, &crop )!=0) {
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: Error setting cropping info\n");
			return -1;
		}
	}

	return 0;
}

static int set_param(struct capture_device *c, int fd) {
	struct v4l2_streamparm param;

	//TODO: for now the FPS is hardcoded to 25. could be improved ?

	CLEAR(param);
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	param.parm.capture.timeperframe.numerator = 1;
	param.parm.capture.timeperframe.denominator = 30;
	ioctl(fd, VIDIOC_S_PARM, &param);

	return 0;
}

int set_cap_param_v4l2(struct video_device *vdev, int *palettes, int nb) {
	struct capture_device *c = vdev->capture;
	int ret = 0;
	int def[NB_SUPPORTED_PALETTE] = DEFAULT_PALETTE_ORDER;

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: Setting capture parameters on device %s.\n", vdev->file);

	if(nb<0 || nb>=NB_SUPPORTED_PALETTE) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: Incorrect number of palettes (%d)\n", nb);
		return LIBV4L_ERR_FORMAT;
	}
	if(nb==0 || palettes==NULL) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: No palettes supplied, trying default order.\n");
		palettes = def;
		nb = NB_SUPPORTED_PALETTE;
	}

	//set desired standard
	if (set_std(c, vdev->fd) !=0 ) {
		ret = LIBV4L_ERR_STD;
		goto fail;
	}

	//set desired input
	if (set_input(c, vdev->fd) != 0) {
		ret = LIBV4L_ERR_CHANNEL;
		goto fail;
	}

	//Set image format
	if (set_image_format(c, palettes, nb, vdev->fd) != 0) {
		ret = LIBV4L_ERR_FORMAT;
		goto fail;
	}

	//Set crop format
	if (set_crop(c, vdev->fd) != 0) {
		info("Listing the reported capabilities:\n");
		ret = LIBV4L_ERR_CROP;
		goto fail;
	}

	//set FPS
	set_param(c, vdev->fd);

	return ret;

fail:
	info("Listing the reported capabilities:\n");
	list_cap_v4l2(vdev->fd);
	return ret;
}

int init_capture_v4l2(struct video_device *vdev) {
	struct capture_device *c = vdev->capture;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	int i=0;
	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: Initialising capture on device %s.\n", vdev->file);

	CLEAR(req);

	//allocates v4l2 buffers
	req.count = c->mmap->req_buffer_nr;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: asking for %d V4L2 buffers\n", req.count);

	if (-1 == ioctl (vdev->fd, VIDIOC_REQBUFS, &req)) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: Error getting mmap information from driver\n");
		return LIBV4L_ERR_REQ_MMAP;
	}

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: driver said %d V4L2 buffers\n", req.count);
	c->mmap->buffer_nr = req.count;
	XMALLOC( c->mmap->buffers, struct mmap_buffer *, (c->mmap->buffer_nr * sizeof(struct mmap_buffer)) );

	for(i=0; i<c->mmap->buffer_nr ; i++) {
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == ioctl (vdev->fd, VIDIOC_QUERYBUF, &buf)){
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: cant query allocated V4L2 buffers\n");
			return LIBV4L_ERR_REQ_MMAP_BUF;
		}

		c->mmap->buffers[i].length = buf.length;
		c->mmap->buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, vdev->fd, (off_t) buf.m.offset);
		if (MAP_FAILED == c->mmap->buffers[i].start) {
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: cant mmap allocated V4L2 buffers\n");
			return LIBV4L_ERR_MMAP_BUF;
		}

		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: mmap'ed %d bytes at %p\n",c->mmap->buffers[i].length, c->mmap->buffers[i].start);
	}

	return 0;
}

int start_capture_v4l2(struct video_device *vdev) {

	int i;
	struct v4l2_buffer *b;
	XMALLOC(b,struct v4l2_buffer *, sizeof(struct v4l2_buffer));

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: Starting capture on device %s.\n", vdev->file);

	//Enqueue all buffers
	for(i=0; i< vdev->capture->mmap->buffer_nr; i++) {
		CLEAR(*b);
		b->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		b->memory = V4L2_MEMORY_MMAP;
		b->index = i;
		if(-1 == ioctl(vdev->fd, VIDIOC_QBUF, b)) {
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: cannot enqueue initial buffers\n");
			return LIBV4L_ERR_IOCTL;
		}
	}

	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(vdev->fd, VIDIOC_STREAMON, &i) < 0 ){
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: cannot start capture\n");
		return LIBV4L_ERR_IOCTL;
	}
	vdev->capture->mmap->tmp = (void *) b;

	return 0;
}

void *dequeue_buffer_v4l2(struct video_device *vdev, int *len) {
	struct v4l2_buffer *b = (struct v4l2_buffer *) vdev->capture->mmap->tmp;
	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG2, "CAP: dequeuing buffer on device %s.\n", vdev->file);

	CLEAR(*b);

	b->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b->memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(vdev->fd, VIDIOC_DQBUF, b)) {
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: error dequeuing buffer\n");
		return NULL;
	}

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG2, "CAP: getting buffer address\n");
	*len = b->bytesused;
	return vdev->capture->mmap->buffers[b->index].start;
}

void enqueue_buffer_v4l2(struct video_device *vdev) {
	struct v4l2_buffer *b = (struct v4l2_buffer *) vdev->capture->mmap->tmp;
	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG2, "CAP: queuing buffer on device %s.\n", vdev->file);
	if (-1 == ioctl(vdev->fd, VIDIOC_QBUF, b))
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: error queuing buffer\n");
}


int stop_capture_v4l2(struct video_device *vdev) {
	int i;
	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: stopping capture on device %s.\n", vdev->file);

	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(vdev->fd, VIDIOC_STREAMOFF, &i ) < 0 ){
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: cannot stop capture\n");
		return LIBV4L_ERR_IOCTL;
	}
	XFREE(vdev->capture->mmap->tmp);

	return 0;
}

void free_capture_v4l2(struct video_device *vdev) {
	int i = 0;

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: freeing capture structure on device %s.\n", vdev->file);

	for(i=0; i < vdev->capture->mmap->buffer_nr; i++){
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "CAP: unmmap %d bytes at %p\n",vdev->capture->mmap->buffers[i].length, vdev->capture->mmap->buffers[i].start);
		if (-1 == munmap(vdev->capture->mmap->buffers[i].start, (size_t) vdev->capture->mmap->buffers[i].length))
			dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "CAP: error unmapping buffer %d\n",i);
	}
	XFREE(vdev->capture->mmap->buffers);
}




/*
 * Control related functions
 */
struct struct_node {
 	int id;
 	struct struct_node *next;
 };
typedef struct struct_node node;

static void add_node(node **list, int id) {
	node *t;
	if((t=*list)) {
		//create the subsequent nodes
		while(t->next) t = t->next;
		XMALLOC(t->next, node *, sizeof(node));
		t->next->id = id;
	} else {
		//create the first node
		XMALLOC((*list), node *, sizeof(node));
		(*list)->id = id;
	}
 }
static  void empty_list(node *list){
 	node *t;
 	while(list) {
		t = list->next;
		XFREE(list);
		list = t;
 	}
 }
static int has_id(node *list, int id){
 	for(;list;list=list->next)
 		if(list->id==id) return 1;

 	return 0;
 }
 //returns the number of controls (standard and private V4L2 controls only)
int count_v4l2_controls(struct video_device *vdev) {
	struct v4l2_queryctrl qctrl;
	node *list=NULL;
	int i, count = 0, current = 0;

	CLEAR(qctrl);


	//std ctrls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1; i++) {
		qctrl.id = i;
		if(ioctl(vdev->fd, VIDIOC_QUERYCTRL, &qctrl) == 0) {
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED || qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS)
				continue;
			count++;
			add_node(&list, i);
		}
	}
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: found %d std ctrls\n", count);

	//priv ctrls
	for (qctrl.id = V4L2_CID_PRIVATE_BASE;; qctrl.id++) {
		if (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED || qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS)
				continue;
			count++;
			add_node(&list, qctrl.id);
		} else {
			if (errno == EINVAL)
				break;

			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: we shouldnt be here...\n");
		}
	}
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: found %d std/priv ctrls\n", count);

	//checking extended controls
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
		if(!has_id(list,qctrl.id) && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED) && qctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG1, "CTRL: found unique ext ctrl\n");
			count++;
		} else {
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG1, "CTRL: found duplicate ext ctrl\n");
		}
		if(qctrl.id<=current){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: found buggy driver\n");
			qctrl.id++;
		}
		current = qctrl.id & ~V4L2_CTRL_FLAG_NEXT_CTRL;
    	qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	empty_list(list);
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: found %d std/priv/ext controls\n", count);
	return count;
}

static void set_query_menu(struct video_device *vd, struct control *c){
	int i, count = 0, idx = 0;
	struct v4l2_querymenu qm, *q;
	CLEAR(qm);
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG,"CTRL: Setting menu for control %#x\n", c->v4l2_ctrl->id);

	//count how many menus there are
	qm.id = c->v4l2_ctrl->id;
	for(i = c->v4l2_ctrl->minimum; i==c->v4l2_ctrl->maximum; i++){
		qm.index = i;
		if(ioctl(vd->fd, VIDIOC_QUERYMENU, &qm) == 0){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG1, "CTRL: found menu item %s - %d\n", qm.name, qm.index);
			count++;
		}
	}

	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: found %d menus\n", count);

	if(count>0){
		//populate struct control->v4l2_querymenu
		XMALLOC(q, struct v4l2_querymenu *, count * sizeof(struct v4l2_querymenu));

		q[idx].id = c->v4l2_ctrl->id;
		for(i = c->v4l2_ctrl->minimum; i==c->v4l2_ctrl->maximum; i++){
			q[idx].index = i;
			if(ioctl(vd->fd, VIDIOC_QUERYMENU, &q[idx]) == 0)
				idx++;
		}
	} else {
		//sometimes, nothing is returned by the ioctl(VIDIOC_QUERYMENU), but the menu still exist and is
		//made of contiguous values between minimum and maximum.
		count = (c->v4l2_ctrl->maximum - c->v4l2_ctrl->minimum)/c->v4l2_ctrl->step + 1;
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: creating %d menus\n", count);
		XMALLOC(q, struct v4l2_querymenu *, count * sizeof(struct v4l2_querymenu));
		for(i = c->v4l2_ctrl->minimum; i<=c->v4l2_ctrl->maximum; i+=c->v4l2_ctrl->step){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG1, "CTRL: menu %d - val: %d\n", idx, i);
			q[idx].id = c->v4l2_ctrl->id;
			sprintf((char *) q[idx].name, "%d", i);
			q[idx++].index = i;
		}
	}
	c->count_menu = count;
	c->v4l2_menu = q;
}

static void fix_quirky_struct(struct v4l2_queryctrl *v){
	if(v->type==V4L2_CTRL_TYPE_INTEGER) {
		if(v->step==0){
			v->step = 1;
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted step value for INTEGER control '%s' from 0 to 1\n", v->name);
		}
	} else if(v->type==V4L2_CTRL_TYPE_MENU) {
		//Not sure about this one...
		if(v->step==0){
			v->step = 1;
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted step value for MENU control '%s' from 0 to 1\n", v->name);
		}
	} else if(v->type==V4L2_CTRL_TYPE_BOOLEAN) {
		if(v->step!=1){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted step value for BOOLEAN control '%s' from %d to 1\n", v->name, v->step);
			v->step = 1;
		}
		if(v->minimum!=0){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted minimum value for BOOLEAN control '%s' from %d to 0\n", v->name, v->minimum);
			v->minimum = 0;
		}
		if(v->maximum!=1){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted maximum value for BOOLEAN control '%s' from %d to 1\n", v->name, v->maximum);
			v->maximum = 1;
		}
	} else if(v->type==V4L2_CTRL_TYPE_BUTTON) {
		if(v->step!=0){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted step value for BUTTON control '%s' from %d to 0\n", v->name, v->step);
			v->step = 1;
		}
		if(v->minimum!=0){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted minimum value for BUTTON control '%s' from %d to 0\n", v->name, v->minimum);
			v->minimum = 0;
		}
		if(v->maximum!=0){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: adjusted maximum value for BUTTON control '%s' from %d to 0\n", v->name, v->maximum);
			v->maximum = 0;
		}
	}
}

//Populate the control_list with reported V4L2 controls
//and returns how many controls were created
int create_v4l2_controls(struct video_device *vdev, struct control *controls, int max){
	struct v4l2_queryctrl qctrl;
	node *list=NULL;
	int count = 0, i, current = 0;

	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: Creating std controls\n");
	//create standard V4L controls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1 && count < max; i++) {
		controls[count].v4l2_ctrl->id = i;
		if(ioctl(vdev->fd, VIDIOC_QUERYCTRL, controls[count].v4l2_ctrl) == 0) {
			dprint_v4l2_control(controls[count].v4l2_ctrl);
			if ( ! (controls[count].v4l2_ctrl->flags & V4L2_CTRL_FLAG_DISABLED) && controls[count].v4l2_ctrl->type!=V4L2_CTRL_TYPE_CTRL_CLASS) {
				fix_quirky_struct(controls[count].v4l2_ctrl);
				if(controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_MENU)
					set_query_menu(vdev, &controls[count]);
				count++;
				add_node(&list, i);
			}
		}
	}

	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: Creating priv controls (found %d std ctrl)\n", count);
	//create device-specific private V4L2 controls
	for (i = V4L2_CID_PRIVATE_BASE;count < max; i++) {
		controls[count].v4l2_ctrl->id = i;
		if (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, controls[count].v4l2_ctrl)) {
			dprint_v4l2_control(controls[count].v4l2_ctrl);
			if( ! (controls[count].v4l2_ctrl->flags & V4L2_CTRL_FLAG_DISABLED) && controls[count].v4l2_ctrl->type!=V4L2_CTRL_TYPE_CTRL_CLASS) {
				fix_quirky_struct(controls[count].v4l2_ctrl);
				if(controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_MENU)
					set_query_menu(vdev, &controls[count]);
				count++;
				add_node(&list, i);
			}
    	} else {
            if (errno == EINVAL)
            	break;

            dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: we shouldnt be here...\n");
    	}
	}

	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: Creating ext controls (created %d std/priv ctrl so far)\n", count);
	//create ext ctrls
	//TODO Add support for group-changes of extended controls. For now, reported ext ctrl can only be changed one at a time.
	//TODO add an extra method that list ext (so move the following to the new method) so apps are aware of which ctrls are
	//TODO extended ones, and can decide whether or not to change multiple ctrls at once or not
	//checking extended controls
	CLEAR(qctrl);
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
		if(!has_id(list,qctrl.id ) && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED) && qctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS ){
			dprint_v4l2_control((&qctrl));
			CLEAR(*controls[count].v4l2_ctrl);
			memcpy(controls[count].v4l2_ctrl, &qctrl, sizeof(struct v4l2_queryctrl));
			fix_quirky_struct(controls[count].v4l2_ctrl);
			if(controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_MENU)
				set_query_menu(vdev, &controls[count]);
			count++;
		} else {
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: duplicate ext ctrl\n");
		}
		if(qctrl.id<=current){
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: found buggy driver\n");
			qctrl.id++;
		}
		current = qctrl.id & ~V4L2_CTRL_FLAG_NEXT_CTRL;
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	empty_list(list);
	return count;
}
static int fix_quirky_values(struct v4l2_queryctrl *qc, int v){
	if(v < qc->minimum) {
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: fixed quirky control value %d below minimum %d\n",v,qc->minimum);
		return qc->minimum;
	} else if (v>qc->maximum) {
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: QUIRK: fixed quirky control value %d above maximum %d\n",v,qc->maximum);
		return qc->maximum;
	}
	return v;

}
//returns the value of a control
int get_control_value_v4l2(struct video_device *vdev, struct v4l2_queryctrl *ctrl, int *val){
	struct v4l2_control vc;
	int ret = LIBV4L_ERR_IOCTL;
	CLEAR(vc);
	vc.id = ctrl->id;
	if( (ret = ioctl(vdev->fd, VIDIOC_G_CTRL, &vc)) == 0 ) {
		*val = fix_quirky_values(ctrl, vc.value);
		ret = 0;
	} else
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: Error getting current value (%d)\n", errno);

	return ret;
}

//sets the value of a control
int set_control_value_v4l2(struct video_device *vdev, struct v4l2_queryctrl *ctrl, int *i) {
	struct v4l2_control vc;
	int prev = 0;

	get_control_value_v4l2(vdev,ctrl, &prev);
	vc.id = ctrl->id;
	vc.value = *i;

	if(ioctl(vdev->fd, VIDIOC_S_CTRL, &vc)!= 0) {
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: Error setting value\n");
		if(errno == EINVAL) dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: einval\n");
		else if(errno == ERANGE) dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: erange\n");
		else dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: unknown error %d\n", errno);
		return LIBV4L_ERR_IOCTL;
	}
	return 0;
}

// ****************************************
// List caps functions
// ****************************************
static void enum_image_fmt_v4l2(int fd) {
	struct v4l2_fmtdesc fmtd;

	printf("============================================\nQuerying image format\n\n");
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;

	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		printf("%d - %s (compressed : %d) (%d) \n", fmtd.index, fmtd.description, fmtd.flags, fmtd.pixelformat);
		fmtd.index++;
	}

	printf("\n");
}

static void query_current_image_fmt_v4l2(int fd) {
	struct v4l2_format fmt;
	struct v4l2_fmtdesc fmtd; //to find a text description of the image format

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
	}
	printf("\n");
}

static void query_capture_intf_v4l2(int fd) {
	struct v4l2_input vin;
	struct v4l2_tuner tun;
	struct v4l2_frequency freq;

	memset(&vin, 0, sizeof(struct v4l2_input));
	vin.index = 0;

	printf("============================================\nQuerying capture capabilities\n");
	while(ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0) {
		printf("Input number: %d\n", vin.index);
		printf("Name: %s\n", vin.name);
		printf("Type: (%d) ", vin.type);
		if(vin.type & V4L2_INPUT_TYPE_TUNER) {
			printf("Tuner\n");
			printf("Tuner index: %d\n", vin.tuner);
			memset(&tun, 0, sizeof(struct v4l2_tuner));
			tun.index = vin.tuner;
			if(ioctl(fd, VIDIOC_G_TUNER, &tun)==0) {
				printf("Name: %s\n", tun.name);
				if(tun.type==V4L2_TUNER_RADIO) printf("It is a RADIO tuner\n");
				if(tun.type==V4L2_TUNER_ANALOG_TV) printf("It is a TV tuner\n");
				if(tun.capability&V4L2_TUNER_CAP_LOW) printf("Frequencies in units of 62.5Hz\n");
				else printf("Frequencies in units of 62.5kHz\n");
				if(tun.capability&V4L2_TUNER_CAP_NORM) printf("Multi-standard tuner\n");
				if(tun.capability&V4L2_TUNER_CAP_STEREO) printf("Stereo reception supported\n");
				/* More flags here */
				printf("lowest tunable frequency: %.2f %s\n", tun.rangelow * 62.5, (tun.capability&V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
				printf("highest tunable frequency: %.2f %s\n", tun.rangehigh * 62.5, (tun.capability&V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
				memset(&freq, 0, sizeof(struct v4l2_frequency));
				freq.tuner = vin.tuner;
				if(ioctl(fd, VIDIOC_G_FREQUENCY, &freq)==0) {
					printf("Current frequency: %.2f %s\n", freq.frequency * 62.5, (tun.capability&V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
				}
			}
		}
		if(vin.type & V4L2_INPUT_TYPE_CAMERA) printf("Camera\n");
		printf("Supported standards: (%d) ", (int)vin.std);
		if(vin.std & V4L2_STD_PAL) printf("PAL ");
		if(vin.std & V4L2_STD_NTSC) printf("NTSC ");
		if(vin.std & V4L2_STD_SECAM) printf("SECAM ");
		printf("\n");
		vin.index++;
	}
	printf("\n");
}

static void query_frame_sizes_v4l2(int fd){
	struct v4l2_frmsizeenum frms;
	struct v4l2_fmtdesc fmtd;

	memset(&frms, 0, sizeof(struct v4l2_frmsizeenum));
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;


	printf("============================================\nQuerying supported frame sizes\n\n");
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
	}

}

static void print_v4l2_control(struct v4l2_queryctrl *qc) {
	printf( "Control: id: 0x%x - name: %s - min: %d -max: %d - step: %d - type: %d(%s) - flags: %d (%s%s%s%s%s%s)\n", \
			qc->id, (char *) &qc->name, qc->minimum, qc->maximum, qc->step, qc->type,
			qc->type == V4L2_CTRL_TYPE_INTEGER ? "Integer" :
			qc->type == V4L2_CTRL_TYPE_BOOLEAN ? "Boolean" :
			qc->type == V4L2_CTRL_TYPE_MENU ? "Menu" :
			qc->type == V4L2_CTRL_TYPE_BUTTON ? "Button" :
			qc->type == V4L2_CTRL_TYPE_INTEGER64 ? "Integer64" :
			qc->type == V4L2_CTRL_TYPE_CTRL_CLASS ? "Class" : "",
			qc->flags,
			qc->flags & V4L2_CTRL_FLAG_DISABLED ? "Disabled " : "",
			qc->flags & V4L2_CTRL_FLAG_GRABBED ? "Grabbed " : "",
			qc->flags & V4L2_CTRL_FLAG_READ_ONLY ? "ReadOnly " : "",
			qc->flags & V4L2_CTRL_FLAG_UPDATE ? "Update " : "",
			qc->flags & V4L2_CTRL_FLAG_INACTIVE ? "Inactive " : "",
			qc->flags & V4L2_CTRL_FLAG_SLIDER ? "slider " : "");
}

static void query_controls_v4l2(int fd) {
	int i;
	struct v4l2_queryctrl qctrl;
	CLEAR(qctrl);
	printf("============================================\nQuerying standard controls\n\n");
	//std ctrls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1; i++) {
		qctrl.id = i;
		if((ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0))
			print_v4l2_control(&qctrl);
	}

	printf("============================================\nQuerying private controls\n\n");
	//priv ctrls
	for (qctrl.id = V4L2_CID_PRIVATE_BASE;; qctrl.id++) {
		if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &qctrl)) {
			print_v4l2_control(&qctrl);
		} else {
			if (errno == EINVAL)
				break;

			printf( "we shouldnt be here...\n");
		}
	}

	printf("============================================\nQuerying extended controls\n\n");
	//checking extended controls
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl (fd, VIDIOC_QUERYCTRL, &qctrl)) {
		print_v4l2_control(&qctrl);
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
}



void list_cap_v4l2(int fd) {
	struct v4l2_capability cap;

	printf("============================================\nQuerying general capabilities\n\n");
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		printf("v4l2 not supported. Maybe a v4l1 device ...");
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

		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) query_capture_intf_v4l2(fd);
		// FIXME Enumerate other capabilites (output, overlay,...

		enum_image_fmt_v4l2(fd);
		query_current_image_fmt_v4l2(fd);
		query_frame_sizes_v4l2(fd);
		query_controls_v4l2(fd);

	}
}
