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
#include <stdio.h>			// for perror
#include "libvideo.h"
#include "log.h"
#include "libvideo-err.h"
#include "libvideo-palettes.h"
#include "v4l2-input.h"
#include "v4l2-query.h"
#include "v4l2-list.h"

//Arbitrary values that hopefully will never be reached
//v4l2 will adjust them to the closest available
#define V4L2_MAX_WIDTH 			4096
#define V4L2_MAX_HEIGHT 		4096

int check_v4l2(int fd, struct v4l2_capability* caps){
	return ioctl(fd, VIDIOC_QUERYCAP, caps);
}

int check_capture_capabilities_v4l2(int fd, char *file) {
	struct v4l2_capability cap;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Checking capture device\n");

	CLEAR(cap);

	if (-1 == check_v4l2(fd, &cap)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Not a V4L2 device.\n");
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		info("The device %s seems to be a valid V4L2 device but without "
				"capture capability.\n", file);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
		info("Listing the reported capabilities:\n");
		list_cap_v4l2(fd);
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		info("The device %s seems to be a valid V4L2 device with capture "
				"capability, but\n", file);
		info("the device does NOT support streaming. Please let the author "
				"know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
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

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"Trying to autodetect standard\n");

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


	//Linux UVC doesnt like to be ioctl'ed(VIDIOC_S_STD)
	//even though v4l2 specs say nothing
	//about usb cam drivers returning EINVAL...
	//so we first try VIDIOC_ENUMSTD. if it returns EINVAL, then we
	//assume it is a webcam
	//otherwise, we need to set a standard
	if(-1 != ioctl(fd, VIDIOC_ENUMSTD, &s)){
		//driver says "I use standards" -
		//check if it is the UNKNOWN one, only used by webcams
		if(s.id != V4L2_STD_UNKNOWN) {
			//driver says "not webcam", check what we want...
			if(c->std == WEBCAM){
				//we want webcam... try to autodetect
				info("The specified standard (%d) is invalid.\n", c->std);
				if(detect_standard(c, fd)!=0) {
					//autodetect failed, so do we
					info("libvideo could not autodetect a standard for this "
							"input.\n");
					return -1;
				}
				//autodetect suceeded keep going
			} else {
				//we want !WEBCAM, so try that standard
				dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
						"Trying standard (%d).\n", c->std);
				if (-1 == try_std(fd, c->std)) {
					//failed, try autodetect
					info("The specified standard (%d) cannot be selected\n",
							c->std);
					if(detect_standard(c, fd)!=0) {
						//failed, exit
						info("libvideo could not autodetect a standard for this"
								" input.\n");
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
					info("The specified standard (%d) cannot be selected\n",
							c->std);
					if(detect_standard(c, fd)!=0) {
						//failed, exit
						dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
								"CAP: Couldnt autodetect a standard for this "
							"input.\n");
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
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Setting input.\n");
		if(-1 == ioctl(fd, VIDIOC_S_INPUT, &(c->channel))) {
			info("The desired input (%d) cannot be selected.\n", c->channel);
			return -1;
		}
		vi.index = c->channel;
		if (-1 == ioctl(fd, VIDIOC_ENUMINPUT, &vi)) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: Failed to get details of input %d, errno(%d)\n",
					c->channel, errno);
			return -1;
		}
		if(vi.type == V4L2_INPUT_TYPE_TUNER)
			c->tuner_nb = vi.tuner;
		else
			c->tuner_nb = -1;
	}
	return 0;
}

static int apply_image_format(struct v4l2_format *fmt, int fd){
	int palette = fmt->fmt.pix.pixelformat;
	if (0 == ioctl(fd, VIDIOC_S_FMT, fmt)) {
		if (fmt->fmt.pix.pixelformat == palette) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_INFO,
					"CAP: palette %#x  - accepted at %dx%d\n",
					palette, fmt->fmt.pix.width, fmt->fmt.pix.height);
			return 0;
		} else {
			info("CAP: VIODIOC_S_FMT succeeded but returned a different "
				"palette from the one requested: requested palette %#x  "
				"- returned palette: %#x\n",
				palette, fmt->fmt.pix.pixelformat);
			info("This is most likely a bug in v4l4j. Please\n");
			info("let the author know about this issue. See README file.\n");
			return -1;
		}
	} else {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: palette %#x rejected (errno: %d)\n",
				palette, errno);
		perror(NULL);
		return -1;
	}
}

