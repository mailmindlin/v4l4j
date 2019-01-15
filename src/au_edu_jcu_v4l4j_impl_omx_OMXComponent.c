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

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Video.h>
#include <IL/OMX_Broadcom.h>

#include "common.h"
#include "debug.h"
#include "jniutils.h"
#include "omx_common.h"

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
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
} OMXStructureHeader;

typedef struct {
	/**
	 * Pointer to 
	 */
	OMX_HANDLETYPE component;
	/**
	 * 
	 */
	OMX_CALLBACKTYPE callbacks;
	/**
	 * Pointer to JVM, for use in obtaining a JNI environment pointer from native callbacks
	 */
	JavaVM *vm;
	/**
	 * (Global) reference to java object
	 */
	jobject self;
} OMXComponentAppData;

//Event handlers for OMX
static OMX_ERRORTYPE event_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2, OMX_PTR eventData) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	(void)appData;
	
	#ifdef DEBUG
		const char* eventName;
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
			default:
				eventName = "[unknown]";
		}
		dprint(LOG_V4L4J, "OMX event %#08x %s: hComponent: %#08x; data1: %#08x; data2: %#08x\n", event, eventName, (uintptr_t) hComponent, data1, data2);
	#endif
	return OMX_ErrorNone;
}

static bool getJNIEnv(JavaVM* vm, JNIEnv** env) {
	//Thanks to https://coderanch.com/t/274379/java/JNIEnv-valid-call-javaVM-GetEnv
	int getEnvResult = (*vm)->GetEnv(vm, (void**)env, JNI_VERSION_1_8);
	if (getEnvResult == JNI_OK) {
		return false;
	} else if (getEnvResult == JNI_EDETACHED) {
		int attachResult = (*vm)->AttachCurrentThread(vm, (void**)env, NULL);
		if (attachResult == 0)
			return true;
		info("Could not get JNI environment for OMX callback (error %d/%d)\n", getEnvResult, attachResult);
		*env = NULL;
		return false;
	} else {
		info("Could not get JNI environment for OMX callback (error %d)\n", getEnvResult);
		*env = NULL;
		return false;
	}
}

//References to JNI stuff
static jclass OMXComponent_class = NULL;
static jmethodID OMXComponent_onBufferDone = NULL;

static jmethodID getJavaCallbackMethod(JNIEnv* env, jobject object, jclass* classRefOut) {
	jclass classRef = (*env)->NewLocalRef(env, OMXComponent_class);
	if (classRef == NULL) {
		classRef = (*env)->GetObjectClass(env, object);
		if (classRef == NULL || (*env)->ExceptionCheck(env)) {
			info("Unable to lookup class OMXComponent\n");
			return NULL;
		}
		OMXComponent_class = (*env)->NewWeakGlobalRef(env, classRef);
		OMXComponent_onBufferDone = (*env)->GetMethodID(env, classRef, "onBufferDone", "(Lau/edu/jcu/v4l4j/impl/omx/OMXFrameBuffer;ZIJIII)V");
		if (OMXComponent_onBufferDone == NULL) {
			info("Unable to find OMXComponent.onBufferDone(Lau/edu/jcu/v4l4j/impl/omx/OMXFrameBuffer;ZIJIII)V\n");
			return NULL;
		}
	}
	*classRefOut = classRef;
	return OMXComponent_onBufferDone;	
}

