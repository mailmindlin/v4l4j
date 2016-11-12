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

#include "common.h"
#include "debug.h"
#include "libvideo.h"
#include "libv4lconvert/types.h"
#include "libv4lconvert/libv4lconvert-flat.h"
/* Header for class au_edu_jcu_v4l4j_encoder_ImageFormatConverter */

#ifndef _Included_au_edu_jcu_v4l4j_encoder_ImageFormatConverter
#define _Included_au_edu_jcu_v4l4j_encoder_ImageFormatConverter
#ifdef __cplusplus
extern "C" {
#endif
static inline struct v4lconvert_encoder* lookupNative(JNIEnv* env, jobject self) __attribute__ ((pure));
static inline struct v4lconvert_encoder* lookupNative(JNIEnv* env, jobject self) {
	jclass ImageFormatConverter_class = lookupClassSafe(env, self);
	if (!ImageFormatConverter_class)
		return NULL;
	jfieldID objectFID = (*env)->GetFieldID(env, ImageFormatConverter_class, "object", "J");
	struct v4lconvert_encoder* result = (struct v4lconvert_encoder*) (uintptr_t) (*env)->GetLongField(env, self, objectFID);
	return result;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_ImageFormatConverter
 * Method:    lookupConverterByConversion
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_ImageFormatConverter_lookupConverterByConversion(JNIEnv *env, jclass me, jint from, jint to) {
	LOG_FN_ENTER();
	return v4lconvert_converter_lookupConverterByConversion((u32) from, (u32) to);
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_ImageFormatConverter
 * Method:    init
 * Signature: (III)J
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_ImageFormatConverter_initWithConverter(JNIEnv *env, jclass me, jint converterId, jint width, jint height) {
	LOG_FN_ENTER();
	v4lconvert_converter_t* converter = v4lconvert_converter_getConverterById(converterId);
	if (!converter) {
		THROW_EXCEPTION(env, NULL_EXCP, "No converter found with id %d", converterId);
		return -1;
	}
	
	struct v4lconvert_encoder* encoder;
	XMALLOC(encoder, struct v4lconvert_encoder*, sizeof(struct v4lconvert_encoder));
	if (!encoder) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error allocating memory for v4lconvert_encoder");
		return -1;
	}
	if (!v4lconvert_encoder_initWithConverter(encoder, converter, width, height)) {
		XFREE(encoder);
		THROW_EXCEPTION(env, INIT_EXCP, "Error initializing v4lconvert_encoder (converter #%d, width %d, height %d)", converter->id, width, height);
		return -1;
	}
	return (jlong)(uintptr_t) encoder;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_ImageFormatConverter
 * Method:    getData
 * Signature: (J)[I
 */
JNIEXPORT jintArray JNICALL Java_au_edu_jcu_v4l4j_encoder_ImageFormatConverter_getData(JNIEnv *env, jclass me, jlong ptr) {
	LOG_FN_ENTER();
	struct v4lconvert_encoder* encoder = (struct v4lconvert_encoder*) (uintptr_t) ptr;
	jintArray result = (*env)->NewIntArray(env, 9);
	if (!result) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not create int array");
		return NULL;
	}
	jint* data = (*env)->GetIntArrayElements(env, result, NULL);
	if (!data) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not get pointer to array");
		return NULL;
	}
	v4lconvert_converter_t* converter = encoder->converter;
	data[0] = converter ? (signed) converter->id : -1;
	data[1] = encoder->src_fmt;
	data[2] = encoder->dst_fmt;
	data[3] = encoder->src_len;
	data[4] = encoder->dst_len;
	if (encoder->getDimensions) {
		encoder->getDimensions(encoder, &(data[5]), &(data[6]), &(data[7]), &(data[8]));
	} else {
		data[5] = -1;
		data[6] = -1;
		data[7] = -1;
		data[8] = -1;
	}
	(*env)->ReleaseIntArrayElements(env, result, data, 0);
	return result;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_ImageFormatConverter
 * Method:    close
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_ImageFormatConverter_close(JNIEnv *env, jobject self) {
	LOG_FN_ENTER();
	struct v4lconvert_encoder* encoder = lookupNative(env, self);
	if (!encoder)
		return;
	encoder->release(encoder);
	XFREE(encoder);
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_ImageFormatConverter
 * Method:    encode
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_ImageFormatConverter_encode(JNIEnv *env, jobject self, jlong bufPtr) {
	LOG_FN_ENTER();
	struct v4lconvert_encoder* encoder = lookupNative(env, self);
	if (!encoder)
		return -1;
	struct v4lconvert_buffer* buffer = (struct v4lconvert_buffer*) (uintptr_t) bufPtr;
	if (!buffer->buf0 || !buffer->buf1) {
		THROW_EXCEPTION(env, NULL_EXCP, "One of the input buffers was null");
		return -1;
	}
	buffer->buf1_len = encoder->apply(encoder, buffer->buf0, buffer->buf1, buffer->buf0_len);
	if (!buffer->buf1_len) {
		if (buffer->buf1_cap < encoder->dst_len)
			THROW_EXCEPTION(env, UNDERFLOW_EXCP, "Output buffer capacity was %d; estimated output was %d", buffer->buf1_cap, encoder->dst_len);
	}
	return (int) buffer->buf1_len;
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_ImageFormatConverter_apply(JNIEnv* env, jobject self, jobject src, jobject dst) {
	LOG_FN_ENTER();
	struct v4lconvert_encoder* encoder = lookupNative(env, self);
	if (!encoder)
		return -1;
	const int limit = getBufferLimit(env, src);
	if (limit < 0)
		return -1;
	jbyte* srcPtr = (*env)->GetDirectBufferAddress(env, src);
	jbyte* dstPtr = (*env)->GetDirectBufferAddress(env, dst);
	if (!srcPtr || !dstPtr) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to get direct pointer to buffer");
		return -1;
	}
	//TODO finish
	return 0;
}

#ifdef __cplusplus
}
#endif
#endif
