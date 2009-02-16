/*
	copyright 2006 Gilles GIGAN (gilles.gigan@gmail.com)

	This file is part of light_cap.

   	light_cap is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    light_cap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with light_cap; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef H_UTILS
#define H_UTILS

#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include "log.h"

#define DEFAULT_NB_ARGS					(9 + 1)

//tcp stuff
#define ACTION_CAPTURE					0
#define ACTION_LIST						1
#define TCP_PORT						8080
#define OUTPUT_BLOCK_SIZE				65536
#define INPUT_BLOCK_SIZE				512
//do NOT change the value for MAX_CLIENT without implementing
//proper locking in main.c !!
#define MAX_CLIENTS						1
#define PARAM_PAGE_SIZE					4096

#define JPEG_QUALITY					10

//MJPEG stuff
#define MP_JPEG_HTTP_BOUNDARY 			"boundary"
#define MP_JPEG_HTTP_BOUNDARY_LENGTH 	8
#define MP_JPEG_HTTP_HEADER 			"HTTP/1.0 200 OK\r\nExpires: 0\r\nPragma: no-cache\r\nCache-Control: no-cache\r\nContent-Type: multipart/x-mixed-replace;boundary=\"%s\"\r\n\r\n"
#define MP_JPEG_HTTP_HEADER_MAX_LENGTH 	(128 + MP_JPEG_HTTP_BOUNDARY_LENGTH + 1)
#define MP_JPEG_FRAME_HEADER			"--boundary\r\nContent-Type: image/jpeg\r\nContent-Length: "//%d\r\n\r\n"
//assuming a 9 digit length field (just under 100Mb)
#define MP_JPEG_FRAME_HEADER_MAX_LENGTH	49//( 58 + 9 + 1)
#define SHOW_FPS_INTERVAL				10

//wrapper for malloc(): 1- allocate mem, 2- check for NULL return, 3-zeroes the allocated mem, 4- print debug info
#define XMALLOC(var, type, size)	\
		do { \
			var = (type) malloc(size); \
			if (!var) { info(LOG_ERR, "MEMALLOC: Cant allocate %d bytes of type %s for var %s.\n", size, #type, #var); } \
			else { memset(var, 0, size);\
				dprint(LOG_SOURCE_MEMALLOC, LOG_LEVEL_DEBUG2, "MEMALLOC: allocated %d bytes of type %s for var %s (%p).\n", size, #type, #var, (void *) var); } \
		} while (0)
//wapper for free(): 1- check free(NULL), 2- print debug info
#define XFREE(var)					\
		do { dprint(LOG_SOURCE_MEMALLOC, LOG_LEVEL_DEBUG2, "MEMALLOC: freeing memory for var %s (%p).\n", #var, var); \
			if (var) { free(var); } \
			else { info(LOG_ERR , "MEMALLOC: Trying to free a NULL pointer.\n");} \
		} while (0)


struct jpeg {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_destination_mgr destmgr;
	int (*jpeg_encode) (void *, int, struct capture_device * , struct jpeg *, void *);
};

struct thread_data {
	int sock;
	struct video_device	*vdev;
};


//function prototypes
void set_fps(int);
void incr_nanosleep();
void decr_nanosleep();
void list_cap_param(int,struct video_device *);
int get_action(int, struct video_device *);
int send_frame(int , void *, int);
void start_thread_client(int, struct video_device *);
int send_mjpeg_header(int);
void *send_stream_to(void *);
int setup_tcp_server_sock(int );
void main_loop(int , struct video_device *);
void print_usage();

#endif
