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
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponentProvider_getComponentHandle(JNIEnv *env, jobject self, jstring componentNameStr) {
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

#ifdef __cplusplus
}
#endif
#endif//Close include guard