static int get_palette_index(int pixelformat){
	int i;
	for(i = 0; i<NB_SUPPORTED_PALETTES; i++)
		if(libvideo_palettes[i].v4l2_palette == pixelformat)
			return i;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
			"CAP: palette %#x unknown\n", pixelformat);
	return -1;
}

//this function takes in 2 struct v4l2_format and a libvideo palette index
//it will try and see if the chosen palette can be obtained, either straight
//from the driver or after conversion using libv4lconvert
//it returns the libvideo palette to use in order to get the requested
//libvideo palette, or -1 upon return, src * dst will contain meaningful values
static int try_image_format(struct capture_device *c, struct v4l2_format *src,
		struct v4l2_format *dst, int palette_idx){
	int index = -1;
	CLEAR(*src);
	CLEAR(*dst);

	dst->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dst->fmt.pix.width = c->width;
	dst->fmt.pix.height = c->height;
	dst->fmt.pix.field = V4L2_FIELD_ANY;
	dst->fmt.pix.pixelformat = libvideo_palettes[palette_idx].v4l2_palette;

	//v4lconvert_try_format may succeed(return 0) even though the given dst
	//format is not supported. In this case, the function will store a supported
	//format in dst. That s why, not only thhe return value is checked, but also
	//the format in dst
	if(0 == v4lconvert_try_format(c->convert->priv, dst, src)
			&&
		dst->fmt.pix.pixelformat == libvideo_palettes[palette_idx].v4l2_palette){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
				"CAP: For dest palette %#x (%s - %d) %dx%d - ...\n",\
				dst->fmt.pix.pixelformat,
				libvideo_palettes[palette_idx].name,
				palette_idx,
				dst->fmt.pix.width,
				dst->fmt.pix.height);
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
				"CAP: libv4lconvert said to use palette %#x %dx%d - ...\n",\
				src->fmt.pix.pixelformat,
				src->fmt.pix.width,
				src->fmt.pix.height);

		if((index = get_palette_index(src->fmt.pix.pixelformat))!= -1){
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
					"CAP: which is libvideo index %d, palette %s\n",\
					index,
					libvideo_palettes[index].name);


			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
					"CAP: libv4lconvert required ? %s\n",
					(v4lconvert_needs_conversion(
							c->convert->priv,
							src,
							dst)==0?"No":"Yes"
					)
				);

		} else {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
					"CAP: palette returned by libv4lconvert is unknown to "
					"libvideo\n");
			info("The source image format returned by libv4l_convert is ");
			info("unknown\nPlease, let the author known about this error:\n");
			info("Destination palette: %#x (%s)\n",\
						libvideo_palettes[palette_idx].v4l2_palette,
						libvideo_palettes[palette_idx].name);
			info("libv4l_convert source palette: %#x", src->fmt.pix.pixelformat);
			info("See the README file on how to submit bug reports.");
		}
	}

	return index;
}

static int find_best_palette(struct capture_device *c, int *palettes,
		int nb, int fd){
	int best_palette=-1, best_width =-1, best_height=-1, i;
	struct v4l2_format src, dst;

	for(i=0; i<nb; i++) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
				"CAP: trying palette %#x (%s) %dx%d - ...\n",
				libvideo_palettes[palettes[i]].v4l2_palette,
				libvideo_palettes[palettes[i]].name, c->width, c->height);

		if(try_image_format(c, &src, &dst, palettes[i])!=-1){

				if( (best_palette == -1) ||
				  (abs(c->width*c->height - dst.fmt.pix.width*dst.fmt.pix.height) <
							 abs(c->width*c->height - best_width*best_height)) ){
					best_palette = palettes[i];
					best_width = dst.fmt.pix.width;
					best_height = dst.fmt.pix.height;
					dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
							"CAP: palette (%s) is best palette so far\n",
							libvideo_palettes[palettes[i]].name);
				}

		} //else
		//palette rejected
	}

	return best_palette;
}


