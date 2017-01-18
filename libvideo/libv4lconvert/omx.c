#include <stdbool.h>
#include <collections/list/list.h>
#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Video.h>

static void* bcm_host;
static void (*_bcm_host_init) ();
static void (*_bcm_host_deinit) ();
static bool hasBcmHost = false;
static bool bcmActive = false;

static void* omxIL;
static bool hasOMXIL = false;
static bool omxILActive = false;
static OMX_ERRORTYPE (*_OMX_Init) ();
static OMX_ERRORTYPE (*_OMX_Deinit) ();
static OMX_ERRORTYPE (*_OMX_ComponentNameEnum) (OMX_OUT OMX_STRING cComponentName, OMX_IN OMX_U32 nNameLength, OMX_IN OMX_U32 nIndex);
static OMX_ERRORTYPE (*_OMX_GetHandle) (OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_STRING cComponentName, OMX_IN OMX_PTR pAppData, OMX_IN OMX_CALLBACKTYPE* pCallBacks);
static OMX_ERRORTYPE (*_OMX_FreeHandle) (OMX_IN OMX_HANDLETYPE hComponent);

static const char *path_prefix[] = {
	"",
	"/opt/vc/lib/",
	"/usr/local/lib/",
	"/usr/lib/",
	NULL,
};

static const char *lib_ext[] = {
	"so",
	"so.1",
	"so.2",
	"dylib",
	"dll",
	NULL,
};

struct libv4lconvert_omxAppdata {
	
};

static List* omxPrototypes;

//OMX error descriptions
static char* getOMXErrorDescription(OMX_ERRORTYPE err) {
	switch(err) {
		case OMX_ErrorNone:
			return "no error";
		case OMX_ErrorInsufficientResources:
			return "insufficient resources";
		case OMX_ErrorInvalidComponentName:
			return "invalid component name";
		case OMX_ErrorComponentNotFound:
			return "component not found";
		case OMX_ErrorInvalidComponent:
			return "invalid component";
		case OMX_ErrorBadParameter:
			return "bad parameter";
		case OMX_ErrorUnderflow:
			return "underflow";
		case OMX_ErrorOverflow:
			return "overflow";
		case OMX_ErrorBadPortIndex:
			return "bad port index";
		case OMX_ErrorIncorrectStateOperation:
			return "invalid state while trying to perform command";
		case OMX_ErrorIncorrectStateTransition:
			return "unallowed state transition";
		case OMX_ErrorBadPortIndex:
			return "bad port index, i.e. incorrect port";
		case OMX_ErrorHardware:
			return "hardware error";
		default:
			return "(unknown)";
	}
}

//From lunixbochs' glshim/src/gl/loader.c
static void *open_lib(const char *name) {
	void *lib = NULL;

	char path_name[PATH_MAX + 1];
	int flags = RTLD_LOCAL | RTLD_NOW;
	for (int p = 0; path_prefix[p]; p++) {
		for (int e = 0; lib_ext[e]; e++) {
			snprintf(path_name, PATH_MAX, "%s%s.%s", path_prefix[p], names[i], lib_ext[e]);
			if ((lib = dlopen(path_name, flags))) {
				printf("libGL:loaded: %s\n", path_name);
				return lib;
			}
		}
	}
	return lib;
}

static bool omx_tryRegisterPrototypes() {
	char componentName[128];
	unsigned int componentIndex = 0;
	while (_OMX_ComponentNameEnum(&componentName, sizeof(componentName), componentIndex) == OMX_ErrorNone) {
		OMX_HANDLETYPE* component;
		struct libv4lconvert_omxAppdata* appData = malloc(sizeof(struct libv4lconvert_omxAppdata));
		if (appData == NULL) {
			printf("OMX: Error allocating appdata (ENOMEM)\n");
			return false;
		}
		OMX_ERRORTYPE r = _OMX_GetHandle(component, componentName, appData, NULL);
		if (r != OMX_ErrorNone) {
			printf("OMX: Error gettting handle for component #%d (%s): %s\n", componentIndex, componentName, getOMXErrorDescription(r));
			//This is recoverable from
			continue;
		}
		OMX_PORT_PARAM_TYPE videoPorts;
		OMX_INIT_STRUCTURE(videoPorts);
		omxPrototypes.add(&omxPrototypes, appData);
	}
}

