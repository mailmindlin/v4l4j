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

#ifndef H_COMMON
#define H_COMMON

#include <asm/types.h>		//for videodev2
#include "videodev2.h"
#include "videodev.h"
#include "libv4lconvert.h"

#define CLEAR(x) memset(&x, 0x0, sizeof(x));


/*
 *
 * C A P TU R E   I N T E R F A C E   S T R U C T S
 *
 */
struct mmap_buffer {
	void *start;		//start of the mmaped buffer
	int length;			//length of the mmaped buffer as given by v4l
						//does NOT indicate the length of the frame,
						//use struct capture_device->imagesize instead
};

struct mmap {
	int req_buffer_nr;				//requested number of buffers
	int buffer_nr;					//actual number of mmap buffers
	struct mmap_buffer *buffers;	//array of buffers
	void * tmp;						//temp buffer pointing to the latest
									//dequeued buffer (V4L2) - last
									//requested frame (V4L1)
	int v4l1_mmap_size;				//used by v4l1 only, to store the overall
									//mmap size
};


//
// Passing these values as desired width and height for the capture
// will result libvideo using the maximun width and height allowed
//
#define MAX_WIDTH					0
#define MAX_HEIGHT					0

//Supported standards
#define		WEBCAM					0
#define		PAL						1
#define		SECAM					2
#define		NTSC					3

//palette formats
//YUV420 is the same as YUV420P - YUYV is the same as YUV422
//YUV411 is the same as YUV411P
//DO NOT USE YUV420P, YUV422 or YUV411 - they re here for compatibility
//USE YUV420, YUYV and YUV411P INSTEAD !!!!
enum {
	// RGB formats
	RGB332 = 0,			// 0
	RGB444,				// 1
	RGB555,				// 2
	RGB565,				// 3
	RGB555X,			// 4
	RGB565X,			// 5
	BGR24,				// 6
	RGB24,				// 7
	BGR32,				// 8
	RGB32,				// 9

	// Grey formats
	GREY,				//10
	Y4,
	Y6,
	Y10,
	Y16,

	// palette formats
	PAL8,

	// YUV formats
	YVU410,
	YVU420,
	YUYV,
	YYUV,
	YVYU,				//20
	UYVY,
	VYUY,
	YUV422P,
	YUV411P,
	Y41P,
	YUV444,
	YUV555,
	YUV565,
	YUV32,
	YUV410,
	YUV420,
	HI240,
	HM12,

	// Two planes - Y and Cb/Cr interleaved
	NV12,
	NV21,
	NV16,
	NV61,

	// Bayer formats
	SBGGR8,
	SGBRG8,
	SGRBG8,
	SRGGB8,
	SBGGR10,
	SGBRG10,
	SGRBG10,
	SRGGB10,
	SGRBG10DPCM8,
	SBGGR16,

	// Compressed formats
	MJPEG,
	JPEG,
	DV,
	MPEG,

	// Vendor-specific formats
	CPIA1,
	WNVA,
	SN9C10X,
	SN9C20X_I420,
	PWC1,
	PWC2,
	ET61X251,
	SPCA501,
	SPCA505,
	SPCA508,
	SPCA561,
	PAC207,
	MR97310A,
	SN9C2028,
	SQ905C,
	PJPG,
	OV511,
	OV518,
	STV0680,
	TM6000,
	CIT_YYVYUY,
	KONICA420,
	NB_SUPPORTED_PALETTES,
	/* Dont use the following three, use YUV420, YUYV or YUV411P instead !! */\
	YUV420P,
	YUV422,
	YUV411,
#define		IDENTICAL_FORMATS		3
};


struct convert_data {
	struct v4lconvert_data *priv;//the libv4l convert struct (used only if V4L2)
	struct v4l2_format *src_fmt; //the source pixel format used for capture
	struct v4l2_format *dst_fmt; //the dest format, after conversion
	int src_palette;			//the source libvideo palette index
	void *frame;				//the last captured frame buffer after conv
								//the length of the buffer is set to
								//dst_fmt->fmt.pix.sizeimage
};

