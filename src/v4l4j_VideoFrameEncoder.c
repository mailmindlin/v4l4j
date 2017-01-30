/*
* Copyright (C) 20016 mailmindlin
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <jni.h>
#include <stdint.h>
#include <stdbool.h>

#include "v4l4j_VideoFrameEncoder.h"
#include "libvideo.h"
#include "libvideo-palettes.h"
#include "common.h"
#include "debug.h"
#include "libv4lconvert/types.h"
#include "libv4lconvert/libv4lconvert-priv.h"
#include "libv4lconvert/libv4lconvert-flat.h"

struct frame_encoder_data {
	struct v4lconvert_buffer** buffers;
	struct v4lconvert_encoder_series* encoder;
};

static jclass AbstractVideoFrameEncoder_class = NULL;
static jfieldID AbstractVideoFrameEncoder_object_fid = NULL;

static bool initJNI(JNIEnv* env, jobject self) {
	
}
static inline struct frame_encoder_data* getPointer(JNIEnv* env, jobject self) {
	if (!AbstractVideoFrameEncoder_class || !AbstractVideoFrameEncoder_object_fid) {
		AbstractVideoFrameEncoder_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/encoder/AbstractVideoFrameEncoder");
		AbstractVideoFrameEncoder_object_fid = (*env)->GetFieldID(env, AbstractVideoFrameEncoder_class, "object", "J");
	}
	
	long ptr = (*env)->GetLongField(env, self, AbstractVideoFrameEncoder_object_fid);
	return (struct frame_encoder_data*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doInit(JNIEnv *env, jobject self, jint from, jint to, jint width, jint height) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder_data* encoder;
	XMALLOC(encoder, struct frame_encoder_data*, sizeof(struct frame_encoder_data));
	
	encoder->input_fmt = from;
	encoder->output_fmt = to;
	encoder->width = width;
	encoder->height = height;
	
	//allocate buffers
	XMALLOC(encoder->in_buffer, struct frame_buffer*, sizeof(struct frame_buffer));
	XMALLOC(encoder->out_buffer, struct frame_buffer*, sizeof(struct frame_buffer));
	
	int converterId = v4lconvert_converter_lookupConverterByConversion(from, to);
	
	if (converterId >= 0) {
		struct v4lconvert_encoder* v4lencoder;
		XMALLOC(v4lencoder, struct v4lconvert_encoder*, sizeof(struct v4lconvert_encoder));
		if (!v4lconvert_encoder_init(v4lencoder, converterId, width, height)) {
			XFREE(v4lencoder);
			THROW_EXCEPTION(env, JNI_EXCP, "Error initializing encoder (fmt %d => %d; converter #%d)", from, to, converterId);
			return -1;
		}
		encoder->is_series = FALSE;
		encoder->encoder = v4lencoder;
		encoder->out_buffer->buffer_capacity = (*v4lencoder->estimate_result_size)(v4lencoder);
	} else {
		//Attempt to upgrade to series
		struct v4lconvert_encoder_series* series;
		XMALLOC(series, struct v4lconvert_encoder_series*, sizeof(struct v4lconvert_encoder_series));
		if (!v4lconvert_encoder_series_computeConversion(series, width, height, from, to, 4)) {
			XFREE(series);
			THROW_EXCEPTION(env, JNI_EXCP, "Error initializing encoder series (fmt %d => %d; 4 iterations)", from, to);
			return -1;
		}
		encoder->is_series = TRUE;
		encoder->encoder_series = series;
		struct v4lconvert_encoder* last_encoder = &(series->encoders[series->num_encoders - 1]);
		encoder->out_buffer->buffer_capacity = (*last_encoder->estimate_result_size)(last_encoder);
	}
	
	XMALLOC(encoder->out_buffer->buffer, void*, encoder->out_buffer->buffer_capacity * sizeof(void*));
	
	return (uintptr_t) encoder;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doRelease(JNIEnv *env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);	
	
	struct frame_encoder_data* encoder = getPointer(env, self);
	
	if (encoder->in_buffer) {
		if (encoder->in_buffer->buffer)
			XFREE(encoder->in_buffer->buffer);
		XFREE(encoder->in_buffer);
		encoder->in_buffer = NULL;
	}
	
	if (encoder->out_buffer) {
		if (encoder->out_buffer->buffer)
			XFREE(encoder->out_buffer->buffer);
		XFREE(encoder->out_buffer);
		encoder->out_buffer = NULL;
	}
	
	if (encoder->is_series) {
		dprint(LOG_V4L4J, "[V4L4J] Freeing v4lconvert encoder series\n");
		v4lconvert_encoder_series_doRelease(encoder->encoder_series);
		XFREE(encoder->encoder_series);
		encoder->encoder_series = NULL;
	} else {
		dprint(LOG_V4L4J, "[V4L4J] Freeing v4lconvert encoder\n");
		encoder-
		XFREE(encoder->encoder);
		encoder->encoder = NULL;
	}
	
	dprint(LOG_V4L4J, "[V4L4J] Freeing frame encoder\n");
	XFREE(encoder);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_setBufferCapacity(JNIEnv* env, jobject self, jint bufferNum, jint capacity) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder_data* encoder = getPointer(env, self);
	
	struct frame_buffer* buffer;
	if (bufferNum == 1) {
		buffer = encoder->in_buffer;
	} else if (bufferNum == 2) {
		buffer = encoder->out_buffer;
	} else {
		THROW_EXCEPTION(env, JNI_EXCP, "Illegal buffer #%d. Only 2 buffers exist", bufferNum);
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
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);
	
	struct frame_encoder_data* encoder = getPointer(env, self);
	
	if (buffer == 1)
		return encoder->in_buffer->buffer_capacity;
	
	if (buffer == 2)
		return encoder->out_buffer->buffer_capacity;
	
	return -1;
}


JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getBufferLimit(JNIEnv* env, jobject self, jint buffer) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);
	
	struct frame_encoder_data* encoder = getPointer(env, self);
	
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
	
	struct frame_encoder_data* encoder = getPointer(env, self);
	
	dprint(LOG_V4L4J, "[V4L4J] Setting JPEG quality to %d\n",quality);
	encoder->jpeg_quality = quality;
}

/**
 * 
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_putBuffer(JNIEnv* env, jobject self, jbyteArray buffer, jint length) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder_data* encoder = getPointer(env, self);
	
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
	
	struct frame_encoder_data* encoder = getPointer(env, self);
	
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

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doConvert(JNIEnv * env, jobject self, jint bufIdx) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder_data* data = getPointer(env, self);
	
	if (!data) {
		
	}
	
	if (bufIdx < 0 || bufIdx > data->num_buffers) {
		THROW_EXCEPTION(env, INVALID_VAL_EXCP, "Buffer index must be a valid index (min: 0, max: %d, value: %d)", data->num_buffers, bufIdx);
		return;
	}
	struct v4lconvert_buffer
	
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
		struct v4lconvert_encoder_series* series = encoder->encoder_series;
		(*series->convert)(series, encoder->in_buffer->buffer, encoder->out_buffer->buffer);
	} else {
		struct v4lconvert_encoder* v4lencoder = encoder->encoder;
		(*v4lencoder->convert)(v4lencoder, encoder->in_buffer->buffer, encoder->out_buffer->buffer);
	}
	
	//Release locks
	encoder->in_buffer->lock = 0;
	encoder->out_buffer->lock = 0;
}
/**
 * 
 */
JNIEXPORT jintArray JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getConverterIds(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct frame_encoder_data* data = getPointer(env, self);
	
	u32 num_encoders = data->encoder->num_encoders;
	//Instantiate the result array
	jintArray result = (*env)->NewIntArray(env, num_encoders);
	
	//Get a pointer to the array
	//We don't really care if it was coppied, because the JVM will handle it later
	int* array = (*env)->GetIntArrayElements(env, result, NULL);
	for (unsigned i = 0; i < num_encoders; i++)
		array[i] = data->encoder->encoders[i]->converter->id;
	
	//Release the array pointer, and possibly copy the elements back.
	(*env)->ReleaseIntArrayElements(env, result, array, 0);
	
	return result;
}