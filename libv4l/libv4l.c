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
#include <fcntl.h>		//for open
#include <string.h>		//for strcpy
#include <sys/ioctl.h>	//for ioctl
#include <unistd.h>		//for write, close


#include "log.h"
#include "libv4l.h"
#include "v4l2-input.h"
#include "v4l1-input.h"
#include "v4l-control.h"


char *get_libv4l_version(char * c) {
	sprintf(c, "%d.%d-%d", VER_MAJ, VER_MIN, VER_REL);
	return c;
}

static int open_device(struct capture_device *c) {
	int retval = 0;
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Opening device file %s.\n", c->file);
	if ((strlen(c->file) == 0) || ((c->fd = open(c->file,O_RDWR )) < 0)) {
		info("V4L: unable to open device file %s. Check the name and permissions\n", c->file);
		retval = -1;
	}
	
	return retval;
}

static void close_device(struct capture_device *c) {
	//Close device file
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: closing device file %s.\n", c->file);
	close(c->fd);
}

static void setup_libv4l_actions(struct capture_device *c) {
	if(c->v4l_version == V4L1_VERSION) {
		c->capture.set_cap_param = set_cap_param_v4l1;
		c->capture.init_capture = init_capture_v4l1;
		c->capture.start_capture = start_capture_v4l1;
		c->capture.dequeue_buffer = dequeue_buffer_v4l1;
		c->capture.enqueue_buffer = enqueue_buffer_v4l1;
		c->capture.stop_capture = stop_capture_v4l1;
		c->capture.free_capture = free_capture_v4l1;
		c->capture.list_cap = list_cap_v4l1;
		c->capture.enum_image_fmt = enum_image_fmt_v4l1;
		c->capture.query_control = query_control;
		c->capture.query_frame_sizes = query_frame_sizes_v4l1;
		c->capture.query_capture_intf = query_capture_intf_v4l1;
		c->capture.query_current_image_fmt = query_current_image_fmt_v4l1;		
	} else {
		c->capture.set_cap_param = set_cap_param_v4l2;
		c->capture.init_capture = init_capture_v4l2;
		c->capture.start_capture = start_capture_v4l2;
		c->capture.dequeue_buffer = dequeue_buffer_v4l2;
		c->capture.enqueue_buffer = enqueue_buffer_v4l2;
		c->capture.stop_capture = stop_capture_v4l2;
		c->capture.free_capture = free_capture_v4l2;
		c->capture.list_cap = list_cap_v4l2;
		c->capture.enum_image_fmt = enum_image_fmt_v4l2;
		c->capture.query_control = query_control;
		c->capture.query_frame_sizes = query_frame_sizes_v4l2;
		c->capture.query_capture_intf = query_capture_intf_v4l2;
		c->capture.query_current_image_fmt = query_current_image_fmt_v4l2;		
	}
}


//device file, width, height, channel, std, nb_buf
struct capture_device *init_libv4l(const char *dev, int w, int h, int ch, int s, int nb_buf){
	//create capture device
	struct capture_device *c;
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Initialising libv4l and creating struct cdev\n");
	XMALLOC(c, struct capture_device *,sizeof(struct capture_device));
	XMALLOC(c->mmap, struct mmap *, sizeof(struct mmap));
	
	//fill in cdev struct
	c->mmap->req_buffer_nr = nb_buf;
	strcpy(c->file, dev);
	c->width = w;
	c->height = h;
	c->channel = ch;
	c->std = s;

	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Opening device %s\n", c->file);
	if(open_device(c)!=0) {
		XFREE(c->mmap);
		XFREE(c);
		return NULL;
	}

	//Check if V4L2 first
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Checking V4L version on device %s\n", c->file);
	if(check_capture_capabilities_v4l2(c)==0) {
		dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: device %s is V4L2\n", c->file);
		c->v4l_version=V4L2_VERSION;
	} else if(check_capture_capabilities_v4l1(c)==0){
		dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: device %s is V4L1\n", c->file);
		c->v4l_version=V4L1_VERSION;
	} else {
		info("libv4l was unable to detect the version of V4L used by device %s\n", c->file);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libv4l README file.\n");
		
		close_device(c);
		XFREE(c->mmap);
		XFREE(c);
		return NULL;
	}
	
	setup_libv4l_actions(c);

	c->ctrls = list_control(c);

	return c;
}

//counterpart of init_libv4l, must be called it init_libv4l was successful
void del_libv4l(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Freeing libv4l on device %s.\n", c->file);
	free_control_list(c->ctrls);
	close_device(c);
	XFREE(c->mmap);
	XFREE(c);
}
