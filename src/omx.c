#include <stdbool.h>

#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Video.h>

#include "xenolib.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dunno where this is originally stolen from... got it from https://github.com/tjormola/rpi-openmax-demos/blob/master/rpi-encode-yuv.c
#define OMX_INIT_STRUCTURE(a) \
    memset(&(a), 0, sizeof(a)); \
    (a).nSize = sizeof(a); \
    (a).nVersion.nVersion = OMX_VERSION; \
    (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
    (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
    (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
    (a).nVersion.s.nStep = OMX_VERSION_STEP


typedef enum libstat_t {
	STATUS_UNLOADED = 0,
	STATUS_BOUND,
	STATUS_LOADED,
	STATUS_ACTIVE,
	STATUS_NOT_FOUND,
	STATUS_ERROR,
} libstat_t;


// libbcm_host wrapper stuff
typedef struct BCMMethods {
	void (*host_init) (void);
	void (*host_deinit) (void);
} BCMMethods;


static void bcm_load();
static void bcm_bind();
static void bcm_init();
static void bcm_deinit();
static void bcm_unload();

static void omxil_load();
static void omxil_bind();
static void omxil_init();
static void omxil_deinit();
static void omxil_unload();


static libptr_t libbcm_host;
static libstat_t libbcm_status = STATUS_UNLOADED;
static BCMMethods libbcm_methods;

static libptr_t libomxil;
static libstat_t libomxil_status = STATUS_UNLOADED;
static OMXMethods libomxil_methods;


//OMX error descriptions
const char *getOMXErrorDescription(OMX_ERRORTYPE err) {
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
			return "bad port index, i.e. incorrect port";
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


#define ASSERT_STATUS(var, val) \
	do { \
		if ((var) != (val)) \
			return; \
	} while (0)


void bcm_load() {
	ASSERT_STATUS(libbcm_status, STATUS_UNLOADED);
	
	if (libbcm_host = xenolib_open("libbcm_host"))
		libbcm_status = STATUS_LOADED;
	else
		libbcm_status = STATUS_NOT_FOUND;
}

void omxil_load() {
	ASSERT_STATUS(libomxil_status, STATUS_UNLOADED);
	
	if (libomxil = xenolib_open("libopenmaxil"))
		libomxil_status = STATUS_LOADED;
	else
		libomxil_status = STATUS_NOT_FOUND;
}


#define LOOKUP_FN(lib, dst, sym) \
	if (!((dst) = xenolib_sym(lib, sym))) { \
		printf("Error looking up symbol: " sym "\n"); \
		goto fail; \
	}

void bcm_bind() {
	ASSERT_STATUS(libbcm_status, STATUS_LOADED);
	
	LOOKUP_FN(libbcm_host, libbcm_methods.init,   "bcm_host_init");
	LOOKUP_FN(libbcm_host, libbcm_methods.deinit, "bcm_host_deinit");

	libbcm_status = STATUS_BOUND;
	return;

fail:
	bcm_unload();
	return;
}

void omxil_bind() {
	ASSERT_STATUS(libomxil_status, STATUS_LOADED);
	
	LOOKUP_FN(libomxil, libomxil_methods.init,                "OMX_Init");
	LOOKUP_FN(libomxil, libomxil_methods.deinit,              "OMX_Deinit");
	LOOKUP_FN(libomxil, libomxil_methods.componentNameEnum,   "OMX_ComponentNameEnum");
	LOOKUP_FN(libomxil, libomxil_methods.getHandle,           "OMX_GetHandle");
	LOOKUP_FN(libomxil, libomxil_methods.freeHandle,          "OMX_FreeHandle");
	LOOKUP_FN(libomxil, libomxil_methods.setupTunnel,         "OMX_SetupTunnel");
	LOOKUP_FN(libomxil, libomxil_methods.getContentPipe,      "OMX_GetContentPipe");
	LOOKUP_FN(libomxil, libomxil_methods.getComponentsOfRole, "OMX_GetComponentsOfRole");
	LOOKUP_FN(libomxil, libomxil_methods.getRolesOfComponent, "OMX_GetRolesOfComponent");

	libomxil_status = STATUS_BOUND;
	return;

fail:
	omxil_unload();
	return;
}

#undef LOOKUP_FN


void bcm_init() {
	ASSERT_STATUS(libbcm_status, STATUS_BOUND);
	
	libbcm_methods.init();
	libbcm_status = STATUS_ACTIVE;
}

void omxil_init() {
	ASSERT_STATUS(libomxil_status, STATUS_BOUND);

	OMX_ERRORTYPE r = libbcm_methods.init();
	if (r == OMX_ErrorNone) {
		libomxil_status = STATUS_ACTIVE;
	} else {
		fprintf(stderr, "OMX: Error initializing OpenMAX IL: %#08x %s\n", r, getOMXErrorDescription(r));
		libomxil_status = STATUS_ERROR;
	}
}


void bcm_deinit() {
	ASSERT_STATUS(libbcm_status, STATUS_ACTIVE);

	libbcm_methods.deinit();
	libbcm_status = STATUS_BOUND;
}

void omxil_deinit() {
	ASSERT_STATUS(libomxil_status, STATUS_ACTIVE);

	OMX_ERRORTYPE r = libomxil_methods.deinit();
	if (r == OMX_ErrorNone) {
		libomxil_status = STATUS_LOADED;
	} else {
		fprintf(stderr, "OMX: Error deinitializing OpenMAX IL: %#08x %s\n", r, getOMXErrorDescription(r));
		libomxil_status = STATUS_ERROR;
	}
}


void bcm_unload() {
	//TODO: which statuses should be valid?
	if (libbcm_status == STATUS_NOT_FOUND || libbcm_status == STATUS_UNLOADED)
		return;
	
	libbcm_methods = { 0 };

	const char *errmsg;
	if (!xenolib_close(libbcm_host, &errmsg))
		fprintf(stderr, "Error closing bcm_host: %s\n", errmsg);
	
	libbcm_host = NULL;
	libbcm_status = STATUS_UNLOADED;
}

void omxil_unload() {
	//TODO: which statuses should be valid?
	if (libomxil_status == STATUS_NOT_FOUND || libomxil_status == STATUS_UNLOADED)
		return;
	
	libomxil_methods = { 0 };

	const char *errmsg;
	if (!xenolib_close(libomxil, &errmsg))
		fprintf(stderr, "Error closing omxil: %s\n", errmsg);
	
	libomxil = NULL;
	libomxil_status = STATUS_UNLOADED;
}



struct OMXMethods *v4lconvert_omx_init() {
	bcm_load();
	bcm_bind();
	
	omxil_load();
	omxil_bind();

	bcm_init();
	omxil_init();

	if (libomxil_status == STATUS_ACTIVE)
		return &libomxil_methods;
	
	v4lconvert_omx_deinit();
	return NULL;
}

OMXMethods *v4lconvert_get_omx() {
	if (libomxil_status == STATUS_ACTIVE)
		return &libomxil_methods;
	return NULL;
}

void v4lconvert_omx_deinit(OMXMethods *methods) {
	bcm_deinit();
	bcm_unload();

	omxil_deinit();
	omxil_unload();
}


#ifdef __cplusplus
}
#endif