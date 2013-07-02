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

#include <limits.h>
#include <sys/ioctl.h>		//for ioctl
#include <sys/mman.h>		//for mmap
#include <sys/time.h>		//for struct timeval
#include <errno.h>			//for errno
#include <string.h>			//for memcpy
#include <stdio.h>			// for perror
#include "libvideo.h"
#include "log.h"
#include "libvideo-err.h"
#include "libvideo-palettes.h"
#include "v4l2-input.h"
#include "v4l2-query.h"

//Arbitrary values that hopefully will never be reached
//v4l2 will adjust them to the closest available
#define V4L2_MAX_WIDTH 			4096
#define V4L2_MAX_HEIGHT 		4096

#ifndef USEC_PER_SEC
#define USEC_PER_SEC			1000000
#endif

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

static int set_image_format(struct capture_device *c, int native_palette,
		int output_palette, int fd){
	int force_native_fmt = 0;
	struct v4l2_format test_fmt;
	XMALLOC(c->convert->src_fmt,struct v4l2_format *,sizeof(struct v4l2_format));
	XMALLOC(c->convert->dst_fmt,struct v4l2_format *,sizeof(struct v4l2_format));

	if(c->width==MAX_WIDTH)
		c->width=V4L2_MAX_WIDTH;
	if(c->height==MAX_HEIGHT)
		c->height=V4L2_MAX_HEIGHT;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Applying palette %s...\n",
			libvideo_palettes[output_palette].name);

	c->convert->src_palette = try_image_format(
			c, c->convert->src_fmt, c->convert->dst_fmt, output_palette);
	if ((native_palette != -1) && (c->convert->src_palette != native_palette)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1, "Asked to use native palette"
				" '%s' instead - Overriding libv4lconvert's choice\n", libvideo_palettes[native_palette].name);
		c->convert->src_palette = native_palette;
		c->convert->src_fmt->fmt.pix.pixelformat = libvideo_palettes[native_palette].v4l2_palette;
	}

	// copy the source format we have to use in order to produce 'best_palette'
	// and try to use this format through libv4lconvert.
	// Sometimes libv4lconvert will refuse to use this format for capture (even
	// though we are about to tell the driver to use it), because libv4lconvert
	// cannot return captured images in this format. For instance:
	// PAC7311-based cameras produces images in Pixart JPEG (PJPG) format. when
	// enumerating supported formats, libv4lconvert will advertises RGB24,
	// BGR24, YUV420 and YVU420 but not PJPG. In other words, we (libvideo)
	// would not have heard of PJPG if it was not for the fact that
	// v4lconvert_try_format(dest=RGB24, src=...) does indeed return PJGP as
	// the src format to be used in order to get RGB24. In other words, PJPG is not
	// advertised at all as a native format and v4l2-query.c picked up on that
	// and did not advertise PJPG at all. Instead, RGB24, BGR24, YUV420 and YVU420
	// are all advertised as native formats (which is not entirely true, but as
	// a user of libv4lconvert, that's what we are told). So we need to implement
	// this logic here (as we did in v4l2-query.c):
	// Try to use the source format for capture and see if it is possible
	// If it isnt, 'best_palette' is considered a native format.
	test_fmt = *c->convert->src_fmt;
	v4lconvert_try_format(c->convert->priv, &test_fmt, NULL);
	if(test_fmt.fmt.pix.pixelformat != c->convert->src_fmt->fmt.pix.pixelformat)
		force_native_fmt = 1;

	if (0 == apply_image_format(c->convert->src_fmt, fd)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
				"CAP: setting src palette (%s) accepted\n",
				libvideo_palettes[c->convert->src_palette].name);
		c->palette = output_palette;
	} else {
		info("Unable to set the palette: %s\n",
				libvideo_palettes[c->convert->src_palette].name);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
		return -1;
	}


	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: capturing (src) using width: %d, "
			" height: %d, bytes/line %d, image size: %d - palette: %d (%s)\n",
			c->convert->src_fmt->fmt.pix.width,
			c->convert->src_fmt->fmt.pix.height,
			c->convert->src_fmt->fmt.pix.bytesperline,
			c->convert->src_fmt->fmt.pix.sizeimage,
			c->convert->src_palette,
			libvideo_palettes[c->convert->src_palette].name
			);
	c->needs_conversion = v4lconvert_needs_conversion(
			c->convert->priv,c->convert->src_fmt, c->convert->dst_fmt);
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: libv4lconvert required ? %s\n",
			(c->needs_conversion==1?"Yes":"No"));

	c->is_native = ((force_native_fmt!=0) || (c->needs_conversion==0)) ? 1 : 0;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: conv to (dst) width: %d, "
			"height: %d, bytes/line %d, image size: %d - palette: %d (%s) - native format: %s\n",
			c->convert->dst_fmt->fmt.pix.width,
			c->convert->dst_fmt->fmt.pix.height,
			c->convert->dst_fmt->fmt.pix.bytesperline,
			c->convert->dst_fmt->fmt.pix.sizeimage,
			c->palette,
			libvideo_palettes[c->palette].name,
			(c->is_native==1?"Yes":"No")
	);

	//Store actual width & height
	c->width = c->convert->dst_fmt->fmt.pix.width;
	c->height= c->convert->dst_fmt->fmt.pix.height;
	if(c->needs_conversion==0){
		//if no need for conversion, libv4lconvert sometimes returns 0 in
		//sizeimage and bytesperline fields, so get values from src palette
		c->imagesize = c->convert->src_fmt->fmt.pix.sizeimage;
	} else {
		c->imagesize = c->convert->dst_fmt->fmt.pix.sizeimage;
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

void get_video_input_std_v4l2(struct video_device *vdev, int *input_num, int *std) {
	*input_num = vdev->capture->channel;
	*std = vdev->capture->std;
}

int set_video_input_std_v4l2(struct video_device *vdev, int input_num, int std) {
	if(-1 == ioctl(vdev->fd, VIDIOC_S_INPUT, &input_num)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR, 
			"The desired input (%d) cannot be selected.\n", input_num);
		return LIBVIDEO_ERR_CHANNEL;
	}

	vdev->capture->channel = input_num;

	if (try_std(vdev->fd, std) != 0) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR, 
			"The desired standard (%d) cannot be used.\n", std);
		return LIBVIDEO_ERR_STD;

	}

	vdev->capture->std = std;

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

