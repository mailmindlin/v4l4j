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
#include "videodev.h"
#include "libv4l.h"
#include "log.h"
#include "libv4l-err.h"


static int get_capabilities(int fd, struct video_capability *vc) {
	if (-1 == ioctl( fd, VIDIOCGCAP, vc)) {
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: Not a V4L1 device.\n");
		return -1;
	}
	return 0;
}

void list_cap_v4l1(struct capture_device *);
//Check whether the device is V4L1 and has capture and mmap capabilities
// get capabilities VIDIOCGCAP - check max height and width
int check_capture_capabilities_v4l1(struct capture_device *c) {
	struct video_capability vc;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG2, "V4L1: Checking capture device\n");

	CLEAR(vc);
	
	if (get_capabilities(c->fd, &vc)!=0){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: error getting capabilities.\n");
		return -1;
	}

	if (!(vc.type & VID_TYPE_CAPTURE)) {
		info("The device %s seems to be a valid V4L1 device but without capture capability\n", c->file);
		info("Please let the author know about this error.\n", c->file);
		info("See the ISSUES section in the libv4l README file.\n", c->file);
		info("Listing the reported capabilities:\n", c->file);
		list_cap_v4l1(c);
		return -1;
	}

	if( c->channel < 0 || c->channel >= vc.channels ) {
		info("The specified input channel (%d) does NOT exist on device %s \n",c->channel, c->file);
		info("Listing reported channels:\n", c->file);
		list_cap_v4l1(c);
		return -1;
	}

	return 0;
}

