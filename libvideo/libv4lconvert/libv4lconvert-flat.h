
#include "types.h"
#include "libvideo.h"
#include "libv4lconvert-priv.h"

#ifndef __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_H
#define __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_H

#ifdef __cplusplus
extern "C" {
#endif
#ifndef LIBV4L_PUBLIC
#if __GNUC__ >= 4
#define LIBV4L_PUBLIC __attribute__ ((visibility("default")))
#else
#define LIBV4L_PUBLIC
#endif
#endif

enum v4lconvert_conversion_fingerprint {
	/**
	 * Value if this fingerprint is unset (struct's will defaultly initialize with this value)
	 */
	v4lconvert_conversion_fingerprint_unset = 0,
	/**
	 * For fingerprint <code>void convert(const u8* src, u8* dst, u32 width, u32 height);</code>
	 */
	v4lconvert_conversion_fingerprint_sdwh_0f,
	v4lconvert_conversion_fingerprint_sdwh_1f,
	v4lconvert_conversion_fingerprint_sdwh_2f,
	v4lconvert_conversion_fingerprint_sd_sf_0f,
	v4lconvert_conversion_fingerprint_sd_sf_1f,
	v4lconvert_conversion_fingerprint_sd_sf_2f
};
union v4lconvert_conversion_fn {
	void (*cvt_sdwh_0f) (const u8* src, u8* dst, u32 width, u32 height);
	void (*cvt_sdwh_1f) (const u8* src, u8* dst, u32 width, u32 height, int flag1);
	void (*cvt_sdwh_2f) (const u8* src, u8* dst, u32 width, u32 height, int flag1, int flag2);
	void (*cvt_sd_sf_0f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt;
	void (*cvt_sd_sf_1f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1);
	void (*cvt_sd_sf_2f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1, int flag2);
};
struct v4lconvert_converter {
	int id;
	enum v4lconvert_conversion_fingerprint fingerprint;
	union v4lconvert_conversion_fn target;
	int src_fmt;
	int dst_fmt;
	int flag1;
	int flag2;
};

typedef struct v4lconvert_converter v4lconvert_converter_t;

struct v4lconvert_encoder {
	void (*convert) (struct v4lconvert_encoder* self, const u8* src, u8* dst);
	int src_fmt;
	int dst_fmt;
	struct v4l2_format* v4l_src_fmt;
	struct v4lconvert_converter* converter;
	u32 width;
	u32 height;
};

struct v4lconvert_encoder_series {
	void (*convert) (struct v4lconvert_encoder_series* self, const u8* src, u8* dst);
	int num_converters;
	int src_fmt;
	int dst_fmt;
	struct v4lconvert_encoder** encoders;
};

void v4lconvert_encoder_init(struct v4lconvert_encoder* encoder, u32 converterId, u32 width, u32 height);
void v4lconvert_encoder_doConvert(struct v4lconvert_encoder* self, const u8* src, u8* dst);

void v4lconvert_encoder_series_init(struct v4lconvert_encoder_series* self, u32 width, u32 height, u32 numConverters, u32* converterIds);
int v4lconvert_encoder_series_computeConversion(struct v4lconvert_encoder_series* self, u32 width, u32 height, u32 from, u32 to);
void v4lconvert_encoder_series_doConvert(struct v4lconvert_encoder_series* self, const u8* src, const u8* dst);

v4lconvert_converter_t* v4lconvert_converter_getConverterById(u32 converterId);
unsigned int v4lconvert_converter_lookupConverterByConversion(u32 from, u32 to);
v4lconvert_converter_t* v4lconvert_converter_getConverterByConversion(u32 from, u32 to);

#ifdef __cplusplus
}
#endif

#endif