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
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
} OMXStructureHeader;

typedef struct {
	OMX_HANDLETYPE component;
	OMX_CALLBACKTYPE callbacks;
	JavaVM *vm;
	jobject self;
} OMXComponentAppData;

//OMX error descriptions
static char* getOMXErrorDescription(OMX_ERRORTYPE err) {
	switch(err) {
		case OMX_ErrorNone:
			return "no error";
		case OMX_ErrorInsufficientResources:
			return "insufficient resources";
		case OMX_ErrorUndefined:
			return "unknown";
		case OMX_ErrorInvalidComponentName:
			return "invalid component name";
		case OMX_ErrorComponentNotFound:
			return "component not found";
		case OMX_ErrorInvalidComponent:
			return "invalid component";
		case OMX_ErrorBadParameter:
			return "bad parameter";
		case OMX_ErrorNotImplemented:
			return "not implemented";
		case OMX_ErrorUnderflow:
			return "underflow";
		case OMX_ErrorOverflow:
			return "overflow";
		case OMX_ErrorHardware:
			return "hardware error";
		case OMX_ErrorInvalidState:
			return "invalid state";
		case OMX_ErrorStreamCorrupt:
			return "stream corrupt";
		case OMX_ErrorPortsNotCompatible:
			return "ports not compatible";
		case OMX_ErrorResourcesLost:
			return "resources lost";
		case OMX_ErrorNoMore:
			return "no more";
		case OMX_ErrorVersionMismatch:
			return "version mismatch";
		case OMX_ErrorNotReady:
			return "not ready";
		case OMX_ErrorTimeout:
			return "timeout";
		case OMX_ErrorSameState:
			return "same state";
		case OMX_ErrorResourcesPreempted:
			return "resources preempted";
		case OMX_ErrorPortUnresponsiveDuringAllocation:
			return "port unresponsive during allocation";
		case OMX_ErrorPortUnresponsiveDuringDeallocation:
			return "port unresponsive during deallocation";
		case OMX_ErrorPortUnresponsiveDuringStop:
			return "port unresponsive during stop";
		case OMX_ErrorIncorrectStateTransition:
			return "unallowed state transition";
		case OMX_ErrorIncorrectStateOperation:
			return "invalid state while trying to perform command";
		case OMX_ErrorUnsupportedSetting:
			return "unsupported setting";
		case OMX_ErrorUnsupportedIndex:
			return "unsupported index";
		case OMX_ErrorBadPortIndex:
			return "bad port index";
		case OMX_ErrorPortUnpopulated:
			return "port unpopulated";
		case OMX_ErrorComponentSuspended:
			return "component suspended";
		case OMX_ErrorDynamicResourcesUnavailable:
			return "dynamic resources unavailable";
		case OMX_ErrorMbErrorsInFrame:
			return "macroblock errors in frame";
		case OMX_ErrorFormatNotDetected:
			return "format not detectd";
		case OMX_ErrorContentPipeOpenFailed:
			return "content pipe open failed";
		case OMX_ErrorContentPipeCreationFailed:
			return "content pipe creation failed";
		case OMX_ErrorSeperateTablesUsed:
			return "seperate tabled used";
		case OMX_ErrorTunnelingUnsupported:
			return "tunneling unsupported";
		case OMX_ErrorDiskFull:
			return "disk full";
		case OMX_ErrorMaxFileSize:
			return "max file size";
		case OMX_ErrorDrmUnauthorised:
			return "DRM unauthorized";
		case OMX_ErrorDrmExpired:
			return "DRM expired";
		case OMX_ErrorDrmGeneral:
			return "DRM general";
		default:
			return "(unknown)";
	}
}

//Event handlers for OMX
static OMX_ERRORTYPE event_handler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2, OMX_PTR eventData) {
	OMXComponentAppData* appData = (OMXComponentAppData*) pAppData;
	(void)appData;
	
	#ifdef DEBUG
		char* eventName;
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
	} else if (getEnvResult == JNI_EDETATCHED) {
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

static jclass OMXComponent_class = NULL;
static jmethodID OMXComponent_onBufferDone = NULL;

static jmethodID getJavaCallbackMethod(JNIEnv* env, jobject object, jclass* classRefOut) {
	jclass classRef = (*env)->NewLocalRef(env, OMXComponent_class);
	if (classRef == NULL) {
		classRef = (*env)->GetObjectClass(env, object);
		if (classRef == NULL || (*env)->ExceptionCheck(env)) {
			info(env, JNI_EXCP, "Unable to lookup class OMXComponent\n");
			return NULL;
		}
		OMXComponent_class = (*env)->NewWeakGlobalRef(env, classRef);
		OMXComponent_onBufferDone = (*env)->GetMethodID(env, classRef, "onBufferDone", "(JZIJIII)V");
		if (OMXComponent_onBufferDone == NULL) {
			info("Unable to find OMXComponent.onBufferDone(JZIJIII)V\n");
			return NULL;
		}
	}
	*classRefOut = classRef;
	return OMXComponent_onBufferDone;	
}

static OMX_ERRORTYPE handleBufferEvent(OMX_HANDLETYPE hComponent, OMXComponentAppData* appData, OMX_BUFFERHEADERTYPE* pBuffer, jboolean empty) {
	LOG_FN_ENTER();
	//The problem that we have is that we don't know if this callback is on the same thread
	//as any spawned by Java. As such, we can't just pass a JNIEnv to let us call our callback;
	//instead, we have to work at it.
	JNIEnv* env;
	JavaVM* vm = appData->vm;
	bool attached = (*vm)->getJNIEnv(vm, &env);
	if (env == NULL)
		return OMX_ErrorUndefined;//Getting env failed
	
	//Get local reference to java OMXCompoenent
	jobject componentRef = (*env)->NewLocalRef(env, env->self);
	if (componentRef == NULL) {
		info("Component ref was null\n");
		if (attached)
			(*vm)->DetachCurrentThread(vm);
		return OMX_ErrorUndefined;
	}
	
	//Look up the callback method (OMXComponent.onBufferDone)
	jclass omxClassRef;
	jmethodID callbackMethod = getJavaCallbackMethod(env, componentRef, &omxClassRef);
	if (callbackMethod == NULL) {
		info("Failed to get callback method\n");
		(*env)->DeleteLocalRef(env, componentRef);
		if (attached)
			(*vm)->DetachCurrentThread(vm);
		return OMX_ErrorUndefined;
	}
	
	(*env)->CallVoidMethod(env, componentRef,
			(jlong) (uintptr_t) pBuffer, empty,
			(jint) pBuffer->nTickCount,
			(jlong) pBuffer->nTimeStamp,
			(jint) pBuffer->nOffset,
			(jint) pBuffer->nFilledLen,
			(jint) pBuffer->nAllocLen);
	
	//Free references , and return (I am not sure if it will just go away, because I don't understand the semantics of all the Invocation API methods.
	if (omxClassRef != NULL)
		(*env)->DeleteLocalRef(env, omxClassRef);
	(*env)->DeleteLocalRef(env, componentRef);
	if (attached)
		(*vm)->DetachCurrentThread(vm);
	return OMX_ErrorNone;
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
		XFREE(appData);
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

static void printBytes(char* bytes, int len) {
	char* hexChars = "0123456789ABCDEF";
	while (len--) {
		char c = *bytes++;
		char a = hexChars[c >> 4];
		char b = hexChars[c & 0x0F];
		dprint(LOG_V4L4J, "%c%c\n", a, b);
	}
}

/*
 * Class:     au_edu_jcu_v4l4j_impl_omx_OMXComponent
 * Method:    getComponentHandle
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_getComponentHandle(JNIEnv *env, jobject self, jstring componentNameStr) {
	LOG_FN_ENTER();
	
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
		(*env)->ReleaseStringUTFChars(env, componentNameStr, componentName);
		THROW_EXCEPTION(env, JNI_EXCP, "Error allocating appdata");
		return -1;
	}
	
	OMX_ERRORTYPE res = OMX_GetHandle(&(appData->component), (char*) componentName, appData, &appData->callbacks);
	
	(*env)->ReleaseStringUTFChars(env, componentNameStr, componentName);
	
	dprint(LOG_V4L4J, "Struct length: %d; value %#08x\n", ((OMX_COMPONENTTYPE*)appData->component)->nSize, (uintptr_t)appData->component);
	printBytes((char*) appData, sizeof(OMXComponentAppData));
	
	if (res != OMX_ErrorNone) {
		THROW_EXCEPTION(env, JNI_EXCP, "OMX Failure: %#08x %s", res, getOMXErrorDescription(res));
		deinitAppData(env, appData);
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
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when querying state: %08x %s", r, getOMXErrorDescription(r));
		return -1;
	}
	return (jint) state;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_setComponentState(JNIEnv *env, jclass me, jlong pointer, jint state) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_ERRORTYPE r = OMX_SendCommand(appData->component, OMX_CommandStateSet, (OMX_STATETYPE) state, NULL);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when setting state: %#08x %s", r, getOMXErrorDescription(r));
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
		//TODO replace with custom OMX exception
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX: Error when getting definition for port %d: %#08x %s", portIndex, res, getOMXErrorDescription(res));
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
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error when %s port %d: %08x %s", enabled ? "enabling" : "disabling", index, r, getOMXErrorDescription(r));
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
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error allocating buffer on port %d: %#08x %s", portIndex, r, getOMXErrorDescription(r));
		return NULL;
	}
	
	if (buffer->nAllocLen != size) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "Buffer allocated does not match size (expected %u; actual %u)", size, buffer->nAllocLen);
		return NULL;
	}
	
	jobject bbuffer = (*env)->NewDirectByteBuffer(env, buffer->pBuffer, size);
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
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error emptying buffer: %#08x %s", r, getOMXErrorDescription(r));
		return;
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doFillThisBuffer(JNIEnv *env, jclass me, jlong pointer, jlong bufferPointer) {
	LOG_FN_ENTER();
	
	OMXComponentAppData* appData = (OMXComponentAppData*) (uintptr_t) pointer;
	
	OMX_BUFFERHEADERTYPE* buffer = (OMX_BUFFERHEADERTYPE*) (uintptr_t) bufferPointer;
	
	OMX_ERRORTYPE r = OMX_FillThisBuffer(appData->component, buffer);
	if (r != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error filling buffer: %#08x %s", r, getOMXErrorDescription(r));
		return;
	}
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_impl_omx_OMXComponent_doAccessConfig(JNIEnv* env, jclass me, jlong pointer, jboolean isConfig, jboolean read, jint configIdx, jobject data) {
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
		return;
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
	if (res != OMX_ErrorNone) {
		THROW_EXCEPTION(env, GENERIC_EXCP, "OMX Error %s %s %#08x: %#08x %s",
			read ? "reading" : "writing",
			isConfig ? "config" : "parameter",
			configIdx,
			res,
			getOMXErrorDescription(res));
		return;
	}
}


#ifdef __cplusplus
}
#endif
#endif//Close include guard
