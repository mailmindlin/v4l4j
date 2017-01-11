#include <stdbool.h>

void* bcm_host;
void (*bcm_host_init) ();
void (*bcm_host_deinit) ();
bool hasBcmHost = false;
bool bcmActive = false;
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

void libv4lconvert_omx_init() {
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;
	bcm_host = open_lib("libbcm_host");
	
	if (bcm_host == NULL) {
		hasBcmHost = false;
		return;
	}
	
	bcm_host_init = dlsym(bcm_host, "bcm_host_init");
	bcm_host_deinit = dlsym(bcm_host, "bcm_host_deinit");
	
	if (!(bcm_host_init && bcm_host_deinit)) {
		hasBcmHost = false;
		return;
	}
	hasBcmHost = true;
	bcm_host_init();
	bcmActive = true;
}

void libv4lconvert_omx_deinit() {
	if (hasBcmHost) {
		bcm_host_deinit();
		hasBcmHost = false;
		bcmActive = false;
	}
}