static int set_image_format(struct capture_device *c, int *palettes,
		int nb, int fd){
	int best_palette = -1, i;
	struct convert_data* convert = c->convert;

	XMALLOC(convert->src_fmt,struct v4l2_format *,sizeof(struct v4l2_format));
	XMALLOC(convert->dst_fmt,struct v4l2_format *,sizeof(struct v4l2_format));

	if(c->width==MAX_WIDTH)
		c->width=V4L2_MAX_WIDTH;
	if(c->height==MAX_HEIGHT)
		c->height=V4L2_MAX_HEIGHT;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: trying palettes (%d to try in total)\n", nb);

	//we try all the supplied palettes and find the best one that gives us
	//a resolution closest to the desired one
	best_palette = find_best_palette(c, palettes, nb, fd);

	if(best_palette == -1) {
		info("libvideo was unable to find a suitable palette. The following "
				"palettes have been tried and failed:\n");
		for(i=0; i<nb;i++)
			info("%s\n",libvideo_palettes[palettes[i]].name);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
		return -1;
	} else {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Setting to best palette %s...\n",
				libvideo_palettes[best_palette].name);

		convert->src_palette = try_image_format(
				c, convert->src_fmt, convert->dst_fmt, best_palette);

		if (0 == apply_image_format(convert->src_fmt, fd)) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
					"CAP: setting src palette (%s) accepted\n",
					libvideo_palettes[convert->src_palette].name);
			c->palette = best_palette;
		} else {
			info("Unable to set the best detected palette: %s\n",
					libvideo_palettes[best_palette].name);
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libvideo README file.\n");
			return -1;
		}
	}

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: capturing (src) using width: %d, "
			" height: %d, bytes/line %d, image size: %d - palette: %d (%s)\n",
			convert->src_fmt->fmt.pix.width,
			convert->src_fmt->fmt.pix.height,
			convert->src_fmt->fmt.pix.bytesperline,
			convert->src_fmt->fmt.pix.sizeimage,
			convert->src_palette,
			libvideo_palettes[convert->src_palette].name
			);
	c->is_native = v4lconvert_needs_conversion(
			convert->priv,convert->src_fmt, convert->dst_fmt)==1?0:1;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: libv4lconvert required ? %s\n",
			(c->is_native==0?"Yes":"No"));

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: conv to (dst) width: %d, "
			"height: %d, bytes/line %d, image size: %d - palette: %d (%s)\n",
			convert->dst_fmt->fmt.pix.width,
			convert->dst_fmt->fmt.pix.height,
			convert->dst_fmt->fmt.pix.bytesperline,
			convert->dst_fmt->fmt.pix.sizeimage,
			c->palette,
			libvideo_palettes[c->palette].name
	);

	//Store actual width & height
	c->width = convert->dst_fmt->fmt.pix.width;
	c->height= convert->dst_fmt->fmt.pix.height;
	if(c->is_native==1){
		//if no need for conversion, libv4lconvert sometimes returns 0 in
		//sizeimage and bytesperline fields, so get values from src palette
		c->imagesize = convert->src_fmt->fmt.pix.sizeimage;
	} else {
		c->imagesize = convert->dst_fmt->fmt.pix.sizeimage;
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
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: Error setting cropping info\n");
			// dont fail if cropping fails
			//return -1;
		}
	}

	return 0;
}

