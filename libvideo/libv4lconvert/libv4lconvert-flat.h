
#include "types.h"
#include "libvideo.h"
#include "libv4lconvert-priv.h"

#ifndef __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_H
#define __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum v4lconvert_conversion_fingerprint {
	/**
	 * Value if this fingerprint is unset (struct's will defaultly initialize with this value)
	 */
	unset = 0,
	/**
	 * For fingerprint <code>void convert(const u8* src, u8* dst, u32 width, u32 height);</code>
	 */
	sdwh_0f,
	sdwh_1f,
	sdwh_2f,
	sd_sf_1f,
	sd_sf_2f
};
union v4lconvert_conversion_fn {
	void (*cvt_sdwh_0f) (const u8* src, u8* dst, u32 width, u32 height);
	void (*cvt_sdwh_1f) (const u8* src, u8* dst, u32 width, u32 height, int flag1);
	void (*cvt_sdwh_2f) (const u8* src, u8* dst, u32 width, u32 height, int flag1, int flag2);
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

void v4lconvert_encoder_doConvert(struct v4lconvert_encoder* self, const u8* src, u8* dst);
void v4lconvert_encoder_init(struct v4lconvert_encoder* encoder, int converterId, int width, int height);
v4lconvert_converter_t* v4lconvert_converter_getConverterById(int converterId);

#ifdef __cplusplus
}
#endif

#endif