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

struct jpeg {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_destination_mgr destmgr;
};

static struct capture_device *c;	//the V4L2 struct
static struct v4l2_buffer *b;		//the latest queued/dequeued buffer
static unsigned char **bufs;		//the buffers holding the RGB data
static struct jpeg j;				//the jpeg compressor
static int jpeg_quality;			//the jpeg quality
static int len;						//the size of the last compressed frame 

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
void jpeg_encode(void *src, int width, int height, int *len, void *dst) {
	//Code for this function is taken from Motion
	//Credit to them !!!
	
	JSAMPROW y[16],cb[16],cr[16]; 
	JSAMPARRAY data[3]; 
	int i, line, rgb_size;
	
	data[0] = y;
	data[1] = cb;
	data[2] = cr;
	
	//init JPEG dest mgr
	rgb_size = width * height * 3;
	j.destmgr.next_output_byte = dst;
	j.destmgr.free_in_buffer = rgb_size;
	jpeg_set_quality(&j.cinfo, jpeg_quality,TRUE);
		
	jpeg_start_compress( &j.cinfo, TRUE );
	for (line=0; line<height; line+=16) {
		for (i=0; i<16; i++) {
			y[i] = (unsigned char *)src + width*(i+line);
			if (i%2 == 0) {
				cb[i/2] = (unsigned char *)src + width*height + width/2*((i+line)/2);
				cr[i/2] = (unsigned char *)src + width*height + width*height/4 + width/2*((i+line)/2);
			}
		}
		jpeg_write_raw_data(&j.cinfo, data, 16);
	}
	jpeg_finish_compress(&j.cinfo);
	*len = rgb_size - j.cinfo.dest->free_in_buffer;
}

/*
 * 
 */



