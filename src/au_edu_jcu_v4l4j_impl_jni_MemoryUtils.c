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

#include "common.h"
#include "debug.h"
#include "jniutils.h"

#ifndef _Included_au_edu_jcu_v4l4j_impl_jni_MemoryUtils
#define _Included_au_edu_jcu_v4l4j_impl_jni_MemoryUtils
#ifdef __cplusplus
extern "C" {
#endif


struct BoolAlignmentStruct {
	uint8_t padding;
	bool test;
};

struct CharAlignmentStruct {
	uint8_t padding;
	char test;
};

struct ShortAlignmentStruct {
	uint8_t padding;
	short test;
};

struct IntAlignmentStruct {
	uint8_t padding;
	int test;
};

struct LongAlignmentStruct {
	uint8_t padding;
	long test;
};

struct LongLongAlignmentStruct {
	uint8_t padding;
	long long test;
};

struct FloatAlignmentStruct {
	uint8_t padding;
	float test;
};

struct DoubleAlignmentStruct {
	uint8_t padding;
	double test;
};

struct LongDoubleAlignmentStruct {
	uint8_t padding;
	long double test;
};

struct PointerAlignmentStruct {
	uint8_t padding;
	void* test;
};

struct Int8AlignmentStruct {
	uint8_t padding;
	uint8_t test;
};

struct Int16AlignmentStruct {
	uint8_t padding;
	uint16_t test;
};

struct Int32AlignmentStruct {
	uint8_t padding;
	uint32_t test;
};

struct Int64AlignmentStruct {
	uint8_t padding;
	uint64_t test;
};

struct F32AlignmentStruct {
	uint8_t padding;
	float test;
};

struct F64AlignmentStruct {
	uint8_t padding;
	double test;
};

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_getAlignment(JNIEnv *env, jclass me, jint type) {
	LOG_FN_ENTER();
	
	switch (type) {
		case 0://Bool
			return offsetof(struct BoolAlignmentStruct, test);
		case 1://char
			return offsetof(struct CharAlignmentStruct, test);
		case 2://short
			return offsetof(struct ShortAlignmentStruct, test);
		case 3://int
			return offsetof(struct IntAlignmentStruct, test);
		case 4://long
			return offsetof(struct LongAlignmentStruct, test);
		case 5://long long
			return offsetof(struct LongAlignmentStruct, test);
		case 6://float
			return offsetof(struct FloatAlignmentStruct, test);
		case 7://double
			return offsetof(struct DoubleAlignmentStruct, test);
		case 8://long double
			return offsetof(struct LongDoubleAlignmentStruct, test);
		case 9://pointer (void*)
			return offsetof(struct PointerAlignmentStruct, test);
		case 10://int8
			return offsetof(struct Int8AlignmentStruct, test);
		case 11://int16
			return offsetof(struct Int16AlignmentStruct, test);
		case 12://int32
			return offsetof(struct Int32AlignmentStruct, test);
		case 13://int64
			return offsetof(struct Int64AlignmentStruct, test);
		case 14://float32
			return offsetof(struct Float32AlignmentStruct, test);
		case 15://float64
			return offsetof(struct Float64AlignmentStruct, test);
		default:
			return -1;
	}	
}


struct EmptyStruct {};
union EmptyUnion {};

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_impl_jni_MemoryUtils_getSize(JNIEnv *env, jclass me, jint type) {
	LOG_FN_ENTER();
	
	switch (type) {
		case 0://Bool
			return sizeof(bool);
		case 1://char
			return sizeof(char);
		case 2://short
			return sizeof(short);
		case 3://int
			return sizeof(int);
		case 4://long
			return sizeof(long);
		case 5://long long
			return sizeof(long);
		case 6://float
			return sizeof(float);
		case 7://double
			return sizeof(double);
		case 8://long double
			return sizeof(long double);
		case 9://pointer (void*)
			return sizeof(void*);
		case 10://int8
			return sizeof(uint8_t);
		case 11://int16
			return sizeof(uint16_t);
		case 12://int32
			return sizeof(uint32_t);
		case 13://int64
			return sizeof(uint64_t);
		case 14://float32
			return sizeof(float);
		case 15://float64
			return sizeof(double);
		case 16://struct
			return sizeof(struct EmptyStruct);
		case 17://union
			return sizeof(union EmptyUnion);
		default:
			return -1;
	}
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