int set_frame_intv_v4l2(struct video_device *vdev, int num, int denom) {
	struct v4l2_streamparm param;
	int ret;

	CLEAR(param);
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	param.parm.capture.timeperframe.numerator = num;
	param.parm.capture.timeperframe.denominator = denom;
	ret = ioctl(vdev->fd, VIDIOC_S_PARM, &param);

	switch (ret){
	case 0:
		//drivers are allowed to return 0 and still
		//change the given frame rate. so we check it here
		//and make sure it s the same as the one we want
		if((param.parm.capture.timeperframe.numerator == num) &&
				(param.parm.capture.timeperframe.denominator == denom)) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Set frame interval to %d/%d\n", num, denom);
			return 0;
		} else {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: failed to set frame interval to %d/%d "
				"(driver changed it to %d/%d)\n", num, denom,
				param.parm.capture.timeperframe.numerator,
				param.parm.capture.timeperframe.denominator);
			return LIBVIDEO_ERR_FORMAT;
		}
	case EINVAL:
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Error IOCTL while setting frame interval to %d/%d\n", num, denom);
		return LIBVIDEO_ERR_IOCTL;
	default:
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Error FORMAT while setting frame interval to %d/%d\n", num, denom);
		return LIBVIDEO_ERR_FORMAT;
	}
}

int get_frame_intv_v4l2(struct video_device *vdev, int *num, int *denom) {
	struct v4l2_streamparm param;

	CLEAR(param);
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(vdev->fd, VIDIOC_G_PARM, &param)==0){
		*num = param.parm.capture.timeperframe.numerator;
		*denom = param.parm.capture.timeperframe.denominator;
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Get frame interval returned %d/%d\n", *num, *denom);
		return 0;
	} else
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Error while getting frame interval\n");
		return LIBVIDEO_ERR_IOCTL;
}

int set_cap_param_v4l2(struct video_device *vdev, int *palettes, int nb) {
	struct capture_device *c = vdev->capture;
	int ret = 0;
	int def[NB_SUPPORTED_PALETTES] = DEFAULT_PALETTE_ORDER;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Setting capture parameters on device %s.\n", vdev->id.device_handle);

	if(nb<0 || nb>=NB_SUPPORTED_PALETTES) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Incorrect number of palettes (%d)\n", nb);
		return LIBVIDEO_ERR_FORMAT;
	}
	if(nb==0 || palettes==NULL) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: No palettes supplied, trying default order.\n");
		palettes = def;
		nb = NB_SUPPORTED_PALETTES;
	}

	//set desired standard
	if (set_std(c, vdev->fd) !=0 ) {
		ret = LIBVIDEO_ERR_STD;
		goto fail;
	}

	//set desired input
	if (set_input(c, vdev->fd) != 0) {
		ret = LIBVIDEO_ERR_CHANNEL;
		goto fail;
	}

	//Set image format
	if (set_image_format(c, palettes, nb, vdev->fd) != 0) {
		ret = LIBVIDEO_ERR_FORMAT;
		goto fail;
	}

	//Set crop format
	if (set_crop(c, vdev->fd) != 0) {
		info("Listing the reported capabilities:\n");
		ret = LIBVIDEO_ERR_CROP;
		goto fail;
	}

	//set FPS
	//set_param(c, vdev->fd);

	return ret;

fail:
	info("Listing the reported capabilities:\n");
	list_cap_v4l2(vdev->fd);
	return ret;
}

