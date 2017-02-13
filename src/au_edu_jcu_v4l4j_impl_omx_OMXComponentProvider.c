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

#include <IL/OMX_Core.h>

#ifndef _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider
#define _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider
 * Method:    enumComponents
 * Signature: (Ljava/lang/List;I)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider_enumComponents(JNIEnv *env, jclass me, jobject list, jint startIndex) {
	LOG_FN_ENTER();
	
	jmethodID listAddMethod = lookupAddMethod(env, list);
	if (listAddMethod == NULL)
		return -1;//Exception already thrown
	
	unsigned int index = startIndex < 0 ? 0 : (unsigned) startIndex;
	OMX_ERRORTYPE res;
	char componentName[128];//TODO should we allocate this on the heap?
	for (; (res = OMX_ComponentNameEnum(componentName, sizeof(componentName), index)) == OMX_ErrorNone; index++) {
		//Wrap the name in a java string and add it to the list
		jstring componentNameStr = (*env)->NewStringUTF(env, componentName);
		(*env)->CallBooleanMethod(env, list, listAddMethod, componentNameStr);
		(*env)->DeleteLocalRef(env, componentNameStr);
	}
	
	//We *should* exit the loop with OMX_ErrorNoMore if we were successful
	if (res != OMX_ErrorNoMore) {
		//TODO throw exception
	}
	
	return (jint) index;
}


JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider_getComponentsByRole(JNIEnv *env, jclass me, jobject result, jstring role, jint maxLen) {
	LOG_FN_ENTER();
	
	jmethodID setAddMethod = lookupAddMethod(env, result);
	if (setAddMethod == NULL)
		return NULL;//Exception already thrown
	
	//TODO finish (not yet implemented)
	return result;
}

#ifdef __cplusplus
}
#endif
#endif//Close include guard