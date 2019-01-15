#include "xenolib.h"
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h> // For dlerror, etc.
#endif

#define PATH_MAX 260


#ifdef __cplusplus
extern "C" {
#endif

// Path prefixes to search for libomxil in
static const char *path_prefix[] = {
	"",
#ifdef _WIN32
    ".\\",
#else
    "./",
	"/opt/vc/lib/",
	"/usr/local/lib/",
	"/usr/lib/",
#endif
	NULL,
};

// Library extensions
static const char *lib_ext[] = {
    "",
	".so",
	".so.1",
	".so.2",
	".dylib",
	".dll",
	NULL,
};

//From lunixbochs' glshim/src/gl/loader.c
libptr_t xenolib_find(const char *path) {
    char path_name[PATH_MAX + 1];
    for (unsigned p = 0; path_prefix[p]; p++) {
		for (unsigned e = 0; lib_ext[e]; e++) {
            int path_len = snprintf(path_name, PATH_MAX + 1, "%s%s%s", path_prefix[p], name, lib_ext[e])

            if (path_len < 0 || path_len > PATH_MAX + 1) {
				// Generated path is too big
				continue;
			}

			libptr_t lib = xenolib_open(path_name, 0);
			if (lib) {
				printf("Loaded %s from %s\n", name, path_name);
				return lib;
			}
        }
    }

    return 0;
}

static void fill_errmsg(const char **errmsg) {
    if (!errmsg)
        return;
    
    #ifdef _WIN32
        DWORD err = GetLastError();
        *errmsg = "???";// TODO: fixme
    #else
        *errmsg = dlerror();
    #endif
}

libptr_t xenolib_open(const char *path, const char **errmsg) {
    libptr_t result;
    #ifdef _WIN32
        result = LoadLibraryA(path);
    #else
        result = dlopen(path, RTLD_LOCAL | RTLD_NOW);
    #endif

    if (result)
        return result;

    fill_errmsg(errmsg);
    return 0;
}

void *xenolib_sym(libptr_t lib restrict, const char *symbol restrict) {
    #ifdef _WIN32
        return (void *) GetProcAddress(lib, symbol);
    #else
        return dlsym(lib, symbol);
    #endif
}

bool xenolib_close(libptr_t lib, const char **errmsg) {
    #ifdef _WIN32
        if ()
            return true;
    #else
        if (dlclose(lib) == 0)
            return true;
    #endif

    fill_errmsg(errmsg);
    return false;
}


#ifdef __cplusplus
}
#endif