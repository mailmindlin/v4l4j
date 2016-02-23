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

static inline struct frame_encoder* mapEncoderPtr(jlong ptr) {
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
	
	encoder->is_series = FALSE;
	
	struct v4lconvert_encoder* v4lencoder;
	XMALLOC(v4lencoder, struct v4lconvert_encoder*, sizeof(struct v4lconvert_encoder));
	v4lconvert_encoder_init(v4lencoder, from, width, height);
	encoder->v4lenc.encoder = v4lencoder;
	
	return (uintptr_t) encoder;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doRelease(JNIEnv *env, jobject self, jlong ptr) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = mapEncoderPtr(ptr);
	
	if (!(encoder->is_series)) {
		dprint(LOG_V4L4J, "[V4L4J] Freeing v4lconvert encoder\n");
		XFREE(encoder->v4lenc.encoder);
	} else {
		//TODO finish
	}
	
	dprint(LOG_V4L4J, "[V4L4J] Freeing frame encoder\n");
	XFREE(encoder);
}

/**
 * 
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getBufferSize(JNIEnv* env, jobject self, jlong ptr) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = mapEncoderPtr(ptr);
	
	return -1;
}

/**
 * 
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_setQuality (JNIEnv* env, jobject self, jlong ptr, jint quality) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = mapEncoderPtr(ptr);
	
	dprint(LOG_V4L4J, "[V4L4J] Setting JPEG quality to %d\n",quality);
	encoder->jpeg_quality = quality;
}

/**
 * 
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_putBuffer(JNIEnv* env, jobject self, jlong ptr, jbyteArray buffer, jint length) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = mapEncoderPtr(ptr);
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getBuffer (JNIEnv* env, jobject self, jlong ptr, jbyteArray buffer) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = mapEncoderPtr(ptr);
	return -1;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_doConvert(JNIEnv * env, jobject self, jlong ptr) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct frame_encoder* encoder = mapEncoderPtr(ptr);
	
	encoder->convert(encoder, encoder->in_buffer, encoder->out_buffer);
}
/**
 * 
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_AbstractVideoFrameEncoder_getConverterIds(JNIEnv* env, jobject self, jlong ptr, jintArray out) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct frame_encoder* encoder = mapEncoderPtr(ptr);
	
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