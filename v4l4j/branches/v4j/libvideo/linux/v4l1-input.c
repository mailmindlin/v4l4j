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
#include "videodev2.h"
#include "videodev.h"
#include "libvideo.h"
#include "log.h"
#include "libvideo-err.h"
#include "libvideo-palettes.h"
#include "v4l1-input.h"


int check_v4l1(int fd, struct video_capability *vc) {
	return ioctl( fd, VIDIOCGCAP, vc);
}

//Check whether the device is V4L1 and has capture and mmap capabilities
// get capabilities VIDIOCGCAP - check max height and width
int check_capture_capabilities_v4l1(int fd, char *file) {
	struct video_capability vc;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Checking capture device\n");

	CLEAR(vc);

	if (check_v4l1(fd, &vc)!=0){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Not a V4L1 device.\n");
		return -1;
	}

	if (!(vc.type & VID_TYPE_CAPTURE)) {
		info("The device %s seems to be a valid V4L1 device but without ",file);
		info("capture capability\n.Please let the author know about this error");
		info(".\nSee the ISSUES section in the libvideo README file.\n");
		info("Listing the reported capabilities:\n");
		list_cap_v4l1(fd);
		return -1;
	}

	return 0;
}

// set the capture parameters
// set video channel 	VIDIOCSCHAN -
// set picture format 	VIDIOCSPICT -
// set window 		VIDIOCSWIN
// get window format	VIDIOCGWIN  (to double check)
int set_cap_param_v4l1(struct video_device *vdev, int *palettes, int nb) {
	struct capture_device *c = vdev->capture;
	struct video_channel chan;
	struct video_picture pict;
	struct video_window win;
	struct video_capability vc;
	int i;
	int def[NB_SUPPORTED_PALETTES] = DEFAULT_PALETTE_ORDER;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Setting capture parameters on device %s.\n", vdev->id.device_handle);

	if(nb<0 || nb>=NB_SUPPORTED_PALETTES) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Incorrect number of palettes (%d)\n", nb);
		return LIBVIDEO_ERR_FORMAT;
	}
	if(nb==0 || palettes==NULL) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: No palettes supplied, trying default order.\n");
		palettes = def;
		nb = NB_SUPPORTED_PALETTES;
	}

	CLEAR(chan);
	CLEAR(pict);
	CLEAR(win);
	CLEAR(vc);

	if (check_v4l1(vdev->fd, &vc)!=0){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: error getting capabilities.\n");
		return LIBVIDEO_ERR_NOCAPS;
	}

	//dont fail if requested width/height outside the allowed range
	if(c->width==MAX_WIDTH || c->width > vc.maxwidth)
		c->width=vc.maxwidth;

	if(c->height==MAX_HEIGHT || c->height > vc.maxheight)
		c->height=vc.maxheight;

	if(c->width < vc.minwidth)
		c->width=vc.minwidth;

	if(c->height < vc.minheight)
		c->height=vc.minheight;

	//Select the input channel
	chan.channel = c->channel;
	switch( c->std )
	{
	case NTSC:
		chan.norm = VIDEO_MODE_NTSC;
		break;
	case PAL:
		chan.norm = VIDEO_MODE_PAL;
		break;
	case SECAM:
		chan.norm = VIDEO_MODE_SECAM;
		break;
	default:
		chan.norm = VIDEO_MODE_AUTO;
		break;
	}
	if (-1 == ioctl( vdev->fd, VIDIOCSCHAN, &chan )) {
		info("The desired input channel(%d)/standard(%d) cannot be selected\n"
				, c->channel, c->std);
		info("Listing the reported capabilities:\n");
		list_cap_v4l1(vdev->fd);
		return LIBVIDEO_ERR_CHAN_SETUP;
	}
	//check for tuner
	chan.channel = c->channel;
	if (-1 == ioctl( vdev->fd, VIDIOCGCHAN, &chan )) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: cannot get the current channel info\n");
		return LIBVIDEO_ERR_CHAN_SETUP;
	}
	if(chan.tuners > 1){
		//v4l1 weirdness
		//support only 1 tuner per input
		c->tuner_nb = -1;
	} else if(chan.tuners == 1)
		c->tuner_nb = 0;
	else
		c->tuner_nb = -1;



	//query the current image format
	if(-1 == ioctl(vdev->fd, VIDIOCGPICT, &pict)) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: cannot get the current palette format\n");
		return LIBVIDEO_ERR_IOCTL;
	}

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: trying palettes (%d to try in total)\n", nb);
	for(i=0; i<nb; i++) {
		if(palettes[i]==VIDEO_PALETTE_UNDEFINED_V4L1) {
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
					"CAP: palette %s not V4L1-compatible, skipping\n",
					libvideo_palettes[palettes[i]].name);
			continue;
		}
		pict.palette = libvideo_palettes[palettes[i]].v4l1_palette;
		pict.depth = libvideo_palettes[palettes[i]].depth;
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
				"CAP: trying palette %s (%d) - depth %d...\n",
				libvideo_palettes[palettes[i]].name, pict.palette, pict.depth);

		if(0 == ioctl(vdev->fd, VIDIOCSPICT, &pict)){
			c->palette = palettes[i];
			c->backend->real_v4l1_palette = palettes[i];
			c->imagesize  = c->width*c->height*pict.depth / 8;
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
					"CAP: palette %s (%d) accepted - image size: %d\n",
				libvideo_palettes[palettes[i]].name, palettes[i], c->imagesize);
			break;
		}

		/*
		 * V4L1 weirdness
		 */
		if(palettes[i] == YUV420) {
			pict.palette = VIDEO_PALETTE_YUV420P;
			pict.depth = libvideo_palettes[palettes[i]].depth;
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
					"CAP: trying palette %s (%d) - depth %d...\n",
					"YUV420-workaround", YUV420, pict.depth);

			if(0 == ioctl(vdev->fd, VIDIOCSPICT, &pict)){
				c->palette = YUV420;
				c->backend->real_v4l1_palette = YUV420P;
				c->imagesize  = c->width*c->height*pict.depth / 8;
				dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
						"CAP: palette %s (%d) accepted - image size: %d\n",
					"YUV420-workaround", YUV420, c->imagesize);
				break;
			}
		}

		/*
		 * More V4L1 weirdness
		 */
		if(palettes[i] == YUYV) {
			pict.palette = VIDEO_PALETTE_YUV422;
			pict.depth = libvideo_palettes[palettes[i]].depth;
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
					"CAP: trying palette %s (%d) - depth %d...\n",
					"YUYV-workaround", YUYV, pict.depth);

			if(0 == ioctl(vdev->fd, VIDIOCSPICT, &pict)){
				c->palette = YUYV;
				c->backend->real_v4l1_palette = YUV422;
				c->imagesize  = c->width*c->height*pict.depth / 8;
				dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
						"CAP: palette %s (%d) accepted - image size: %d\n",
						"YUYV-workaround", YUYV, c->imagesize);
				break;
			}
		}

		/*
		 * More V4L1 weirdness
		 */
		if(palettes[i] == YUV411) {
			pict.palette = VIDEO_PALETTE_YUV411P;
			pict.depth = libvideo_palettes[palettes[i]].depth;
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
					"CAP: trying palette %s (%d) - depth %d...\n",
					"YUV411-workaround", YUV411, pict.depth);

			if(0 == ioctl(vdev->fd, VIDIOCSPICT, &pict)){
				c->palette = YUV411;
				c->backend->real_v4l1_palette = YUV411P;
				c->imagesize  = c->width*c->height*pict.depth / 8;
				dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
						"CAP: palette %s (%d) accepted - image size: %d\n",
						"YUYV-workaround", YUYV, c->imagesize);
				break;
			}
		}

		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG1,
				"CAP: palette %s rejected\n",
				libvideo_palettes[palettes[i]].name);
	}
	if(i==nb) {
		info("libvideo was unable to find a suitable palette. "
				"The following palettes have been tried and failed:\n");
		for(i=0; i<nb;i++)
			info("%s\n",libvideo_palettes[palettes[i]].name);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
		info("Listing the reported capabilities:\n");
		list_cap_v4l1(vdev->fd);
		return LIBVIDEO_ERR_FORMAT;
	}
	c->is_native = 1;

	win.x = win.y = 0;
	win.width = c->width;
	win.height = c->height;
	win.chromakey = 0;
	win.flags = 0;
	win.clips = NULL;
	win.clipcount = 0;
	if(-1 == ioctl(vdev->fd, VIDIOCSWIN,&win))	{
		info("libvideo was unable to set the requested capture size (%dx%d).\n",
				c->width, c->height);
		info("Maybe the device doesnt support this combination of width and "
				"height.\n");
		return LIBVIDEO_ERR_DIMENSIONS;
	}

	CLEAR(win);

	if(-1 == ioctl(vdev->fd, VIDIOCGWIN, &win)){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: cannot verify the image size\n");
		return LIBVIDEO_ERR_DIMENSIONS;
	}

	if( win.width != c->width || win.height != c->height ){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: V4L1 resized image from %dx%d to %dx%d\n",
				c->width, c->height,win.width, win.height);
		c->width = win.width;
		c->height = win.height;
	}

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: capture resolution: %dx%d\n", c->width, c->height);

	return 0;
}

