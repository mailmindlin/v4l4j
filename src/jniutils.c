#ifndef C_JNIUTILS__
#define C_JNIUTILS__
#include "common.h"
#include "debug.h"

inline jmethodID lookupAddMethod(JNIEnv *env, jobject list) {
	jclass listClass = (*env)->GetObjectClass(env, list);
	if (listClass == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up list class");
		return NULL;
	}
	jmethodID addMethod = (*env)->GetMethodID(env, listClass, "add", "(Ljava/lang/Object;)Z");
	if (addMethod == NULL)
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up add method of list");
	return addMethod;
}

#endif