int set_cap_param_v4l2(struct video_device *vdev, int src_palette, int dest_palette) {
	struct capture_device *c = vdev->capture;
	int ret = 0;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Setting capture parameters on device %s.\n", vdev->file);

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
	if (src_palette != -1) {
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
					"Native format '%s' requested\n", libvideo_palettes[src_palette].name);
	}

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
					"Output format '%s' requested\n", libvideo_palettes[dest_palette].name);

	if (set_image_format(c, src_palette, dest_palette, vdev->fd) != 0) {
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

int init_capture_v4l2(struct video_device *vdev) {
	struct capture_device *c = vdev->capture;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	int i=0;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Initialising capture on device %s.\n", vdev->file);

	CLEAR(req);

	//allocates v4l2 buffers
	req.count = c->mmap->req_buffer_nr;
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
	c->mmap->buffer_nr = req.count;
	XMALLOC( c->mmap->buffers, struct mmap_buffer *,
			(c->mmap->buffer_nr * sizeof(struct mmap_buffer)) );

	for(i=0; i<c->mmap->buffer_nr ; i++) {
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == ioctl (vdev->fd, VIDIOC_QUERYBUF, &buf)){
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: cant query allocated V4L2 buffers\n");
			return LIBVIDEO_ERR_REQ_MMAP_BUF;
		}

		c->mmap->buffers[i].length = buf.length;
		c->mmap->buffers[i].start = mmap(NULL,
				buf.length,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				vdev->fd,
				(off_t) buf.m.offset);

		if (MAP_FAILED == c->mmap->buffers[i].start) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: cant mmap allocated V4L2 buffers\n");
			return LIBVIDEO_ERR_MMAP_BUF;
		}

		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: mmap'ed %d bytes at %p\n",c->mmap->buffers[i].length,
				c->mmap->buffers[i].start);
	}

	return 0;
}

int start_capture_v4l2(struct video_device *vdev) {
	int i;
	struct v4l2_buffer b;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Starting capture on device %s.\n", vdev->file);

	//Enqueue all buffers
	for(i=0; i< vdev->capture->mmap->buffer_nr; i++) {
		CLEAR(b);
		b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		b.memory = V4L2_MEMORY_MMAP;
		b.index = i;
		if(-1 == ioctl(vdev->fd, VIDIOC_QBUF, &b)) {
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

	return 0;
}

unsigned int convert_buffer_v4l2(struct video_device *vdev, int index,
		unsigned int src_len, void *dest_buffer) {
	struct convert_data *conv = vdev->capture->convert;
	unsigned int dest_buffer_len = -1;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: Passing buffer %d of len %d at %p with format %#x"
			" to be stored in buffer at %p of length %d with format %#x\n",
			index, src_len,
			vdev->capture->mmap->buffers[index].start,
			conv->src_fmt->fmt.pix.pixelformat,
			dest_buffer, conv->dst_fmt->fmt.pix.sizeimage,
			conv->dst_fmt->fmt.pix.pixelformat
			);

	START_TIMING;
	dest_buffer_len=v4lconvert_convert(conv->priv, conv->src_fmt, conv->dst_fmt,
			vdev->capture->mmap->buffers[index].start, src_len,
			dest_buffer, conv->dst_fmt->fmt.pix.sizeimage);
	END_TIMING("libvideo conversion took ");

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: dest buffer has %d bytes after conv\n",dest_buffer_len
	);

	return dest_buffer_len;
}

