#pragma once

#ifndef __EXTLIB_H
#define __EXTLIB_H


#if defined(_WIN32)
    #include <windows.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#if defined(_WIN32)
    typedef HMODULE libptr_t;
#elif defined(__linux__)
    // Linux (dlopen)
    typedef void *libptr_t;
#endif

/**
 * Searches for library with `name`.
 */
libptr_t xenolib_find(const char *name);

/**
 * Platform independent-ish dlopen.
 * 
 * @param [in] path
 *      Path to library 
 * @param [out] error
 *      If loading the library fails, and this pointer is nonnull, it attempts
 *      to fill it with an error message.
 * @return Pointer to loaded library (NULL if failed)
 */
libptr_t xenolib_open(const char *path, const char **error);

void *xenolib_sym(libptr_t lib restrict, const char *symbol restrict);

bool xenolib_close(libptr_t, const char **errmsg);


#ifdef __cplusplus
}
#endif

#endif