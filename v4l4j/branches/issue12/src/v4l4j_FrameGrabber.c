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
 * Copy d->capture_len bytes from s to dst, and sets the size of image in
 * dst accordingly
 */
static inline void raw_copy(struct v4l4j_device *d, unsigned char *s,
		unsigned char *dst){
	memcpy(dst, s, d->capture_len);
	d->len = d->capture_len;
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
		} else if(d->output_fmt==OUTPUT_RGB24){
			d->len = get_buffer_length(d);
			//check who does it
			if(d->vdev->capture->is_native==1){
				dprint(LOG_V4L4J, "[V4L4J] Initialising RGB converter\n");
				//we do it
				ret = init_rgb_converter(d);
				if(ret!=0)
					dprint(LOG_V4L4J,
							"[V4L4J] Error initialising the RGB converter\n");
			} else {
				//libvideo convert does it
				dprint(LOG_V4L4J, "[V4L4J] libvideo does it\n");
				d->convert = raw_copy;
			}
		}
	} else {
		dprint(LOG_LIBVIDEO, "[V4L4J] no conversion done by v4l4j - raw copy\n");
		d->len = get_buffer_length(d);
		d->convert = raw_copy;
	}

	return ret;
}

static void release_format_converter(struct v4l4j_device *d){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	if(d->need_conv==1){
		if(d->output_fmt==OUTPUT_JPG)
			destroy_jpeg_compressor(d);
		else if(d->output_fmt == OUTPUT_RGB24)
			if(d->vdev->capture->is_native==1)
				destroy_rgb_converter(d);
	}
}

/*
 * this function checks the output format and returns the capture image format
 * to use, depending on whether format conversion is done by v4l4j or libvideo
 * input is a libvideo palette index, output is enum output_format in common.h
 * the returned value is a libvideo palette index
 */
static int init_capture_format(struct v4l4j_device *d, int output, int input){
	int ret=-1, i;
	int rgb_conv_formats[] = RGB24_CONVERTIBLE_FORMATS;

	dprint(LOG_LIBVIDEO, "[V4L4J] Setting output to %s - input format: %s\n",
						output==OUTPUT_RAW?"RAW":
						output==OUTPUT_JPG?"JPEG":
						output==OUTPUT_RGB24?"RGB24":
						output==OUTPUT_BGR24?"BGR24":
						output==OUTPUT_YUV420?"YUV420":
						output==OUTPUT_YVU420?"YVU420":"UNKNOWN",
						libvideo_palettes[input].name);

	//check if libvideo does the conv
	switch(output){
	case OUTPUT_JPG:
		//for JPEG, v4l4j always does the conv
		dprint(LOG_LIBVIDEO,
				"[V4L4J] JPEG conversion done by v4l4j\n");
		ret=input;
		d->need_conv=1;
		break;

	case OUTPUT_RGB24:
		//check who does it
		for(i=0; i<ARRAY_SIZE(rgb_conv_formats);i++){
			if(rgb_conv_formats[i]==input){
				//we do it
				dprint(LOG_LIBVIDEO,
						"[V4L4J] RGB24 conversion done by v4l4j\n");
				d->need_conv=1;
				return input;
			}
		}
		//libvideo does it
		dprint(LOG_LIBVIDEO,
				"[V4L4J] RGB24 conversion done by libvideo\n");
		ret=RGB24;
		d->need_conv=0;
		break;

	case OUTPUT_RAW:
		ret=input;
		dprint(LOG_LIBVIDEO,
				"[V4L4J] raw format - no conversion\n");
		d->need_conv=0;
		break;

		//for all other output formats,
		//let libvideo handle it
	case OUTPUT_BGR24:
		dprint(LOG_LIBVIDEO,
				"[V4L4J] BGR24 conversion done by libvideo\n");
		ret=BGR24;
		d->need_conv=0;
		break;
	case OUTPUT_YUV420:
		ret=YUV420;
		dprint(LOG_LIBVIDEO,
				"[V4L4J] YUV420 conversion done by libvideo\n");
		d->need_conv=0;
		break;
	case OUTPUT_YVU420:
		ret=YVU420;
		dprint(LOG_LIBVIDEO,
				"[V4L4J] YVU420 conversion done by libvideo\n");
		d->need_conv=0;
		break;
	default:
		info("[V4L4J] Error: unknown output format %d\n", output);
		break;
	}
	return ret;
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
		jint fmt, jint output){

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int i=0;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	struct capture_device *c;
	int fmts;

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
	d->output_fmt=output;
	if((fmts = init_capture_format(d, output, fmt))==-1){
		free_capture_device(d->vdev);
		THROW_EXCEPTION(e, INIT_EXCP, "unknown output format %d\n", output);
		return 0;
	}

	dprint(LOG_LIBVIDEO, "[V4L4J] input format: %s\n",
						libvideo_palettes[fmts].name);

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] calling 'set_cap_param'\n");
	if((i=(*c->actions->set_cap_param)(d->vdev, &fmts, 1))!=0){
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
					libvideo_palettes[fmt].name);
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
 * Get a new frame and return its size
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_fillBuffer(
		JNIEnv *e, jobject t, jlong object, jarray byteArray) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	void *frame;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	unsigned char *array = NULL;
	jboolean isCopy;

	//get frame from libvideo
	if((frame = (*d->vdev->capture->actions->dequeue_buffer)
			(d->vdev, &d->capture_len)) == NULL) {
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
		(*d->vdev->capture->actions->enqueue_buffer)(d->vdev);
		dprint(LOG_V4L4J, "[V4L4J] Error getting the byte array\n");
		THROW_EXCEPTION(e, GENERIC_EXCP,
				"Error getting the byte array");
		return 0;
	}

	// convert frame
	(*d->convert)(d, frame, array);

	// release pointer to java byte array
	(*e)->ReleasePrimitiveArrayCritical(e, byteArray, array, 0);

	// enqueue v4l buffer
	(*d->vdev->capture->actions->enqueue_buffer)(d->vdev);

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
		//(free_capture,free_capture_device2)
		//because its state will be stuck in capture...
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