static bool omx_tryInitialize() {
	OMX_ERRORTYPE r = _OMX_Init();
	if (r != OMX_ErrorNone) {
		printf("OMX: Error initializing OMX: %#08x %s\n", r, getOMXErrorDescription(r));
		return false;
	}
	//Register prototypes
	if (!ArrayList_create(&omxPrototypes)) {
		_OMX_Deinit();
		printf("OMX: Error initializing list\n");
		return false;
	}
	if (!omx_tryRegisterPrototypes()) {
		printf("OMX: Error registerring prototypes\n");
		//Deregister any prototypes
		Iterator* crPrototypes = omxPrototypes.iterator(&omxPrototypes);
		if (crPrototypes) {
			while (crPrototypes->hasNext(crPrototypes)) {
				ImageTransformerPrototype* prototype = crPrototypes->next(crPrototypes);
				//TODO deregister
				free(prototype);
			}
			crPrototypes->release(crPrototypes);
		}
		omxPrototypes.release(&omxPrototypes);
		return false;
	}
	return true;
}
/**
 * Call once.
 */
void libv4lconvert_omx_init() {
	static bool initialized = false;
	#define LOOKUP_FN(lib, sym) \
		if (!((_##sym) = dlsym(lib, (#sym)))) {\
			printf("Error looking up symbol: " #sym "\n");\
			break;\
		}
	if (!initialized) {
		bcm_host = open_lib("libbcm_host");
		hasBcmHost = false;
		if (bcm_host != NULL) {
			do {
				LOOKUP_FN(bcm_host, bcm_host_init)
				LOOKUP_FN(bcm_host, bcm_host_deinit)
				hasBcmHost = true;
			} while (0);
			
			if (!hasBcmHost)
				dlclose(bcm_host);
		}
		
		omxIL = open_lib("libopenmaxil");
		hasOMXIL = false;
		if (omxIL != NULL) {
			do {
				LOOKUP_FN(omxIL, OMX_Init);
				LOOKUP_FN(omxIL, OMX_Deinit);
				LOOKUP_FN(omxIL, OMX_ComponentNameEnum);
				LOOKUP_FN(omxIL, OMX_GetHandle);
				LOOKUP_FN(omxIL, OMX_FreeHandle);
				hasOMXIL = true;
			} while (0);
			if (!hasOMXIL)
				dlclose(omxIL);
		}
		initialized = true;
	}
	if (hasBcmHost && !bcmActive) {
		_bcm_host_init();
		bcmActive = true;
	}
	
	if (hasOMXIL && !omxILActive)
		omxILActive = omx_tryInitialize();
}

void libv4lconvert_omx_deinit() {
	if (hasBcmHost && bcmActive) {
		bcm_host_deinit();
		bcmActive = false;
	}
	if (hasBcmHost) {
		bcm_host_init = NULL;
		bcm_host_deinit = NULL;
		if (dlclose(bcm_host) != 0)
			fprintf(stderr, "Error closing bcm_host: %s\n", dlerror());
		bcm_host = NULL;
		hasBcmHost = false;
	}
	
	if (hasOMXIL && omxILActive) {
		_OMX_Deinit();
		omxILActive = false;
	}
	
	if (hasOMXIL) {
		//TODO maybe null out the symbol pointers
		if (dlclose(omxIL) != 0)
			fprintf(stderr, "Error closing openMAX: %s\n", dlerror());
		omxIL = NULL;
		hasOMXIL = false;
	}
}