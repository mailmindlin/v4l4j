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

#ifndef H_V4L1_INPUT
#define H_V4L1_INPUT

#include "libv4l.h"


/*
 * Init methods
 * each of the init methods has a counterpart method that free resources created
 * by the corresponding init method. counterpart methods must be called if call
 * the init one was successful
 */


//Check whether the device is V4L1 and has capture and mmap capabilities
// get capabilities VIDIOCGCAP - check max height and width
int check_capture_capabilities_v4l1(struct capture_device *);

// set the capture parameters trying the nb(last arg) palettes(second arg) in order
int set_cap_param_v4l1(struct capture_device *, int *, int);
// set video channel 	VIDIOCSCHAN - 
// set picture format 	VIDIOCSPICT - 
// set window 		VIDIOCSWIN  
// get window format	VIDIOCGWIN  (to double check)


//initialise streaming, request V4L2 buffers and create mmap'ed buffers
int init_capture_v4l1(struct capture_device *);
// get streaming cap details VIDIOCGMBUF

//tell V4L2 to start the capture
int start_capture_v4l1(struct capture_device *);
// start the capture of first buffer VIDIOCMCAPTURE(0)


/*
 * capture methods
 * these methods can be called if calls to all the init methods were successful
 */

//dequeue the next buffer with available frame
void *dequeue_buffer_v4l1(struct capture_device *);
// start the capture of next buffer VIDIOCMCAPTURE(x)
// wait till the previous buffer is available VIDIOCSYNC(x-1)


//enqueue the buffer when done using the frame
void enqueue_buffer_v4l1(struct capture_device *);
 

/*
 * Freeing resources
 * these methods free resources created by matching init methods. Note that
 * set_cap_param doesnt have a counterpart since it only sets values and doesnt
 * create additional resources.
 */

//counterpart of start_capture, must be called it start_capture was successful
int stop_capture_v4l1(struct capture_device *);

//counterpart of init_capture, must be called it init_capture was successful
void free_capture_v4l1(struct capture_device *);

//counterpart of init_libv4l1, must be called it init_libv4l2 was successful
void del_libv4l1(struct capture_device *);



/*
 * Control related functions
 */
 //returns the number of controls (standard and private V4L2 controls only)
int count_v4l1_controls(struct capture_device *);
//Populate the control_list with fake V4L2 controls matching V4L1 video
//controls and returns how many fake controls were created
int create_v4l1_controls(struct capture_device *, struct control_list *);
//returns the value of a control
int get_control_value_v4l1(struct capture_device *c, struct v4l2_queryctrl *ctrl);
//sets the value of a control
void set_control_value_v4l1(struct capture_device *, struct v4l2_queryctrl *, int);


/*
 * Query and list methods (printf to stdout, use to debug)
 * these methods can be called after init_libv4l2 and before del_libv4l2
 */
void list_cap_v4l1(struct capture_device *);			//prints results from query methods listed below
void enum_image_fmt_v4l1(struct capture_device *);		//lists all supported image formats
void query_frame_sizes_v4l1(struct capture_device *);		//lists min and max frame sizes
void query_capture_intf_v4l1(struct capture_device *);		//prints capabilities
void query_current_image_fmt_v4l1(struct capture_device *);	//

#endif