void *dequeue_buffer_v4l2(struct video_device *vdev, int *len,
		unsigned int *index, unsigned long long *capture_time,
		unsigned long long *sequence) {
	struct v4l2_buffer b;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: dequeuing buffer on device %s.\n", vdev->file);

	CLEAR(b);

	b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(vdev->fd, VIDIOC_DQBUF, &b)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: error dequeuing buffer\n");
		return NULL;
	}

	// return buffer metadata
	*len = b.bytesused; // it is an error if len or index are NULL
	*index = b.index;
	if (capture_time)
		*capture_time = b.timestamp.tv_usec + b.timestamp.tv_sec * USEC_PER_SEC ;
	if (sequence)
		*sequence = b.sequence;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: dequeued buffer %d length: %d - seq: %lu - time %llu\n",
			*index,	*len, (unsigned long) b.sequence,
			(unsigned long long)(b.timestamp.tv_usec + b.timestamp.tv_sec * USEC_PER_SEC ));


	return vdev->capture->mmap->buffers[b.index].start;
}

void enqueue_buffer_v4l2(struct video_device *vdev, unsigned int index) {
	struct v4l2_buffer b;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: queuing buffer %d on device %s.\n", index, vdev->file);

	CLEAR(b);
	b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b.memory = V4L2_MEMORY_MMAP;
	b.index = index;

	if (-1 == ioctl(vdev->fd, VIDIOC_QBUF, &b))
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: error queuing buffer\n");
}


int stop_capture_v4l2(struct video_device *vdev) {
	int i;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: stopping capture on device %s.\n", vdev->file);

	i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(vdev->fd, VIDIOC_STREAMOFF, &i ) < 0 ){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: cannot stop capture\n");
		return LIBVIDEO_ERR_IOCTL;
	}

	return 0;
}

void free_capture_v4l2(struct video_device *vdev) {
	int i = 0;
	struct v4l2_requestbuffers req;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: freeing capture structure on device %s.\n", vdev->file);

	// unmmap v4l2 buffers
	for(i=0; i < vdev->capture->mmap->buffer_nr; i++){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: unmmap %d bytes at %p\n",
				vdev->capture->mmap->buffers[i].length,
				vdev->capture->mmap->buffers[i].start);

		if (-1 == munmap(vdev->capture->mmap->buffers[i].start,
				(size_t) vdev->capture->mmap->buffers[i].length))
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: error unmapping buffer %d\n",i);
	}

	// free buffers struct
	XFREE(vdev->capture->mmap->buffers);

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
		//if(ioctl(vdev->fd, VIDIOC_QUERYCTRL, &qctrl) == 0) {
		if(v4lconvert_vidioc_queryctrl(vdev->control->priv, &qctrl) == 0) {
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
					qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS)
				continue;
			count++;
			add_node(&list, i);
		}
	}
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found %d std ctrls\n", count);

	//priv ctrls
	for (qctrl.id = V4L2_CID_PRIVATE_BASE;; qctrl.id++) {
		//if (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
		if(v4lconvert_vidioc_queryctrl(vdev->control->priv, &qctrl) == 0) {
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
					qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS)
				continue;
			count++;
			add_node(&list, qctrl.id);
		} else {
			if (errno == EINVAL)
				break;

			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: we shouldnt be here...\n");
		}
	}
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found %d std/priv ctrls\n", count);

	//checking extended controls
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	//while (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
	while(0 == v4lconvert_vidioc_queryctrl(vdev->control->priv, &qctrl)) {
		if(!has_id(list,qctrl.id) && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				&& qctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: found unique ext ctrl\n");
			count++;
		} else {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: found duplicate ext ctrl\n");
		}
		if(qctrl.id<=current){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: found buggy driver\n");
			qctrl.id++;
		}
		current = qctrl.id & ~V4L2_CTRL_FLAG_NEXT_CTRL;
    	qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	empty_list(list);
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found %d std/priv/ext controls\n", count);
	return count;
}

static void set_menu(struct v4l2_querymenu *q, int id, int idx, char *val){
	q->id = id;
	q->index = idx;
	snprintf((char *)q->name, 32, "%s", val);
}

