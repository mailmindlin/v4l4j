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

#include "libvideo.h"
#include "libvideo-palettes.h"
#include "common.h"
#include "debug.h"
#include "libv4lconvert/types.h"
#include "libv4lconvert/libv4lconvert-priv.h"
#include "libv4lconvert/libv4lconvert-flat.h"

static jclass V4lconvertBuffer_class = NULL;
static jfieldID V4lconvertBuffer_object_fid = NULL;
static jfieldID V4lconvertBuffer_src_fid = NULL;

static void initFIDs(JNIEnv* env) {
	if (!V4lconvertBuffer_class)
		V4lconvertBuffer_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/encoder/V4lconvertBuffer");
	if (!V4lconvertBuffer_object_fid)
		V4lconvertBuffer_object_fid = (*env)->GetFieldID(env, V4lconvertBuffer_class, "object", "J");
	if (!V4lconvertBuffer_src_fid)
		V4lconvertBuffer_src_fid = (*env)->GetFieldID(env, V4lconvertBuffer_class, "src", "Ljava/nio/ByteBuffer");
	if (!V4lconvertBuffer_dst_fid)
		V4lconvertBuffer_dst_fid = (*env)->GetFieldID(env, V4lconvertBuffer_class, "dst", "Ljava/nio/ByteBuffer");
	if (!V4lconvertBuffer_imed_fid)
		V4lconvertBuffer_imed_fid = (*env)->GetFieldID(env, V4lconvertBuffer_class, "imed", "Ljava/nio/ByteBuffer");
}
static inline struct v4lconvert_buffer* lookupNative(JNIEnv* env, jobject self) {
	initFIDs(env);
	return (struct v4lconvert_buffer*) (uintptr_t) (*env)->GetLongField(env, self, V4lconvertBuffer_object_fid);
}

JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_wrapBuffer(JNIEnv* env, jobject self, jint bufId) {
	struct v4lconvert_buffer* buffer = lookupNative(env, self);
	// Buffer capacity
	u32 cap;
	// Pointer to actual buffer
	void* ptr;
	// Length of buffer
	u32 len = 0;
	switch(bufId) {
		case 0:
			cap = buffer->buf0_cap;
			ptr = buffer->buf0;
			len = buffer->buf0_len;
			break;
		case 1:
			cap = buffer->buf1_cap;
			ptr = buffer->buf1;
			len = buffer->buf1_len;
			break;
		case 2:
			cap = buffer->buf2_cap;
			ptr = buffer->buf2;
			break;
		default:
			THROW_EXCEPTION(env, INVALID_VAL_EXCP, "Buffer ID must be within range 0...2 (invalid value %d)", bufId);
			return NULL;
	}
	if (!ptr) {
		THROW_EXCEPTION(env, JNI_EXCP, "Buffer %d pointer is null (address %d, len %d, cap %d)", bufId, (unsigned int) ptr, len, cap);
		return NULL;
	}
	jobject result = (*env)->NewDirectByteBuffer(env, ptr, cap);
	if (!result) {
		if (!(*env)->ExceptionOccurred(env))
			THROW_EXCEPTION(env, JNI_EXCP, "Error instantiating direct ByteBuffer (address %d, len %d, cap %d)", (unsigned int) ptr, len, cap);
		return NULL;
	}
	return result;
}