//all the fields in the following structure are read only
struct capture_device {
	struct mmap *mmap;				//do not touch
	int palette;					//the image format returned by libvideo
									//see #define above
	int width;						//captured frame width
	int height;						//captured frame width
	int std;						//v4l standard - see #define enum above
	int channel;					//channel number (0 for webcams)
	int imagesize;					//in bytes
	int tuner_nb;					//the index of the tuner associated with
									//this capture_device, -1 if not tuner input
	struct capture_actions *actions;	//see def below
	int is_native;					// 1 if the selected image format is native, or
									// or if the selected format is not native,
									// but is still reported as a native format
									// because libv4lconvert cannot handle capture
									// in the actual native format. 0 oterhwise.

	int real_v4l1_palette;			//v4l1 weirdness: v4l1 defines 2 distinct
									//palettes YUV420 and YUV420P but they are
									//the same (same goes for YUYV and YUV422).
									//In this field we store the real palette
									//used by v4l1. In the palette field above,
									//we store what the application should know
									//(YUYV instead of YUV422)
	int needs_conversion;			//this field is meaningful only with v4l2.
									//for v4l1, it is always set to 0.
									//it specifies whether or not captured frames
									//needs to be passed to libv4lconvert after
									//being captured to be converted.if set to 1,
									//then the convert member is valid
	struct convert_data* convert;	//do not touch - libv4lconvert stuff
									//(used only when v4l2)
									//only valid if is_native is 0
};


/*
 *
 * Q U E R Y   I N T E R F A C E   S T R U C T S
 *
 */
#define NAME_FIELD_LENGTH 			32 + 1

struct tuner_info {
#define RADIO_TYPE					1
#define TV_TYPE						2
	int type;
	//In units of 62.5 Khz if unit==MHZ_UNIT
	//of 62.5Hz if unit==KHZ_UNIT
	unsigned long rangelow;
	unsigned long rangehigh;
#define KHZ_UNIT					1
#define MHZ_UNIT					2
	int unit;
	int rssi;
	char name[NAME_FIELD_LENGTH];
	int index;
};

struct video_input_info {
#define INPUT_TYPE_TUNER			1
#define INPUT_TYPE_CAMERA			2
	int type;
	int nb_stds;
	int *supported_stds;
	struct tuner_info *tuner;
	char name[NAME_FIELD_LENGTH];
	int index;
};

/*
 * Frame intervals
 */

struct frame_intv_discrete{
	unsigned int numerator;
	unsigned int denominator;
};

struct frame_intv_continuous{
	struct frame_intv_discrete min;
	struct frame_intv_discrete max;
	struct frame_intv_discrete step;
};

enum frame_intv_types{
	FRAME_INTV_UNSUPPORTED=0,
	FRAME_INTV_DISCRETE,
	FRAME_INTV_CONTINUOUS
};

/*
 * Frame size
 */
struct frame_size_discrete {
	int width;
	int height;

	//the type of frame frame intervals (V4L2 only)
	enum frame_intv_types interval_type;

	//if interval_type==FRAME_INTV_CONTINUOUS, then only the continuous member
	//of the union is valid and points to a single struct frame_intv_continuous
	//if interval_type==FRAME_INTV_DISCRETE, then only the discrete member
	//of the union is valid and points to an array of struct
	//frame_intv_discrete. The last element in the array has its members (num
	//& denom) set to 0.
	//if interval_type==FRAME_INTV_UNSUPPORTED, then none of the two members are
	//valid
	union {
		struct frame_intv_continuous *continuous;
		struct frame_intv_discrete *discrete;
	} intv;
};

struct frame_size_continuous {
	int min_width;
	int max_width;
	int step_width;
	int min_height;
	int max_height;
	int step_height;

