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
#include "v4l-control.h"
#include "common.h"
#include "jpeg.h"
#include "debug.h"
#include "libv4l-err.h"

#define SUPPORTED_FORMATS		{JPEG, MJPEG, YUV420, YUYV, RGB24}
#define NB_SUPPORTED_FORMATS	5
#define INCR_BUF_ID(i, max)		do { (i) = ((i) >= (max)) ? 0 : ((i) + 1); } while(0)

/*
 * Allocate a new v4l4j device
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_allocateObject(JNIEnv *e, jobject t){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d;
	XMALLOC(d, struct v4l4j_device *, sizeof(struct v4l4j_device));
	if(d==NULL) THROW_EXCEPTION(e, INIT_EXCP, "Error creating new v4l4j object - out of memory");
	return (uintptr_t) d;
}

/*
 * initialise LIBV4L (open, set_cap_param, init_capture)
 * creates the Java ByteBuffers
 * creates the V4L2Controls
 * initialise the JPEG compressor
 */
JNIEXPORT jobjectArray JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_init_1v4l(JNIEnv *e, jobject t, jlong object, jstring f, jint w, jint h, jint ch, jint std, jint n, jint q){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int i=0, jpeg_buf_len;
	jclass this_class, v4l2ControlClass;
	jmethodID ctor;
	jfieldID field;
	jobject element;
	jobjectArray arr, ctrls;
	const char * device_file = (*e)->GetStringUTFChars(e, f, 0);
	struct control_list *l;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int fmts[NB_SUPPORTED_FORMATS] = SUPPORTED_FORMATS;

	dprint(LOG_LIBV4L, "[LIBV4L] Calling 'init_capture_device(dev:%s, w:%d,h:%d,"
			"ch:%d, std:%d, nb_buf:%d, dev:%p)'\n",device_file,w,h,ch,std,n, (void *) d);
	d->c = init_capture_device(device_file, w,h,ch,std,n);

	if(d->c==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] init_capture_device failed\n");
		THROW_EXCEPTION(e, INIT_EXCP, "Error initialising device '%s'."\
							" Make sure it is a valid V4L device file and"\
							" check the file permissions.", device_file);
		return 0;
	}
	(*e)->ReleaseStringUTFChars(e, f,device_file);

	dprint(LOG_LIBV4L, "[LIBV4L] Calling 'set_cap_param(dev: %s)'\n",d->c->file);
	if((i=(*d->c->capture->set_cap_param)(d->c, fmts, NB_SUPPORTED_FORMATS))!=0){
		dprint(LOG_V4L4J, "[V4L4J] set_cap_param failed\n");
		free_capture_device(d->c);
		if(i==LIBV4L_ERR_DIMENSIONS)
			THROW_EXCEPTION(e, DIM_EXCP, "The requested dimensions (%dx%d) are not supported", d->c->width, d->c->height);
		else if(i==LIBV4L_ERR_CHANNEL_SETUP)
			THROW_EXCEPTION(e, CHANNEL_EXCP, "The requested channel (%d) is invalid", d->c->channel);
		else if(i==LIBV4L_ERR_FORMAT)
			THROW_EXCEPTION(e, FORMAT_EXCP, "No supported image formats were found");
		else if(i==LIBV4L_ERR_STD)
			THROW_EXCEPTION(e, STD_EXCP, "The requested standard (%d) is invalid", d->c->std);
		else
			THROW_EXCEPTION(e, GENERIC_EXCP, "Error applying capture parameters (error=%d)",i);

		return 0;
	}


	dprint(LOG_LIBV4L, "[LIBV4L] Calling 'init_capture(dev: %s)'\n",d->c->file);
	if((i=(*d->c->capture->init_capture)(d->c))<0){
		dprint(LOG_V4L4J, "[V4L4J] init_capture failed\n");
		free_capture_device(d->c);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error initialising capture (error=%d)",i);
		return 0;
	}

	//Updates the FrameCapture class width and height fields with the values returned by V4L2
	dprint(LOG_V4L4J, "[V4L4J] Updating width(%d) in the FrameGrabber class\n", d->c->width);
	this_class = (*e)->GetObjectClass(e,t);
	field = (*e)->GetFieldID(e, this_class, "width", "I");
	(*e)->SetIntField(e, t, field, d->c->width);

	dprint(LOG_V4L4J, "[V4L4J] Updating height(%d) in the FrameGrabber class\n", d->c->height);
	field = (*e)->GetFieldID(e, this_class, "height", "I");
	(*e)->SetIntField(e, t, field, d->c->height);

	//TODO fix the following, buffer way too large, but what would be a correct value ?
	//The buffers which will hold the JPEG compressed frame
	jpeg_buf_len = d->c->width * d->c->height*3;

	//Create the ByteBuffer array
	dprint(LOG_V4L4J, "[V4L4J] Creating the ByteBuffer array[%d]\n",d->c->mmap->buffer_nr);
	arr = (*e)->NewObjectArray(e, d->c->mmap->buffer_nr, (*e)->FindClass(e, BYTEBUFER_CLASS), NULL);
	XMALLOC(d->bufs, unsigned char **, d->c->mmap->buffer_nr * sizeof(void *));

	for(i=0; i<d->c->mmap->buffer_nr;i++) {
		//for each v4l2 buffers created, we create a corresponding java Bytebuffer
		dprint(LOG_V4L4J, "[V4L4J] Creating ByteBuffer %d - length: %d\n", i, jpeg_buf_len);
		XMALLOC(d->bufs[i], unsigned char *, (size_t) jpeg_buf_len);
		element = (*e)->NewDirectByteBuffer(e, d->bufs[i], (jlong) jpeg_buf_len);
		(*e)->SetObjectArrayElement(e, arr, i, element);
	}

	//Creates the java objects matching v4l2 controls
	l = d->c->ctrls;
	dprint(LOG_V4L4J, "[V4L4J] Creating the Control array[%d]\n", l->count);
	v4l2ControlClass = (*e)->FindClass(e,CONTROL_CLASS);
	ctor = (*e)->GetMethodID(e, v4l2ControlClass, "<init>", "(ILjava/lang/String;IIIL" FRAMEGRABBER_CLASS ";)V");
	if(ctor == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the Control class\n");
		(*d->c->capture->free_capture)(d->c);
		free_capture_device(d->c);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up Control java objects");
		return 0;
	}

	//initialise the ctrls field of FrameGrabber object (this)
	ctrls = (*e)->NewObjectArray(e, l->count, v4l2ControlClass, NULL);

	//Construct a V4L2Control Java object for each V4L2control in l
	for(i = 0; i< l->count; i++) {
		dprint(LOG_V4L4J, "[V4L4J] Creating Control %d - name: %s\n", i, l->ctrl[i].name);
		element = (*e)->NewObject(e, v4l2ControlClass, ctor, i, (*e)->NewStringUTF(e, (const char *)l->ctrl[i].name), l->ctrl[i].minimum, l->ctrl[i].maximum, l->ctrl[i].step, t);
		(*e)->SetObjectArrayElement(e, ctrls, i, element);
	}
	field = (*e)->GetFieldID(e, this_class, "ctrls", "[L" CONTROL_CLASS ";");
	(*e)->SetObjectField(e, t, field, ctrls);

	//setup jpeg param
	if(init_jpeg_compressor(d, q)!=0) {
		dprint(LOG_V4L4J, "[V4L4J] Error initialising the JPEG compressor\n");
		(*d->c->capture->free_capture)(d->c);
		free_capture_device(d->c);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up Control java objects");
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
	dprint(LOG_LIBV4L, "[LIBV4L] Calling start_capture(dev: %s)\n", d->c->file);
	if((*d->c->capture->start_capture)(d->c)<0){
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
	dprint(LOG_V4L4J, "[V4L4J] Setting JPEG quality to %d\n",q);
	d->jpeg_quality = q;
}

/*
 * Set a new value on a v4l2 control
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_setCtrlValue(JNIEnv *e, jobject t, jlong object, jint id, jint value){
	int ret;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	dprint(LOG_LIBV4L, "[LIBV4L] Calling set_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->c->file,d->c->ctrls->ctrl[id].name,value);
	ret = set_control_value(d->c, &(d->c->ctrls->ctrl[id]), value);
	if(ret != 0)
		THROW_EXCEPTION(e, CTRL_EXCP, "Error setting current value for control '%s'", d->c->ctrls->ctrl[id].name);
}

/*
 * get the current value of a v4l2 control
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getCtrlValue(JNIEnv *e, jobject t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int val = 0, ret;
	ret = get_control_value(d->c,&(d->c->ctrls->ctrl[id]), &val);
	if(ret != 0) {
		THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for control '%s'", d->c->ctrls->ctrl[id].name);
		return -1;
	}

	dprint(LOG_LIBV4L, "[LIBV4L] Calling get_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->c->file,d->c->ctrls->ctrl[id].name,val);
	return val;
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
	dprint(LOG_LIBV4L, "[LIBV4L] Calling dequeue_buffer(dev: %s)\n", d->c->file);
	if((frame = (*d->c->capture->dequeue_buffer)(d->c, &d->capture_len)) != NULL) {
		i = d->buf_id = (d->buf_id == (d->c->mmap->buffer_nr-1)) ? 0 : d->buf_id+1;
		dprint(LOG_LIBV4L, "[LIBV4L] i=%d\n", i);
		(*d->j->jpeg_encode)(d, frame, d->bufs[i]);
		dprint(LOG_LIBV4L, "[LIBV4L] Calling enqueue_buffer(dev: %s)\n", d->c->file);
		(*d->c->capture->enqueue_buffer)(d->c);
		return i;
	}
	dprint(LOG_V4L4J, "Error dequeuing buffer for capture\n");
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
	dprint(LOG_LIBV4L, "[LIBV4L] Calling stop_capture(dev: %s)\n", d->c->file);
	if((*d->c->capture->stop_capture)(d->c)<0) {
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
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_delete(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int i;

	destroy_jpeg_compressor(d);

	dprint(LOG_LIBV4L, "[LIBV4L] Calling free_capture(dev: %s)\n", d->c->file);
	(*d->c->capture->free_capture)(d->c);

	dprint(LOG_V4L4J, "[V4L4J] Freeing %d ByteBuffers areas and array\n",d->c->mmap->buffer_nr);
	for(i=0; i<d->c->mmap->buffer_nr;i++)
		XFREE(d->bufs[i]);
	XFREE(d->bufs);

	dprint(LOG_LIBV4L, "[LIBV4L] Calling free_capture_device(dev: %s)\n", d->c->file);
	free_capture_device(d->c);
}

/*
 * free v4l4j device
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_freeObject(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_V4L4J, "[V4L4J] Freeing v4l4j device\n");
	XFREE(d);
}