// set the capture parameters (hardcoded to require YUV420 for now)
// set video channel 	VIDIOCSCHAN - 
// set picture format 	VIDIOCSPICT - 
// set window 		VIDIOCSWIN  
// get window format	VIDIOCGWIN  (to double check)
int set_cap_param_v4l1(struct capture_device *c, int *palettes, int nb) {
	struct video_channel chan;
	struct video_picture pict;
	struct video_window win;
	struct video_capability vc;
	int i;
	int def[NB_SUPPORTED_PALETTE] = DEFAULT_PALETTE_ORDER;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG2, "V4L1: Setting capture parameters on device %s.\n", c->file);
	if(nb<0 || nb>=NB_SUPPORTED_PALETTE) {
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG2, "V4L1: Incorrect number of palettes (%d)\n", nb);
		return LIBV4L_ERR_FORMAT;
	}
	if(nb==0 || palettes==NULL) {
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG2, "V4L1: No palettes supplied, trying default order.\n");
		palettes = def;
		nb = NB_SUPPORTED_PALETTE;
	}
	
	CLEAR(chan);
	CLEAR(pict);
	CLEAR(win);
	CLEAR(vc);
	
	if (get_capabilities(c->fd, &vc)!=0){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: error getting capabilities.\n");
		return LIBV4L_ERR_NOCAPS;
	}

	if (c->width > vc.maxwidth) {
		info("The specified width capture (%d) is greater than supported (%d)\n", c->width, vc.maxwidth);
		info("Listing the reported capabilities:\n", c->file);
		list_cap_v4l1(c);
		return LIBV4L_ERR_WIDTH;
	}

	if (c->height > vc.maxheight) {
		info("The specified height capture (%d) is greater than supported (%d)\n", c->height, vc.maxheight);
		info("Listing the reported capabilities:\n", c->file);
		list_cap_v4l1(c);
		return LIBV4L_ERR_HEIGHT;
	}

	if (c->width < vc.minwidth) {
		info("The specified width capture (%d) is lower than supported (%d)\n", c->width, vc.minwidth);
		info("Listing the reported capabilities:\n", c->file);
		list_cap_v4l1(c);
		return LIBV4L_ERR_WIDTH;
	}

	if (c->height < vc.minheight) {
		info("The specified height capture (%d) lower than supported (%d)\n", c->height, vc.minheight);
		info("Listing the reported capabilities:\n", c->file);
		list_cap_v4l1(c);
		return LIBV4L_ERR_HEIGHT;
	}
	
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
	if (-1 == ioctl( c->fd, VIDIOCSCHAN, &chan )) {
		info("The desired input channel(%d)/standard(%d) cannot be selected\n", c->channel, c->std);
		info("Listing the reported capabilities:\n", c->file);
		list_cap_v4l1(c);
		return LIBV4L_ERR_CHANNEL_SETUP;
	}
	
	//query the current image format
	if(-1 == ioctl(c->fd, VIDIOCGPICT, &pict)) {
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: cannot get the current palette format\n");
		return LIBV4L_ERR_IOCTL;
	}

	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: trying palettes (%d to try in total)\n", nb);
	for(i=0; i<nb; i++) {
		pict.palette = libv4l_palettes[palettes[i]].v4l1_palette;
		pict.depth = libv4l_palettes[palettes[i]].depth;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: trying palette %s (%d) - depth %d...\n",\
				libv4l_palettes[palettes[i]].name, pict.palette, pict.depth);
		
		if(0 == ioctl(c->fd, VIDIOCSPICT, &pict)){
			c->palette = palettes[i];
			c->real_v4l1_palette = palettes[i];
			c->imagesize  = c->width*c->height*pict.depth / 8;
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: palette %s (%d) accepted - image size: %d\n",
				libv4l_palettes[palettes[i]].name, palettes[i], c->imagesize);
			break;
		}
		
		/*
		 * V4L1 weirdness
		 */
		if(libv4l_palettes[palettes[i]].libv4l_palette == YUV420) {
			pict.palette = VIDEO_PALETTE_YUV420P;
			pict.depth = libv4l_palettes[palettes[i]].depth;
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: trying palette %s (%d) - depth %d...\n",\
					"YUV420-workaround", YUV420, pict.depth);
			
			if(0 == ioctl(c->fd, VIDIOCSPICT, &pict)){
				c->palette = YUV420;
				c->real_v4l1_palette = YUV420P;
				c->imagesize  = c->width*c->height*pict.depth / 8;
				dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: palette %s (%d) accepted - image size: %d\n",
					"YUV420-workaround", YUV420, c->imagesize);
				break;
			}
		}
		
		/*
		 * More V4L1 weirdness
		 */
		if(libv4l_palettes[palettes[i]].libv4l_palette == YUYV) {
			pict.palette = VIDEO_PALETTE_YUV422;
			pict.depth = libv4l_palettes[palettes[i]].depth;
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: trying palette %s (%d) - depth %d...\n",\
					"YUYV-workaround", YUYV, pict.depth);
			
			if(0 == ioctl(c->fd, VIDIOCSPICT, &pict)){
				c->palette = YUYV;
				c->real_v4l1_palette = YUV422;
				c->imagesize  = c->width*c->height*pict.depth / 8;
				dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: palette %s (%d) accepted - image size: %d\n",
					"YUYV-workaround", YUYV, c->imagesize);
				break;
			}
		}
		
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: palette %s rejected\n", libv4l_palettes[palettes[i]].name);
	}
	if(i==nb) {
		info("libv4l was unable to find a suitable palette. The following palettes have been tried and failed:\n");
		for(i=0; i<nb;i++)
			info("%s\n",libv4l_palettes[palettes[i]].name);
		info("Please let the author know about this error.\n", c->file);
		info("See the ISSUES section in the libv4l README file.\n", c->file);
		info("Listing the reported capabilities:\n", c->file);
		list_cap_v4l1(c);
		return LIBV4L_ERR_FORMAT;
	}

	win.x = win.y = 0;
	win.width = c->width;
	win.height = c->height;
	win.chromakey = 0;
	win.flags = 0;
	win.clips = NULL;
	win.clipcount = 0;
	if(-1 == ioctl(c->fd, VIDIOCSWIN,&win))	{
		info("libv4l was unable to set the requested capture size (%dx%d).\n", c->width, c->height);
		info("Maybe the device doesnt support this combination of width and height.\n");
		return LIBV4L_ERR_FORMAT;
	}

	CLEAR(win);

	if(-1 == ioctl(c->fd, VIDIOCGWIN, &win)){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: cannot verify the image size\n");
		return LIBV4L_ERR_CROP;
	}

	if( win.width != c->width || win.height != c->height ){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: V4L1 resized image from %dx%d to %dx%d\n",\
			c->width, c->height,win.width, win.height);
		c->width = win.width;
		c->height = win.height;
	}

	c->bytesperline = 0;

	return 0;
}

