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
#include <IL/OMX_Component.h>
#include <IL/OMX_Video.h>
#include <IL/OMX_Broadcom.h>

#ifndef _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponent
#define _Included_au_edu_jcu_v4l4j_impl_omx_OMXComponent
#ifdef __cplusplus
extern "C" {
#endif

// Stole from github.com/tjormola/rpi-openmax-demos
#define OMX_INIT_STRUCTURE(a) \
    memset(&(a), 0, sizeof(a)); \
    (a).nSize = sizeof(a); \
    (a).nVersion.nVersion = OMX_VERSION; \
    (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
    (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
    (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
    (a).nVersion.s.nStep = OMX_VERSION_STEP

typedef struct {
	OMX_HANDLETYPE component;
	OMX_CALLBACKTYPE callbacks;
	jobject self;
} OMXComponentAppData;

//Event handlers for OMX
static OMX_ERRORTYPE event_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2, OMX_PTR eventData) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	(void)appData;
	
	#ifdef DEBUG
		char* eventName = "[unknown]";
		switch (event) {
			case OMX_EventCmdComplete:
				eventName = "CmdComplete";
				break;
			case OMX_EventError:
				eventName = "Error";
				break;
			case OMX_EventMark:
				eventName = "Mark";
				break;
			case OMX_EventPortSettingsChanged:
				eventName = "PortSettingsChanged";
				break;
			case OMX_EventBufferFlag:
				eventName = "BufferFlag";
				break;
			case OMX_EventResourcesAcquired:
				eventName = "ResourcesAcquired";
				break;
			case OMX_EventComponentResumed:
				eventName = "ComponentResumed";
				break;
			case OMX_EventDynamicResourcesAvailable:
				eventName = "DynamicResourcesAvailable";
				break;
			case OMX_EventPortFormatDetected:
				eventName = "PortFormatDetected";
				break;
			case OMX_EventParamOrConfigChanged:
				eventName = "ParamOrConfigChanged";
				break;
		}
		dprint(LOG_V4L4J, "OMX event %#08x %s: hComponent: %#08x; data1: %#08x; data2: %#08x\n", event, eventName, hComponent, data1, data2);
	#endif
	return OMX_ErrorNone;
}


static OMX_ERRORTYPE empty_buffer_done_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	(void)appData;
	
	dprint(LOG_V4L4J, "OMX empty buffer done: hComponent: %#08x; buffer: %#08x\n", hComponent, pBuffer);
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE fill_buffer_done_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	(void)appData;
	dprint(LOG_V4L4J, "OMX fill buffer done: hComponent: %#08x; buffer: %#08x\n", hComponent, pBuffer);
	return OMX_ErrorNone;
}

static inline OMXComponentAppData* initAppData(JNIEnv *env, jobject self) {
	OMXComponentAppData* appData;
	XMALLOC(appData, OMXComponentAppData*, sizeof(OMXComponentAppData));
	if (appData == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Failed to allocate OMX app data");
		return NULL;
	}
	
	//Get a global ref to the self object, so it's guaranteed to remain valid after we
	//return to Java code
	jobject globalSelf = (*env)->NewGlobalRef(env, self);
	if (globalSelf == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Failed to get global pointer to self");
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
	const jchar* componentName = (*env)->GetStringChars(env, componentNameStr, NULL);
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
	
	OMX_ERRORTYPE res = OMX_GetHandle(&appData->component, componentName, appData, &appData->callbacks);
	
	(*env)->ReleaseStringChars(env, componentNameStr, componentName);
	
	if (res != OMX_ErrorNone) {
		deinitAppData(env, appData);
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
		if (OMX_GetParameter(appData->component, types[i], &ports) == OMX_ErrorNone) {
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
	OMX_ERRORTYPE r = OMX_GetState(appData->component, &state);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when querying state: %08x", r);
		return -1;
	}
	return (jint) state;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_setComponentState(JNIEnv *env, jclass me, jlong pointer, jint state) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_ERRORTYPE r = OMX_SendCommand(appData->component, OMX_CommandStateSet, (OMX_STATETYPE) state, NULL);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when setting state: %08x", r);
		return;
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_enablePort(JNIEnv *env, jclass me, jlong pointer, jint index, jboolean enabled) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	
	OMX_ERRORTYPE r = OMX_SendCommand(appData->component, enabled ? OMX_CommandPortEnable : OMX_CommandPortDisable, index, NULL);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when %s port %d: %08x", enabled ? "enabling" : "disabling", index, r);
		return;
	}
}

JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doAllocateBuffer(JNIEnv *env, jclass me, jlong pointer, jint portIndex, jint size) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	//Look these up early, so we don't have much to clean up if they fail
	jclass framebufferClass = (*env)->FindClass(env, "au/edu/jcu/v4l4j/impl/omx/OMXFrameBuffer");
	if (framebufferClass == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up class OMXFrameBuffer");
		return NULL;
	}
	
	jmethodID framebufferCtor = (*env)->GetMethodID(env, framebufferClass, "<init>", "(JLjava/nio/ByteBuffer;)V");
	if (framebufferCtor == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up constructor OMXFrameBuffer(long, ByteBuffer)");
		return NULL;
	}
	
	OMX_BUFFERHEADERTYPE* buffer;
	//Actually allocate the buffer
	OMX_ERRORTYPE r = OMX_AllocateBuffer(appData->component, &buffer, portIndex, NULL, size);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error allocating buffer on port %d: %#08x", portIndex, r);
		return NULL;
	}
	
	if (buffer->nAllocLen != size) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "Buffer allocated does not match size (expected %u; actual %u)", size, buffer->nAllocLen);
		return NULL;
	}
	
	jobject bbuffer = (*env)->NewDirectByteBuffer(env, buffer, size);
	//Wrap with FrameBuffer
	jobject framebuffer = (*env)->NewObject(env, framebufferClass, framebufferCtor, buffer, bbuffer);
	return framebuffer;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doEmptyThisBuffer(JNIEnv *env, jclass me, jlong pointer, jlong bufferPointer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_BUFFERHEADERTYPE* buffer = (OMX_BUFFERHEADERTYPE*) (uintptr_t) bufferPointer;
	
	OMX_ERRORTYPE r = OMX_EmptyThisBuffer(appData->component, buffer);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error emptying buffer: %#08x", r);
		return;
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doFillThisBuffer(JNIEnv *env, jclass me, jlong pointer, jlong bufferPointer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_BUFFERHEADERTYPE* buffer = (OMX_BUFFERHEADERTYPE*) (uintptr_t) bufferPointer;
	
	OMX_ERRORTYPE r = OMX_FillThisBuffer(appData->component, buffer);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error emptying buffer: %#08x", r);
		return;
	}
}


#ifdef __cplusplus
}
#endif
#endif//Close include guard