static void set_query_menu(struct video_device *vd, struct control *c){
	int i, count = 0, idx = 0, id = c->v4l2_ctrl->id;
	struct v4l2_querymenu qm, *q;
	CLEAR(qm);
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Setting menu for control %#x\n", c->v4l2_ctrl->id);

	//check for known menu controls and put in some sensible menu items
	if(id == V4L2_CID_POWER_LINE_FREQUENCY){
		//populate struct control->v4l2_querymenu
		XMALLOC(q, struct v4l2_querymenu *, 3*sizeof(struct v4l2_querymenu));
		set_menu(&q[count++], id, V4L2_CID_POWER_LINE_FREQUENCY_DISABLED, "Disabled");
		set_menu(&q[count++], id, V4L2_CID_POWER_LINE_FREQUENCY_50HZ, "50 Hz");
		set_menu(&q[count++], id, V4L2_CID_POWER_LINE_FREQUENCY_60HZ, "60 Hz");
	} else if(id == V4L2_CID_COLORFX){
		XMALLOC(q, struct v4l2_querymenu *, 3*sizeof(struct v4l2_querymenu));
		set_menu(&q[count++], id, V4L2_COLORFX_NONE, "None");
		set_menu(&q[count++], id, V4L2_COLORFX_BW, "Black/White");
		set_menu(&q[count++], id, V4L2_COLORFX_SEPIA, "Sepia");
	} else if(id == V4L2_CID_MPEG_STREAM_TYPE){
		XMALLOC(q, struct v4l2_querymenu *, 6*sizeof(struct v4l2_querymenu));
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_PS, "MPEG2 - Program stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_TS, "MPEG2 - Transport stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG1_SS, "MPEG1 - System stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_DVD, "MPEG2 - DVD-compatible stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG1_VCD, "MPEG1 - VCD-compatible stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_SVCD, "MPEG2 - SVCD-compatible stream");
	} else if(id == V4L2_CID_EXPOSURE_AUTO){
		XMALLOC(q, struct v4l2_querymenu *, 4*sizeof(struct v4l2_querymenu));
		set_menu(&q[count++], id, V4L2_EXPOSURE_AUTO, "Auto");
		set_menu(&q[count++], id, V4L2_EXPOSURE_MANUAL, "Manual");
		set_menu(&q[count++], id, V4L2_EXPOSURE_SHUTTER_PRIORITY, "Shutter priority");
		set_menu(&q[count++], id, V4L2_EXPOSURE_APERTURE_PRIORITY, "Aperture priority");
		//TODO: finish this by addinng all the MPEG-specific menus
/*
	} else if(id == V4L2_CID_COLORFX){
		XMALLOC(q, struct v4l2_querymenu *, 3);
		set_menu(&q[count++], id, V4L2_COLORFX_NONE, "None");
		set_menu(&q[count++], id, V4L2_COLORFX_BW, "Black/White");
		set_menu(&q[count++], id, V4L2_COLORFX_SEPIA, "Sepia");
	} else if(id == V4L2_CID_COLORFX){
		XMALLOC(q, struct v4l2_querymenu *, 3);
		set_menu(&q[count++], id, V4L2_COLORFX_NONE, "None");
		set_menu(&q[count++], id, V4L2_COLORFX_BW, "Black/White");
		set_menu(&q[count++], id, V4L2_COLORFX_SEPIA, "Sepia");
*/
	} else {
		//Not defined in the videodev2 header

		//count how many menus there are
		qm.id = id;
		for(i = c->v4l2_ctrl->minimum; i<=c->v4l2_ctrl->maximum; i++){
			qm.index = i;
			if(ioctl(vd->fd, VIDIOC_QUERYMENU, &qm) == 0){
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
						"CTRL: found menu item %s - %d\n", qm.name, qm.index);
				count++;
			}
		}

		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
				"CTRL: found %d menus\n", count);

		if(count>0){
			//populate struct control->v4l2_querymenu
			XMALLOC(q, struct v4l2_querymenu *, count * sizeof(struct v4l2_querymenu));

			for(i = c->v4l2_ctrl->minimum; i<=c->v4l2_ctrl->maximum; i++){
				q[idx].id = id;
				q[idx].index = i;
				if(ioctl(vd->fd, VIDIOC_QUERYMENU, &q[idx]) == 0)
					idx++;
			}
		} else {
			//sometimes, nothing is returned by ioctl(VIDIOC_QUERYMENU),
			//but the menu still exists and is
			//made of contiguous values between minimum and maximum.
			count = (c->v4l2_ctrl->maximum - c->v4l2_ctrl->minimum)/
					c->v4l2_ctrl->step + 1;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
					"CTRL: creating %d fake menus\n", count);
			XMALLOC(q, struct v4l2_querymenu *,count*sizeof(struct v4l2_querymenu));
			for(i = c->v4l2_ctrl->minimum;
				i<=c->v4l2_ctrl->maximum;
				i+=c->v4l2_ctrl->step){
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
						"CTRL: menu %d - val: %d\n", idx, i);
				q[idx].id = c->v4l2_ctrl->id;
				sprintf((char *) q[idx].name, "%d", i);
				q[idx++].index = i;
			}
		}
	}
	c->count_menu = count;
	c->v4l2_menu = q;

}