// get streaming cap details VIDIOCGMBUF, initialise streaming and create mmap'ed buffers
int init_capture_v4l1(struct capture_device *c) {
	struct video_mbuf vm;
	CLEAR(vm);
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG2, "V4L1: initialising capture on device %s.\n", c->file);

	if(-1 == ioctl(c->fd, VIDIOCGMBUF, &vm)){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: Error getting mmap information from driver\n");
		return LIBV4L_ERR_REQ_MMAP;
	}
	
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: driver allocated %d simlutaneous buffers\n", vm.frames);
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: first offset [0] %d\n", vm.offsets[0]);
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: second offset [1] %d\n", vm.offsets[1]);
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: total size %d\n", vm.size);

	/*
	 * we only use two buffers, regardless of what the driver returned, unless it said 1, in which case we abort.
	 * For info, the QC driver returns vm.offset[0]=vm.offset[1]=0 ... gspca doesnt...
	 * because of this, we will store vm.size in c->mmap->v4l1_mmap_size so we can re-use it when unmmap'ing
	 * and we set c->mmap->buffers[0] == c->mmap->buffers[1] = vm.offset[1] - 1, 
	 * so we have sensible values in the length fields, and we can still unmmap the area with the right value 
	 */

	if(vm.frames>2) {
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: Using only 2 buffers\n", vm.frames);
	} else if (vm.frames<2) {
		//although it wont require much fixing...
		//do drivers allocate only 1 buffer anyway ?
		info("The video driver returned an unsupported number of MMAP buffers(%d).\n", vm.frames);
		info("Please let the author know about this error.\n", c->file);
		info("See the ISSUES section in the libv4l README file.\n", c->file);
		return LIBV4L_ERR_INVALID_BUF_NB;
	}
	
	c->mmap->buffer_nr = 2;
		
	XMALLOC( c->mmap->buffers, struct mmap_buffer *, (c->mmap->buffer_nr * sizeof(struct mmap_buffer)) );

	c->mmap->buffers[0].start = mmap(NULL, vm.size, PROT_READ, MAP_SHARED, c->fd, 0);
	if(MAP_FAILED == c->mmap->buffers[0].start){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: Cant allocate mmap'ed memory\n");
		return LIBV4L_ERR_MMAP_BUF;
	}
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: mmap'ed %d bytes at %p\n", vm.size, c->mmap->buffers[0].start);
	c->mmap->v4l1_mmap_size = vm.size;
	c->mmap->buffers[1].start = (void *)c->mmap->buffers[0].start + vm.offsets[1];
	c->mmap->buffers[0].length = c->mmap->buffers[1].length = vm.size - 1;

	c->mmap->tmp = 0;

	return 0;
}

// start the capture of first buffer VIDIOCMCAPTURE(0)
int start_capture_v4l1(struct capture_device *c) {
	struct video_mmap mm;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG2, "V4L1: starting capture on device %s.\n", c->file);
	
	CLEAR(mm);
	mm.frame = 0;
	mm.width = c->width;
	mm.height = c->height;
	mm.format = libv4l_palettes[c->real_v4l1_palette].v4l1_palette;

	if(-1 == ioctl(c->fd, VIDIOCMCAPTURE, &mm))	{
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: Cant start the capture\n");
		return LIBV4L_ERR_IOCTL;
	}
	c->mmap->tmp = 0;

	return 0;
}

//dequeue the next buffer with available frame
// start the capture of next buffer VIDIOCMCAPTURE(x)
void *dequeue_buffer_v4l1(struct capture_device *c) {
	struct video_mmap mm;
	int curr_frame = (int) c->mmap->tmp;
	int next_frame = curr_frame ^ 1;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG2, "V4L1: dequeuing buffer on device %s.\n", c->file);

	CLEAR(mm);
	
	mm.frame =  next_frame;
	mm.width = c->width;
	mm.height = c->height;
	mm.format = libv4l_palettes[c->real_v4l1_palette].v4l1_palette;

	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: Starting capture of next frame (%d)\n", next_frame);
	if(-1 == ioctl(c->fd, VIDIOCMCAPTURE, &mm)){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: Cant initiate the capture of next frame\n");
		return NULL;
	}

	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: Waiting for frame (%d)\n", curr_frame);
	if(-1 == ioctl(c->fd, VIDIOCSYNC, &curr_frame)){
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: Error waiting for next frame(%d)\n", curr_frame);
		return NULL;
	}
	c->mmap->tmp = (void *)next_frame;
	return c->mmap->buffers[curr_frame].start;
}