JNIEXPORT jobjectArray JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_init_1v4l(JNIEnv *e, jobject t, jstring f, jint w, jint h, jint ch, jint std, jint n, jint q){
	int i=0, jpeg_buf_len;
	jclass this_class, v4l2ControlClass;
	jmethodID ctor;
	jfieldID field;
	jobject element;
	jobjectArray arr, ctrls;
	const char * device = (*e)->GetStringUTFChars(e, f, 0);
	struct control_list *l;
	
	c = init_libv4l2(device, w,h,ch,std,n);
	(*e)->ReleaseStringUTFChars(e, f,device);

	if(open_device(c)<0) {
		throwV4L4JException(e, "Cant open device");
		return 0;
	}

	if(check_capture_capabilities(c)<0){
		close_device(c);
		throwV4L4JException(e, "Device doesnt have capture/streaming capabilities");
		return 0;
	}

	
	if(set_cap_param(c)){
		close_device(c);
		throwV4L4JException(e, "Cant set capture parameters");
		return 0;
	}


	if(init_capture(c)<0){
		close_device(c);
		throwV4L4JException(e, "Cant initialise capture ");
		return 0;
	}

	//Updates the FrameCapture class width and height fields with the values returned by V4L2
	this_class = (*e)->GetObjectClass(e,t);
	field = (*e)->GetFieldID(e, this_class, "width", "I");
	(*e)->SetIntField(e, t, field, c->width);

	field = (*e)->GetFieldID(e, this_class, "height", "I");
	(*e)->SetIntField(e, t, field, c->height);
	
	
	//TODO fix the following, buffer way to large, but what is a correct value ?
	//The buffers which will hold the JPEG compressed frame
	jpeg_buf_len = c->width*c->height*3;
	
	//Create the ByteBuffer array
	arr = (*e)->NewObjectArray(e, c->mmap->buffer_nr, (*e)->FindClass(e, "java/nio/ByteBuffer"), NULL);
	bufs = (unsigned char **) malloc(c->mmap->buffer_nr * sizeof(void *));
	
	for(i=0; i<c->mmap->buffer_nr;i++) {
		//for each v4l2 buffers created, we create a corresponding java Bytebuffer
		bufs[i] = (unsigned char *) malloc( (size_t) jpeg_buf_len);
		element = (*e)->NewDirectByteBuffer(e, bufs[i], (jlong) jpeg_buf_len);
		(*e)->SetObjectArrayElement(e, arr, i, element);
	}
	
	//Creates the java objects matching v4l2 controls
	l = c->ctrls;
	v4l2ControlClass = (*e)->FindClass(e,"au/edu/jcu/v4l4j/V4L2Control");
	ctor = (*e)->GetMethodID(e, v4l2ControlClass, "<init>", "(ILjava/lang/String;IIILau/edu/jcu/v4l4j/FrameGrabber;)V");
	if(ctor == NULL){
		close_device(c);
		throwV4L4JException(e, "Cant create V4L2control java objects");
		return 0;
	}
	
	//initialise the ctrls field of FrameGrabber object (this)
	ctrls = (*e)->NewObjectArray(e, l->count, v4l2ControlClass, NULL);
		
	//Construct a V4L2Control Java object for each V4L2control in l
	for(i = 0; i< l->count; i++) {
		element = (*e)->NewObject(e, v4l2ControlClass, ctor, i, (*e)->NewStringUTF(e, (const char *)l->ctrl[i].name), l->ctrl[i].minimum, l->ctrl[i].maximum, l->ctrl[i].step, t);
		(*e)->SetObjectArrayElement(e, ctrls, i, element);
	}
	field = (*e)->GetFieldID(e, this_class, "ctrls", "[Lau/edu/jcu/v4l4j/V4L2Control;");
	(*e)->SetObjectField(e, t, field, ctrls);
	
	
	//setup jpeg param
	j.cinfo.err = jpeg_std_error(&j.jerr);
	jpeg_create_compress(&j.cinfo);
	j.cinfo.image_width = w;
	j.cinfo.image_height = h;
	j.cinfo.input_components = 3;
	jpeg_set_defaults(&j.cinfo) ;
	jpeg_set_colorspace(&j.cinfo, JCS_YCbCr);
	
	j.cinfo.raw_data_in = TRUE; // supply downsampled data
	j.cinfo.comp_info[0].h_samp_factor = 2;
	j.cinfo.comp_info[0].v_samp_factor = 2;
	j.cinfo.comp_info[1].h_samp_factor = 1;
	j.cinfo.comp_info[1].v_samp_factor = 1;
	j.cinfo.comp_info[2].h_samp_factor = 1;
	j.cinfo.comp_info[2].v_samp_factor = 1;
	
	jpeg_set_quality(&j.cinfo, q, TRUE);
	j.cinfo.dct_method = JDCT_FASTEST;
	
	j.destmgr.init_destination = init_destination;
	j.destmgr.empty_output_buffer = empty_output_buffer;
	j.destmgr.term_destination = term_destination;
		
	j.cinfo.dest = &j.destmgr;
	
	jpeg_quality=q;
	
	return arr;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_start(JNIEnv *e, jobject t){
	if(start_capture(c)<0){
		close_device(c);
		throwV4L4JException(e, "Cant start capture");
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_setQuality(JNIEnv *e, jobject t, jint q){
	jpeg_quality = q;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_setCtrlValue(JNIEnv *e, jobject t, jint id, jint value){
	set_control_value(c, &(c->ctrls->ctrl[id]), value);
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getCtrlValue(JNIEnv *e, jobject t, jint id){
	return get_control_value(c,&(c->ctrls->ctrl[id]));
}


JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getBuffer(JNIEnv *e, jobject t){
	int i;
	//get frame from v4l2 
	if((b = dequeue_buffer(c)) != NULL) {
		i=b->index;
		jpeg_encode(c->mmap->buffers[i].start, c->width, c->height, &len, bufs[i]);
		enqueue_buffer(c, b);
		return i;
	}

	throwV4L4JException(e, "Cant get buffer ");
	return -1;
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_getBufferLength(JNIEnv *e, jobject t){
	return len;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_stop(JNIEnv *e, jobject t){
	if(stop_capture(c)<0)
		fprintf(stderr, "Error stopping capture\n");
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameGrabber_delete(JNIEnv *e, jobject t){
	int i;
	jpeg_destroy_compress(&j.cinfo);
	free_capture(c);
	close_device(c);
	for(i=0; i<c->mmap->buffer_nr;i++) {
		free(bufs[i]);
	}
	free(bufs);
	del_libv4l2(c);
}


