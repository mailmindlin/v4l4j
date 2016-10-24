#ifndef C_JNIUTILS__
#define C_JNIUTILS__
#include <jni.h>
#include <stdint.h>

#include "common.h"
#include "debug.h"

inline jmethodID lookupAddMethod(JNIEnv *env, jobject list) __attribute__ ((pure));
inline jclass lookupClassSafe(JNIEnv *env, jobject obj) __attribute__ ((pure));
inline int getBufferPosition(JNIEnv *env, jobject buffer) __attribute__ ((pure));
inline int getBufferLimit(JNIEnv *env, jobject limit) __attribute__ ((pure));

inline jmethodID lookupAddMethod(JNIEnv *env, jobject list) {
	jclass listClass = (*env)->GetObjectClass(env, list);
	if (listClass == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up list class");
		return NULL;
	}
	jmethodID addMethod = (*env)->GetMethodID(env, listClass, "add", "(Ljava/lang/Object;)Z");
	(*env)->DeleteLocalRef(env, listClass);
	if (addMethod == NULL)
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up add method of list");
	return addMethod;
}

inline jclass lookupClassSafe(JNIEnv *env, jobject obj) {
	if (!obj) {
		THROW_EXCEPTION(env, NULL_EXCP, "Cannot lookup class of null");
		return NULL;
	}
	jclass result = (*env)->GetObjectClass(env, obj);
	if (!result)
		THROW_EXCEPTION(env, JNI_EXCP, "Could not get class for object");
	return result;
}

/**
 * Set the position value for a ByteBuffer. Note that this method will create <4
 * @param env The JNI environment pointer
 * @param buffer The buffer object to set the position on
 */
inline void setBufferPosition(JNIEnv *env, jobject buffer, int position) {
	jclass bufferClass = lookupClassSafe(env, buffer);
	if (!bufferClass)
		return;
	jmethodID positionSetMethodID = (*env)->GetMethodID(env, bufferClass, "position", "(I)Ljava/nio/Buffer");
	(*env)->DeleteLocalRef(env, bufferClass);
	if (!positionSetMethodID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not lookup ByteBuffer#position(I) for buffer.");
		return;
	}
	(*env)->CallObjectMethod(env, buffer, positionSetMethodID, position);
}

inline int getBufferPosition(JNIEnv *env, jobject buffer) {
	jclass bufferClass = lookupClassSafe(env, buffer);
	if (!bufferClass)
		return -1;
	jmethodID positionMethodID = (*env)->GetMethodID(env, bufferClass, "position", "()I");
	(*env)->DeleteLocalRef(env, bufferClass);
	if (!positionMethodID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not lookup ByteBuffer#position() for buffer.");
		return -1;
	}
	return (*env)->CallIntMethod(env, buffer, positionMethodID);
}

/**
 * Set the limit value for a ByteBuffer.
 * @param env The JNI environment pointer
 * @param buffer The buffer object to set the limit on
 * @param limit the value to set the limit to
 */
inline void setBufferLimit(JNIEnv *env, jobject buffer, int limit) {
	jclass bufferClass = lookupClassSafe(env, buffer);
	if (!bufferClass)
		return;
	jmethodID limitSetMethodID = (*env)->GetMethodID(env, bufferClass, "limit", "(I)Ljava/nio/Buffer");
	(*env)->DeleteLocalRef(env, bufferClass);
	if (!limitSetMethodID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not lookup ByteBuffer#limit(I) for buffer.");
		return;
	}
	(*env)->CallVoidMethod(env, buffer, limitSetMethodID, limit);
}

inline int getBufferLimit(JNIEnv *env, jobject buffer) {
	jclass bufferClass = lookupClassSafe(env, buffer);
	if (!bufferClass)
		return -1;
	jmethodID limitMethodID = (*env)->GetMethodID(env, bufferClass, "limit", "()I");
	(*env)->DeleteLocalRef(env, bufferClass);
	if (!limitMethodID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not lookup ByteBuffer#limit() for buffer.");
		return -1;
	}
	return (*env)->CallIntMethod(env, buffer, limitMethodID);
}
#endif