//enqueue the buffer when done using the frame
void enqueue_buffer_v4l1(struct capture_device *cdev) {}

//counterpart of start_capture, must be called it start_capture was successful
int stop_capture_v4l1(struct capture_device *c) {
	return 0;
}

//counterpart of init_capture, must be called it init_capture was successful
void free_capture_v4l1(struct capture_device *c) {
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: freeing capture structures on device %s.\n", c->file);
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: unmmap %d bytes at %p\n", c->mmap->v4l1_mmap_size, c->mmap->buffers[0].start);
	if (-1 == munmap(c->mmap->buffers[0].start, (size_t) c->mmap->v4l1_mmap_size))
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: error unmapping mmap'ed buffer\n");
	XFREE(c->mmap->buffers);

}


 /*
 * Control related functions
 */
 //returns the number of controls (standard and private V4L1 controls only)
int count_v4l1_controls(struct capture_device *c) {
	//4 basic controls in V4L1
	return 4;
}

//Populate the control_list with fake V4L2 controls matching V4L1 video
//controls and returns how many fake controls were created
int create_v4l1_controls(struct capture_device *c, struct control_list *l){
	int count = 0;
			
	//list standard V4L controls
	//brightness
	l->ctrl[count].id = V4L2_CID_BRIGHTNESS;
	l->ctrl[count].type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)l->ctrl[count].name, "Brightness\0");
	l->ctrl[count].minimum = 0;
	l->ctrl[count].maximum = 65535;
	l->ctrl[count].step = 1;
	l->ctrl[count].default_value = 32768;
	l->ctrl[count].flags = 0;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: found control(id: %d - name: %s - min: %d -max: %d - val: %d)\n"\
					,l->ctrl[count].id, (char *) &l->ctrl[count].name, l->ctrl[count].minimum, l->ctrl[count].maximum, \
					get_control_value_v4l1(c, &l->ctrl[count]));
	count++;
	
	//hue
	l->ctrl[count].id = V4L2_CID_HUE;
	l->ctrl[count].type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)l->ctrl[count].name, "Hue\0");
	l->ctrl[count].minimum = 0;
	l->ctrl[count].maximum = 65535;
	l->ctrl[count].step = 1;
	l->ctrl[count].default_value = 32768;
	l->ctrl[count].flags = 0;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: found control(id: %d - name: %s - min: %d -max: %d - val: %d)\n"\
					,l->ctrl[count].id, (char *) &l->ctrl[count].name, l->ctrl[count].minimum, l->ctrl[count].maximum, \
					get_control_value_v4l1(c, &l->ctrl[count]));
	count++;
	
	//color
	l->ctrl[count].id = V4L2_CID_SATURATION;
	l->ctrl[count].type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)l->ctrl[count].name, "Saturation\0");
	l->ctrl[count].minimum = 0;
	l->ctrl[count].maximum = 65535;
	l->ctrl[count].step = 1;
	l->ctrl[count].default_value = 32768;
	l->ctrl[count].flags = 0;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: found control(id: %d - name: %s - min: %d -max: %d - val: %d)\n"\
					,l->ctrl[count].id, (char *) &l->ctrl[count].name, l->ctrl[count].minimum, l->ctrl[count].maximum, \
					get_control_value_v4l1(c, &l->ctrl[count]));
	count++;
	
	//contrast
	l->ctrl[count].id = V4L2_CID_CONTRAST;
	l->ctrl[count].type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)l->ctrl[count].name, "Contrast\0");
	l->ctrl[count].minimum = 0;
	l->ctrl[count].maximum = 65535;
	l->ctrl[count].step = 1;
	l->ctrl[count].default_value = 32768;
	l->ctrl[count].flags = 0;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG, "V4L1: found control(id: %d - name: %s - min: %d -max: %d - val: %d)\n"\
					,l->ctrl[count].id, (char *) &l->ctrl[count].name, l->ctrl[count].minimum, l->ctrl[count].maximum, \
					get_control_value_v4l1(c, &l->ctrl[count]));
	count++;
	
	return count;
}

