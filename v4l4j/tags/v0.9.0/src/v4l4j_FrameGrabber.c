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

#include <jni.h>
#include <stdio.h>
#include <jpeglib.h>
#include <stdint.h>

#include "common.h"
#include "debug.h"
#include "libvideo.h"
#include "libvideo-err.h"
#include "jpeg.h"
#include "libvideo-palettes.h"
#include "rgb.h"

// static variables
static jfieldID		last_captured_frame_sequence_fID = NULL;
static jfieldID		last_captured_frame_time_usec_fID = NULL;
static jfieldID		last_captured_frame_buffer_index_fID = NULL;


/*
 * Updates the width, height, standard & format fields in a framegrabber object
 */
static void update_width_height(JNIEnv *e, jobject this, struct v4l4j_device *d){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	jclass this_class;
	jfieldID field;
	int i;

	//Updates the FrameGrabber class width, height & format fields with the
	//values returned by V4L2
	this_class = (*e)->GetObjectClass(e,this);
	if(this_class==NULL) {
		info("[V4L4J] error looking up FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up FrameGrabber class");
		return;
	}

	//width
	field = (*e)->GetFieldID(e, this_class, "width", "I");
	if(field==NULL) {
		info("[V4L4J] error looking up width field in FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up width field in "
				"FrameGrabber class");
		return;
	}
	(*e)->SetIntField(e, this, field, d->vdev->capture->width);

	//height
	field = (*e)->GetFieldID(e, this_class, "height", "I");
	if(field==NULL) {
		info("[V4L4J] error looking up height field in FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up height field in "
				"FrameGrabber class");
		return;
	}
	(*e)->SetIntField(e, this, field, d->vdev->capture->height);

	//standard
	field = (*e)->GetFieldID(e, this_class, "standard", "I");
	if(field==NULL) {
		info("[V4L4J] error looking up height standard in FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up standard field in "
				"FrameGrabber class");
		return;
	}
	(*e)->SetIntField(e, this, field, d->vdev->capture->std);


	//format
	if(d->output_fmt!=OUTPUT_RAW){
		field = (*e)->GetFieldID(e, this_class, "format",
				"I");
		if(field==NULL) {
			info("[V4L4J] error looking up format field in FrameGrabber "
					"class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "error looking up format field in "
					"FrameGrabber class");
			return;
		}

		if(d->vdev->capture->is_native==1)
			i = d->vdev->capture->palette;
		else
			i = d->vdev->capture->convert->src_palette;

		dprint(LOG_V4L4J, "[V4L4J] Setting format field to '%s' image format\n",
			libvideo_palettes[i].name);
		(*e)->SetIntField(e, this, field, i);
	}
}

static int get_buffer_length(struct v4l4j_device *d){

	if(d->output_fmt==OUTPUT_RAW || d->output_fmt==OUTPUT_JPG) {
		//shall we trust what the driver says ?
		dprint(LOG_V4L4J, "[V4L4J] OUTPUT: RAW / JPEG - Using byte array of "
				"size %d\n", d->vdev->capture->imagesize );
		return d->vdev->capture->imagesize;
	} else if(d->output_fmt==OUTPUT_RGB24) {
		//RGB24 means w * h * 3
		dprint(LOG_V4L4J, "[V4L4J] OUTPUT: RGB24 - Using byte array of "
				"size %d\n",
				d->vdev->capture->width * d->vdev->capture->height * 3);

		return d->vdev->capture->width * d->vdev->capture->height * 3;
	} else if(d->output_fmt==OUTPUT_BGR24) {
		//BGR24 means w * h * 3
		dprint(LOG_V4L4J, "[V4L4J] OUTPUT: BGR24 - Using byte array of "
				"size %d\n",
				d->vdev->capture->width * d->vdev->capture->height * 3);

		return d->vdev->capture->width * d->vdev->capture->height * 3;
	} else if(d->output_fmt==OUTPUT_YUV420) {
		//YUV420 means w * h * 3/2
		dprint(LOG_V4L4J, "[V4L4J] OUTPUT: YUV420 - Using byte array of "
				"size %d\n",
				d->vdev->capture->width * d->vdev->capture->height * 3/2);

		return d->vdev->capture->width * d->vdev->capture->height * 3/2;
	} else if(d->output_fmt==OUTPUT_YVU420) {
		//YVU420 means w * h * 3/2
		dprint(LOG_V4L4J, "[V4L4J] OUTPUT: YVU420 - Using byte array of "
				"size %d\n",
				d->vdev->capture->width * d->vdev->capture->height * 3/2);

		return d->vdev->capture->width * d->vdev->capture->height * 3/2;
	} else {
		dprint(LOG_V4L4J,"[V4L4J] Unknown output format...");
	}
	return 0;
}

