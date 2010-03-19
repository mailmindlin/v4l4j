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

#include <stdint.h>

#ifdef __linux__
#include <asm/types.h>		//for videodev2
#include "videodev2.h"
#include "videodev.h"
#include "libv4lconvert.h"
#include "libvideo-linux.h"
#endif		// linux



/*
 *
 * C A P TU R E   I N T E R F A C E   S T R U C T S
 *
 */

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
	Y10,
	Y16,				//12

	// palette formats
	PAL8,				// 13

	// YUV formats
	YVU410,				// 14
	YVU420,
	YUYV,
	YYUV,
	YVYU,
	UYVY,
	VYUY,				// 20
	YUV422P,
	YUV411P,
	Y41P,
	YUV444,
	YUV555,
	YUV565,
	YUV32,
	YUV410,
	YUV420,
	HI240,				// 30
	HM12,

	// Two planes - Y and Cb/Cr interleaved
	NV12,				// 32
	NV21,
	NV16,
	NV61,				// 35

	// Bayer formats
	SBGGR8,				// 36
	SGBRG8,
	SGRBG8,
	SRGGB8,
	SBGGR10,			// 40
	SGBRG10,
	SGRBG10,
	SRGGB10,
	SGRBG10DPCM8,
	SBGGR16,			// 45

	// Compressed formats
	MJPEG,				//46
	JPEG,
	DV,
	MPEG,				// 49

	// Vendor-specific formats
	WNVA,				// 50
	SN9C10X,
	SN9C20X_I420,
	PWC1,
	PWC2,
	ET61X251,			// 55
	SPCA501,
	SPCA505,
	SPCA508,
	SPCA561,
	PAC207,				// 60
	MR97310A,
	SQ905C,
	PJPG,
	OV511,
	OV518,				// 65
	STV0680,
	TM6000,				// 67
	NB_SUPPORTED_PALETTES,	// 68
	/* Dont use the following three, use YUV420, YUYV or YUV411P instead !! */\
	YUV420P,
	YUV422,
	YUV411,
#define		IDENTICAL_FORMATS		3
};

//the default order in which palettes are tried if "set_cap_param(c, NULL, 0)"
//is used
#define		DEFAULT_PALETTE_ORDER {\
		JPEG,\
		YUV420,\
		MJPEG,\
		RGB24,\
		RGB32,\
		RGB332,\
		RGB444,\
		RGB555,\
		RGB565,\
		RGB555X,\
		RGB565X,\
		BGR24,\
		BGR32,\
		GREY,\
		Y10,\
		Y16,\
		PAL8,\
		YVU410,\
		YVU420,\
		YUYV,\
		YYUV,\
		YVYU,\
		UYVY,\
		VYUY,\
		YUV422P,\
		YUV411P,\
		Y41P,\
		YUV444,\
		YUV555,\
		YUV565,\
		YUV32,\
		YUV410,\
		HI240,\
		HM12,\
		NV12,\
		NV21,\
		NV16,\
		NV61,\
		SBGGR8,\
		SGBRG8,\
		SGRBG8,\
		SRGGB8,\
		SBGGR10,\
		SGBRG10,\
		SGRBG10,\
		SRGGB10,\
		SGRBG10DPCM8,\
		SBGGR16,\
		MPEG,\
		WNVA,\
		SN9C10X,\
		SN9C20X_I420,\
		PWC1,\
		PWC2,\
		ET61X251,\
		SPCA501,\
		SPCA505,\
		SPCA508,\
		SPCA561,\
		PAC207,\
		MR97310A,\
		SQ905C,\
		PJPG,\
		OV511,\
		OV518,\
		STV0680,\
		TM6000\
	}

