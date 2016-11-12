#ifndef C_JNIUTILS__
#define C_JNIUTILS__
#include <jni.h>
#include <stdint.h>

#include "common.h"
#include "debug.h"

inline jmethodID lookupAddMethod(JNIEnv *env, jobject list) {
	jclass listClass = (*env)->GetObjectClass(env, list);
	if (listClass == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up list class");
		return NULL;
	}
	jmethodID addMethod = (*env)->GetMethodID(env, listClass, "add", "(Ljava/lang/Object;)Z");
	(*env)->DeleteLocalRef(env, listClass);
	if (addMethod == NULL || (*env)->ExceptionCheck(env))
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
 * Get a pointer from a buffer, whether direct or not.
 * @param env the JNI environment for this thread
 * @param buffer The buffer to get a pointer to
 * @param arrayRef If not null, will be set to NULL if a direct pointer can be recieved, else the array that is backing the returned pointer
 * @param len If not null, will be set to the length of the returned pointer, in bytes.
 */
inline unsigned char* getBufferPointer(JNIEnv *env, jobject buffer, jbyteArray* arrayRef, unsigned int* len) {
	unsigned char* result = (*env)->GetDirectBufferAddress(env, buffer);
	if (result) {
		if (arrayRef)
			*arrayRef = NULL;
		if (len)
			*len = (*env)->GetDirectBufferCapacity(env, buffer);
		return result;
	}
	if ((*env)->ExceptionCheck(env))
		(*env)->ExceptionDescribe(env);
	if (buffer == NULL) {
		THROW_EXCEPTION(env, NULL_EXCP, "Buffer was null");
		return NULL;
	}
	jclass bufferClass = (*env)->GetObjectClass(env, buffer);
	if (!bufferClass) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to get buffer class");
		return NULL;
	}
	jmethodID bufferHasArrayMID = (*env)->GetMethodID(env, bufferClass, "hasArray", "()Z");
	if (!bufferHasArrayMID) {
		(*env)->DeleteLocalRef(env, bufferClass);
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to get MID for boolean Buffer#hasArray()");
		return NULL;
	}
	jboolean hasArray = (*env)->CallBooleanMethod(env, buffer, bufferHasArrayMID);
	if ((*env)->ExceptionCheck(env) || !hasArray) {
		(*env)->DeleteLocalRef(env, bufferClass);
		return NULL;
	}
	jmethodID bufferGetArrayMID = (*env)->GetMethodID(env, bufferClass, "array", "()[B");
	(*env)->DeleteLocalRef(env, bufferClass);
	if (!bufferGetArrayMID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to get MID for Object Buffer#array()");
		return NULL;
	}
	jbyteArray array = (jbyteArray) (*env)->CallObjectMethod(env, buffer, bufferGetArrayMID);
	if (!array) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to get array from Buffer");
		return NULL;
	}
	jbyte* arrayElements = (*env)->GetByteArrayElements(env, array, NULL);
	if (!arrayElements) {
		(*env)->DeleteLocalRef(env, array);
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to get elements from array");
		return NULL;
	}
	if (len)
		*len = (*env)->GetArrayLength(env, array);
	if (arrayRef)
		*arrayRef = array;
	return (unsigned char*) arrayElements;
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
	jmethodID positionSetMethodID = (*env)->GetMethodID(env, bufferClass, "position", "(I)Ljava/nio/Buffer;");
	(*env)->DeleteLocalRef(env, bufferClass);
	if (!positionSetMethodID) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not lookup ByteBuffer#position(I) for buffer.");
		return;
	}
	jobject result = (*env)->CallObjectMethod(env, buffer, positionSetMethodID, position);
	if (result)
		(*env)->DeleteLocalRef(env, result);
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
	jmethodID limitSetMethodID = (*env)->GetMethodID(env, bufferClass, "limit", "(I)Ljava/nio/Buffer;");
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
