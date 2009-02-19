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

#include "libv4l.h"
#include "common.h"
#include "jpeg.h"
#include "debug.h"
#include "libv4l-err.h"

#define INCR_BUF_ID(i, max)		do { (i) = ((i) >= (max)) ? 0 : ((i) + 1); } while(0)

/*
 * Updates the width and height fields in a framegrabber object
 */
static void update_width_height(JNIEnv *e, jobject this, int w, int h){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	jclass this_class;
	jfieldID field;

	//Updates the FrameCapture class width and height fields with the values returned by V4L2
	this_class = (*e)->GetObjectClass(e,this);
	if(this_class==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] error looking up FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up FrameGrabber class");
		return;
	}

	field = (*e)->GetFieldID(e, this_class, "width", "I");
	if(field==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] error looking up width field in FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up width field in FrameGrabber class");
		return;
	}
	(*e)->SetIntField(e, this, field, w);

	field = (*e)->GetFieldID(e, this_class, "height", "I");
	if(field==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] error looking up height field in FrameGrabber class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "error looking up height field in FrameGrabber class");
		return;
	}
	(*e)->SetIntField(e, this, field, h);
}

/*
 * returns an appropriate size for the ByteBuffers holding captured frames
 */
static int get_buffer_length(int width, int height, int driver_size, int palette){
	//shall we trust what the driver says ?
	dprint(LOG_V4L4J, "[V4L4J] Using ByteBuffer of size %d\n", driver_size );
	return driver_size;
}

/*
 * initialise LIBV4L (open, set_cap_param, init_capture)
 * creates the Java ByteBuffers
 * creates the V4L2Controls
 * initialise the JPEG compressor
 */
JNIEXPORT jobjectArray JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_doInit(JNIEnv *e, jobject t, jlong object, jint w, jint h, jint ch, jint std, jint n, jint q, jint fmt){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int i=0, buf_len;
	jobject element;
	jobjectArray arr;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	struct capture_device *c;
	int jpeg_fmts[NB_JPEG_SUPPORTED_FORMATS] = JPEG_SUPPORTED_FORMATS;
	int *fmts, nb_fmts;

	/*
	 * i n i t _ c a p t u r e _ d e v i c e ( )
	 */
	c = init_capture_device(d->vdev, w,h,ch,std,n);

	if(c==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] init_capture_device failed\n");
		THROW_EXCEPTION(e, INIT_EXCP, "Error initialising device '%s'."\
							" Make sure it is a valid V4L device file and"\
							" check the file permissions.", d->vdev->file);
		return 0;
	}


	/*
	 * s e t _ c a p _ p a r a m
	 */
	if(fmt==-1){
		dprint(LOG_LIBV4L, "[LIBV4L] Calling 'set_cap_param(JPEG - dev: %s)'\n",d->vdev->file);
		fmts = jpeg_fmts;
		nb_fmts = NB_JPEG_SUPPORTED_FORMATS;
	} else {
		dprint(LOG_LIBV4L, "[LIBV4L] Calling 'set_cap_param(RAW - dev: %s)'\n",d->vdev->file);
		fmts = &fmt;
		nb_fmts = 1;
	}
	if((i=(*c->actions->set_cap_param)(d->vdev, fmts, nb_fmts))!=0){
		dprint(LOG_V4L4J, "[V4L4J] set_cap_param failed\n");
		free_capture_device(d->vdev);
		if(i==LIBV4L_ERR_DIMENSIONS)
			THROW_EXCEPTION(e, DIM_EXCP, "The requested dimensions (%dx%d) are not supported", c->width, c->height);
		else if(i==LIBV4L_ERR_CHANNEL_SETUP)
			THROW_EXCEPTION(e, CHANNEL_EXCP, "The requested channel (%d) is invalid", c->channel);
		else if(i==LIBV4L_ERR_FORMAT)
			THROW_EXCEPTION(e, FORMAT_EXCP, "No supported image formats were found");
		else if(i==LIBV4L_ERR_STD)
			THROW_EXCEPTION(e, STD_EXCP, "The requested standard (%d) is invalid", c->std);
		else
			THROW_EXCEPTION(e, GENERIC_EXCP, "Error applying capture parameters (error=%d)",i);

		return 0;
	}


	/*
	 * i n i t _ c a p t u r e ( )
	 */
	dprint(LOG_LIBV4L, "[LIBV4L] Calling 'init_capture(dev: %s)'\n",d->vdev->file);
	if((i=(*c->actions->init_capture)(d->vdev))<0){
		dprint(LOG_V4L4J, "[V4L4J] init_capture failed\n");
		free_capture_device(d->vdev);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error initialising capture (error=%d)",i);
		return 0;
	}

	//update width and height in FrameGrabber class
	update_width_height(e, t, w, h);

	//The length of the buffers which will hold the last captured frame
	buf_len = get_buffer_length(c->width, c->height, c->imagesize, c->palette);

	//Create the ByteBuffer array
	dprint(LOG_V4L4J, "[V4L4J] Creating the ByteBuffer array[%d]\n",c->mmap->buffer_nr);
	arr = (*e)->NewObjectArray(e, c->mmap->buffer_nr, (*e)->FindClass(e, BYTEBUFER_CLASS), NULL);
	if(arr==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] error creating byte buffer array\n");
		free_capture_device(d->vdev);
		THROW_EXCEPTION(e, JNI_EXCP, "error creating byte buffer array");
		return 0;
	}
	XMALLOC(d->bufs, unsigned char **, c->mmap->buffer_nr * sizeof(void *));

	for(i=0; i<c->mmap->buffer_nr;i++) {
		//for each v4l2 buffers created, we create a corresponding java Bytebuffer
		dprint(LOG_V4L4J, "[V4L4J] Creating ByteBuffer %d - length: %d\n", i, buf_len);
		XMALLOC(d->bufs[i], unsigned char *, (size_t) buf_len);
		element = (*e)->NewDirectByteBuffer(e, d->bufs[i], (jlong) buf_len);
		if(element==NULL) {
			dprint(LOG_V4L4J, "[V4L4J] error creating byte buffer\n");
			free_capture_device(d->vdev);
			THROW_EXCEPTION(e, JNI_EXCP, "error creating byte buffer");
			return 0;
		}
		(*e)->SetObjectArrayElement(e, arr, i, element);
	}

	//setup jpeg param
	if(init_jpeg_compressor(d, q)!=0) {
		dprint(LOG_V4L4J, "[V4L4J] Error initialising the JPEG compressor\n");
		(*c->actions->free_capture)(d->vdev);
		free_capture_device(d->vdev);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error initialising the JPEG compressor");
		return 0;
	}
	d->buf_id = -1;
	return arr;
}

