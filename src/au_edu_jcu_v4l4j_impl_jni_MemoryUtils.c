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
	uint8_t test;
};
struct ShortAlignmentStruct {
	uint8_t padding;
	uint16_t test;
};
struct IntAlignmentStruct {
	uint8_t padding;
	uint32_t test;
};
struct LongAlignmentStruct {
	uint8_t padding;
	uint64_t test;
};
struct FloatAlignmentStruct {
	uint8_t padding;
	float test;
};
struct DoubleAlignmentStruct {
	uint8_t padding;
	double test;
};
struct PointerAlignmentStruct {
	uint8_t padding;
	void* test;
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
		case 5://float
			return offsetof(struct FloatAlignmentStruct, test);
		case 6://double
			return offsetof(struct DoubleAlignmentStruct, test);
		case 7://pointer
			return offsetof(struct PointerAlignmentStruct, test);
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
		case 1://byte
			return sizeof(uint8_t);
		case 2://short
			return sizeof(uint16_t);
		case 3://int
			return sizeof(uint32_t);
		case 4://long
			return sizeof(uint64_t);
		case 5://float
			return sizeof(float);
		case 6://double
			return sizeof(double);
		case 7:
			return sizeof(void*);
		case 8:
			return sizeof(struct EmptyStruct);
		case 9:
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

#ifdef __cplusplus
}
#endif
#endif