/*
 * Call init routines of RGB, JPEG or raw depending on requested
 * output image format
 */
static int init_format_converter(struct v4l4j_device *d){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int ret = 0;

	if(d->need_conv==1){
		if(d->output_fmt==OUTPUT_JPG){
			dprint(LOG_V4L4J, "[V4L4J] Initialising JPEG converter\n");
			ret = init_jpeg_compressor(d, 80);
			if(ret!=0)
				dprint(LOG_V4L4J, "[V4L4J] Error initialising JPEG converter\n");
		}

		if (d->vdev->capture->is_native==0){
			dprint(LOG_V4L4J, "[V4L4J] Setting up double conversion\n");
			XMALLOC(d->double_conversion_buffer, unsigned char *, d->vdev->capture->imagesize);
		}
	} else {
		dprint(LOG_LIBVIDEO, "[V4L4J] no conversion done by v4l4j - raw copy\n");
	}

	return ret;
}

static void release_format_converter(struct v4l4j_device *d){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	if(d->need_conv==1){
		if(d->output_fmt==OUTPUT_JPG)
			destroy_jpeg_compressor(d);

		if (d->vdev->capture->is_native==0)
			XFREE(d->double_conversion_buffer);
	}
}

/*
 * this function checks the output format and returns the capture image format
 * to use, depending on whether format conversion is done by v4l4j or libvideo
 * input is a libvideo palette index, output is enum output_format in common.h
 * the returned value is a libvideo palette index
 */
static int init_capture_format(struct v4l4j_device *d, int fg_out_fmt, int* src_fmt, int* dest_fmt){
	dprint(LOG_LIBVIDEO, "[V4L4J] Setting output to %s - input format: %s\n",
			fg_out_fmt==OUTPUT_RAW?"RAW":
			fg_out_fmt==OUTPUT_JPG?"JPEG":
			fg_out_fmt==OUTPUT_RGB24?"RGB24":
			fg_out_fmt==OUTPUT_BGR24?"BGR24":
			fg_out_fmt==OUTPUT_YUV420?"YUV420":
			fg_out_fmt==OUTPUT_YVU420?"YVU420":"UNKNOWN",
			libvideo_palettes[*src_fmt].name);

	//check if libvideo does the conv
	switch(fg_out_fmt){
	case OUTPUT_JPG:
		//for JPEG, v4l4j always does the conv
		dprint(LOG_LIBVIDEO,
				"[V4L4J] JPEG conversion done by v4l4j\n");
		*dest_fmt = *src_fmt;
		*src_fmt = -1;
		d->need_conv=1;
		return 0;

	case OUTPUT_RGB24:
		*dest_fmt = RGB24;
		// leave native capture format in src_fmt
		dprint(LOG_LIBVIDEO,
				"[V4L4J] RGB24 conversion done by libvideo\n");
		d->need_conv=0;
		return 0;

	case OUTPUT_RAW:
		*dest_fmt = *src_fmt;
		*src_fmt = -1;
		dprint(LOG_LIBVIDEO,
				"[V4L4J] raw format - no conversion\n");
		d->need_conv=0;
		return 0;

	case OUTPUT_BGR24:
		*dest_fmt = BGR24;
		// leave native capture format in src_fmt
		dprint(LOG_LIBVIDEO,
				"[V4L4J] BGR24 conversion done by libvideo\n");
		d->need_conv=0;
		return 0;
	case OUTPUT_YUV420:
		*dest_fmt = YUV420;
		// leave native capture format in src_fmt
		dprint(LOG_LIBVIDEO,
				"[V4L4J] YUV420 conversion done by libvideo\n");
		d->need_conv=0;
		return 0;
	case OUTPUT_YVU420:
		*dest_fmt = YVU420;
		// leave native capture format in src_fmts
		dprint(LOG_LIBVIDEO,
				"[V4L4J] YVU420 conversion done by libvideo\n");
		d->need_conv=0;
		return 0;
	default:
		info("[V4L4J] Error: unknown output format %d\n", fg_out_fmt);
		return -1;
	}
}


/*
 * Gets the fieldIDs for members that have to be updated every time a frame is captured
 * return 0 if an exception is thrown, 1 otherwise
 */