//all the fields in the following structure are read only
struct capture_device {
	struct capture_actions *actions;	//see def below
	struct capture_backend *backend;		// platform-specific struct
	struct convert_data* convert;	//do not touch - libv4lconvert stuff
									//(used only when v4l2)
									//only valid if is_native is 0
	int is_native;					//this field is meaningful only with v4l2.
									//for v4l1, it is always set to 1.
									//it specifies whether or not the palette is
									//native, ie, whether it is converted from
									//a native format or it actually is a native
									//format. if it is converted (by libv4l
									//convert), then the convert member is valid
	int palette;					//the image format returned by libvideo
									//see #define above
	int width;						//captured frame width
	int height;						//captured frame width
	int std;						//v4l standard - see #define enum above
	int channel;					//channel number (0 for webcams)
	int imagesize;					//in bytes
	int tuner_nb;					//the index of the tuner associated with
									//this capture_device, -1 if not tuner input
	int buffer_nr;					//actual number of buffers
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
	struct palette_info *palettes;	// array of nb_palettes elements
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

struct menu {	// based on v4l2_ctrlmenu
	unsigned int		id;
	unsigned int		index;
	char				name[32];	/* Whatever */
	unsigned int		reserved;
};

enum ctrl_type {
	CTRL_TYPE_INTEGER	     = 1,
	CTRL_TYPE_BOOLEAN	     = 2,
	CTRL_TYPE_MENU	     = 3,
	CTRL_TYPE_BUTTON	     = 4,
	CTRL_TYPE_INTEGER64     = 5,
	CTRL_TYPE_CTRL_CLASS    = 6,
	CTRL_TYPE_STRING        = 7,
};

//struct used to represent a single control (based on v4l2_queryctrl)
struct control {
	unsigned int		id;
	enum ctrl_type		type;
	char				name[32];	/* Whatever */
	int					minimum;	/* Note signedness */
	int					maximum;
	int					step;
	int					default_value;
	unsigned int		flags;
	unsigned int		reserved[2];
	struct menu 		*menus;//array of 'count_menu'
	int 				count_menu;
};


struct control_list {
	struct control *controls;		//array of 'count' struct control's
	struct control_backend *backend;
	int count;						//how many controls are available
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

#ifdef __linux__
typedef char * pdevice_t;
#endif

#define kLinuxPlatform 		1
#define kWindowsPlatform	2
int getPlatform();

// uniquely identifies a single video device
struct device_id {
	char *name;
	pdevice_t device_handle;
};

struct video_device {
	struct capture_device *capture;
	struct device_info *info;
	struct control_list *control;
	struct tuner_actions *tuner_action;
	struct device_id id;
	int fd;
#define V4L1_INTERFACE				1
#define V4L2_INTERFACE				2
	int interface_type;
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

// returns an array of struct device_id, one for each detected video device
struct device_id ** create_device_id_list();

// frees a device_id list generated with the above method
void release_device_id_list(struct device_id **);

// returns a struct device_id from a device file(linux), or moniker (win)
struct device_id *create_device_id(pdevice_t);

void release_device_id(struct device_id *);


/*
 *
 * VIDEO DEVICE INTERFACE
 *
 */
//Creates a video_device (must call close_device() when done)
// (the device_id will be copied so it can safely be freed when open_device
// returns)
struct video_device *open_device(struct device_id *);
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
	int (*set_cap_param)(struct video_device *, int *, int);

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
	void * (*dequeue_buffer)(struct video_device *, int *);

//enqueue the buffer when done using the frame
	void (*enqueue_buffer)(struct video_device *);


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
int get_control_value(struct video_device *, struct control *, int *);
//returns 0, LIBVIDEO_ERR_WRONG_VERSION, LIBVIDEO_ERR_IOCTL or LIBVIDEO_ERR_STREAMING
//the last argument (int *) will be set to the previous value of this control
int set_control_value(struct video_device *, struct control *,  int *);
void release_control_list(struct video_device *);

/*
 *
 * TUNER INTERFACE
 *
 */
struct tuner_actions *get_tuner_actions(struct video_device *);
void release_tuner_actions(struct video_device *);


#endif