static void fix_quirky_struct(struct v4l2_queryctrl *v){
	if(v->type==V4L2_CTRL_TYPE_INTEGER) {
		if(v->step==0){
			v->step = 1;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for INTEGER control '%s' "
					"from 0 to 1\n", v->name);
		}
		// Make sure min < max
		if (v->minimum > v->maximum) {
			int temp;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: ctl '%s' has min %d > max: %d - swapped min/max\n",
					v->name, v->minimum, v->maximum);
			temp = v->maximum;
			v->maximum = v->minimum;
			v->minimum = temp;
		}

	} else if((v->type==V4L2_CTRL_TYPE_MENU) || (v->type == V4L2_CTRL_TYPE_INTEGER_MENU)) {
		//Not sure about this one...
		if(v->step==0){
			v->step = 1;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for MENU control '%s' "
					"from 0 to 1\n", v->name);
		}
	} else if(v->type==V4L2_CTRL_TYPE_BOOLEAN) {
		if(v->step!=1){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for BOOLEAN control '%s' "
					"from %d to 1\n", v->name, v->step);
			v->step = 1;
		}
		if(v->minimum!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted minimum value for BOOLEAN control "
					"'%s' from %d to 0\n", v->name, v->minimum);
			v->minimum = 0;
		}
		if(v->maximum!=1){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted maximum value for BOOLEAN control "
					"'%s' from %d to 1\n", v->name, v->maximum);
			v->maximum = 1;
		}
	} else if(v->type==V4L2_CTRL_TYPE_BUTTON) {
		if(v->step!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for BUTTON control "
					"'%s' from %d to 0\n", v->name, v->step);
			v->step = 0;
		}
		if(v->minimum!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted minimum value for BUTTON control "
					"'%s' from %d to 0\n", v->name, v->minimum);
			v->minimum = 0;
		}
		if(v->maximum!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted maximum value for BUTTON control "
					"'%s' from %d to 0\n", v->name, v->maximum);
			v->maximum = 0;
		}
	} else if(v->type==V4L2_CTRL_TYPE_INTEGER64) {
		// The step, min and max cannot be queried, so hardcode them to sensible values
		v->step = 1;
		v->minimum = 0;
		v->maximum = 0;
	} else if(v->type==V4L2_CTRL_TYPE_BITMASK) {
		if (v->minimum != 0) {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: QUIRK: adjusted minimum for BITMASK control\n");
			v->minimum = 0;
		}
	}
}

