/*
* Copyright (C) 2017 mailmindlin
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
*/
#include <jni.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>//aligned_alloc, free
#include <string.h>//memset
#include <stdalign.h>//alignof

#include "common.h"
#include "debug.h"
#include "jniutils.h"

#ifndef _Included_au_edu_jcu_v4l4j_impl_jni_MemoryUtils
#define _Included_au_edu_jcu_v4l4j_impl_jni_MemoryUtils
#ifdef __cplusplus
extern "C" {
#endif

#define INFO(type) {alignof(type), sizeof(type)}

struct EmptyStruct {};
union EmptyUnion {};

/*
 * Info about c datatypes.
 * Must reflect PrimitiveStructFieldType.
 */
static int const PRIMITIVE_INFO[][2] = {
	INFO(bool),
	INFO(char),
	INFO(signed char),
	INFO(unsigned char),
	INFO(short),
	INFO(unsigned short),
	INFO(int),
	INFO(unsigned int),
	INFO(long),
	INFO(unsigned long),
	INFO(long long),
	INFO(unsigned long long),
	INFO(float),
	INFO(double),
	INFO(long double),
	INFO(void*),
	INFO(int8_t),
	INFO(uint8_t),
	INFO(int16_t),
	INFO(uint16_t),
	INFO(int32_t),
	INFO(uint32_t),
	INFO(int64_t),
	INFO(uint64_t),
	INFO(float),
	INFO(double),
	INFO(struct EmptyStruct),
	INFO(union EmptyUnion)
};

#undef INFO

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_getAlignment(JNIEnv *env, jclass me, jint type) {
	LOG_FN_ENTER();
	
	if (type > (sizeof(PRIMITIVE_INFO) / sizeof(PRIMITIVE_INFO[0])))
		return -1;
	
	return PRIMITIVE_INFO[type][0];
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_getSize(JNIEnv *env, jclass me, jint type) {
	LOG_FN_ENTER();
	
	if (type > (sizeof(PRIMITIVE_INFO) / sizeof(PRIMITIVE_INFO[0])))
		return -1;
	
	return PRIMITIVE_INFO[type][1];
}


JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_unwrap(JNIEnv *env, jclass me, jobject buffer) {
	LOG_FN_ENTER();
	
	return (uintptr_t) (*env)->GetDirectBufferAddress(env, buffer);
}

JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_doWrap(JNIEnv *env, jclass me, jlong pointer, jlong length) {
	LOG_FN_ENTER();
	
	return (*env)->NewDirectByteBuffer(env, (void*) (uintptr_t) pointer, length);
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_alloc(JNIEnv *env, jclass me, jint alignment, jlong length) {
	LOG_FN_ENTER();
	
	void* result = aligned_alloc(alignment, length);
	
	if (!result) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to allocate %lu bytes of memory with %u alignment", (unsigned long) length, alignment);
		return -1;
	}
	
	dprint(LOG_MEMALLOC, "[MEMALLOC]: allocated %lu bytes of type void* aligned %u (%p).\n", (long unsigned int)length, (unsigned int) alignment, result);
	
	return (jlong) (uintptr_t) result;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_free(JNIEnv *env, jclass me, jlong pointer) {
	LOG_FN_ENTER();
	
	void* ptr = (void*) (uintptr_t) pointer;
	dprint(LOG_MEMALLOC, "[MEMALLOC]: freeing memory @ (%p).\n", ptr);
	
	free(ptr);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_memset(JNIEnv *env, jclass me, jlong pointer, jlong length, jint value) {
	LOG_FN_ENTER();
	memset((void*) (uintptr_t) pointer, value, length);
}

#ifdef __cplusplus
}
#endif
#endif