//returns the value of a control
int get_control_value_v4l1(struct capture_device *c, struct v4l2_queryctrl *ctrl){
	struct video_picture pict;
	CLEAR(pict);
	//query the current image format
	if(-1 == ioctl(c->fd, VIDIOCGPICT, &pict)) {
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: cannot get the value for control %s\n", (char *) &ctrl->name );
		return 0;
	}
	switch(ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
			return pict.brightness;
		case V4L2_CID_HUE:
			return pict.hue;
		case V4L2_CID_SATURATION:
			return pict.colour;
		case V4L2_CID_CONTRAST:
			return pict.contrast;
		default:
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: unknown control %s (id: %d)\n", (char *) &ctrl->name, ctrl->id);
			return 0;
	}
}

//sets the value of a control
void set_control_value_v4l1(struct capture_device *c, struct v4l2_queryctrl *ctrl, int v){
	struct video_picture pict;
	CLEAR(pict);
	//query the current image format
	if(-1 == ioctl(c->fd, VIDIOCGPICT, &pict)) {
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: cannot get the current value for control %s\n", (char *) &ctrl->name );
		return;
	}
	
	switch(ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
			pict.brightness = v;
			break;
		case V4L2_CID_HUE:
			pict.hue = v;
			break;
		case V4L2_CID_SATURATION:
			pict.colour = v;
			break;
		case V4L2_CID_CONTRAST:
			pict.contrast = v;
			break;
		default:
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: unknown control %s (id: %d)\n", (char *) &ctrl->name, ctrl->id);
			return;
	}
	
	//set the current image format
	if((-1 == ioctl(c->fd, VIDIOCSPICT, &pict)))
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: Error setting the new value (%d) for control %s\n", v, (char *) &ctrl->name );
	if (get_control_value_v4l1(c, ctrl)!= v)
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_ERR, "V4L1: Error setting the new value (%d) for control %s\n", v, (char *) &ctrl->name );
}



// ****************************************
// List caps functions
// ****************************************