int set_frame_intv_v4l1(struct video_device *vdev, int num, int denom) {
	return LIBVIDEO_ERR_IOCTL;
}

int get_frame_intv_v4l1(struct video_device *vdev, int *num, int *denom) {
		return LIBVIDEO_ERR_IOCTL;
}

// get streaming cap details VIDIOCGMBUF, initialise streaming and
// create mmap'ed buffers
int init_capture_v4l1(struct video_device *vdev) {
	struct capture_device *c = vdev->capture;
	struct mmap *m = c->backend->mmap;
	struct video_mbuf vm;
	CLEAR(vm);
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: initialising capture on device %s.\n", vdev->id.device_handle);

	if(-1 == ioctl(vdev->fd, VIDIOCGMBUF, &vm)){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Error getting mmap information from driver\n");
		return LIBVIDEO_ERR_REQ_MMAP;
	}

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: driver allocated %d simultaneous buffers\n", vm.frames);
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: first offset [0] %d\n", vm.offsets[0]);
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: second offset [1] %d\n", vm.offsets[1]);
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: total size %d\n", vm.size);

	/*
	 * we only use two buffers, regardless of what the driver returned,
	 * unless it said 1, in which case we abort. For info, the QC driver
	 * returns vm.offset[0]=vm.offset[1]=0 ... gspca doesnt... because of
	 * this, we will store vm.size in c->mmap->v4l1_mmap_size so we can
	 * re-use it when unmmap'ing and we set
	 * c->mmap->buffers[0] == c->mmap->buffers[1] = vm.offset[1] - 1,
	 * so we have sensible values in the length fields, and we can still
	 * unmmap the area with the right value
	 */

	if(vm.frames>2) {
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
				"CAP: Using only 2 buffers\n");
	} else if (vm.frames<2) {
		//although it wont require much fixing...
		//do drivers allocate only 1 buffer anyway ?
		info("The video driver returned an unsupported number of MMAP "
				"buffers(%d).\n", vm.frames);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
		return LIBVIDEO_ERR_INVALID_BUF_NB;
	}

	c->buffer_nr = 2;

	XMALLOC(m->buffers, struct mmap_buffer *,
			(long unsigned int)(c->buffer_nr*sizeof(struct mmap_buffer)));

	m->buffers[0].start = mmap(NULL, vm.size, PROT_READ, MAP_SHARED, vdev->fd, 0);

	if(MAP_FAILED == m->buffers[0].start){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Cant allocate mmap'ed memory\n");
		return LIBVIDEO_ERR_MMAP_BUF;
	}
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: mmap'ed %d bytes at %p\n", vm.size, m->buffers[0].start);

	m->v4l1_mmap_size = vm.size;
	m->buffers[1].start =(void *)m->buffers[0].start + vm.offsets[1];
	m->buffers[0].length = m->buffers[1].length = vm.size - 1;

	m->tmp = 0;

	return 0;
}