/*
 * tell LIBV4L to start the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_start(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	if((*d->vdev->capture->actions->start_capture)(d->vdev)<0){
		dprint(LOG_V4L4J, "[V4L4J] start_capture failed\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error starting the capture");
	}
}

/*
 * tell the JPEG compressor the new compression factor
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_setQuality(JNIEnv *e, jobject t, jlong object, jint q){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	if(d->jpeg_quality==-1)
		return;
	dprint(LOG_V4L4J, "[V4L4J] Setting JPEG quality to %d\n",q);
	d->jpeg_quality = q;
}


/*
 * get a new JPEG-compressed frame from the device
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getBuffer(JNIEnv *e, jobject t, jlong object) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int i;
	void *frame;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	//get frame from v4l2
	if((frame = (*d->vdev->capture->actions->dequeue_buffer)(d->vdev, &d->capture_len)) != NULL) {
		i = d->buf_id = (d->buf_id == (d->vdev->capture->mmap->buffer_nr-1)) ? 0 : d->buf_id+1;
		dprint(LOG_V4L4J, "[V4L4J] got frame in buffer %d\n", i);
		(*d->j->jpeg_encode)(d, frame, d->bufs[i]);
		(*d->vdev->capture->actions->enqueue_buffer)(d->vdev);
		return i;
	}
	dprint(LOG_V4L4J, "[V4L4J] Error dequeuing buffer for capture\n");
	THROW_EXCEPTION(e, GENERIC_EXCP, "Error dequeuing buffer for capture");
	return -1;
}

/*
 * return the length of the last captured frame
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getBufferLength(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	dprint(LOG_V4L4J, "[V4L4J] Getting last frame length: %d\n", d->len);
	return d->len;
}

/*
 * tell LIBV4L to stop the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_stop(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	dprint(LOG_LIBV4L, "[LIBV4L] Calling stop_capture(dev: %s)\n", d->vdev->file);
	if((*d->vdev->capture->actions->stop_capture)(d->vdev)<0) {
		dprint(LOG_V4L4J, "Error stopping capture\n");
		//not sure whether we should throw an exception here...
		//if we do, FrameGrabber wont let us call delete (free_capture,free_capture_device2)
		//because its state will be stuck in capture...
		//THROW_EXCEPTION(e, GENERIC_EXCP,"Cant stop capture");
	}
}

/*
 * free JPEG compressor
 * free LIBV4L (free_capture, free_capture_device)
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_doRelease(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int i;

	destroy_jpeg_compressor(d);

	dprint(LOG_V4L4J, "[V4L4J] Freeing %d ByteBuffers areas and array\n",d->vdev->capture->mmap->buffer_nr);
	for(i=0; i<d->vdev->capture->mmap->buffer_nr;i++)
		XFREE(d->bufs[i]);
	XFREE(d->bufs);

	(*d->vdev->capture->actions->free_capture)(d->vdev);

	free_capture_device(d->vdev);
}



