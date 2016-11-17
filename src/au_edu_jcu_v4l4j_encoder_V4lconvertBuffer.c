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
#include "jniutils.h"
#include "libvideo.h"
#include "types.h"
#include "libv4lconvert/libv4lconvert-flat.h"

#ifndef _Included_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer
#define _Included_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer
#ifdef __cplusplus
extern "C" {
#endif

static inline jclass getOwnClass(JNIEnv* env) __attribute__((pure));
static inline struct v4lconvert_buffer* lookupNative(JNIEnv* env, jobject self) __attribute__ ((pure));
static inline jclass getOwnClass(JNIEnv* env) {
	jclass result = (*env)->FindClass(env, "au/edu/jcu/v4l4j/encoder/V4lconvertBuffer");
	if (!result)
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup class au.edu.jcu.v4l4j.encoder.V4lconvertBuffer");
	return result;
}

static inline struct v4lconvert_buffer* lookupNative(JNIEnv* env, jobject self) {
	jclass V4lconvertBuffer_class = getOwnClass(env);
	if (!V4lconvertBuffer_class)
		return NULL;
	jfieldID V4lconvertBuffer_object_fid = (*env)->GetFieldID(env, V4lconvertBuffer_class, "object", "J");
	struct v4lconvert_buffer* result = (struct v4lconvert_buffer*) (uintptr_t) (*env)->GetLongField(env, self, V4lconvertBuffer_object_fid);
	return result;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_V4lconvertBuffer
 * Method:    allocate
 * Signature: (IIIZZ)J
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_allocate(JNIEnv *env, jclass clazz, jint srcLen, jint dstLen, jint imedLen, jboolean allocSrc, jboolean allocDst) {
	LOG_FN_ENTER();
	struct v4lconvert_buffer* buffer;
	XMALLOC(buffer, struct v4lconvert_buffer*, sizeof(struct v4lconvert_buffer));
	if (!buffer) {
		//TODO throw exception
		return -1;
	}
	buffer->buf0_cap = srcLen;
	buffer->buf1_cap = dstLen;
	buffer->buf2_cap = imedLen;
	if (allocSrc)
		XCALLOC(buffer->buf0, u8*, sizeof(u8), srcLen);
	if (allocDst)
		XCALLOC(buffer->buf1, u8*, sizeof(u8), dstLen);
	if (imedLen > 0)
		XCALLOC(buffer->buf2, u8*, sizeof(u8), imedLen);
	//TODO error handling if bad allocation
	return (jlong) (uintptr_t) buffer;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_wrap(JNIEnv* env, jclass clazz, jobject src, jobject dst, jobject imed) {
	LOG_FN_ENTER();
	struct v4lconvert_buffer* buffer;
	XMALLOC(buffer, struct v4lconvert_buffer*, sizeof(struct v4lconvert_buffer));
	if (!buffer) {
		THROW_EXCEPTION(env, INIT_EXCP, "Error allocating v4lconvert_buffer");
		return -1;
	}
	
	if (src) {
		if (!(buffer->buf0 = (*env)->GetDirectBufferAddress(env, src))) {
			THROW_EXCEPTION(env, INVALID_VAL_EXCP, "src is not a direct buffer");
			XFREE(buffer);
			return -1;
		}
		buffer->buf0_cap = (*env)->GetDirectBufferCapacity(env, src);
		buffer->buf0_len = getBufferPosition(env, src);
	} else {
		buffer->buf0 = NULL;
		buffer->buf0_cap = 0;
		buffer->buf0_len = 0;
	}
	
	if (dst) {
		if (!(buffer->buf1 = (*env)->GetDirectBufferAddress(env, dst))) {
			THROW_EXCEPTION(env, INVALID_VAL_EXCP, "dst is not a direct buffer");
			XFREE(buffer);
			return -1;
		}
		buffer->buf1_cap = (*env)->GetDirectBufferCapacity(env, dst);
		buffer->buf1_len = getBufferPosition(env, dst);
	} else {
		buffer->buf1 = NULL;
		buffer->buf1_cap = 0;
		buffer->buf1_len = 0;
	}
	
	if (imed) {
		if (!(buffer->buf2 = (*env)->GetDirectBufferAddress(env, imed))) {
			THROW_EXCEPTION(env, INVALID_VAL_EXCP, "imed is not a direct buffer");
			XFREE(buffer);
			return -1;
		}
		buffer->buf2_cap = (*env)->GetDirectBufferCapacity(env, src);
		//No buf2_len
	} else {
		buffer->buf2 = NULL;
		buffer->buf2_cap = 0;
	}
	
	//Return a pointer to the struct
	return (jlong) (uintptr_t) buffer;
}
JNIEXPORT jboolean JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_setSource(JNIEnv* env, jobject self, jobject src) {
	LOG_FN_ENTER();
	jclass clazz = getOwnClass(env);
	if (!clazz)
		//Exception already thrown
		return FALSE;
	
	//Check if the current src is bound
	jfieldID srcBoundID = (*env)->GetFieldID(env, clazz, "srcBound", "Z");
	if (!srcBoundID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup srcBound FID from V4lconvertBuffer");
		return FALSE;
	}
	
	//Check whether the source is bound
	if (!(*env)->GetBooleanField(env, self, srcBoundID))
		return FALSE;
	
	void* ptr = (*env)->GetDirectBufferAddress(env, src);
	if (!ptr) {
		//Buffer passed isn't direct
		//Don't throw an exception, just return false
		return FALSE;
	}
	
	struct v4lconvert_buffer* buffer = lookupNative(env, self);
	if (!buffer) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup native struct");
		return FALSE;
	}
	
	buffer->buf0 = ptr;
	buffer->buf0_cap = (*env)->GetDirectBufferCapacity(env, src);
	buffer->buf0_len = getBufferLimit(env, src);
	return TRUE;
}
JNIEXPORT jboolean JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_setOutput(JNIEnv* env, jobject self, jobject dst) {
	LOG_FN_ENTER();
	jclass clazz = getOwnClass(env);
	if (!clazz)
		//Exception already thrown
		return FALSE;
	
	//Check if the current dst is bound
	jfieldID dstBoundID = (*env)->GetFieldID(env, clazz, "dstBound", "Z");
	if (!dstBoundID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup dstBound FID from V4lconvertBuffer");
		return FALSE;
	}
	
	//Check whether the output is bound
	if (!(*env)->GetBooleanField(env, self, dstBoundID))
		return FALSE;
	
	void* ptr = (*env)->GetDirectBufferAddress(env, dst);
	if (!ptr) {
		//Buffer passed isn't direct
		//Don't throw an exception, just return false
		return FALSE;
	}
	
	struct v4lconvert_buffer* buffer = lookupNative(env, self);
	if (!buffer) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup native struct");
		return FALSE;
	}
	
	buffer->buf1 = ptr;
	buffer->buf1_cap = (*env)->GetDirectBufferCapacity(env, dst);
	buffer->buf1_len = getBufferLimit(env, dst);
	return TRUE;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_createBuffers(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	jclass V4lconvertBuffer_class = getOwnClass(env);
	if (!V4lconvertBuffer_class)
		return;
	
	struct v4lconvert_buffer* buffer = lookupNative(env, self);
	if (!buffer)
		return;
	
	if (buffer->buf0) {
		jfieldID srcFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "src", "Ljava/nio/ByteBuffer;");
		if (!srcFID) {
			THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for V4lconvertBuffer#src");
			return;
		}
		jobject src = (*env)->NewDirectByteBuffer(env, buffer->buf0, buffer->buf0_cap);
		setBufferLimit(env, src, buffer->buf0_len);
		(*env)->SetObjectField(env, self, srcFID, src);
		(*env)->DeleteLocalRef(env, src);
	}
	if (buffer->buf1) {
		jfieldID dstFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "dst", "Ljava/nio/ByteBuffer;");
		if (!dstFID) {
			THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for V4lconvertBuffer#dst");
			return;
		}
		jobject dst = (*env)->NewDirectByteBuffer(env, buffer->buf1, buffer->buf1_cap);
		setBufferLimit(env, dst, buffer->buf1_len);
		(*env)->SetObjectField(env, self, dstFID, dst);
		(*env)->DeleteLocalRef(env, dst);
	}
	if (buffer->buf2) {
		jfieldID imedFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "imed", "Ljava/nio/ByteBuffer;");
		if (!imedFID) {
			THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for V4lconvertBuffer#imed");
			return;
		}
		jobject imed = (*env)->NewDirectByteBuffer(env, buffer->buf2, buffer->buf2_cap);
		setBufferLimit(env, imed, buffer->buf2_cap);
		(*env)->SetObjectField(env, self, imedFID, imed);
		(*env)->DeleteLocalRef(env, imed);
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_pull(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	jclass V4lconvertBuffer_class = getOwnClass(env);
	if (!V4lconvertBuffer_class)
		return;
	
	struct v4lconvert_buffer* buffer = lookupNative(env, self);
	if (!buffer)
		return;
	
	jfieldID srcFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "src", "Ljava/nio/ByteBuffer;");
	if (!srcFID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for V4lconvertBuffer#src");
		return;
	}
	jobject src = (*env)->GetObjectField(env, self, srcFID);
	setBufferPosition(env, src, 0);
	setBufferLimit(env, src, buffer->buf0_len);
	if ((*env)->ExceptionCheck(env))
		return;
	(*env)->DeleteLocalRef(env, src);
	
	jfieldID dstFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "dst", "Ljava/nio/ByteBuffer;");
	if (!dstFID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for V4lconvertBuffer#dst");
		return;
	}
	jobject dst = (*env)->GetObjectField(env, self, dstFID);
	setBufferPosition(env, dst, buffer->buf0_len);
	setBufferLimit(env, dst, buffer->buf1_len);
	//No point in cleaning up or handling exceptions here
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_push(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	jclass V4lconvertBuffer_class = getOwnClass(env);
	if (!V4lconvertBuffer_class)
		return;
	
	struct v4lconvert_buffer* buffer = lookupNative(env, self);
	if (!buffer)
		return;
	
	jfieldID srcFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "src", "Ljava/nio/ByteBuffer;");
	if (!srcFID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for V4lconvertBuffer#src");
		return;
	}
	jobject src = (*env)->GetObjectField(env, self, srcFID);
	if (!src) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to access field V4lconvertBuffer#src");
		return;
	}
	{
		int len = getBufferPosition(env, src);
		if (len < 0 || (*env)->ExceptionCheck(env))
			return;
		buffer->buf0_len = len;
	}
	(*env)->DeleteLocalRef(env, src);
	//TODO check whether it makes sense to clean up the local references here (profile)
	
	jfieldID dstFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "dst", "Ljava/nio/ByteBuffer;");
	if (!dstFID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for V4lconvertBuffer#dst");
		return;
	}
	jobject dst = (*env)->GetObjectField(env, self, dstFID);
	if (!dst) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to access field V4lconvertBuffer#dst");
		return;
	}
	{
		int len = getBufferPosition(env, dst);
		if (len < 0 || (*env)->ExceptionCheck(env))
			return;
		buffer->buf1_len = len;
	}
	//No point in cleaning up/handling exceptions b/c we are exiting anyways
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_V4lconvertBuffer_release(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	jclass V4lconvertBuffer_class = getOwnClass(env);
	if (!V4lconvertBuffer_class)
		return;
	
	struct v4lconvert_buffer* buffer = lookupNative(env, self);
	//Fail silently if the buffer is null, because close() can be called multiple times without side effects
	if (buffer) {
		//Only release src/dst if they are bound to this buffer
		jfieldID srcBoundFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "srcBound", "Z");
		if (!srcBoundFID) {
			THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for boolean V4lconvertBuffer#srcBound");
			return;
		}
		if ((*env)->GetBooleanField(env, self, srcBoundFID)) {
			XFREE(buffer->buf0);
			buffer->buf0 = NULL;
			buffer->buf0_cap = 0;
			buffer->buf0_len = 0;
		}
		
		jfieldID dstBoundFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "dstBound", "Z");
		if (!dstBoundFID) {
			THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for boolean V4lconvertBuffer#dstBound");
			return;
		}
		
		if ((*env)->GetBooleanField(env, self, dstBoundFID)) {
			XFREE(buffer->buf1);
			buffer->buf1 = NULL;
			buffer->buf1_cap = 0;
			buffer->buf1_len = 0;
		}
		
		XFREE(buffer->buf2);
		buffer->buf2 = NULL;
		buffer->buf2_cap = 0;
		
		XFREE(buffer);
	}
	
	// Set the ByteBuffers to 0
	jfieldID srcFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "src", "Ljava/nio/ByteBuffer;");
	if (!srcFID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for ByteBuffer V4lconvertBuffer#src");
		return;
	}
	(*env)->SetObjectField(env, self, srcFID, NULL);
	
	jfieldID dstFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "dst", "Ljava/nio/ByteBuffer;");
	if (!dstFID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for ByteBuffer V4lconvertBuffer#dst");
		return;
	}
	(*env)->SetObjectField(env, self, dstFID, NULL);
	
	jfieldID imedFID = (*env)->GetFieldID(env, V4lconvertBuffer_class, "imed", "Ljava/nio/ByteBuffer;");
	if (!imedFID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to lookup FID for ByteBuffer V4lconvertBuffer#imed");
		return;
	}
	(*env)->SetObjectField(env, self, imedFID, NULL);
}
#endif