static OMX_ERRORTYPE handleBufferEvent(OMX_HANDLETYPE hComponent, OMXComponentAppData* appData, OMX_BUFFERHEADERTYPE* pBuffer, jboolean empty) {
	LOG_FN_ENTER();

	OMX_ERRORTYPE result = OMX_ErrorNone;

	//The problem that we have is that we don't know if this callback is on the same thread
	//as any spawned by Java. As such, we can't just pass a JNIEnv to let us call our callback;
	//instead, we have to work at it.
	JNIEnv* env;
	JavaVM* vm = appData->vm;
	bool attached = getJNIEnv(vm, &env);
	if (env == NULL)
		return OMX_ErrorUndefined;//Getting env failed
	
	//Get local reference to java OMXCompoenent
	jobject componentRef = (*env)->NewLocalRef(env, appData->self);
	if (componentRef == NULL) {
		info("Component ref was null\n");
		result = OMX_ErrorUndefined;
		goto cleanup1;
	}
	
	//Look up the callback method (OMXComponent.onBufferDone)
	jclass omxClassRef;
	jmethodID callbackMethod = getJavaCallbackMethod(env, componentRef, &omxClassRef);
	if (callbackMethod == NULL) {
		info("Failed to get callback method\n");
		result = OMX_ErrorUndefined;
		goto cleanup2;
	}
	
	union tickConv {
		OMX_TICKS ticks;
		int64_t val;
	};
	
	(*env)->CallVoidMethod(env, componentRef, callbackMethod,
			(jobject) pBuffer->pAppPrivate,
			empty,
			(jint) pBuffer->nTickCount,
			(jlong) ((union tickConv)pBuffer->nTimeStamp).val,
			(jint) pBuffer->nOffset,
			(jint) pBuffer->nFilledLen,
			(jint) pBuffer->nAllocLen);
	
	//Free references , and return (I am not sure if it will just go away, because I don't understand the semantics of all the Invocation API methods).
	if (omxClassRef != NULL)
		(*env)->DeleteLocalRef(env, omxClassRef);
cleanup2:
	(*env)->DeleteLocalRef(env, componentRef);
cleanup1:
	// Release attached thread
	if (attached)
		(*vm)->DetachCurrentThread(vm);
	return result;
}

static OMX_ERRORTYPE empty_buffer_done_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer) {
	dprint(LOG_V4L4J, "OMX empty buffer done: hComponent: %#08x; buffer: %#08x\n", (uintptr_t) hComponent, (uintptr_t) pBuffer);
	
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	return handleBufferEvent(hComponent, appData, pBuffer, JNI_TRUE);
}

static OMX_ERRORTYPE fill_buffer_done_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer) {
	dprint(LOG_V4L4J, "OMX fill buffer done: hComponent: %#08x; buffer: %#08x\n", (uintptr_t) hComponent, (uintptr_t) pBuffer);
	
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	return handleBufferEvent(hComponent, appData, pBuffer, JNI_FALSE);
}

static inline OMXComponentAppData* initAppData(JNIEnv *env, jobject self) {
	OMXComponentAppData* appData;
	XMALLOC(appData, OMXComponentAppData*, sizeof(OMXComponentAppData));
	if (appData == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Failed to allocate OMX app data");
		return NULL;
	}
	
	//Get a pointer to the JVM, for callbacks
	int getVmResult = (*env)->GetJavaVM(env, &appData->vm);
	//TODO figure out how to release JVM pointers
	if (getVmResult != 0) {
		THROW_EXCEPTION(env, JNI_EXCP, "Failed to get reference to JVM (error %d)", getVmResult);
		goto fail;
	}
	
	//Get a global ref to the self object, so it's guaranteed to remain valid after we
	//return to Java code
	jobject globalSelf = (*env)->NewGlobalRef(env, self);
	if (globalSelf == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Failed to get global pointer to self");
		goto fail;
	}
	
	appData->self = globalSelf;
	memset(&appData->callbacks, 0, sizeof(OMX_CALLBACKTYPE));
	appData->callbacks.EventHandler = &event_handler;
	appData->callbacks.EmptyBufferDone = &empty_buffer_done_handler;
	appData->callbacks.FillBufferDone = &fill_buffer_done_handler;
	
	return appData;

fail:
	XFREE(appData);
	return NULL;
}

static inline void deinitAppData(JNIEnv* env, OMXComponentAppData* appData) {
	(*env)->DeleteGlobalRef(env, appData->self);
	XFREE(appData);
}

static void printBytes(char* bytes, int len) {
	char* hexChars = "0123456789ABCDEF";
	char line[25];
	size_t rowOffset = 0;
	while (len--) {
		char c = *bytes++;
		line[rowOffset++] = hexChars[c >> 4];
		line[rowOffset++] = hexChars[c & 0x0F];
		line[rowOffset++] = ' ';
		if (rowOffset == 12) {
			line[rowOffset++] = ' ';
		} else if (rowOffset >= 24) {
			dprint(LOG_V4L4J, "%.*s\n", rowOffset - 1, line);
			rowOffset = 0;
		}
	}
	if (rowOffset > 0)
		dprint(LOG_V4L4J, "%.*s\n", rowOffset - 1, line);
}

