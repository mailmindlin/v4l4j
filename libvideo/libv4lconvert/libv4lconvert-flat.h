
#include "types.h"
#include "libvideo.h"
#include "libv4lconvert-priv.h"

#ifndef __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_H
#define __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_H

extern enum v4lconvert_conversion_fingerprint;
struct v4lconvert_converter;
struct v4lconvert_encoder;
struct v4lconvert_encoder_series;
typedef struct v4lconvert_converter v4lconvert_converter_t;

void v4lconvert_encoder_doConvert(struct v4lconvert_encoder* self, const u8* src, u8* dst);
void v4lconvert_encoder_init(struct v4lconvert_encoder* encoder, int converterId, int width, int height);
v4lconvert_converter_t* v4lconvert_converter_getConverterById(int converterId);

#endif