	//the type of frame frame intervals (V4L2 only)
	//for the minimum and maximum resolutions
	//(min_width X min_height) and (max_width X max_height)
	enum frame_intv_types interval_type_min_res;
	enum frame_intv_types interval_type_max_res;

	//if interval_type==FRAME_INTV_CONTINUOUS, then only the continuous member
	//of the union is valid and points to a single struct frame_intv_continuous
	//if interval_type==FRAME_INTV_DISCRETE, then only the discrete member
	//of the union is valid and points to an array of struct
	//frame_intv_discrete. The last element in the array has its members (num
	//& denom) set to 0.
	//if interval_type==FRAME_INTV_UNSUPPORTED, then none of the two members are
	//valid
	union {
		struct frame_intv_continuous *continuous;
		struct frame_intv_discrete *discrete;
	} intv_min_res;
	union {
		struct frame_intv_continuous *continuous;
		struct frame_intv_discrete *discrete;
	} intv_max_res;
};

enum frame_size_types{
	FRAME_SIZE_UNSUPPORTED=0,
	FRAME_SIZE_DISCRETE,
	FRAME_SIZE_CONTINUOUS
};

/*
 * a palette (or image format) can be either raw or converted.
 * A raw palette is a palette produced by the driver natively.
 * A converted palette is a palette which is converted by libvideo from
 * a raw palette
 */
struct palette_info{
	//this palette's index
	int index;

	//if raw_palettes is not NULL, then this palette is a converted palette and
	//raw_palettes contains an array of indexes of raw palettes. It is used
	//for V4L2 only. The last element in the array is -1
	//if raw_palettes is NULL, this palette is raw, and size_type and
	//(continuous or discrete) are valid (check value of size_type).
	//If raw_palettes is not NULL, then
	//size_type=FRAME_SIZE_UNSUPPORTED, continuous and discrete and not valid.
	int *raw_palettes;

	//the type of frame sizes (V4L2 only)
	enum frame_size_types size_type;

	//if size_type==FRAME_SIZE_CONTINUOUS, then only the continuous member
	//of this union is valid, and points to a single
	//struct frame_size_continuous
	//if size_type==FRAME_SIZE_DISCRETE, then only the discrete member
	//of this union is valid, and points to an array of
	//struct frame_size_discrete. The last element in the array has its members
	//(width and height) set to 0.
	//if size_type==FRAME_SIZE_UNSUPPORTED, then none of the two members are
	//valid
	union {
		struct frame_size_continuous *continuous;
		struct frame_size_discrete *discrete;
	};
};

struct device_info {
	int nb_inputs;
	struct video_input_info *inputs;
	int nb_palettes;
	struct palette_info *palettes;
	char name[NAME_FIELD_LENGTH];
	//this function enumerates the frame intervals for a given video format
	//, width and height and modifies the pointer at p to point to either
	//NULL, a struct frame_intv_discrete, struct frame_intv_continuous. It returns
	//FRAME_INTV_UNSUPPORTED (p points  to NULL),
	//FRAME_INTV_DISCRETE (p points to a an array of struct frame_intv_discrete, the
	//last struct has its members set to 0) or
	//FRAME_INTV_CONTINUOUS (p points to a struct frame_intv_continuous)
	//The returned pointer must be freed by the caller (using free()).
	//						fmt, width, height, p
	int (*list_frame_intv)(struct device_info*, int, int , int , void **);
	//valid only for v4l2, dont touch
	struct v4lconvert_data *convert;
};


/*
 *
 * C O N T R O L   I N T E R F A C E   S T R U C T S
 *
 */

//struct used to represent a driver probe.
struct control {
	struct v4l2_queryctrl *v4l2_ctrl;
	struct v4l2_querymenu *v4l2_menu;//array of 'count_menu' v4l2_menus
	int count_menu;
};

