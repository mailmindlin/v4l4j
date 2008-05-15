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
#include "libv4l-err.h"
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
		info("Please let the author know about this error.\n", c->file);
		info("See the ISSUES section in the libv4l README file.\n", c->file);
		
		close_device(c);
		XFREE(c->mmap);
		XFREE(c);
		return NULL;
	}

	c->ctrls = list_control(c);

	return c;
}

// set the capture parameters, trying all 'nb' palettes in 'palettes' in order. 
int set_cap_param(struct capture_device *c, int *palettes, int nb) {
	int def[NB_SUPPORTED_PALETTE] = DEFAULT_PALETTE_ORDER;
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Setting capture parameters on device %s.\n", c->file);
	if(nb<0 || nb>=NB_SUPPORTED_PALETTE) {
		dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Incorrect number of palettes (%d)\n", nb);
		return LIBV4L_ERR_FORMAT;
	}
	if(nb==0 || palettes==NULL) {
		dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: No palettes supplied, trying default order.\n");
		palettes = def;
		nb = NB_SUPPORTED_PALETTE;
	}
	if(c->v4l_version==V4L1_VERSION)
		return set_cap_param_v4l1(c, palettes, nb); 
	else
		return set_cap_param_v4l2(c, palettes, nb);
}


//initialise streaming, request V4L2 buffers and create mmap'ed buffers
int init_capture(struct capture_device *c) {
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Initialising capture on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION)
		return init_capture_v4l1(c);
	else
		return init_capture_v4l2(c);
}

//tell V4L2 to start the capture
int start_capture(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Starting capture on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION)
		return start_capture_v4l1(c);
	else
		return start_capture_v4l2(c);
}

//dequeue the next buffer with available frame
void *dequeue_buffer(struct capture_device *c) {
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Dequeuing buffer on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION)
		return dequeue_buffer_v4l1(c);
	else
		return (void *) dequeue_buffer_v4l2(c);
}


//get the address of the buffer where frame is
void *get_frame_buffer(struct capture_device *c, void *b, int *i){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Getting buffer address on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION)
		return get_frame_buffer_v4l1(c, NULL, i);
	else
		return get_frame_buffer_v4l2(c, (struct v4l2_buffer *) b, i);
}

//enqueue the buffer when done using the frame
void enqueue_buffer(struct capture_device *c, void *b){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Queuing buffer on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION)
		return enqueue_buffer_v4l1(c, NULL);
	else
		return enqueue_buffer_v4l2(c, (struct v4l2_buffer *) b);
}
 

/*
 * Freeing resources
 * these methods free resources created by matching init methods. Note that
 * set_cap_param doesnt have a counterpart since it only sets values and doesnt
 * create additional resources.
 */

//counterpart of start_capture, must be called it start_capture was successful
int stop_capture(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Stopping capture on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION)
		return stop_capture_v4l1(c);
	else
		return stop_capture_v4l2(c);
}

//counterpart of init_capture, must be called it init_capture was successful
void free_capture(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Freeing capture on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION) {
		return free_capture_v4l1(c);
	} else {
		return free_capture_v4l2(c);
	}
}

//counterpart of init_libv4l, must be called it init_libv4l was successful
void del_libv4l(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Freeing libv4l on device %s.\n", c->file);
	free_control_list(c->ctrls);
	close_device(c);
	XFREE(c->mmap);
	XFREE(c);
}

/*
 * Query and list methods (printf to stdout, use to debug)
 * these methods can be called after init_libv4l and before del_libv4l
 */
//prints results from query methods listed below
void list_cap(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Listing capabilities on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION) {
		return list_cap_v4l1(c);
	} else {
		return list_cap_v4l2(c);
	}
}

//lists all supported image formats
void enum_image_fmt(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Enumerating image formats on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION) {
		enum_image_fmt_v4l1(c);
	} else {
		enum_image_fmt_v4l2(c);
	}
}

//lists supported frame sizes
void query_frame_sizes(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Listing frame sizes on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION) {
		query_frame_sizes_v4l1(c);
	} else {
		query_frame_sizes_v4l2(c);
	}
}

//prints capabilities
void query_capture_intf(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Querying capture channels on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION) {
		query_capture_intf_v4l1(c);
	} else {
		query_capture_intf_v4l2(c);
	}
}

void query_current_image_fmt(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_V4L, LIBV4L_LOG_LEVEL_DEBUG2, "V4L: Listing current settings on device %s.\n", c->file);
	if(c->v4l_version==V4L1_VERSION) {
		query_current_image_fmt_v4l1(c);
	} else {
		query_current_image_fmt_v4l2(c);
	}
}