/*
 * Class:     au_edu_jcu_v4l4j_impl_omx_OMXComponent
 * Method:    getComponentHandle
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_getComponentHandle(JNIEnv *env, jobject self, jstring componentNameStr) {
	LOG_FN_ENTER();

	//Get OMX method handles
	OMXMethods *omx_methods = v4lconvert_get_omx();
	if (omx_methods == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error getting OMX methods");
		return -1;
	}
	
	//Get string into native memory
	const char* componentName = (*env)->GetStringUTFChars(env, componentNameStr, NULL);
	if (componentName == NULL) {
		THROW_EXCEPTION(env, NULL_EXCP, "Error getting string chars");
		return -1;
	}
	dprint(LOG_V4L4J, "Getting handle for component '%s'\n", componentName);
	
	//Set up appData (context used for all the callbacks)
	OMXComponentAppData* appData = initAppData(env, self);
	if (appData == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error allocating appdata");
		goto cleanup;
	}
	
	OMX_ERRORTYPE res = (*omx_methods->getHandle)(&(appData->component), (char*) componentName, appData, &appData->callbacks);
	
	
	dprint(LOG_V4L4J, "Struct length: %d; value %#08x\n", ((OMX_COMPONENTTYPE*)appData->component)->nSize, (uintptr_t)appData->component);
	printBytes((char*) appData, sizeof(OMXComponentAppData));
	
	if (res != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, res, "OMX Failure getting component '%s' handle", componentName);
		deinitAppData(env, appData);
		goto cleanup;
	}

cleanup:
	(*env)->ReleaseStringUTFChars(env, componentNameStr, componentName);
	
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
		THROW_OMX_EXCP(env, r, "OMX Error when querying state");
		return -1;
	}

	return (jint) state;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_setComponentState(JNIEnv *env, jclass me, jlong pointer, jint state) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_ERRORTYPE r = OMX_SendCommand(appData->component, OMX_CommandStateSet, (OMX_STATETYPE) state, NULL);
	if (r != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, r, "OMX Error when setting state");
		return;
	}
}

JNIEXPORT jstring JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_getPortInfo(JNIEnv *env, jclass me, jlong pointer, jint portIndex, jintArray resultArr) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	//Initialize the query
	OMX_PARAM_PORTDEFINITIONTYPE portdef;
	OMX_INIT_STRUCTURE(portdef);
	portdef.nPortIndex = portIndex;
	char mimeType[128];//MIME type pointer (we have to allocate the memory)
	memset(mimeType, 0x00, sizeof(mimeType));
	portdef.format.audio.cMIMEType = mimeType;
	
	//Query the port
	dprint(LOG_V4L4J, "Querying definition of port %d\n", portIndex);
	OMX_ERRORTYPE res = OMX_GetParameter(appData->component, OMX_IndexParamPortDefinition, &portdef);
	if (res != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, res, "OMX: Error when getting definition for port %d", portIndex);
		return NULL;
	}
	
	//Sanity check that the port that we asked for is, in fact, the port that we asked for
	if (portdef.nPortIndex != portIndex) {
		THROW_EXCEPTION(env, INVALID_VAL_EXCP, "OMX: Port index changed from %d to %d", portIndex, portdef.nPortIndex);
		return NULL;
	}
	
	/*
	 * Because JNI calls are relatively slow, this method puts all the values into an int array
	 * that was passed from Java, reducing an upwards of 19 JNI calls + OMX queries (if every parameter
	 * had its own getter method) to just one, at the cost of some readibility.
	 */
	int result[19];
	int i = 0;//Entry #0 is going to be set at the end
	
	//Entries 1-9 are common for all port types
	result[++i] = portdef.eDir == OMX_DirInput ? 1 : 0;
	result[++i] = (int) portdef.nBufferCountActual;
	result[++i] = (int) portdef.nBufferCountMin;
	result[++i] = (int) portdef.nBufferSize;
	result[++i] = portdef.bEnabled != OMX_FALSE ? 1 : 0;
	result[++i] = portdef.bPopulated != OMX_FALSE ? 1 : 0;
	result[++i] = (int) portdef.eDomain;
	result[++i] = portdef.bBuffersContiguous != OMX_FALSE ? 1 : 0;
	result[++i] = (int) portdef.nBufferAlignment;
	//i == 9
	
	//Get port-type-specific parameters
	switch (portdef.eDomain) {
		case OMX_PortDomainAudio:
			//mimeType = portdef.format.audio.cMIMEType;
			result[++i] = portdef.format.audio.bFlagErrorConcealment != OMX_FALSE ? 1 : 0;
			result[++i] = (int) portdef.format.audio.eEncoding;
			//i == 11
			break;
		case OMX_PortDomainVideo:
			//mimeType = portdef.format.video.cMIMEType;
			result[++i] = portdef.format.video.bFlagErrorConcealment != OMX_FALSE ? 1 : 0;
			//These parameters are the same as for an image port
			result[++i] = (int) portdef.format.video.nFrameWidth;
			result[++i] = (int) portdef.format.video.nFrameHeight;
			result[++i] = (int) portdef.format.video.nStride;
			result[++i] = (int) portdef.format.video.nSliceHeight;
			result[++i] = (int) portdef.format.video.eColorFormat;
			//Video port only values
			result[++i] = (int) portdef.format.video.eCompressionFormat;
			result[++i] = (int) portdef.format.video.nBitrate;
			result[++i] = (int) portdef.format.video.xFramerate;
			//i == 18
			break;
		case OMX_PortDomainImage:
			//mimeType = portdef.format.image.cMIMEType;
			result[++i] = portdef.format.image.bFlagErrorConcealment != OMX_FALSE ? 1 : 0;
			result[++i] = (int) portdef.format.image.nFrameWidth;
			result[++i] = (int) portdef.format.image.nFrameHeight;
			result[++i] = (int) portdef.format.image.nStride;
			result[++i] = (int) portdef.format.image.nSliceHeight;
			result[++i] = (int) portdef.format.image.eColorFormat;
			//Image port only values
			result[++i] = (int) portdef.format.image.eCompressionFormat;
			//i == 16
			break;
		case OMX_PortDomainOther:
			result[++i] = (int) portdef.format.other.eFormat;
			//i == 10
			break;
		default:
			break;
	}
	
	//dprint(LOG_V4L4J, "Port MIME: '%s'\n", mimeType);
	
	//Store the length in the first cell
	const int resultLen = i + 1;
	result[0] = resultLen;
	
	//Store data into result array
	
	//Check that the passed array is big enough
	const int resultActualLen = (*env)->GetArrayLength(env, resultArr);
	if (resultActualLen < resultLen) {
		THROW_EXCEPTION(env, ARG_EXCP, "Array length too short (expect %d; actual %d)", resultLen, resultActualLen);
		return NULL;
	}
	
	//Put our values into the array
	(*env)->SetIntArrayRegion(env, resultArr, 0, i + 1, result);
	
	//Return the MIME type (if available)
	jstring mimeTypeStr = NULL;
	if (mimeType[0] != '\0') {
		if ((mimeTypeStr = (*env)->NewStringUTF(env, mimeType)) == NULL) {
			THROW_EXCEPTION(env, JNI_EXCP, "Could not wrap MIME string (%s)", mimeType);
			return NULL;
		}
	}
	
	return mimeTypeStr;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_getPortFormats(JNIEnv *env, jclass me, jlong pointer, jint portIndex, jobject resultList) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	jmethodID listAddMethod = lookupAddMethod(env, resultList);
	if (listAddMethod == NULL)
		return;//Exception already thrown
	
	//Look these up early, so we don't have much to clean up if they fail
	jclass resultTypeClass = (*env)->FindClass(env, "au/edu/jcu/v4l4j/impl/omx/OMXVideoFormatOption");
	if (resultTypeClass == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up class OMXVideoFormatOption");
		return;
	}
	
	jmethodID resultTypeCtor = (*env)->GetMethodID(env, resultTypeClass, "<init>", "(III)V");
	if (resultTypeCtor == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up constructor OMXVideoFormatOption(int, int, int)");
		return;
	}
	
	OMX_VIDEO_PARAM_PORTFORMATTYPE portFormat;
	OMX_INIT_STRUCTURE(portFormat);
	portFormat.nPortIndex = portIndex;
	portFormat.nIndex = 0;
	
	OMX_ERRORTYPE res;
	while ((res = OMX_GetParameter(appData->component, OMX_IndexParamVideoPortFormat, &portFormat)) == OMX_ErrorNone) {
		OMX_VIDEO_CODINGTYPE compression = portFormat.eCompressionFormat;
		OMX_COLOR_FORMATTYPE format = portFormat.eColorFormat;
		unsigned int frameRate = portFormat.xFramerate;
		dprint(LOG_V4L4J, "OMX: Found format %d compression %d framerate %d\n", compression, format, frameRate);
		
		
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_enablePort(JNIEnv *env, jclass me, jlong pointer, jint index, jboolean enabled) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	
	OMX_ERRORTYPE r = OMX_SendCommand(appData->component, enabled ? OMX_CommandPortEnable : OMX_CommandPortDisable, index, NULL);
	if (r != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, r, "OMX Error when %s port %d", enabled ? "enabling" : "disabling", index);
		return;
	}
}

JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doUseBuffer(JNIEnv* env, jclass me, jlong pointer, jint portIndex, jboolean allocate, jint size, jobject bBuffer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	//Look these up early, so we don't have much to clean up if they fail
	jclass framebufferClass = (*env)->FindClass(env, "au/edu/jcu/v4l4j/impl/omx/OMXFrameBuffer");
	if (framebufferClass == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up class OMXFrameBuffer");
		return NULL;
	}
	
	jmethodID framebufferCtor = (*env)->GetMethodID(env, framebufferClass, "<init>", "(JLjava/nio/ByteBuffer;II)V");
	if (framebufferCtor == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up constructor OMXFrameBuffer(long, ByteBuffer)");
		return NULL;
	}
	
	//We want to use the framebuffer jobject in the appPrivate field of the BufferHeader,
	//but the constructor requires the framebuffer pointer, so we allocate it now, and invoke
	//the constructor as a method after getting the BufferHeader.
	jobject framebuffer = (*env)->AllocObject(env, framebufferClass);
	if (framebuffer == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error allocating framebuffer");
		return NULL;
	}
	
	//Make a global reference to our allocated framebuffer to store in the native object
	jobject framebufferRef = (*env)->NewGlobalRef(env, framebuffer);
	if (framebufferRef == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error getting global ref to framebuffer");
		return NULL;
	}
	
	OMX_BUFFERHEADERTYPE* buffer;
	OMX_ERRORTYPE r;
	if (allocate) {
		r = OMX_AllocateBuffer(appData->component, &buffer, portIndex, framebufferRef, size);
		//Wrap allocated buffer with ByteBuffer
		if (r == OMX_ErrorNone)
			bBuffer = (*env)->NewDirectByteBuffer(env, buffer->pBuffer, size);
	} else {
		//Actually allocate the buffer
		unsigned char* bufferPointer = (*env)->GetDirectBufferAddress(env, bBuffer);
		if (bufferPointer == NULL) {
			THROW_EXCEPTION(env, JNI_EXCP, "Error getting pointer to direct buffer");
			goto fail;
		}
		r = OMX_UseBuffer(appData->component, &buffer, portIndex, framebufferRef, size, bufferPointer);
	}
		
	if (r != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, r, "OMX Error %s buffer on port %d", allocate ? "allocating" : "attatching", portIndex);
		goto fail;
	}
	
	if (buffer->nAllocLen != size) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "Buffer allocated does not match size (expected %u; actual %u)", size, buffer->nAllocLen);
		goto fail;
	}
	
	//Invoke constructor
	(*env)->CallVoidMethod(env, framebuffer, framebufferCtor, (jlong) (uintptr_t) buffer, bBuffer, buffer->nInputPortIndex, buffer->nOutputPortIndex);
	return framebuffer;