struct video_device;
struct v4l_driver_probe {
	int (*probe) (struct video_device *, void **);
	int (*list_ctrl)(struct video_device *, struct control *, void *);
	int (*get_ctrl)(struct video_device *, struct v4l2_queryctrl *,
			void *, int *);
	int (*set_ctrl)(struct video_device *,  struct v4l2_queryctrl *,
			int *, void *);
	void *priv;
};
/*
 * element in linked list of driver probe
 */
typedef struct struct_elem {
	struct v4l_driver_probe *probe;
 	struct struct_elem *next;
} driver_probe;


struct control_list {
	int count;						//how many controls are available
	struct control *controls;		//array of 'count' struct control's
	driver_probe *probes; 			//linked list of driver probes, allocated in
									//libvideo.c:get_control_list()
	struct v4lconvert_data *priv;//the libv4l convert struct (used only if V4L2)
								//DO NOT TOUCH
};

/*
 *
 *  T U N E R   I N T E R F A C E
 *
 */
struct tuner_actions{
	//returns 0 if OK, LIBVIDEO_ERR_IOCTL otherwise
	int (*set_tuner_freq)(struct video_device *, int, unsigned int);
	int (*get_tuner_freq)(struct video_device *, int, unsigned int *);
	int (*get_rssi_afc)(struct video_device *, int, int *, int *);
};

/*
 *
 * V I D E O   D E V I C E   I N T E R F A C E   S T R U C T S
 *
 */
#define FILENAME_LENGTH				99 + 1

struct video_device {
	int fd;

#define V4L1_VERSION				1
#define V4L2_VERSION				2
	int v4l_version;
	char file[FILENAME_LENGTH];
	struct device_info *info;
	struct capture_device *capture;
	struct control_list *control;
	struct tuner_actions *tuner_action;
};




/*
 *
 * P U B L I C   I N T E R F A C E S   M E T H O D S
 *
 */

//Put the version in string & return it. allocation and freeing
//must be done by caller
//passing a char[10] is enough.
char *get_libvideo_version(char *);

/*
 *
 * VIDEO DEVICE INTERFACE
 *
 */
//Creates a video_device (must call close_device() when done)
struct video_device *open_device(char *);
int close_device(struct video_device *);

/*
 *
 * CAPTURE INTERFACE
 *
 */

//init_capture_device creates and initialises a struct capture_device,
//opens the device file, checks what version of v4l the device supports,
//and whether capture and streaming are supported. Then creates the V4L
//control list. Arguments: device file, width, height, channel, std, nb_buf
struct capture_device *init_capture_device(struct video_device *, int, int,
		int, int, int);


/*
 * functions pointed to by the members of this structure should be used
 * by the calling application, to capture frame from the video device.
 */
struct capture_actions {
/*
 * Init methods
 */
//set the capture parameters
//int * point to an array of image formats (palettes) to try
//(see bottom of libvideo.h for a list of supported palettes)
//the last argument (int) tells how many formats there are in
//the previous argument, arg2 can be set to NULL and arg3 to 0 to
//try the default order (again, see libvideo.h)
//returns: LIBVIDEO_ERR_FORMAT (no supplied format could be used),
//LIBVIDEO_ERR_STD (the supplied standard could not be used),
// LIBVIDEO_ERR_CHANNEL (the supplied channel is invalid)
// LIBVIDEO_ERR_CROP (error applying cropping parameters)
// or LIBVIDEO_ERR_NOCAPS (error checking capabilities)
	int (*set_cap_param)(struct video_device *, int , int);

//set the frame interval for capture, ie the number of seconds in between
//each captured frame. This function is available only for V4L2 devices
//whose driver supports this feature. It cannot be called during capture.
//This function returns LIBVIDEO_ERR_IOCTL on v4l1 devices and on v4l2
//device which do not support setting frame intervals. It returns
//LIBVIDEO_ERR_FORMAT if the given parameters are incorrect, or 0 if
//everything went fine. The driver may adjust the given values to the closest
//supported ones, which can be check with get_frame_interval()
	int (*set_frame_interval)(struct video_device *, int, int);

//get the current frame interval for capture, ie the number of seconds in
//between each captured frame. This function is available only for V4L2 devices
//whose driver supports this feature. It cannot be called during capture.
//This function returns LIBVIDEO_ERR_IOCTL on v4l1 devices and on v4l2
//device which do not support setting frame intervals or 0 if
//everything went fine
	int (*get_frame_interval)(struct video_device *, int *, int *);


//Change the current video input and standard during capture
	int (*set_video_input_std)(struct video_device*, int, int);
//Get the current video input and standard during capture
	void (*get_video_input_std)(struct video_device*, int*, int*);

//initialise streaming, request create mmap'ed buffers
//returns 0 if ok, LIBVIDEO_ERR_REQ_MMAP if error negotiating mmap params,
//LIBVIDEO_ERR_INVALID_BUF_NB if the number of requested buffers is incorrect