void enum_image_fmt_v4l1(struct capture_device *cdev) {
	struct video_picture pic;
	int fd = cdev->fd;
	int i;
	CLEAR(pic);
	
	printf("============================================\nQuerying image format\n\n");
	
	if(ioctl(fd, VIDIOCGPICT, &pic) >= 0) {
		printf("brightness: %d - hue: %d - colour: %d - contrast: %d - depth: %d (palette %d)\n",\
			pic.brightness, pic.hue, pic.colour, pic.contrast, pic.depth, pic.palette);
		i = pic.palette;

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_GREY;
		printf("Palette GREY: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_HI240;
		printf("Palette HI240: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB565;
		printf("Palette RGB565: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB555;
		printf("Palette RGB555: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB24;
		printf("Palette RGB24: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB32;
		printf("Palette RGB32: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV422;
		printf("Palette YUV422: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUYV;
		printf("Palette YUYV: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_UYVY;
		printf("Palette UYVY: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV420;
		printf("Palette YUV420: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV411;
		printf("Palette YUV411: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RAW;
		printf("Palette RAW: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV422P;
		printf("Palette YUV422P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV411P;
		printf("Palette YUV411P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV420P;
		printf("Palette YUV420P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV410P;
		printf("Palette YUV410P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

	} else 
		printf("Not supported ...\n");
	printf("\n");
}

void query_current_image_fmt_v4l1(struct capture_device *cdev) {
	struct video_window win;
	CLEAR(win);
	printf("============================================\nQuerying current image size\n");

	if(-1 == ioctl(cdev->fd, VIDIOCGWIN, &win)){
		printf("Cannot get the image size\n");
		return;
	}	
	printf("Current width: %d\n", win.width);
	printf("Current height: %d\n", win.height);
	printf("\n");
}

void query_capture_intf_v4l1(struct capture_device *cdev) {
	struct video_capability vc;
	struct video_channel chan;
	int i;
	CLEAR(vc);
	int fd = cdev->fd;
	
	if (-1 == ioctl( fd, VIDIOCGCAP, &vc)) {
		printf("Failed to get capabilities.\n");
		return;
	}	


	printf("============================================\nQuerying capture interfaces\n");
	for (i=0; i<vc.channels; i++) {
		CLEAR(chan);
		chan.channel=i;
		if (-1 == ioctl(fd, VIDIOCGCHAN, &chan)) {
			printf("Failed to get input details.");
			return;
		}	
		printf("Input number: %d\n", chan.channel);
		printf("Name: %s\n", chan.name);
		if(chan.flags & VIDEO_VC_TUNER) {
			printf("Has tuners\n");
			printf("\tNumber of tuners: (%d) ", chan.tuners);
			//TODO: list tuner using struct video_tuner and VIDIOCGTUNER		
		} else 
			printf("Doesnt have tuners\n");
		if(chan.flags & VIDEO_VC_AUDIO)
			printf("Has audio\n");		
		
		printf("Type: ");
		if(chan.type & VIDEO_TYPE_TV) printf("TV\n");
		if(chan.type & VIDEO_TYPE_CAMERA) printf("Camera\n");
		printf("\n");
	}
	printf("\n");
}

void query_frame_sizes_v4l1(struct capture_device *cdev){
	struct video_capability vc;
	CLEAR(vc);
	int fd = cdev->fd;
	
	if (-1 == ioctl( fd, VIDIOCGCAP, &vc)) {
		printf("Failed to get capabilities.");
		return;
	}	

	printf("============================================\nQuerying supported frame sizes\n\n");
	printf("Min width: %d - Min height %d\n", vc.minwidth, vc.minheight);
	printf("Max width: %d - Max height %d\n", vc.maxwidth, vc.maxheight);
	printf("\n");
}


void query_control(struct capture_device *);
void list_cap_v4l1(struct capture_device *c) {
	struct video_capability vc;
	CLEAR(vc);
	int fd = c->fd;
	
	if (-1 == ioctl( fd, VIDIOCGCAP, &vc)) {
		printf("Failed to get capabilities.");
		return;
	}
	
	printf("============================================\nQuerying general capabilities\n\n");
	
	//print capabilities
	printf("Driver name: %s\n",vc.name);
	if (vc.type & VID_TYPE_CAPTURE) printf("Has"); else printf("Does NOT have");
	printf(" capture capability\n");
	if (vc.type & VID_TYPE_TUNER) printf("Has"); else printf("Does NOT have");
	printf(" tuner\n");
	if (vc.type & VID_TYPE_TELETEXT) printf("Has"); else printf("Does NOT have");
	printf(" teletext capability\n");
	if (vc.type & VID_TYPE_OVERLAY) printf("Has"); else printf("Does NOT have");
	printf(" overlay capability\n");
	if (vc.type & VID_TYPE_CHROMAKEY) printf("Has"); else printf("Does NOT have");
	printf(" overlay chromakey capability\n");
	if (vc.type & VID_TYPE_CLIPPING) printf("Has"); else printf("Does NOT have");
	printf(" clipping capability\n");
	if (vc.type & VID_TYPE_FRAMERAM) printf("Has"); else printf("Does NOT have");
	printf(" frame buffer overlay capability\n");
	if (vc.type & VID_TYPE_SCALES) printf("Has"); else printf("Does NOT have");
	printf(" scaling capability\n");
	if (vc.type & VID_TYPE_MONOCHROME) printf("Has"); else printf("Does NOT have");
	printf(" monochrome only capture\n");
	if (vc.type & VID_TYPE_SUBCAPTURE) printf("Has"); else printf("Does NOT have");
	printf(" sub capture capability\n");
	
	query_capture_intf_v4l1(c);
	enum_image_fmt_v4l1(c);
	query_current_image_fmt_v4l1(c);
	query_frame_sizes_v4l1(c);
	query_control(c);
}
