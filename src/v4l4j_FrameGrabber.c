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

#include "v4l2-input.h"
#include "debug.h"

struct jpeg {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_destination_mgr destmgr;
};

struct v4l4j_device {
	struct capture_device *c;	//the V4L2 struct
	unsigned char **bufs;		//the buffers holding the RGB data
	struct jpeg j;				//the jpeg compressor
	int jpeg_quality;			//the jpeg quality
	int len;					//the size of the last compressed frame
}; 

/* Dummy JPEG methods */
void init_destination( j_compress_ptr cinfo ){}
boolean empty_output_buffer( j_compress_ptr cinfo ){return TRUE;}
void term_destination( j_compress_ptr cinfo ){}

/* Exception throwing helper */
void throwV4L4JException(JNIEnv *e, const char *message) {
  jclass JV4L4JException = (*e)->FindClass(e,"au/edu/jcu/v4l4j/V4L4JException");
  if(JV4L4JException == 0) {
    return;
  }
  (*e)->ThrowNew(e, JV4L4JException, message);
}

/* Encodes a YUV planar frame of width "width and height "height" at "src" straight 
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and 
 * must be initialised correctly by the caller
 */
void jpeg_encode(struct v4l4j_device *d, void *src, int width, int height, int *len, void *dst) {
	//Code for this function is taken from Motion
	//Credit to them !!!
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	JSAMPROW y[16],cb[16],cr[16]; 
	JSAMPARRAY data[3]; 
	int i, line, rgb_size;
	
	data[0] = y;
	data[1] = cb;
	data[2] = cr;
	
	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j.destmgr.next_output_byte = dst;
	d->j.destmgr.free_in_buffer = rgb_size;
	jpeg_set_quality(&(d->j.cinfo),d->jpeg_quality,TRUE);
	
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", rgb_size);	
	jpeg_start_compress( &(d->j.cinfo), TRUE );
	for (line=0; line<height; line+=16) {
		for (i=0; i<16; i++) {
			y[i] = (unsigned char *)src + width*(i+line);
			if (i%2 == 0) {
				cb[i/2] = (unsigned char *)src + width*height + width/2*((i+line)/2);
				cr[i/2] = (unsigned char *)src + width*height + width*height/4 + width/2*((i+line)/2);
			}
		}
		jpeg_write_raw_data(&(d->j.cinfo), data, 16);
	}
	jpeg_finish_compress(&(d->j.cinfo));
	d->len = rgb_size - d->j.cinfo.dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
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
 * initialise libv4l2 (open, set_cap_param, init_capture)
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
	
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling 'init_libv4l2(dev:%s, w:%d,h:%d, ch:%d, std:%d, nb_buf:%d)'\n",device_file,w,h,ch,std,n);
	d->c = init_libv4l2(device_file, w,h,ch,std,n);
	(*e)->ReleaseStringUTFChars(e, f,device_file);

	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling 'open_device(dev: %s)'\n",d->c->file);
	if(open_device(d->c)<0) {
		throwV4L4JException(e, "Cant open device");
		return 0;
	}

	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling 'check_capture_capabilities(dev: %s)'\n",d->c->file);
	if(check_capture_capabilities(d->c)<0){
		close_device(d->c);
		throwV4L4JException(e, "Device doesnt have capture/streaming capabilities");
		return 0;
	}

	
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling 'set_cap_param(dev: %s)'\n",d->c->file);
	if(set_cap_param(d->c)){
		close_device(d->c);
		throwV4L4JException(e, "Cant set capture parameters");
		return 0;
	}


	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling 'init_capture(dev: %s)'\n",d->c->file);
	if(init_capture(d->c)<0){
		close_device(d->c);
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
		close_device(d->c);
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
	dprint(LOG_V4L4J, "[V4L4J] Setting up the JPEG compressor\n");
	d->j.cinfo.err = jpeg_std_error(&(d->j.jerr));
	jpeg_create_compress(&(d->j.cinfo));
	d->j.cinfo.image_width = w;
	d->j.cinfo.image_height = h;
	d->j.cinfo.input_components = 3;
	jpeg_set_defaults(&(d->j.cinfo)) ;
	jpeg_set_colorspace(&(d->j.cinfo), JCS_YCbCr);
	
	d->j.cinfo.raw_data_in = TRUE; // supply downsampled data
	d->j.cinfo.comp_info[0].h_samp_factor = 2;
	d->j.cinfo.comp_info[0].v_samp_factor = 2;
	d->j.cinfo.comp_info[1].h_samp_factor = 1;
	d->j.cinfo.comp_info[1].v_samp_factor = 1;
	d->j.cinfo.comp_info[2].h_samp_factor = 1;
	d->j.cinfo.comp_info[2].v_samp_factor = 1;
	
	jpeg_set_quality(&(d->j.cinfo), q, TRUE);
	d->j.cinfo.dct_method = JDCT_FASTEST;
	
	d->j.destmgr.init_destination = init_destination;
	d->j.destmgr.empty_output_buffer = empty_output_buffer;
	d->j.destmgr.term_destination = term_destination;
		
	d->j.cinfo.dest = &(d->j.destmgr);
	
	d->jpeg_quality=q;
	
	return arr;
}

/*
 * tell libv4l2 to start the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_start(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object; 
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling start_capture(dev: %s)\n", d->c->file);
	if(start_capture(d->c)<0){
		close_device(d->c);
		throwV4L4JException(e, "Cant start capture");
	}
}

/*
 * tell the JPEG compressor the compression factor
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
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling set_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->c->file,d->c->ctrls->ctrl[id].name,value);
	set_control_value(d->c, &(d->c->ctrls->ctrl[id]), value);
}

/*
 * get the current value of a v4l2 control
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getCtrlValue(JNIEnv *e, jobject t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	int val = get_control_value(d->c,&(d->c->ctrls->ctrl[id])); 
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling get_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->c->file,d->c->ctrls->ctrl[id].name,val);
	return val;
}


/*
 * get a new JPEG-compressed frame from the device
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getBuffer(JNIEnv *e, jobject t, jlong object) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	int i;
	struct v4l2_buffer *b;
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	
	//get frame from v4l2 
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling dequeue_buffer(dev: %s)\n", d->c->file);
	if((b = dequeue_buffer(d->c)) != NULL) {
		i=b->index;
		jpeg_encode(d, d->c->mmap->buffers[i].start, d->c->width, d->c->height, &(d->len), d->bufs[i]);
		dprint(LOG_LIBV4L2, "[LIBV4L2] Calling enqueue_buffer(dev: %s)\n", d->c->file);
		enqueue_buffer(d->c, b);
		return i;
	}

	throwV4L4JException(e, "Cant get buffer ");
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
 * tell libv4l2 to stop the capture
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_stop(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling stop_capture(dev: %s)\n", d->c->file);
	if(stop_capture(d->c)<0)
		fprintf(stderr, "Error stopping capture\n");
}

/*
 * free v4l4j device
 * free JPEG compressor
 * free libv4l2 (free_capture, close, del_libv4l2)
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_delete(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (jint) object;
	int i;
	
	dprint(LOG_JPEG, "[JPEG] Destroying JPEG compressor\n");
	jpeg_destroy_compress(&(d->j.cinfo));
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling free_capture(dev: %s)\n", d->c->file);
	free_capture(d->c);
	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling close_device(dev: %s)\n", d->c->file);
	close_device(d->c);

	dprint(LOG_V4L4J, "[V4L4J] Freeing %d ByteBuffers areas\n",d->c->mmap->buffer_nr);
	for(i=0; i<d->c->mmap->buffer_nr;i++)
		XFREE(d->bufs[i]);
	XFREE(d->bufs);	

	dprint(LOG_LIBV4L2, "[LIBV4L2] Calling del_libv4l2(dev: %s)\n", d->c->file);
	del_libv4l2(d->c);
	
	dprint(LOG_V4L4J, "[V4L4J] Freeing v4l4j device\n");
	XFREE(d);
}

