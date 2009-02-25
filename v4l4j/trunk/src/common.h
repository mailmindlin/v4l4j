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

#ifndef H_COMMON__
#define H_COMMON__

#include <stdio.h>
#include <jpeglib.h>

#include "libv4l.h"

struct v4l4j_device;

struct jpeg_data {
	struct jpeg_compress_struct *cinfo;
	struct jpeg_destination_mgr *destmgr;
	struct jpeg_error_mgr *jerr;
	int jpeg_quality;			//the jpeg quality, set to -1 if disable
};

struct rgb_data {
	struct jpeg_decompress_struct *cinfo;
	struct jpeg_source_mgr *srcmgr;
	struct jpeg_error_mgr *jerr;
	int nb_pixel;
};

enum output_format {
	OUTPUT_RAW,
	OUTPUT_JPG,
	OUTPUT_RGB24
} ;


struct v4l4j_device {
	void (*convert) (struct v4l4j_device *, unsigned char *, unsigned char *);
	struct video_device *vdev;	//the libv4l struct
	union {
		struct jpeg_data *j;	//the converter's data
		struct rgb_data *r;
	};
	enum output_format output_fmt;	//the output format (see enum above)
	unsigned char **bufs;		//the buffers holding the last JPEG compressed frame
	int capture_len;			//the size of the last captured frame returned by libv4l
	int len;					//the size of the frame after conversion
	int buf_id;					//the index of the buffer where the next frame goes
};

#define JPEG_SUPPORTED_FORMATS		{JPEG, MJPEG, YUV420, YUYV, RGB24, RGB32, BGR24, YVYU, UYVY, BGR32}
#define NB_JPEG_SUPPORTED_FORMATS	10

#define RGB24_SUPPORTED_FORMATS		{RGB24, BGR24, BGR32, RGB32, YUYV, UYVY, YVYU, YUV420, JPEG, MJPEG}
#define NB_RGB24_SUPPORTED_FORMATS	10

#define BYTEBUFER_CLASS			"java/nio/ByteBuffer"
#define V4L4J_PACKAGE			"au/edu/jcu/v4l4j/"
#define FRAMEGRABBER_CLASS		V4L4J_PACKAGE "FrameGrabber"
#define CONTROL_CLASS			V4L4J_PACKAGE "Control"
#define CONSTANTS_CLASS			V4L4J_PACKAGE "V4L4JConstants"
#define EXCEPTION_PACKAGE		V4L4J_PACKAGE "exceptions/"
#define GENERIC_EXCP			EXCEPTION_PACKAGE "V4L4JException"
#define INIT_EXCP				EXCEPTION_PACKAGE "InitialisationException"
#define DIM_EXCP				EXCEPTION_PACKAGE "ImageDimensionException"
#define CHANNEL_EXCP			EXCEPTION_PACKAGE "CaptureChannelException"
#define FORMAT_EXCP				EXCEPTION_PACKAGE "ImageFormatException"
#define STD_EXCP				EXCEPTION_PACKAGE "VideoStandardException"
#define CTRL_EXCP				EXCEPTION_PACKAGE "ControlException"
#define RELEASE_EXCP			EXCEPTION_PACKAGE "ReleaseException"
#define INVALID_VAL_EXCP		EXCEPTION_PACKAGE "InvalidValue"
#define JNI_EXCP				EXCEPTION_PACKAGE "JNIException"


/* Exception throwing helper */
#define EXCEPTION_MSG_LENGTH	100
#define THROW_EXCEPTION(e, c, format, ...)\
								do {\
									char msg[EXCEPTION_MSG_LENGTH+1];\
									jclass JV4L4JException = (*e)->FindClass(e,c);\
									snprintf(msg, EXCEPTION_MSG_LENGTH, format, ## __VA_ARGS__);\
									if(JV4L4JException!=0) (*e)->ThrowNew(e, JV4L4JException, msg);\
								} while(0)



#endif /*H_COMMON_*/
