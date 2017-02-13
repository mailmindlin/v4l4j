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

#ifndef _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponent
#define _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponent
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	OMX_HANDLETYPE component;
	OMX_CALLBACKTYPE callbacks;
	jobject self;
} OMXComponentAppData;

//Event handlers for OMX
static OMX_ERRORTYPE event_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2, OMX_PTR eventData) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	
}


static OMX_ERRORTYPE empty_buffer_done_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
}

static OMX_ERRORTYPE fill_buffer_done_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
}

static inline OMXComponentAppData* initAppData(JNIEnv *env, jobject self) {
	OMXComponentAppData* appData;
	XMALLOC(appData, OMXComponentAppData*, sizeof(OMXComponentAppData));
	if (appData == NULL) {
		//TODO throw exception
		return NULL;
	}
	
	//Get a global ref to the self object, so it's guaranteed to remain valid after we
	//return to Java code
	jobject globalSelf = (*env)->NewGlobalRef(env, self);
	if (globalSelf == NULL) {
		//TODO throw exception
		XFREE(appData);
		return NULL;
	}
	
	appData->self = globalSelf;
	memset(&appData->callbacks, 0, sizeof(OMX_CALLBACKTYPE));
	appData->callbacks.EventHandler = &event_handler;
	appData->callbacks.EmptyBufferDone = &empty_buffer_done_handler;
	appData->callbacks.FillBufferDone = &fill_buffer_done_handler;
	
	return appData;
}

static inline void deinitAppData(JNIEnv* env, OMXComponentAppData* appData) {
	(*env)->DeleteGlobalRef(env, appData->self);
	XFREE(appData);
}

/*
 * Class:     au_edu_jcu_v4l4j_impl_omx_OMXComponent
 * Method:    getComponentHandle
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_getComponentHandle(JNIEnv *env, jobject self, jstring componentNameStr) {
	LOG_FN_ENTER();
	
	//Get string into native memory
	const jchar* componentName = (*env)->GetStringChars(env, componentNameStr);
	if (componentName == NULL) {
		//TODO throw exception
		return -1;
	}
	
	//Set up appData (context used for all the callbacks)
	OMXComponentAppData* appData = initAppData(env, self);
	if (appData == NULL) {
		(*env)->ReleaseStringChars(env, componentNameStr, componentName);
		//TODO throw exception
		return -1;
	}
	
	OMX_RESULTTYPE res = OMX_GetHandle(&appData->component, componentName, appData, &appData->callbacks);
	
	(*env)->ReleaseStringChars(env, componentNameStr, componentName);
	
	if (res != OMX_ErrorNone) {
		deinitAppData(appData);
		//TODO throw exception
		return -1;
	}
	
	return (jlong) (uintptr_t) appData;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_getPortOffsets(JNIEnv *env, jclass me, jlong pointer, jintArray resultArr) {
	LOG_FN_ENTER();
	
	jint* result = (*env)->GetIntArrayElements(env, resultArr, NULL);
	if (result == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Could not access result array elements");
		return;
	}
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_PORT_PARAM_TYPE ports;
	OMX_INIT_STRUCTURE(ports);
	
	OMX_INDEXTYPE types[] = {
		OMX_IndexParamAudioInit,
		OMX_IndexParamVideoInit,
		OMX_IndexParamImageInit,
		OMX_IndexParamOtherInit
	};
	
	for (unsigned int i = 0; i < 4; i++) {
		if (OMX_GetParameter(*appData->component, types[i], &ports) == OMX_ErrorNone) {
			result[i * 2] = ports.nStartPortNumber;
			result[i * 2 + 1] = ports.nPorts;
		}
	}
	
	(*env)->ReleaseIntArrayElements(env, resultArr, result, JNI_COMMIT);
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_getComponentState(JNIEnv *env, jclass me, jlong pointer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_STATETYPE state;
	OMX_RESULTTYPE r = appData->component->GetState(appData->component, state);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when querying state: %08x", r);
		return -1;
	}
	return (jint) state;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_setComponentState(JNIEnv *env, jclass me, jlong pointer, jint state) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_RESULTTYPE r = appData->component->SendCommand(appData->component, OMX_CommandStateSet, (OMX_STATETYPE) state, NULL);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when setting state: %08x", r);
		return;
	}
}

#ifdef __cplusplus
}
#endif
#endif//Close include guard