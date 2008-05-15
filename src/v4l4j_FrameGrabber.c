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

#include "libv4l.h"
#include "common.h"
#include "jpeg.h"
#include "debug.h"

#define SUPPORTED_FORMATS		{YUV420, RGB24}
#define NB_SUPPORTED_FORMATS	2
#define INCR_BUF_ID(i, max)			do { (i) = ((i) >= (max)) ? 0 : ((i) + 1); } while(0)

/* Exception throwing helper */
void throwV4L4JException(JNIEnv *e, const char *message) {
  jclass JV4L4JException = (*e)->FindClass(e,"au/edu/jcu/v4l4j/V4L4JException");
  if(JV4L4JException == 0) {
    return;
  }
  (*e)->ThrowNew(e, JV4L4JException, message);
}

/*
 * Allocate a new v4l4j device
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_allocateObject(JNIEnv *e, jobject t){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d;
	XMALLOC(d, struct v4l4j_device *, sizeof(struct v4l4j_device));
	return (jint) d;
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
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object; 
	int fmts[NB_SUPPORTED_FORMATS] = SUPPORTED_FORMATS; 
	
	dprint(LOG_LIBV4L, "[LIBV4L] Calling 'init_libv4l(dev:%s, w:%d,h:%d, ch:%d, std:%d, nb_buf:%d)'\n",device_file,w,h,ch,std,n);
	d->c = init_libv4l(device_file, w,h,ch,std,n);
	(*e)->ReleaseStringUTFChars(e, f,device_file);

	if(d->c==NULL) {
		dprint(LOG_V4L4J, "[V4L4J] init_libv4l failed\n");
		throwV4L4JException(e, "Cant initialise device (check capture resolution / device file permission)");
		return 0;
	}

	dprint(LOG_LIBV4L, "[LIBV4L] Calling 'set_cap_param(dev: %s)'\n",d->c->file);
	if(set_cap_param(d->c, fmts, NB_SUPPORTED_FORMATS)){
		dprint(LOG_V4L4J, "[V4L4J] set_cap_param failed\n");
		del_libv4l(d->c);
		throwV4L4JException(e, "Cant set capture parameters");
		return 0;
	}


	dprint(LOG_LIBV4L, "[LIBV4L] Calling 'init_capture(dev: %s)'\n",d->c->file);
	if(init_capture(d->c)<0){
		dprint(LOG_V4L4J, "[V4L4J] init_capture failed\n");
		del_libv4l(d->c);
		throwV4L4JException(e, "Cant initialise capture ");
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
	
	//TODO fix the following, buffer way to large, but what is a correct value ?
	//The buffers which will hold the JPEG compressed frame
	jpeg_buf_len = d->c->width * d->c->height*3;
	
	//Create the ByteBuffer array
	dprint(LOG_V4L4J, "[V4L4J] Creating the ByteBuffer array[%d]\n",d->c->mmap->buffer_nr);
	arr = (*e)->NewObjectArray(e, d->c->mmap->buffer_nr, (*e)->FindClass(e, "java/nio/ByteBuffer"), NULL);
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
	dprint(LOG_V4L4J, "[V4L4J] Creating the V4L2Control array[%d]\n", l->count);
	v4l2ControlClass = (*e)->FindClass(e,"au/edu/jcu/v4l4j/V4L2Control");
	ctor = (*e)->GetMethodID(e, v4l2ControlClass, "<init>", "(ILjava/lang/String;IIILau/edu/jcu/v4l4j/FrameGrabber;)V");
	if(ctor == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the V4L2Control class\n");
		free_capture(d->c);
		del_libv4l(d->c);
		throwV4L4JException(e, "Cant create V4L2control java objects");
		return 0;
	}
	
	//initialise the ctrls field of FrameGrabber object (this)
	ctrls = (*e)->NewObjectArray(e, l->count, v4l2ControlClass, NULL);
		
	//Construct a V4L2Control Java object for each V4L2control in l
	for(i = 0; i< l->count; i++) {
		dprint(LOG_V4L4J, "[V4L4J] Creating V4L2Control %d - name: %s\n", i, l->ctrl[i].name);
		element = (*e)->NewObject(e, v4l2ControlClass, ctor, i, (*e)->NewStringUTF(e, (const char *)l->ctrl[i].name), l->ctrl[i].minimum, l->ctrl[i].maximum, l->ctrl[i].step, t);
		(*e)->SetObjectArrayElement(e, ctrls, i, element);
	}
	field = (*e)->GetFieldID(e, this_class, "ctrls", "[Lau/edu/jcu/v4l4j/V4L2Control;");
	(*e)->SetObjectField(e, t, field, ctrls);
	
	//setup jpeg param
	if(init_jpeg_compressor(d, q)!=0) {
		dprint(LOG_V4L4J, "[V4L4J] Error initialising the JPEG compressor\n");
		free_capture(d->c);
		del_libv4l(d->c);
		throwV4L4JException(e, "Cant initialise the JPEG compressor");
	}	
	d->buf_id = -1;
	return arr;
}

/*
 * tell LIBV4L to start the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_start(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object; 
	dprint(LOG_LIBV4L, "[LIBV4L] Calling start_capture(dev: %s)\n", d->c->file);
	if(start_capture(d->c)<0){
		dprint(LOG_V4L4J, "[V4L4J] start_capture failed\n");
		throwV4L4JException(e, "Cant start capture");
	}
}

/*
 * tell the JPEG compressor the new compression factor
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_setQuality(JNIEnv *e, jobject t, jlong object, jint q){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	dprint(LOG_V4L4J, "[V4L4J] Setting JPEG quality to %d\n",q); 
	d->jpeg_quality = q;
}

/*
 * Set a new value on a v4l2 control
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_setCtrlValue(JNIEnv *e, jobject t, jlong object, jint id, jint value){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	dprint(LOG_LIBV4L, "[LIBV4L] Calling set_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->c->file,d->c->ctrls->ctrl[id].name,value);
	set_control_value(d->c, &(d->c->ctrls->ctrl[id]), value);
}

/*
 * get the current value of a v4l2 control
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getCtrlValue(JNIEnv *e, jobject t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	int val = get_control_value(d->c,&(d->c->ctrls->ctrl[id])); 
	dprint(LOG_LIBV4L, "[LIBV4L] Calling get_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->c->file,d->c->ctrls->ctrl[id].name,val);
	return val;
}


/*
 * get a new JPEG-compressed frame from the device
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getBuffer(JNIEnv *e, jobject t, jlong object) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int i, len;
	struct v4l2_buffer *b;
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	
	//get frame from v4l2 
	dprint(LOG_LIBV4L, "[LIBV4L] Calling dequeue_buffer(dev: %s)\n", d->c->file);
	if((b = dequeue_buffer(d->c)) != NULL) {
		i = d->buf_id = (d->buf_id == (d->c->mmap->buffer_nr-1)) ? 0 : d->buf_id+1;
		dprint(LOG_LIBV4L, "[LIBV4L] i=%d\n", i); 
		(*d->j.jpeg_encode)(d, get_frame_buffer(d->c, b, &len), d->bufs[i]);
		dprint(LOG_LIBV4L, "[LIBV4L] Calling enqueue_buffer(dev: %s)\n", d->c->file);
		enqueue_buffer(d->c, b);
		return i;
	}
	dprint(LOG_V4L4J, "Error dequeuing buffer for capture\n");
	throwV4L4JException(e, "Cant get buffer");
	return -1;
}

/*
 * return the length of the last captured frame
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getBufferLength(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	dprint(LOG_V4L4J, "[V4L4J] Getting last frame length: %d\n", d->len);
	return d->len;
}

/*
 * tell LIBV4L to stop the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_stop(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	dprint(LOG_LIBV4L, "[LIBV4L] Calling stop_capture(dev: %s)\n", d->c->file);
	if(stop_capture(d->c)<0) {
		dprint(LOG_V4L4J, "Error stopping capture\n");
		//not sure whether we should throw an exception here...
		//if we do, FrameGrabber wont let us call delete (free_capture,del_libv4l2)
		//because its state will be stuck in capture...
		//throwV4L4JException(e, "Cant stop capture");
	}
}

/*
 * free JPEG compressor
 * free LIBV4L (free_capture, del_libv4l2)
 * free v4l4j device
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_delete(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	int i;
	
	destroy_jpeg_compressor(d);
	
	dprint(LOG_LIBV4L, "[LIBV4L] Calling free_capture(dev: %s)\n", d->c->file);
	free_capture(d->c);

	dprint(LOG_V4L4J, "[V4L4J] Freeing %d ByteBuffers areas and array\n",d->c->mmap->buffer_nr);
	for(i=0; i<d->c->mmap->buffer_nr;i++)
		XFREE(d->bufs[i]);
	XFREE(d->bufs);	

	dprint(LOG_LIBV4L, "[LIBV4L] Calling del_libv4l2(dev: %s)\n", d->c->file);
	del_libv4l(d->c);
	
	dprint(LOG_V4L4J, "[V4L4J] Freeing v4l4j device\n");
	XFREE(d);
}