fail:
	(*env)->DeleteGlobalRef(env, framebufferRef);
	return NULL;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doReleaseBuffer(JNIEnv* env, jclass me, jlong pointer, jint portIndex, jlong bufferPointer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_BUFFERHEADERTYPE* buffer = (OMX_BUFFERHEADERTYPE*) (uintptr_t) bufferPointer;
	
	OMX_ERRORTYPE r = OMX_FreeBuffer(appData->component, portIndex, buffer);
	
	if (r != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, r, "OMX Error allocating buffer on port %d", portIndex);
		return;
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doEmptyThisBuffer(JNIEnv *env, jclass me, jlong pointer, jlong bufferPointer, jint position, jint size, jint sequence, jlong timestamp) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_BUFFERHEADERTYPE* buffer = (OMX_BUFFERHEADERTYPE*) (uintptr_t) bufferPointer;
	
	dprint(LOG_V4L4J, "Setting offset to %d; limit %d; sequence %d; timestamp %llu;\n", position, size, sequence, (unsigned long long) timestamp);
	buffer->nOffset = position;
	buffer->nFilledLen = size; 
	buffer->nTickCount = sequence;
	//Because OMX_TICKS could either be a union or an int64, we get it to compile
	//by casting our int64 to this union.
	union tickConv {
		OMX_TICKS ticks;
		int64_t val;
	};
	buffer->nTimeStamp = ((union tickConv)timestamp).ticks;
	
	OMX_ERRORTYPE r = OMX_EmptyThisBuffer(appData->component, buffer);
	if (r != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, r, "OMX Error emptying buffer");
		return;
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doFillThisBuffer(JNIEnv *env, jclass me, jlong pointer, jlong bufferPointer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_BUFFERHEADERTYPE* buffer = (OMX_BUFFERHEADERTYPE*) (uintptr_t) bufferPointer;
	
	OMX_ERRORTYPE r = OMX_FillThisBuffer(appData->component, buffer);
	if (r != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, r, "OMX Error filling buffer");
		return;
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doFlushPort(JNIEnv *env, jclass me, jlong pointer, jint portIndex) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_ERRORTYPE r = OMX_SendCommand(appData->component, OMX_CommandFlush, portIndex, NULL);
	if (r != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, r, "OMX Error flushing port %u", (unsigned int) portIndex);
		return;
	}
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doAccessConfig(JNIEnv* env, jclass me, jlong pointer, jboolean isConfig, jboolean read, jboolean throwOnError, jint configIdx, jobject data) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	OMX_COMPONENTTYPE* component = appData->component;
	
	jbyteArray arrayRef = NULL;
	unsigned int arrayLength = 0;
	unsigned int arrayOffset = 0;
	void (*releaseArray)(JNIEnv* env, jbyteArray arrayRef, unsigned char* ptr);
	unsigned char* buf = getBufferPointer(env, data, &arrayRef, &arrayOffset, &arrayLength, &releaseArray);
	
	//Check that we have a valid reference to buf
	if (!buf) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error getting pointer to buffer");
		return -1;
	}
	
	dprint(LOG_V4L4J, "OMX: %s %s %#010x\n", read ? "Reading" : "Writing", isConfig ? "config" : "parameter", configIdx);
	
	//Get pointer to actual data structure
	//arrayOffset can be nonzero for a few reasons, including alignment, so
	//deal with that.
	void* dataStruct = (void*) (&buf[arrayOffset]);
	
	{
		//Fill out OMX query header
		OMXStructureHeader* queryHeader = (OMXStructureHeader*) dataStruct;
		queryHeader->nSize = arrayLength - arrayOffset;
		queryHeader->nVersion.nVersion = OMX_VERSION;
		queryHeader->nVersion.s.nVersionMajor = OMX_VERSION_MAJOR;
		queryHeader->nVersion.s.nVersionMinor = OMX_VERSION_MINOR;
		queryHeader->nVersion.s.nRevision = OMX_VERSION_REVISION;
		queryHeader->nVersion.s.nStep = OMX_VERSION_STEP;
	}
	
	//Actually query stuff here
	OMX_ERRORTYPE res;
	if (isConfig) {
		if (read)
			res = component->GetConfig(component, configIdx, dataStruct);
		else
			res = component->SetConfig(component, configIdx, dataStruct);
	} else {
		if (read)
			res = component->GetParameter(component, configIdx, dataStruct);
		else
			res = component->SetParameter(component, configIdx, dataStruct);
	}
	
	//Release native memory
	releaseArray(env, arrayRef, buf);
	
	//Handle errors, if any
	if (res != OMX_ErrorNone && throwOnError) {
		THROW_OMX_EXCP(env, res, "OMX Error %s %s %#08x",
			read ? "reading" : "writing",
			isConfig ? "config" : "parameter",
			configIdx);
	}
	return res;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_freeComponentHandle(JNIEnv* env, jclass me, jlong pointer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;

	//Get OMX method handles
	OMXMethods *omx_methods = v4lconvert_get_omx();
	if (omx_methods == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error getting OMX methods");
		return;
	}
	
	OMX_ERRORTYPE res = (*omx_methods->freeHandle)(appData->component);
	
	deinitAppData(env, appData);
	
	if (res != OMX_ErrorNone) {
		THROW_OMX_EXCP(env, res, "OMX: Failed to free component handle");
		return;
	}
}

#ifdef __cplusplus
}
#endif
#endif//Close include guard
