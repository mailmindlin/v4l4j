#ifndef H_LIBVIDEO_UTILS
#define H_LIBVIDEO_UTILS

#define CLEAR_ARR(x,s) memset((x), 0x0, (s));
#define ARRAY_SIZE(x) ( ((x)==NULL) ? 0 : (sizeof(x)/sizeof((x)[0])) )

#endif