	int (*init_capture)(struct video_device *);

//tell V4L to start the capture
//returns 0 if ok, LIBVIDEO_ERR_IOCTL otherwise.
	int (*start_capture)(struct video_device *);

/*
 * capture methods
 * these methods can be called if calls to ALL the init methods
 * (above) were successful
 */

//dequeue the next buffer with available frame, or NULL if there is an error
//the second int * argument receive the frame length
//the third argument receives the index of the buffer which contains the returned frame
//the fourth argument receives the capture time in microseconds (for v4l2 devices only)
//the last argument receives the capture frame sequence number (for v4l2 devices only)
	void * (*dequeue_buffer)(struct video_device *, int * ,	unsigned int *, unsigned long long *, unsigned long long *);

//convert the previously dequeued buffer at the given index. Call me only if
//the conversion is needed (if the requested format is not native)
	unsigned int (*convert_buffer)(struct video_device *vdev, int index, unsigned int src_len, void *dest_buffer);

//enqueue the buffer (given its index) when done using the frame
	void (*enqueue_buffer)(struct video_device *, unsigned int);


/*
 * Freeing resources
 * these methods free resources created by matching init methods. Note that
 * set_cap_param doesnt have a counterpart since it only sets values and doesnt
 * create additional resources.
 */

//counterpart of start_capture, must be called it start_capture was successful
//returns 0 if ok, LIBVIDEO_ERR_IOCTL otherwise
	int (*stop_capture)(struct video_device *);

//counterpart of init_capture, must be called it init_capture was successful
	void (*free_capture)(struct video_device *);

/*
 * Dump to stdout methods
 * Must be called after init_capture_device and before free_capture_device
 */
	void (*list_cap)(int);	//lists all supported image formats
							//prints capabilities
							//print max width max height for v4l1
							//and current settings for v4l2
};

//counterpart of init_capture_device, must be called if init_capture_device
//was successful
void free_capture_device(struct video_device *);



/*
 *
 * QUERY INTERFACE
 *
 *
 */
//returns NULL if unable to get device info
struct device_info * get_device_info(struct video_device *);
void print_device_info(struct video_device *);
void release_device_info(struct video_device *);

/*
 *
 * CONTROL INTERFACE
 *
 */
struct control_list *get_control_list(struct video_device *);
//returns 0, LIBVIDEO_ERR_WRONG_VERSION, LIBVIDEO_ERR_IOCTL
int get_control_value(struct video_device *, struct v4l2_queryctrl *, void *, int);
//returns 0, LIBVIDEO_ERR_WRONG_VERSION, LIBVIDEO_ERR_IOCTL or LIBVIDEO_ERR_STREAMING
int set_control_value(struct video_device *, struct v4l2_queryctrl *,  void *, int);

void release_control_list(struct video_device *);

/*
 *
 * TUNER INTERFACE
 *
 */
struct tuner_actions *get_tuner_actions(struct video_device *);
void release_tuner_actions(struct video_device *);


#endif