//Populate the control_list with reported V4L2 controls
//and return how many controls were created
int create_v4l2_controls(struct video_device *vdev, struct control *controls,
		int max){
	struct v4l2_queryctrl qctrl;
	node *list=NULL;
	int count = 0, i, current = 0;

	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Creating std controls\n");

	//create standard V4L controls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1 && count < max; i++) {
		controls[count].v4l2_ctrl->id = i;

		if(v4lconvert_vidioc_queryctrl(vdev->control->priv, controls[count].v4l2_ctrl) == 0) {
			if ( !(controls[count].v4l2_ctrl->flags & V4L2_CTRL_FLAG_DISABLED)&&
					controls[count].v4l2_ctrl->type!=V4L2_CTRL_TYPE_CTRL_CLASS){
				fix_quirky_struct(controls[count].v4l2_ctrl);
				if((controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_MENU) ||
					(controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_INTEGER_MENU))
					set_query_menu(vdev, &controls[count]);
				count++;
				add_node(&list, i);
			}

			dprint_v4l2_control(controls[count-1]);
		}
	}

	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Creating priv controls (found %d std ctrl)\n", count);
	//create device-specific private V4L2 controls
	for (i = V4L2_CID_PRIVATE_BASE;count < max; i++) {
		controls[count].v4l2_ctrl->id = i;
		//if (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, controls[count].v4l2_ctrl)) {
		if(v4lconvert_vidioc_queryctrl(vdev->control->priv, controls[count].v4l2_ctrl) == 0) {
			if( ! (controls[count].v4l2_ctrl->flags & V4L2_CTRL_FLAG_DISABLED) &&
					controls[count].v4l2_ctrl->type!=V4L2_CTRL_TYPE_CTRL_CLASS){
				fix_quirky_struct(controls[count].v4l2_ctrl);
				if((controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_MENU) ||
					(controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_INTEGER_MENU))
					set_query_menu(vdev, &controls[count]);
				count++;
				add_node(&list, i);
			}

			dprint_v4l2_control(controls[count-1]);
    	} else {
            if (errno == EINVAL)
            	break;

            dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
            		"CTRL: we shouldnt be here...\n");
    	}
	}

	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Creating ext controls (created %d std/priv ctrl so far)\n",
			count);
	//create ext ctrls
	//TODO Add support for group-changes of extended controls. For now,
	//TODO reported ext ctrl can only be changed one at a time.
	//TODO add an extra method that list ext (so move the following to the new
	//TODO method) so apps are aware of which ctrls are
	//TODO extended ones, and can decide whether or not to change multiple ctrls
	//TODO at once or not
	//checking extended controls
	CLEAR(qctrl);
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

	while (v4lconvert_vidioc_queryctrl(vdev->control->priv, &qctrl) == 0) {
		if(!has_id(list,qctrl.id ) && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				&& qctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS ){
			CLEAR(*controls[count].v4l2_ctrl);
			memcpy(controls[count].v4l2_ctrl, &qctrl,sizeof(struct v4l2_queryctrl));
			fix_quirky_struct(controls[count].v4l2_ctrl);
			if((controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_MENU) ||
				(controls[count].v4l2_ctrl->type == V4L2_CTRL_TYPE_INTEGER_MENU))
				set_query_menu(vdev, &controls[count]);
			dprint_v4l2_control(controls[count]);
			count++;
		} else {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
					"CTRL: duplicate ext ctrl\n");
		}
		if(qctrl.id<=current){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
					"CTRL: found buggy driver\n");
			qctrl.id++;
		}
		current = qctrl.id & ~V4L2_CTRL_FLAG_NEXT_CTRL;
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	empty_list(list);
	return count;
}
static int fix_quirky_values(struct v4l2_queryctrl *qc, int v){
	if ((qc->type == V4L2_CTRL_TYPE_INTEGER) || (qc->type == V4L2_CTRL_TYPE_BOOLEAN)
			|| (qc->type == V4L2_CTRL_TYPE_MENU) || (qc->type ==  V4L2_CTRL_TYPE_BUTTON)
			|| (qc->type == V4L2_CTRL_TYPE_INTEGER_MENU)) {
		if(v < qc->minimum) {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: fixed quirky control value %d below minimum %d\n",
					v,qc->minimum);
			return qc->minimum;
		} else if (v>qc->maximum) {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: fixed quirky control value %d above maximum %d\n",
					v,qc->maximum);
			return qc->maximum;
		}
	} else if (qc->type == V4L2_CTRL_TYPE_BITMASK) {
		if ((unsigned int) v > (unsigned int) qc->maximum) {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: fixed quirky bitmask value %u above max %u\n",
					(unsigned int)v, (unsigned int) qc->maximum);
			return qc->maximum;
		}
	}
	return v;

}
//returns the value of a control
int get_control_value_v4l2(struct video_device *vdev,
		struct v4l2_queryctrl *qctrl, void *val, int size){

	int ret = LIBVIDEO_ERR_IOCTL;

	// Check the class of the control
	if(V4L2_CTRL_ID2CLASS(qctrl->id)==V4L2_CID_USER_CLASS) {
		struct v4l2_control vc;
		CLEAR(vc);
		vc.id = qctrl->id;

		if( (ret = v4lconvert_vidioc_g_ctrl(vdev->control->priv, &vc)) == 0 )
			*(int32_t*)val = fix_quirky_values(qctrl, vc.value);
		else
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: Error getting current value (%d)\n", errno);
	} else {
		struct v4l2_ext_controls ext_ctrl;
		struct v4l2_ext_control ctrl;

		// Reset control structs and set members
		CLEAR(ext_ctrl);
		CLEAR(ctrl);

		ext_ctrl.count = 1;
		ext_ctrl.ctrl_class = V4L2_CTRL_ID2CLASS(qctrl->id);
		ext_ctrl.controls = &ctrl;

		ctrl.id = qctrl->id;

		if (qctrl->type == V4L2_CTRL_TYPE_STRING) {
			ctrl.size = size;
			ctrl.string = val;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: getting string @ 0x%p- max size: %d\n", ctrl.string, ctrl.size);

		} else
			ctrl.size = 0;

		ret = ioctl(vdev->fd, VIDIOC_G_EXT_CTRLS, &ext_ctrl);
		if ((ret != 0) && (errno == ENOTTY)) {
			// Older kernels may not support extended controls - retry with g_ctrl
			struct v4l2_control vc;

			CLEAR(vc);
			vc.id = qctrl->id;

			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: workaround: g_ext_ctrl failed with ENOTTY, trying g_ctrl\n");

			ret = v4lconvert_vidioc_g_ctrl(vdev->control->priv, &vc);
			if(ret == 0)
				ctrl.value = vc.value;
		}

		if( ret == 0 ) {
			if (qctrl->type == V4L2_CTRL_TYPE_INTEGER64) {
				*((int64_t*)val) = ctrl.value64;
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: Read 64-bit val: %lld\n", ctrl.value64);
			} else if (qctrl->type != V4L2_CTRL_TYPE_STRING) {
				*((int32_t*)val) = fix_quirky_values(qctrl, ctrl.value);
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: Read 32-bit val: %d\n", ctrl.value);
			} else
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: Read string val: %s\n", ctrl.string);
		} else {
			if (errno == ENOSPC) {
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: Error getting current value: not enough space"
						" to store result - need %d bytes, got %d\n", ctrl.size,
						size);
			} else {
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: Error getting current value (%d) for ext ctrl\n", errno);
			}

			ret = LIBVIDEO_ERR_IOCTL;
		}
	}

	return ret;
}