static int get_lastFrame_field_ids(JNIEnv *e, jobject this, struct v4l4j_device *d){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	jclass this_class;

	//gets the fields of members updated every frame captured.
	this_class = (*e)->GetObjectClass(e,this);
	if(this_class==NULL) {
		info("[V4L4J] error looking up FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up FrameGrabber class");
		return 0;
	}

	//last_captured_frame_sequence_fID
	last_captured_frame_sequence_fID =
			(*e)->GetFieldID(e, this_class, "lastCapturedFrameSequence", "J");
	if(last_captured_frame_sequence_fID==NULL) {
		info("[V4L4J] error looking up last_captured_frame_sequence_fID "
				" in FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up "
				" last_captured_frame_sequence_fID field in FrameGrabber class");
		return 0;
	}

	// last_captured_frame_time_usec_fID
	last_captured_frame_time_usec_fID =
			(*e)->GetFieldID(e, this_class, "lastCapturedFrameTimeuSec", "J");
	if(last_captured_frame_time_usec_fID==NULL) {
		info("[V4L4J] error looking up lastCapturedFrameTimeuSec field in "
				"FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up lastCapturedFrameTimeuSec"
				" field in FrameGrabber class");
		return 0;
	}

	// last_captured_frame_buffer_index_fID
	last_captured_frame_buffer_index_fID =
			(*e)->GetFieldID(e, this_class, "lastCapturedFrameBufferIndex", "I");
	if(last_captured_frame_buffer_index_fID==NULL) {
		info("[V4L4J] error looking up lastCapturedFrameBufferIndex field in "
				"FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up lastCapturedFrameBufferIndex"
				" field in FrameGrabber class");
		return 0;
	}
	return 1;
}


/*
 * initialise LIBVIDEO (open, set_cap_param, init_capture)
 * creates the Java ByteBuffers
 * creates the V4L2Controls
 * initialise the JPEG compressor
 * fmt is the input format
 * output is the output format (enum output_format in common.h)
 * return the number of mmap''ed buffers
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doInit(
		JNIEnv *e, jobject t, jlong object, jint w, jint h, jint ch, jint std,
		jint in_fmt, jint fg_out_fmt){

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int i=0;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	struct capture_device *c;
	int src_fmt = in_fmt, dest_fmt;

	// Get the field IDs if we dont have them already. If error getting them, return.
	if ((! last_captured_frame_sequence_fID || ! last_captured_frame_time_usec_fID)
			&& ! get_lastFrame_field_ids(e, t, d))
			return 0;

	/*
	 * i n i t _ c a p t u r e _ d e v i c e ( )
	 */
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling init_capture_device()\n");
	c = init_capture_device(d->vdev, w,h,ch,std, 4); // ask for 4 buffers by default

	if(c==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] init_capture_device failed\n");
		THROW_EXCEPTION(e, INIT_EXCP, "Error initialising device '%s'."
							" Make sure it is a valid V4L device file and"
							" check the file permissions.", d->vdev->file);
		return 0;
	}


	/*
	 * s e t _ c a p _ p a r a m
	 */
	d->output_fmt=fg_out_fmt;
	if(init_capture_format(d, fg_out_fmt, &src_fmt, &dest_fmt)==-1){
		free_capture_device(d->vdev);
		THROW_EXCEPTION(e, INIT_EXCP, "unknown output format %d\n", fg_out_fmt);
		return 0;
	}

	dprint(LOG_LIBVIDEO, "[V4L4J] src format: %s\n",
						(src_fmt != -1) ? libvideo_palettes[src_fmt].name : "'chosen by libvideo'");

	dprint(LOG_LIBVIDEO, "[V4L4J] dest format: %s\n",
							libvideo_palettes[dest_fmt].name);

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] calling 'set_cap_param'\n");
	if((i=(*c->actions->set_cap_param)(d->vdev, src_fmt, dest_fmt))!=0){
		dprint(LOG_V4L4J, "[V4L4J] set_cap_param failed\n");
		free_capture_device(d->vdev);
		if(i==LIBVIDEO_ERR_DIMENSIONS)
			THROW_EXCEPTION(e, DIM_EXCP,
					"The requested dimensions (%dx%d) are not supported",
					c->width, c->height);
		else if(i==LIBVIDEO_ERR_CHAN_SETUP)
			THROW_EXCEPTION(e, CHANNEL_EXCP,
					"The requested channel (%d) is invalid", c->channel);
		else if(i==LIBVIDEO_ERR_FORMAT)
			THROW_EXCEPTION(e, FORMAT_EXCP,
					"Image format %s not supported",
					libvideo_palettes[in_fmt].name);
		else if(i==LIBVIDEO_ERR_STD)
			THROW_EXCEPTION(e, STD_EXCP,
					"The requested standard (%d) is invalid", c->std);
		else
			THROW_EXCEPTION(e, GENERIC_EXCP,
					"Error applying capture parameters (error=%d)",i);

		return 0;
	}


	/*
	 * i n i t _ c a p t u r e ( )
	 */
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling 'init_capture(dev: %s)'\n",
			d->vdev->file);
	if((i=(*c->actions->init_capture)(d->vdev))<0){
		dprint(LOG_V4L4J, "[V4L4J] init_capture failed\n");
		free_capture_device(d->vdev);
		THROW_EXCEPTION(e, GENERIC_EXCP,
				"Error initialising capture (error=%d)",i);
		return 0;
	}

	//setup format converter
	if(init_format_converter(d)!=0){
		dprint(LOG_V4L4J, "[V4L4J] Error initialising the format converter\n");
		(*c->actions->free_capture)(d->vdev);
		free_capture_device(d->vdev);
		THROW_EXCEPTION(e, GENERIC_EXCP,
				"Error initialising the format converter");
		return 0;
	}


	//update width, height, standard & image format in FrameGrabber class
	update_width_height(e, t, d);

	return c->mmap->buffer_nr;
}

