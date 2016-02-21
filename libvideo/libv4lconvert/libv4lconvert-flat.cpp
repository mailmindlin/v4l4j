
#include "libv4lconvert-flat.h"
#include "libv4lconvert-priv.h"
#include "types.h"
#include "libvideo.h"

#ifndef __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_CPP
#define __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_CPP

#ifndef NULL
#define NULL 0
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

#define GENERATE_CONVERTER_SDWH_0F(id, fn, src_fmt, dst_fmt) {(id), v4lconvert_conversion_fingerprint::sdwh_0f, {.cvt_sdwh_0f = (fn)}, (src_fmt), (dst_fmt), NULL, NULL}

#define GENERATE_CONVERTER_SDWH_1F(id, fn, src_fmt, dst_fmt, flag1) {(id), v4lconvert_conversion_fingerprint::sdwh_1f, {.cvt_sdwh_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), NULL}

#define GENERATE_CONVERTER_SDWH_2F(id, fn, src_fmt, dst_fmt, flag1, flag2) {(id), v4lconvert_conversion_fingerprint::sdwh_1f, {.cvt_sdwh_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), (flag2)}

#define GENERATE_CONVERTER_SD_SF_1F(id, fn, src_fmt, dst_fmt, flag1) {(id), v4lconvert_conversion_fingerprint::sd_sf_1f, {.cvt_sd_sf_2f = (fn)}, (src_fmt), (dst_fmt), (flag1), NULL}

#define GENERATE_CONVERTER_SD_SF_2F(id, fn, src_fmt, dst_fmt, flag1, flag2) {(id), v4lconvert_conversion_fingerprint::sd_sf_2f, {.cvt_sd_sf_2f = (fn)}, (src_fmt), (dst_fmt), (flag1), (flag2)}

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

v4lconvert_converter_t v4lconvert_converters[10] = {
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
	GENERATE_CONVERTER_SDWH_0f(17, v4lconvert_swap_rgb, RGB24, BGR24),
	GENERATE_CONVERTER_SDWH_0f(18, v4lconvert_swap_rgb, BGR24, RGB24)
	//TODO add other converters
};

void v4lconvert_encoder_doConvert(struct v4lconvert_encoder* self, const u8* src, u8* dst) {
	switch (self->fingerprint) {
		case v4lconvert_conversion_fingerprint::sdwh_0f:
			self->target.cvt_sdwh_0f (src, dst, self->width, self->height);
			return;
		case v4lconvert_conversion_fingerprint::sdwh_1f:
			self->target.cvt_sdwh_1f (src, dst, self->width, self->height, self->flag1);
			return;
		case convert_conversion_fingerprint::sdwh_2f:
			self->target.cvt_sdwh_2f (src, dst, self->width, self->height, self->flag1, self->flag2);
			return;
		case convert_conversion_fingerprint::sd_sf_1f:
			self->target.cvt_sdwh_2f (src, dst, self->v4l_src_format, self->flag1);
			return;
		case convert_conversion_fingerprint::sd_sf_2f:
			self->target.cvt_sdwh_2f (src, dst, self->v4l_src_format, self->flag1, self->flag2);
			return;
	}
}

void v4lconvert_encoder_init(struct v4lconvert_encoder* encoder, int converterId, int width, int height) {
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
#endif
