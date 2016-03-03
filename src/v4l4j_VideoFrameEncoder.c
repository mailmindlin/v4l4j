/*
* Copyright (C) 20016 mailmindlin
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
#include <stdint.h>

#include "v4l4j_VideoFrameEncoder.h"
#include "libvideo.h"
#include "libvideo-palettes.h"
#include "common.h"
#include "debug.h"
#include "libv4lconvert/types.h"
#include "libv4lconvert/libv4lconvert-priv.h"
#include "libv4lconvert/libv4lconvert-flat.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct frame_buffer {
	u32 buffer_capacity;
	u32 buffer_limit;
	u8 lock;
	u8* buffer;
};

struct frame_encoder {
	u32 input_fmt;
	u32 output_fmt;
	u32 width;
	u32 height;
	u32 jpeg_quality;
	struct frame_buffer* in_buffer;
	struct frame_buffer* out_buffer;
	u8 is_series;
	union {
		struct v4lconvert_encoder* encoder;
		struct v4lconvert_encoder_series* encoder_series;
	} v4lenc;
};

static jclass AbstractVideoFrameEncoder_class = NULL;
static jfieldID AbstractVideoFrameEncoder_object_fid = NULL;

static struct frame_encoder* getPointer(JNIEnv* env, jobject self) {
	if (AbstractVideoFrameEncoder_class == NULL) {
		AbstractVideoFrameEncoder_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/encoder/AbstractVideoFrameEncoder");
		AbstractVideoFrameEncoder_object_fid = (*env)->GetFieldID(env, AbstractVideoFrameEncoder_class, "object", "J");
	}
	
	long ptr = (*env)->GetLongField(env, self, AbstractVideoFrameEncoder_object_fid);
	return (struct frame_encoder*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doInit(JNIEnv *env, jobject self, jint from, jint to, jint width, jint height) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder;
	XMALLOC(encoder, struct frame_encoder*, sizeof(struct frame_encoder));
	
	encoder->input_fmt = from;
	encoder->output_fmt = to;
	encoder->width = width;
	encoder->height = height;
	encoder->jpeg_quality = 101;
	
	//allocate buffers
	XMALLOC(encoder->in_buffer, struct frame_buffer*, sizeof(struct frame_buffer));
	XMALLOC(encoder->out_buffer, struct frame_buffer*, sizeof(struct frame_buffer));
	
	encoder->is_series = FALSE;
	
	struct v4lconvert_encoder* v4lencoder;
	XMALLOC(v4lencoder, struct v4lconvert_encoder*, sizeof(struct v4lconvert_encoder));
	
	unsigned int converterId = v4lconvert_converter_lookupConverterByConversion(from, to);
	v4lconvert_encoder_init(v4lencoder, converterId, width, height);
	
	encoder->v4lenc.encoder = v4lencoder;
	
	return (uintptr_t) encoder;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doRelease(JNIEnv *env, jobject self, jlong ptr) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	if (encoder->in_buffer->buffer != NULL)
		XFREE(encoder->in_buffer->buffer);
	XFREE(encoder->in_buffer);
	
	if (encoder->out_buffer->buffer != NULL)
		XFREE(encoder->out_buffer->buffer);
	XFREE(encoder->out_buffer);
	
	if (!(encoder->is_series)) {
		dprint(LOG_V4L4J, "[V4L4J] Freeing v4lconvert encoder\n");
		XFREE(encoder->v4lenc.encoder);
	} else {
		//TODO finish
	}
	
	dprint(LOG_V4L4J, "[V4L4J] Freeing frame encoder\n");
	XFREE(encoder);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_setBufferCapacity(JNIEnv* env, jobject self, jint bufferNum, jint capacity) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	struct frame_buffer* buffer;
	if (bufferNum == 1) {
		buffer = encoder->in_buffer;
	} else if (bufferNum == 2) {
		buffer = encoder->out_buffer;
	} else {
		THROW_EXCEPTION(env, JNI_EXCP, "Illegal buffer #");
		return;
	}
	
	if (buffer->lock) {
		THROW_EXCEPTION(env, JNI_EXCP, "Buffer is in use");
		return;
	}
	buffer->lock = 1;
	
	if (buffer->buffer != NULL)
		XFREE(buffer->buffer);
	
	XCALLOC(buffer->buffer, u8*, sizeof(u8), capacity);
	
	buffer->lock = 0;
}

/**
 * 
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getBufferCapacity(JNIEnv* env, jobject self, jint buffer) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	if (buffer == 1)
		return encoder->in_buffer->buffer_capacity;
	
	if (buffer == 2)
		return encoder->out_buffer->buffer_capacity;
	
	return -1;
}


JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getBufferLimit(JNIEnv* env, jobject self, jint buffer) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	if (buffer == 1)
		return encoder->in_buffer->buffer_limit;
	
	if (buffer == 2)
		return encoder->out_buffer->buffer_limit;
	
	return -1;
}

/**
 * 
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_setQuality (JNIEnv* env, jobject self, jint quality) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	dprint(LOG_V4L4J, "[V4L4J] Setting JPEG quality to %d\n",quality);
	encoder->jpeg_quality = quality;
}

/**
 * 
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_putBuffer(JNIEnv* env, jobject self, jbyteArray buffer, jint length) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	if (length > encoder->in_buffer->buffer_capacity) {
		THROW_EXCEPTION(env, JNI_EXCP, "Tried to store more data than can fit in the buffer");
		return;
	}
	
	//TODO use some atomic thing
	if (encoder->in_buffer->lock) {
		THROW_EXCEPTION(env, JNI_EXCP, "Input buffer is in use");
		return;
	}
	
	encoder->in_buffer->lock = 1;
	
	(*env)->GetByteArrayRegion(env, buffer, 0, length, (jbyte*) (encoder->in_buffer->buffer));
	
	encoder->in_buffer->buffer_limit = length;
		
	encoder->in_buffer->lock = 0;
	
	//success
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getBuffer (JNIEnv* env, jobject self, jbyteArray buffer) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	jsize length = (*env)->GetArrayLength(env, buffer);
	
	if (length > encoder->out_buffer->buffer_limit)
		length = encoder->out_buffer->buffer_limit;
	
	//TODO use some atomic thing
	if (encoder->out_buffer->lock) {
		THROW_EXCEPTION(env, JNI_EXCP, "Output buffer is in use");
		return 0;
	}
	
	encoder->out_buffer->lock = 1;
	
	(*env)->GetByteArrayRegion(env, buffer, 0, length, (jbyte*) (encoder->out_buffer->buffer));
		
	encoder->out_buffer->lock = 0;
	
	return length;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doConvert(JNIEnv * env, jobject self, jlong ptr) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = getPointer(env, self);
	
	//Lock buffers
	if (encoder->in_buffer->lock) {
		THROW_EXCEPTION(env, JNI_EXCP, "Input buffer is in use");
		return;
	}
	encoder->in_buffer->lock = 1;
	if (encoder->out_buffer->lock) {
		encoder->in_buffer->lock = 0;
		THROW_EXCEPTION(env, JNI_EXCP, "Output buffer is in use");
		return;
	}
	
	if (encoder->is_series) {
		struct v4lconvert_encoder_series* v4lencoders = encoder->v4lenc.encoder_series;
		//TODO finish
	} else {
		struct v4lconvert_encoder* v4lencoder = encoder->v4lenc.encoder;
		v4lencoder->convert(v4lencoder, encoder->in_buffer->buffer, encoder->out_buffer->buffer);
	}
	
	encoder->in_buffer->lock = 0;
	encoder->out_buffer->lock = 0;
}
/**
 * 
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getConverterIds(JNIEnv* env, jobject self, NULLjintArray out) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct frame_encoder* encoder = getPointer(env, self);
	
	jsize out_length;
	if (out == NULL) {
		//array was null, so we're just goint to return the number of encoders
		out_length = 0;
	} else {
		out_length = (*env)->GetArrayLength(env, out);
	}
	
	int num_converters = 1;
	if (encoder->is_series)
		num_converters = encoder->v4lenc.encoder_series->num_converters;
	
	if (num_converters >  out_length)
		return -num_converters;
	
	jint* converter_ids;
	XCALLOC(converter_ids, jint*, sizeof(jint), out_length);
	if (converter_ids == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to allocate memory.");
		return 0;
	}
	
	if (encoder->is_series) {
		struct v4lconvert_encoder_series* v4lcvt_series = encoder->v4lenc.encoder_series;
		for (int i=0; i < num_converters; i++)
			converter_ids[i] = v4lcvt_series->encoders[i]->converter->id;
	} else {
		converter_ids[0] = encoder->v4lenc.encoder->converter->id;
	}
	
	(*env)->SetIntArrayRegion(env, out, 0, num_converters, converter_ids);
	
	XFREE(converter_ids);
	//TODO check that it was freed
	
	return num_converters;
}