//sets the value of a control
int set_control_value_v4l2(struct video_device *vdev,
		struct v4l2_queryctrl *qctrl, void *val, int size) {

	// Check the class of the control
	if(V4L2_CTRL_ID2CLASS(qctrl->id)==V4L2_CID_USER_CLASS) {
		struct v4l2_control vc;

		vc.id = qctrl->id;
		vc.value = *(int32_t*)val;

		if( v4lconvert_vidioc_s_ctrl(vdev->control->priv, &vc) != 0 ){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: Error setting value\n");
			if(errno == EINVAL)
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
						"CTRL: einval\n");
			else if(errno == ERANGE)
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
						"CTRL: erange\n");
			else
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
						"CTRL: unknown error %d\n", errno);
			return LIBVIDEO_ERR_IOCTL;
		}
	} else {
		struct v4l2_ext_controls ext_ctrl;
		struct v4l2_ext_control ctrl;
		int result;

		// Reset control structs and set members
		CLEAR(ext_ctrl);
		CLEAR(ctrl);

		ext_ctrl.count = 1;
		ext_ctrl.ctrl_class = V4L2_CTRL_ID2CLASS(qctrl->id);
		ext_ctrl.controls = &ctrl;

		ctrl.id = qctrl->id;

		if (qctrl->type == V4L2_CTRL_TYPE_STRING) {
			ctrl.size = size;
			ctrl.string = (char *)val;

			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: Writing string val: %s - size: %d\n", ctrl.string, ctrl.size);
		} else {
			ctrl.size = 0;

			if (qctrl->type == V4L2_CTRL_TYPE_INTEGER64) {
				ctrl.value64 = *(int64_t*) val;
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
						"CTRL: Writing 64-bit val: %lld\n", ctrl.value64);
			} else {
				ctrl.value = *(int32_t*) val;
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
						"CTRL: Writing 32-bit val: %d\n", ctrl.value);
			}
		}

		result = ioctl(vdev->fd, VIDIOC_S_EXT_CTRLS, &ext_ctrl);
		if((result != 0 ) && (errno == ENOTTY)) {
			// older kernels may not support extended controls - retry with VIDIOC_S_CTRL
			struct v4l2_control vc;

			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
						"CTRL: workaround: s_ext_ctrl failed with ENOTTY, trying s_ctrl\n");

			vc.id = qctrl->id;
			vc.value = *(int32_t*)val;

			result = v4lconvert_vidioc_s_ctrl(vdev->control->priv, &vc);
		}

		if (result != 0) {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: Error setting value\n");
			if(errno == EINVAL)
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
						"CTRL: einval\n");
			else if(errno == ERANGE)
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
						"CTRL: erange\n");
			else
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
						"CTRL: unknown error %d\n", errno);
			return LIBVIDEO_ERR_IOCTL;
		}
	}

	return 0;
}

