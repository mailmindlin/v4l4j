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

#ifndef H_V4L2_INPUT
#define H_V4L2_INPUT

#include "common.h"


/*
 * Init methods
 * each of the init methods has a counterpart method that free resources created
 * by the corresponding init method. counterpart methods must be called if call
 * the init one was successful
 */

//init_libv4l2 initialises required struct, opens the device file, and check if
//device supports v4l2, capture and streaming. Then creates the V4L control list
struct capture_device *init_libv4l2(const char *, int, int, int, int, int);

//Check whether the device is V4L2 and has capture and mmap capabilities
int check_capture_capabilities_v4l2(struct capture_device *);

// set the capture parameters (hardcoded to require YUV420 for now
int set_cap_param_v4l2(struct capture_device *, int *, int);

//initialise streaming, request V4L2 buffers and create mmap'ed buffers
int init_capture_v4l2(struct capture_device *);

//tell V4L2 to start the capture
int start_capture_v4l2(struct capture_device *);


/*
 * capture methods
 * these methods can be called if calls to all the init methods were successful
 */

//dequeue the next buffer with available frame
struct v4l2_buffer *dequeue_buffer_v4l2(struct capture_device *);

//get the address of the buffer where frame is
void *get_frame_buffer_v4l2(struct capture_device *, struct v4l2_buffer *, int *);

//enqueue the buffer when done using the frame
void enqueue_buffer_v4l2(struct capture_device *, struct v4l2_buffer *);
 

/*
 * Freeing resources
 * these methods free resources created by matching init methods. Note that
 * set_cap_param doesnt have a counterpart since it only sets values and doesnt
 * create additional resources.
 */

//counterpart of start_capture, must be called it start_capture was successful
int stop_capture_v4l2(struct capture_device *);

//counterpart of init_capture, must be called it init_capture was successful
void free_capture_v4l2(struct capture_device *);

//counterpart of init_libv4l2, must be called it init_libv4l2 was successful
void del_libv4l2(struct capture_device *);


/*
 * Control related functions
 */
 //returns the number of controls (standard and private V4L2 controls only)
int count_v4l2_controls(struct capture_device *);
//Populate the control_list with reported V4L2 controls
//and returns how many controls were created
int create_v4l2_controls(struct capture_device *, struct control_list *);
//returns the value of a control
int get_control_value_v4l2(struct capture_device *, struct v4l2_queryctrl *);
//sets the value of a control
void set_control_value_v4l2(struct capture_device *, struct v4l2_queryctrl *, int);


/*
 * Query and list methods (printf to stdout, use to debug)
 * these methods can be called after init_libv4l2 and before del_libv4l2
 */
void list_cap_v4l2(struct capture_device *);			//prints results from query methods listed below
void enum_image_fmt_v4l2(struct capture_device *);		//lists all supported image formats
void query_frame_sizes_v4l2(struct capture_device *);		// not implemented
void query_capture_intf_v4l2(struct capture_device *);		//prints capabilities
void query_current_image_fmt_v4l2(struct capture_device *);	//useless...

#endif