//needed because this function adjusts the struct capture_action if
//libv4l_convert is required.
void *dequeue_buffer_v4l2_convert(struct video_device *, int *);
int init_capture_v4l2(struct video_device *vdev) {
	struct capture_device *c = vdev->capture;
	struct mmap *m = c->backend->mmap;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	int i=0;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Initialising capture on device %s.\n", vdev->id.device_handle);

	CLEAR(req);

	//allocates v4l2 buffers
	req.count = m->req_buffer_nr;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: asking for %d V4L2 buffers\n", req.count);

	if (-1 == ioctl (vdev->fd, VIDIOC_REQBUFS, &req)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Error getting mmap information from driver\n");
		return LIBVIDEO_ERR_REQ_MMAP;
	}

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: driver said %d V4L2 buffers\n", req.count);
	c->buffer_nr = req.count;
	XMALLOC( m->buffers, struct mmap_buffer *,
			(c->buffer_nr * sizeof(struct mmap_buffer)) );

	for(i=0; i<c->buffer_nr ; i++) {
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == ioctl (vdev->fd, VIDIOC_QUERYBUF, &buf)){
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: cant query allocated V4L2 buffers\n");
			return LIBVIDEO_ERR_REQ_MMAP_BUF;
		}

		m->buffers[i].length = buf.length;
		m->buffers[i].start = mmap(NULL,
				buf.length,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				vdev->fd,
				(off_t) buf.m.offset);

		if (MAP_FAILED == m->buffers[i].start) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: cant mmap allocated V4L2 buffers\n");
			return LIBVIDEO_ERR_MMAP_BUF;
		}

		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: mmap'ed %d bytes at %p\n",m->buffers[i].length,
				m->buffers[i].start);
	}

	if(vdev->capture->is_native!=1){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: need conversion, create temp buffer (%d bytes)\n",
				vdev->capture->convert->dst_fmt->fmt.pix.sizeimage);
		XMALLOC(vdev->capture->convert->frame, void *,
				vdev->capture->convert->dst_fmt->fmt.pix.sizeimage);
		vdev->capture->actions->dequeue_buffer = dequeue_buffer_v4l2_convert;
	}

	return 0;
}

int start_capture_v4l2(struct video_device *vdev) {

	int i;
	struct v4l2_buffer *b;
	XMALLOC(b,struct v4l2_buffer *, sizeof(struct v4l2_buffer));

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Starting capture on device %s.\n", vdev->id.device_handle);

	//Enqueue all buffers
	for(i=0; i< vdev->capture->buffer_nr; i++) {
		CLEAR(*b);
		b->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		b->memory = V4L2_MEMORY_MMAP;
		b->index = i;
		if(-1 == ioctl(vdev->fd, VIDIOC_QBUF, b)) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: cannot enqueue initial buffers\n");
			return LIBVIDEO_ERR_IOCTL;
		}
	}

	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(vdev->fd, VIDIOC_STREAMON, &i) < 0 ){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: cannot start capture\n");
		return LIBVIDEO_ERR_IOCTL;
	}
	vdev->capture->backend->mmap->tmp = (void *) b;

	return 0;
}

//needed because dequeue may need to re-enqueue if libv4lconvert fails
void enqueue_buffer_v4l2(struct video_device *);
void *dequeue_buffer_v4l2_convert(struct video_device *vdev, int *len) {
	struct convert_data *conv = vdev->capture->convert;
	struct v4l2_buffer *b = (struct v4l2_buffer *) vdev->capture->backend->mmap->tmp;
	int try = 2;
	*len = -1;
	while(*len==-1){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
				"CAP: dequeuing buffer on device %s.\n", vdev->id.device_handle);

		CLEAR(*b);

		b->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		b->memory = V4L2_MEMORY_MMAP;

		if (-1 == ioctl(vdev->fd, VIDIOC_DQBUF, b)) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: Error dequeuing buffer (%d)\n", errno);
			return NULL;
		}

		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
				"CAP: Passing buffer %d of len %d at %p with format %#x"
				" to be stored in buffer at %p of length %d with format %#x\n",
				b->index, b->bytesused,
				vdev->capture->backend->mmap->buffers[b->index].start,
				conv->src_fmt->fmt.pix.pixelformat,
				conv->frame, conv->dst_fmt->fmt.pix.sizeimage,
				conv->dst_fmt->fmt.pix.pixelformat
				);

		*len=v4lconvert_convert(conv->priv, conv->src_fmt, conv->dst_fmt,
				vdev->capture->backend->mmap->buffers[b->index].start, b->bytesused,
				conv->frame, conv->dst_fmt->fmt.pix.sizeimage);

		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
								"CAP: dest buffer has %d bytes after conv\n",
						*len
						);

		if(*len==-1){
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
						"CAP: libv4lconvert error: %s\n",
						v4lconvert_get_error_message(conv->priv));
			enqueue_buffer_v4l2(vdev);
			if(try--==0){
				dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
						"CAP: unable to libv4l_convert frame\n");
				//give up
				return NULL;
			}
		}
	}
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: after conversion buffer length: %d\n", *len);
	return conv->frame;
}