// ****************************************
// List caps functions
// ****************************************
static void enum_image_fmt_v4l2(int fd) {
	struct v4l2_fmtdesc fmtd;

	printf("============================================\n"
			"Querying image format\n\n");
	memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;

	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		printf("%d - %s (compressed : %d) (%#x) \n",
				fmtd.index, fmtd.description, fmtd.flags, fmtd.pixelformat);
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

	printf("============================================\n"
			"Querying current image format settings\n\n");
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

	printf("============================================\n"
			"Querying capture capabilities\n");
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
				if(tun.type==V4L2_TUNER_RADIO)
					printf("It is a RADIO tuner\n");
				if(tun.type==V4L2_TUNER_ANALOG_TV)
					printf("It is a TV tuner\n");
				if(tun.capability&V4L2_TUNER_CAP_LOW)
					printf("Frequencies in units of 62.5Hz\n");
				else
					printf("Frequencies in units of 62.5kHz\n");

				if(tun.capability&V4L2_TUNER_CAP_NORM)
					printf("Multi-standard tuner\n");
				if(tun.capability&V4L2_TUNER_CAP_STEREO)
					printf("Stereo reception supported\n");
				/* More flags here */
				printf("lowest tunable frequency: %.2f %s\n",
						tun.rangelow * 62.5,
						(tun.capability&V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
				printf("highest tunable frequency: %.2f %s\n",
						tun.rangehigh * 62.5,
						(tun.capability&V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
				memset(&freq, 0, sizeof(struct v4l2_frequency));
				freq.tuner = vin.tuner;
				if(ioctl(fd, VIDIOC_G_FREQUENCY, &freq)==0) {
					printf("Current frequency: %.2f %s\n",
							freq.frequency * 62.5,
							(tun.capability&V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
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


	printf("============================================\n"
			"Querying supported frame sizes\n\n");
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		printf("Image format: %s\n",fmtd.description);
		frms.index = 0;
		frms.pixel_format = fmtd.pixelformat;
		while(ioctl (fd, VIDIOC_ENUM_FRAMESIZES, &frms) >=0) {
			printf("index %d", frms.index);
			if (frms.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
				printf("\tHeight: %d - Width: %d\n",
						frms.discrete.height, frms.discrete.width);
				frms.index++;
			}
			else {
				printf("\tMin, max & step height: %d - %d - %d Min, max & step"
						"width: %d - %d - %d",
					frms.stepwise.min_height, frms.stepwise.max_height,
					frms.stepwise.step_height, frms.stepwise.min_width,
					frms.stepwise.max_width, frms.stepwise.step_width);
				break;
			}
		}
		fmtd.index++;
	}

}

static void print_v4l2_control(struct v4l2_queryctrl *qc) {
	printf( "Control: id: 0x%x - name: %s - min: %d -max: %d - step: %d - "
			"type: %d(%s) - flags: %d (%s%s%s%s%s%s%s)\n", \
			qc->id, (char *) &qc->name, qc->minimum, qc->maximum, qc->step,
			qc->type,
			qc->type == V4L2_CTRL_TYPE_INTEGER ? "Integer" :
			qc->type == V4L2_CTRL_TYPE_BOOLEAN ? "Boolean" :
			qc->type == V4L2_CTRL_TYPE_MENU ? "Menu" :
			qc->type == V4L2_CTRL_TYPE_BUTTON ? "Button" :
			qc->type == V4L2_CTRL_TYPE_INTEGER64 ? "Integer64" :
			qc->type == V4L2_CTRL_TYPE_CTRL_CLASS ? "Class" :
			qc->type == V4L2_CTRL_TYPE_BITMASK ? "Bitmask" :
			qc->type == V4L2_CTRL_TYPE_STRING ? "String" : 
			qc->type == V4L2_CTRL_TYPE_INTEGER_MENU ? "IntMenu" : "",
			qc->flags,
			qc->flags & V4L2_CTRL_FLAG_DISABLED ? "Disabled " : "",
			qc->flags & V4L2_CTRL_FLAG_GRABBED ? "Grabbed " : "",
			qc->flags & V4L2_CTRL_FLAG_READ_ONLY ? "ReadOnly " : "",
			qc->flags & V4L2_CTRL_FLAG_UPDATE ? "Update " : "",
			qc->flags & V4L2_CTRL_FLAG_INACTIVE ? "Inactive " : "",
			qc->flags & V4L2_CTRL_FLAG_SLIDER ? "slider " : "",
			qc->flags & V4L2_CTRL_FLAG_WRITE_ONLY ? "write-only" : ""
			);
}

static void query_controls_v4l2(int fd) {
	int i;
	struct v4l2_queryctrl qctrl;
	CLEAR(qctrl);
	struct v4lconvert_data *d = v4lconvert_create(fd);
	printf("============================================\n"
			"Querying standard controls\n\n");
	//std ctrls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1; i++) {
		qctrl.id = i;
		//if((ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0))
		if(v4lconvert_vidioc_queryctrl(d,&qctrl)==0)
			print_v4l2_control(&qctrl);
	}

	printf("============================================\n"
			"Querying private controls\n\n");
	//priv ctrls
	for (qctrl.id = V4L2_CID_PRIVATE_BASE;; qctrl.id++) {
		if(v4lconvert_vidioc_queryctrl(d,&qctrl)==0) {
			print_v4l2_control(&qctrl);
		} else {
			if (errno == EINVAL)
				break;

			printf( "we shouldnt be here...\n");
		}
	}

	printf("============================================\n"
			"Querying extended controls\n\n");
	//checking extended controls
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (v4lconvert_vidioc_queryctrl(d,&qctrl)==0) {
		print_v4l2_control(&qctrl);
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
	v4lconvert_destroy(d);
}



void list_cap_v4l2(int fd) {
	struct v4l2_capability cap;

	printf("============================================\n"
			"Querying general capabilities\n\n");
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		printf("v4l2 not supported. Maybe a v4l1 device ...");
	}
	else {
		//print capabilities
		printf("Driver name: %s\n",cap.driver);
		printf("Device name: %s\n",cap.card);
		printf("bus_info: %s\n", cap.bus_info);
		printf("version: %u.%u.%u\n",
				(cap.version >> 16) & 0xFF,(cap.version >> 8) & 0xFF,
				cap.version & 0xFF);
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
			printf("Has"); else printf("Does NOT have");
		printf(" capture capability\n");
		if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
			printf("Has"); else printf("Does NOT have");
		printf(" output capability\n");
		if (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
			printf("Has"); else printf("Does NOT have");
		printf(" overlay capability\n");
		if (cap.capabilities & V4L2_CAP_VBI_CAPTURE)
			printf("Has"); else printf("Does NOT have");
		printf(" VBI capture capability\n");
		if (cap.capabilities & V4L2_CAP_VBI_OUTPUT)
			printf("Has"); else printf("Does NOT have");
		printf(" VBI output capability\n");
		if (cap.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE)
			printf("Has"); else printf("Does NOT have");
		printf(" SLICED VBI capture capability\n");
		if (cap.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT)
			printf("Has"); else printf("Does NOT have");
		printf(" SLICED VBI output capability\n");
		if (cap.capabilities & V4L2_CAP_RDS_CAPTURE)
			printf("Has"); else printf("Does NOT have");
		printf(" RDS capability\n");
		if (cap.capabilities & V4L2_CAP_TUNER)
			printf("Has"); else printf("Does NOT have");
		printf(" tuner capability\n");
		if (cap.capabilities & V4L2_CAP_AUDIO)
			printf("Has"); else printf("Does NOT have");
		printf(" audio capability\n");
		if (cap.capabilities & V4L2_CAP_RADIO)
			printf("Has"); else printf("Does NOT have");
		printf(" radio capability\n");
		if (cap.capabilities & V4L2_CAP_READWRITE)
			printf("Has"); else printf("Does NOT have");
		printf(" read/write capability\n");
		if (cap.capabilities & V4L2_CAP_ASYNCIO)
			printf("Has"); else printf("Does NOT have");
		printf(" asyncIO capability\n");
		if (cap.capabilities & V4L2_CAP_STREAMING)
			printf("Has"); else printf("Does NOT have");
		printf(" streaming capability\n");
		printf("\n");

		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
			query_capture_intf_v4l2(fd);
		// FIXME Enumerate other capabilites (output, overlay,...

		enum_image_fmt_v4l2(fd);
		query_current_image_fmt_v4l2(fd);
		query_frame_sizes_v4l2(fd);
		query_controls_v4l2(fd);

	}
}
