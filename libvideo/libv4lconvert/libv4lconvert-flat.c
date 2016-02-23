
#include "libv4lconvert-flat.h"
#include "libv4lconvert-priv.h"
#include "types.h"
#include "libvideo.h"

#ifndef __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_CPP
#define __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_CPP

#ifdef __cplusplus
extern "C" {
#endif

#ifndef N_A
#define N_A 0
#endif

#define GENERATE_CONVERTER_SDWH_0F(id, fn, src_fmt, dst_fmt) {(id), v4lconvert_conversion_fingerprint_sdwh_0f, {.cvt_sdwh_0f = (fn)}, (src_fmt), (dst_fmt), N_A, N_A}

#define GENERATE_CONVERTER_SDWH_1F(id, fn, src_fmt, dst_fmt, flag1) {(id), v4lconvert_conversion_fingerprint_sdwh_1f, {.cvt_sdwh_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), N_A}

#define GENERATE_CONVERTER_SDWH_2F(id, fn, src_fmt, dst_fmt, flag1, flag2) {(id), v4lconvert_conversion_fingerprint_sdwh_1f, {.cvt_sdwh_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), (flag2)}

#define GENERATE_CONVERTER_SD_SF_1F(id, fn, src_fmt, dst_fmt, flag1) {(id), v4lconvert_conversion_fingerprint_sd_sf_1f, {.cvt_sd_sf_2f = (fn)}, (src_fmt), (dst_fmt), (flag1), N_A}

#define GENERATE_CONVERTER_SD_SF_2F(id, fn, src_fmt, dst_fmt, flag1, flag2) {(id), v4lconvert_conversion_fingerprint_sd_sf_2f, {.cvt_sd_sf_2f = (fn)}, (src_fmt), (dst_fmt), (flag1), (flag2)}

#define GENERATE_CONVERTER_SDWH_1F_x2(id, fn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SDWH_1F(id    , (fn), (src_fmt_0), (dst_fmt_0), 0),\
	GENERATE_CONVERTER_SDWH_1F(id + 1, (fn), (src_fmt_1), (dst_fmt_1), 1)

#define GENERATE_CONVERTER_SD_SF_1F_x2(id, fn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SD_SF_1F(id    , (fn), (src_fmt_0), (dst_fmt_0), 0),\
	GENERATE_CONVERTER_SD_SF_1F(id + 1, (fn), (src_fmt_1), (dst_fmt_1), 1)

#define GENERATE_CONVERTER_SD_SF_2F_x4(id, fn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SD_SF_2F(id    , (fn), (src_fmt_0), (dst_fmt_0), 0, 0),\
	GENERATE_CONVERTER_SD_SF_2F(id + 1, (fn), (src_fmt_1), (dst_fmt_0), 1, 0),\
	GENERATE_CONVERTER_SD_SF_2F(id + 2, (fn), (src_fmt_0), (dst_fmt_1), 0, 1),\
	GENERATE_CONVERTER_SD_SF_2F(id + 3, (fn), (src_fmt_1), (dst_fmt_1), 1, 1)

#define NUM_CONVERTERS 20
v4lconvert_converter_t v4lconvert_converters[NUM_CONVERTERS] = {
	GENERATE_CONVERTER_SD_SF_2F_x4(0, v4lconvert_rgb24_to_yuv420, RGB32, BGR32, YUV420, YVU420),
	GENERATE_CONVERTER_SDWH_1F_x2(4, v4lconvert_yuv420_to_rgb24, YUV420, YVU420, RGB24, RGB24),
	GENERATE_CONVERTER_SDWH_1F_x2(6, v4lconvert_yuv420_to_bgr24, YUV420, YVU420, BGR24, BGR24),
	GENERATE_CONVERTER_SDWH_0F(8, v4lconvert_yuyv_to_rgb24, YUYV, RGB24),
	GENERATE_CONVERTER_SDWH_0F(9, v4lconvert_yuyv_to_bgr24, YUYV, BGR24),
	GENERATE_CONVERTER_SDWH_1F_x2(10, v4lconvert_yuyv_to_yuv420, YUYV, YUYV, YUV420, YVU420),
	GENERATE_CONVERTER_SDWH_0F(12, v4lconvert_yvyu_to_rgb24, YUYV, RGB24),
	GENERATE_CONVERTER_SDWH_0F(13, v4lconvert_yvyu_to_bgr24, YUYV, BGR24),
	GENERATE_CONVERTER_SDWH_0F(14, v4lconvert_uyvy_to_rgb24, UYVY, RGB24),
	GENERATE_CONVERTER_SDWH_0F(15, v4lconvert_uyvy_to_bgr24, UYVY, BGR24),
	GENERATE_CONVERTER_SDWH_1F_x2(16, v4lconvert_uyvy_to_yuv420, UYVY, UYVY, YUV420, YVU420),
	GENERATE_CONVERTER_SDWH_0F(18, v4lconvert_swap_rgb, RGB24, BGR24),
	GENERATE_CONVERTER_SDWH_0F(19, v4lconvert_swap_rgb, BGR24, RGB24)
	//TODO add other converters
};

void v4lconvert_encoder_doConvert(struct v4lconvert_encoder* self, const u8* src, u8* dst) {
	v4lconvert_converter_t* converter = self->converter;
	
	switch (converter->fingerprint) {
		case v4lconvert_conversion_fingerprint_unset:
			//TODO maybe throw error or something
			return;
		case v4lconvert_conversion_fingerprint_sdwh_0f:
			converter->target.cvt_sdwh_0f (src, dst, self->width, self->height);
			return;
		case v4lconvert_conversion_fingerprint_sdwh_1f:
			converter->target.cvt_sdwh_1f (src, dst, self->width, self->height, converter->flag1);
			return;
		case v4lconvert_conversion_fingerprint_sdwh_2f:
			converter->target.cvt_sdwh_2f (src, dst, self->width, self->height, converter->flag1, converter->flag2);
			return;
		case v4lconvert_conversion_fingerprint_sd_sf_1f:
			converter->target.cvt_sd_sf_1f (src, dst, self->v4l_src_fmt, converter->flag1);
			return;
		case v4lconvert_conversion_fingerprint_sd_sf_2f:
			converter->target.cvt_sd_sf_2f (src, dst, self->v4l_src_fmt, converter->flag1, converter->flag2);
			return;
		default:
			return;
	}
}

void v4lconvert_encoder_init(struct v4lconvert_encoder* encoder, unsigned int converterId, unsigned int width, unsigned int height) {
	encoder->convert = v4lconvert_encoder_doConvert;
	encoder->converter = &(v4lconvert_converters[converterId]);
	encoder->src_fmt = encoder->converter->src_fmt;
	encoder->dst_fmt = encoder->converter->dst_fmt;
	encoder->width = width;
	encoder->height = height;
}

v4lconvert_converter_t* v4lconvert_converter_getConverterById(int converterId) {
	return &(v4lconvert_converters[converterId]);
}

unsigned int v4lconvert_converter_lookupConverterByConversion(unsigned int from, unsigned int to) {
	int i;
	v4lconvert_converter_t* converter;
	
	//TODO use better lookup algorithm than O(n)
	for (i = 0; i < NUM_CONVERTERS; i++) {
		converter = &(v4lconvert_converters[converterId]);
		if ((converter->src_fmt == from) && (converter->dst_fmt == to))
			return i;
	}
	return NULL;
}
/**
 * Find a converter that does the conversion that you want
 */
v4lconvert_converter_t* v4lconvert_converter_getConverterByConversion(unsigned int from, unsigned int to) {
	int i;
	v4lconvert_converter_t* converter;
	
	//TODO use better lookup algorithm than O(n)
	for (i = 0; i < NUM_CONVERTERS; i++) {
		converter = &(v4lconvert_converters[converterId]);
		if ((converter->src_fmt == from) && (converter->dst_fmt == to))
			return converter;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif
