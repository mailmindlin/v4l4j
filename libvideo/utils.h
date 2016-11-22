#ifndef H_LIBVIDEO_UTILS
#define H_LIBVIDEO_UTILS

#define CLEAR_ARR(x,s) memset((x), 0x0, (s));
#define ARRAY_SIZE(x) ( ((x)==NULL) ? 0 : (sizeof(x)/sizeof((x)[0])) )

#ifndef UNUSED
// To hide unused parameter warnings
#define UNUSED(x) (void)(x)
#endif

#if __GNUC__ >= 4
#define LIBVIDEO_PUBLIC __attribute__ ((visibility("default")))
#define LIBVIDEO_LOCAL  __attribute__ ((visibility("hidden")))
#else
#define LIBVIDEO_PUBLIC
#define LIBVIDEO_LOCAL
#endif

#endif