/*
 * returns an appropriate size for a byte array holding converted frames
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_getBufferSize(
		JNIEnv *e, jobject t, jlong object) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	return get_buffer_length(d);
}

/*
 * tell LIBVIDEO to start the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_start(
		JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling 'start_capture(dev: %s)'\n",
			d->vdev->file);
	if((*d->vdev->capture->actions->start_capture)(d->vdev)<0){
		dprint(LOG_V4L4J, "[V4L4J] start_capture failed\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error starting the capture");
	}
}

/*
 * tell the JPEG compressor the new compression factor
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_setQuality(
		JNIEnv *e, jobject t, jlong object, jint q){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	if(d->output_fmt!=OUTPUT_JPG)
		return;
	dprint(LOG_V4L4J, "[V4L4J] Setting JPEG quality to %d\n",q);
	d->j->jpeg_quality = q;
}

/*
 * sets the frame interval
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doSetFrameIntv(
		JNIEnv *e, jobject t, jlong object, jint num, jint denom){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int ret = 0;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_V4L4J, "[LIBVIDEO] Setting frame interval to %d/%d\n",num, denom);
	ret = d->vdev->capture->actions->set_frame_interval(d->vdev, num, denom);

	switch(ret){
	case LIBVIDEO_ERR_FORMAT:
		dprint(LOG_V4L4J, "[V4L4J] Invalid frame interval\n");
		THROW_EXCEPTION(e, INVALID_VAL_EXCP, "Error setting frame interval: "
				"invalid values %d/%d", num, denom);
		return;
	case LIBVIDEO_ERR_IOCTL:
		dprint(LOG_V4L4J, "[V4L4J] Setting frame interval not supported\n");
		THROW_EXCEPTION(e, UNSUPPORTED_METH_EXCP,
				"Setting frame interval not supported");
		return;
	}

}

/*
 * get the frame interval numerator (what=0) or denominator(what!=0)
 * (expects some lock to be obtained so calling this method to obtain the other
 * frac part of the frame intv does NOT interleave with doSetFrameIntv())
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doGetFrameIntv(
		JNIEnv *e, jobject t, jlong object, jint what){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	int num, denom;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	if(d->vdev->capture->actions->get_frame_interval(d->vdev, &num, &denom)!=0){
		dprint(LOG_V4L4J, "[V4L4J] Getting frame interval not supported\n");
		THROW_EXCEPTION(e, UNSUPPORTED_METH_EXCP,
				"Getting frame interval not supported");
		return 0;
	}

	if(what==0)
		return num;
	else
		return denom;
}

/*
 * sets the video input and standard
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doSetVideoInputNStandard(
		JNIEnv *e, jobject t, jlong object, jint input_num, jint standard){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int ret = 0;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_V4L4J, "[LIBVIDEO] Setting input to %d and standard to %d\n", input_num, standard);
	ret = d->vdev->capture->actions->set_video_input_std(d->vdev, input_num, standard);

	switch(ret){
	case LIBVIDEO_ERR_CHANNEL:
		dprint(LOG_V4L4J, "[V4L4J] Invalid input\n");
		THROW_EXCEPTION(e, CHANNEL_EXCP, "Error setting new input %d", input_num);
		break;
	case LIBVIDEO_ERR_STD:
		dprint(LOG_V4L4J, "[V4L4J] Error setting standard to %d\n", standard);
		THROW_EXCEPTION(e, STD_EXCP, "The requested standard (%d) is invalid", standard);
		break;
	}


}

/*
 * gets the video input
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doGetVideoInput(
		JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int input_num, standard;

	d->vdev->capture->actions->get_video_input_std(d->vdev, &input_num, &standard);

	return (jint)input_num;
}

/*
 * gets the video standard
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doGetVideoStandard(
		JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int input_num, standard;

	d->vdev->capture->actions->get_video_input_std(d->vdev, &input_num, &standard);

	return (jint)standard;
}


/*
 * enqueue a buffer
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_enqueueBuffer(
		JNIEnv *e, jobject t, jlong object, jint buffer_index){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	(*d->vdev->capture->actions->enqueue_buffer)(d->vdev, buffer_index);
}

/*
 * dequeue a buffer, perform conversion if required and return frame
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_fillBuffer(
		JNIEnv *e, jobject this, jlong object, jarray byteArray) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	void *frame;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	unsigned char *array = NULL;
	unsigned long long captureTime, sequence;
	unsigned int buffer_index;
	jboolean isCopy;

	//get frame from libvideo
	if((frame = (*d->vdev->capture->actions->dequeue_buffer)
			(d->vdev, &d->capture_len, &buffer_index, &captureTime, &sequence)) == NULL) {
		dprint(LOG_V4L4J, "[V4L4J] Error dequeuing buffer for capture\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error dequeuing buffer for capture");
		return 0;
	}

	// get a pointer to the java array
	array = (*e)->GetPrimitiveArrayCritical(e, byteArray, &isCopy);
	if (isCopy == JNI_TRUE)
		dprintf(LOG_V4L4J, "[V4L4J] Slow path: cant get direct pointer to byte array\n");

	// check we have a valid pointer
	if (! array)
	{
		(*d->vdev->capture->actions->enqueue_buffer)(d->vdev, buffer_index);
		dprint(LOG_V4L4J, "[V4L4J] Error getting the byte array\n");
		THROW_EXCEPTION(e, GENERIC_EXCP,
				"Error getting the byte array");
		return 0;
	}

	START_TIMING;
	// Perform required conversion
	if(d->vdev->capture->is_native!=1) {
		// Check whether we can convert directly to the byte[] memory
		if(d->need_conv!=1) {
			// Only libv4l conversion is required
			(*d->vdev->capture->actions->convert_buffer)(d->vdev, buffer_index, d->capture_len, array);
		} else {
			// both libv4l and v4l4j conversions required
			(*d->vdev->capture->actions->convert_buffer)(d->vdev, buffer_index, d->capture_len, d->double_conversion_buffer);
			(*d->convert)(d, d->double_conversion_buffer, array);
		}
	} else {
		// No libv4l conversion required. Check if v4l4j conversion is required
		if (d->need_conv!=1) {
			// No v4l4j conversion required. So copy the frame to byte[] memory. This
			// is definitely NOT an optimal solution, but I cant see any other way to do it:
			// We could mmap the byte[] memory and used it as the buffer, but the JVM specs
			// clearly says the memory can go at anytime, or be moved to somewhere else.
			// And we can only hold on to it (between GetPrimitiveArrayCritical() and
			// ReleasePrimitiveArrayCritical() ) for a short amount of time. If you
			// find yourself reading this comment and you have a better idea, let me know.
			memcpy(array, frame, d->capture_len);
			d->len = d->capture_len;
		} else {
			(*d->convert)(d, frame, array);
		}
	}
	END_TIMING("JNI Conversion took ");

	// release pointer to java byte array
	(*e)->ReleasePrimitiveArrayCritical(e, byteArray, array, 0);

	// update class members
	(*e)->SetLongField(e, this, last_captured_frame_sequence_fID,
			sequence);
	(*e)->SetLongField(e, this, last_captured_frame_time_usec_fID,
			captureTime);
	(*e)->SetIntField(e, this, last_captured_frame_buffer_index_fID,
			buffer_index);

	return d->len;
}

/*
 * tell LIBVIDEO to stop the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_stop(
		JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling stop_capture(dev: %s)\n",
			d->vdev->file);
	if((*d->vdev->capture->actions->stop_capture)(d->vdev)<0) {
		dprint(LOG_V4L4J, "Error stopping capture\n");
		//dont throw an exception here...
		//if we do, FrameGrabber wont let us call delete
		//free_capture because its state will be stuck in capture...
	}
}

/*
 * free JPEG compressor
 * free LIBVIDEO (free_capture, free_capture_device)
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doRelease(
		JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	release_format_converter(d);

	(*d->vdev->capture->actions->free_capture)(d->vdev);

	free_capture_device(d->vdev);
}