void *dequeue_buffer_v4l2(struct video_device *vdev, int *len) {
	struct v4l2_buffer *b = (struct v4l2_buffer *) vdev->capture->backend->mmap->tmp;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: dequeuing buffer on device %s.\n", vdev->id.device_handle);

	CLEAR(*b);

	b->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b->memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(vdev->fd, VIDIOC_DQBUF, b)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: error dequeuing buffer\n");
		return NULL;
	}

	*len = b->bytesused;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: buffer length: %d\n", *len);
	return vdev->capture->backend->mmap->buffers[b->index].start;
}

void enqueue_buffer_v4l2(struct video_device *vdev) {
	struct v4l2_buffer *b = (struct v4l2_buffer *) vdev->capture->backend->mmap->tmp;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: queuing buffer on device %s.\n", vdev->id.device_handle);
	if (-1 == ioctl(vdev->fd, VIDIOC_QBUF, b))
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: error queuing buffer\n");
}


int stop_capture_v4l2(struct video_device *vdev) {
	int i;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: stopping capture on device %s.\n", vdev->id.device_handle);

	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(vdev->fd, VIDIOC_STREAMOFF, &i ) < 0 ){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: cannot stop capture\n");
		return LIBVIDEO_ERR_IOCTL;
	}
	XFREE(vdev->capture->backend->mmap->tmp);

	return 0;
}

void free_capture_v4l2(struct video_device *vdev) {
	int i = 0;
	struct v4l2_requestbuffers req;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: freeing capture structure on device %s.\n", vdev->id.device_handle);

	// free temp frame buffer if required
	if(vdev->capture->is_native!=1)
		XFREE(vdev->capture->convert->frame);

	// unmmap v4l2 buffers
	for(i=0; i < vdev->capture->buffer_nr; i++){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: unmmap %d bytes at %p\n",
				vdev->capture->backend->mmap->buffers[i].length,
				vdev->capture->backend->mmap->buffers[i].start);

		if (-1 == munmap(vdev->capture->backend->mmap->buffers[i].start,
				(size_t) vdev->capture->backend->mmap->buffers[i].length))
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: error unmapping buffer %d\n",i);
	}

	// free buffers struct
	XFREE(vdev->capture->backend->mmap->buffers);

	// fix for issue reported on v4l4j ML:
	// http://groups.google.com/group/v4l4j/browse_thread/thread/a80cb345876acf76
	// summary: "v4l2 specs say VIDIOC_S_FMT can fail if rsrcs are currently
	// being used". In this case, this means if a previous capture has currently
	// already mmap'ed v4l2 buffers, VIDIOC_S_FMT will fail.
	// so the workaround is to try and release v4l2 buffers after un'mmap'ping
	// them. According to v4l2 specs, releasing buffers can be done by requesting
	// 0 buffers using VIDIOC_REQBUFS. Problem is: as of today, not all drivers
	// support this. So for now, we try it anyway and ignore the returned value.
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Applying workaround - releasing v4l2 buffers\n");

	CLEAR(req);

	//allocates v4l2 buffers
	req.count = 0;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl (vdev->fd, VIDIOC_REQBUFS, &req)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Error releasing v4l2 buffers\n");

	}

	XFREE(vdev->capture->convert->dst_fmt);
	XFREE(vdev->capture->convert->src_fmt);
}