// start the capture of first buffer VIDIOCMCAPTURE(0)
int start_capture_v4l1(struct video_device *vdev) {
	struct capture_device *c = vdev->capture;
	struct video_mmap mm;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: starting capture on device %s.\n", vdev->id.device_handle);

	CLEAR(mm);
	mm.frame = 0;
	mm.width = c->width;
	mm.height = c->height;
	mm.format = libvideo_palettes[c->backend->real_v4l1_palette].v4l1_palette;

	if(-1 == ioctl(vdev->fd, VIDIOCMCAPTURE, &mm))	{
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Cant start the capture\n");
		return LIBVIDEO_ERR_IOCTL;
	}
	c->backend->mmap->tmp = 0;

	return 0;
}

//dequeue the next buffer with available frame
// start the capture of next buffer VIDIOCMCAPTURE(x)
void *dequeue_buffer_v4l1(struct video_device *vdev, int *len) {
	struct capture_device *c = vdev->capture;
	struct video_mmap mm;
	int curr_frame = (int) c->backend->mmap->tmp;
	int next_frame = curr_frame ^ 1;
	*len=c->imagesize;
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: dequeuing buffer on device %s.\n", vdev->id.device_handle);

	CLEAR(mm);

	mm.frame =  next_frame;
	mm.width = c->width;
	mm.height = c->height;
	mm.format = libvideo_palettes[c->backend->real_v4l1_palette].v4l1_palette;

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: Starting capture of next frame (%d)\n", next_frame);
	if(-1 == ioctl(vdev->fd, VIDIOCMCAPTURE, &mm)){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Cant initiate the capture of next frame\n");
		*len = 0;
		return NULL;
	}

	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG2,
			"CAP: Waiting for frame (%d)\n", curr_frame);
	if(-1 == ioctl(vdev->fd, VIDIOCSYNC, &curr_frame)){
		dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
				"CAP: Error waiting for next frame(%d)\n", curr_frame);
		*len = 0;
		return NULL;
	}
	c->backend->mmap->tmp = (void *)next_frame;
	return c->backend->mmap->buffers[curr_frame].start;
}

//enqueue the buffer when done using the frame
void enqueue_buffer_v4l1(struct video_device *vdev) {}

//counterpart of start_capture, must be called if start_capture was successful
int stop_capture_v4l1(struct video_device *vdev) {
	return 0;
}

//counterpart of init_capture, must be called it init_capture was successful
void free_capture_v4l1(struct video_device *vdev) {
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: freeing capture structures on device %s.\n", vdev->id.device_handle);
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: unmmap %d bytes at %p\n", vdev->capture->backend->mmap->v4l1_mmap_size,
			vdev->capture->backend->mmap->buffers[0].start);

	if (-1 == munmap(vdev->capture->backend->mmap->buffers[0].start,
			(size_t) vdev->capture->backend->mmap->v4l1_mmap_size))
			dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_ERR,
					"CAP: error unmapping mmap'ed buffer\n");

	XFREE(vdev->capture->backend->mmap->buffers);
}

