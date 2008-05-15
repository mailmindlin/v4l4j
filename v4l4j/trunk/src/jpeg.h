#ifndef H_JPEG
#define H_JPEG

#include "common.h"

//Initialises the JPEG compressor in struct v4l4j_device->j
//the 2nd arg is the initial jpeg quality
int init_jpeg_compressor(struct v4l4j_device *, int );

void destroy_jpeg_compressor(struct v4l4j_device *);

#endif /*H_JPEG*/
