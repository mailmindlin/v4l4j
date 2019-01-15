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
#include "omx_common.h"

#include <IL/OMX_Core.h>

#ifndef _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider
#define _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider
#ifdef __cplusplus
extern "C" {
#endif

static OMXMethods *omx_methods;

/*
 * Class:     au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider
 * Method:    enumComponents
 * Signature: (Ljava/lang/List;I)I
 */
JNIEXPORT jboolean JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider_init(JNIEnv *env, jclass me) {
	LOG_FN_ENTER();
	omx_methods = v4lconvert_omx_init();
	return omx_methods ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider
 * Method:    enumComponents
 * Signature: (Ljava/lang/List;I)I
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider_deinit(JNIEnv *env, jclass me) {
	LOG_FN_ENTER();
	v4lconvert_omx_deinit(omx_methods);
}


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
	
	unsigned index = startIndex < 0 ? 0 : (unsigned) startIndex;
	OMX_ERRORTYPE res;
	char componentName[OMX_NAME_LEN];//TODO should we allocate this on the heap?
	for (; (res = (*omx_methods->componentNameEnum)(componentName, sizeof(componentName), index)) == OMX_ErrorNone; index++) {
		dprint(LOG_V4L4J, "OMX: Found component #%-3u| %s\n", index, componentName);
		//Wrap the name in a java string and add it to the list
		jstring componentNameStr = (*env)->NewStringUTF(env, componentName);
		(*env)->CallBooleanMethod(env, list, listAddMethod, componentNameStr);
		(*env)->DeleteLocalRef(env, componentNameStr);
	}
	
	//We *should* exit the loop with OMX_ErrorNoMore if we were successful
	if (res != OMX_ErrorNoMore) {
		THROW_OMX_EXCP(env, res, "Error enumerating component");
		return index;
	}
	
	dprint(LOG_V4L4J, "OMX: Discovered %u components\n", index);
	
	return (jint) index;
}


JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider_getComponentsByRole(JNIEnv *env, jclass me, jobject result, jstring role) {
	LOG_FN_ENTER();
	
	jmethodID setAddMethod = lookupAddMethod(env, result);
	if (setAddMethod == NULL)
		return NULL;//Exception already thrown
	
	const char *roleName = (*env)->GetStringUTFChars(env, role, NULL);

	if ((roleName == NULL) || (*env)->ExceptionCheck(env))
		goto cleanup1;

	// Query to figure out how many components have the role
	OMX_U32 numComps = 0;
	OMX_ERRORTYPE res = (*omx_methods->getComponentsOfRole)(roleName, &numComps, NULL);
	if (res != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, res, "OMX Error querying number of components in role %s", roleName);
		goto cleanup1;
	}
	
	dprint(LOG_V4L4J, "OMX: Discovered %u components for role %s\n", numComps, roleName);
	if (numComps == 0)
		return result;

	// Allocate compNames array (numComps x 128)
	//TODO: out-of-memory check?
	OMX_U8 **compNames = NULL;
	XCALLOC(compNames, OMX_U8**, sizeof(OMX_U8*), numComps);
	for (unsigned i = 0; i < numComps; i++)
		XCALLOC(compNames[i], OMX_U8*, sizeof(OMX_U8), OMX_NAME_LEN);
	
	// Populate compNames
	OMX_U32 numCompsActual = numComps;
	res = (*omx_methods->getComponentsOfRole)(roleName, &numCompsActual, compNames);
	if (res != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, res, "OMX Error querying names of %u components in role %s",
			numComps,
			roleName);
		goto cleanup2;
	}

	// Add all names to set
	for (unsigned i = 0; i < numCompsActual; i++) {
		jstring componentNameStr = (*env)->NewStringUTF(env, compNames[i]);
		(*env)->CallBooleanMethod(env, result, setAddMethod, componentNameStr);
		(*env)->DeleteLocalRef(env, componentNameStr);
		//TODO: should we be polling exceptionCheck?
	}

cleanup2:
	// Release compNames
	for (unsigned i = 0; i < numComps; i++)
		XFREE(compNames[i]);
	XFREE(compNames);
cleanup1:
	// Release roleName
	(*env)->ReleaseStringUTFChars(env, role, roleName);
	return result;
}

#ifdef __cplusplus
}
#endif
#endif//Close include guard