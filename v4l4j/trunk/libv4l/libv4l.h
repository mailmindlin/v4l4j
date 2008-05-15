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

#ifndef H_LIBV4L
#define H_LIBV4L

#include "common.h"

//Put the version in string & return it. allocation and freeing must be done by caller
//passing a char[10] is enough.
char *get_libv4l_version(char *);

/*
 * Init methods
 * each of the init methods has a counterpart method that free resources created
 * by the corresponding init method. counterpart methods must be called if call
 * the init one was successful
 */

//init_libv4l initialises required struct, opens the device file, and check what
//version of v4l the device supports, and whether capture and streaming are supported.
//Then creates the V4L control list.
//Arguments: device file, width, height, channel, std, nb_buf
struct capture_device *init_libv4l(const char *, int, int, int, int, int);


//set the capture parameters
//int * point to an array of image formats to try (see libv4l.h for a list of supported formats)
//the last argument (int) tells how many formats there are in the previous argument
//arg2 can be set to NULL and arg3 to 0 to try the default order (again, see libv4l.h) 
int set_cap_param(struct capture_device *, int *, int);

//initialise streaming, request create mmap'ed buffers
int init_capture(struct capture_device *);

//tell V4L to start the capture
int start_capture(struct capture_device *);

/*
 * capture methods
 * these methods can be called if calls to ALL the init methods (above) were successful
 */

//dequeue the next buffer with available frame
struct v4l2_buffer *dequeue_buffer(struct capture_device *);

//get the address of the buffer where frame is
void *get_frame_buffer(struct capture_device *, struct v4l2_buffer *, int *);

//enqueue the buffer when done using the frame
void enqueue_buffer(struct capture_device *, struct v4l2_buffer *);
 

/*
 * Freeing resources
 * these methods free resources created by matching init methods. Note that
 * set_cap_param doesnt have a counterpart since it only sets values and doesnt
 * create additional resources.
 */

//counterpart of start_capture, must be called it start_capture was successful
int stop_capture(struct capture_device *);

//counterpart of init_capture, must be called it init_capture was successful
void free_capture(struct capture_device *);

//counterpart of init_libv4l2, must be called it init_libv4l2 was successful
void del_libv4l(struct capture_device *);


/*
 * Control methods
 * the available controls are listed in the 'ctrls' member of the struct 
 * capture_device returned by init_libv4l2.
 * to use the following methods, find the desired v4l2_queryctrl to be
 * accessed and call either get or set _control_value. These methods can
 * be used if call to init_libv4l2 was succesful, and until del_libv4l2 
 * is called
 */
int get_control_value(struct capture_device *, struct v4l2_queryctrl *);
void set_control_value(struct capture_device *, struct v4l2_queryctrl *,  int);

/*
 * Query and list methods (printf to stdout, use to debug)
 * these methods can be called after init_libv4l2 and before del_libv4l2
 */
void list_cap(struct capture_device *);				//prints results from query methods listed below
void enum_image_fmt(struct capture_device *);		//lists all supported image formats
void query_control(struct capture_device *);		//lists all supported controls
void query_frame_sizes(struct capture_device *);	// not implemented
void query_capture_intf(struct capture_device *);	//prints capabilities
void query_current_image_fmt(struct capture_device *);	//print max width max height for v4l1 and current settings for v4l2

#endif
