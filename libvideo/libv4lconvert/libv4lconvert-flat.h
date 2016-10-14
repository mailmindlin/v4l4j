
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

enum v4lconvert_conversion_signature {
	/**
	 * Value if this signature is unset (struct's will defaultly initialize with this value)
	 */
	v4lconvert_conversion_signature_unset = 0,
	/**
	 * For signature <code>void convert(const u8* src, u8* dst, u32 width, u32 height);</code>
	 */
	v4lconvert_conversion_signature_sdwh_0f,
	v4lconvert_conversion_signature_sdwh_1f,
	v4lconvert_conversion_signature_sdwh_2f,
	v4lconvert_conversion_signature_sd_sf_0f,
	v4lconvert_conversion_signature_sd_sf_1f,
	v4lconvert_conversion_signature_sd_sf_2f
};
union v4lconvert_conversion_fn {
	void (*cvt_sdwh_0f) (const u8* src, u8* dst, u32 width, u32 height);
	void (*cvt_sdwh_1f) (const u8* src, u8* dst, u32 width, u32 height, int flag1);
	void (*cvt_sdwh_2f) (const u8* src, u8* dst, u32 width, u32 height, int flag1, int flag2);
	void (*cvt_sd_sf_0f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt);
	void (*cvt_sd_sf_1f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1);
	void (*cvt_sd_sf_2f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1, int flag2);
};
struct v4lconvert_converter {
	u32 id;
	enum v4lconvert_conversion_signature signature;
	union v4lconvert_conversion_fn target;
	u32 src_fmt;
	u32 dst_fmt;
	int flag1;
	int flag2;
};

typedef struct v4lconvert_converter v4lconvert_converter_t;

struct v4lconvert_buffer {
	int preserve_buf0;
	u32 buf0_len;
	/**
	 * Input buffer
	 */
	u8* buf0;
	u32 buf1_len;
	/**
	 * Output buffer
	 */
	u8* buf1;
	u32 buf2_len;
	/**
	 * Intermediate buffer. May be null.
	 */
	u8* buf2;
}
struct v4lconvert_transformation {
	void (*apply) (struct v4lconvert_transformation* self, const u8* src, u8* dst);
	/**
	 * Estimated minimum length of destination buffer
	 */
	u32 dst_size;
	struct v4lconvert_converter* converter;
	union {
		struct {
			u32 width;
			u32 height;
			u32 src_fmt;
			u32 dst_fmt;
		} imf_cvt_simple;
		struct {
			v4l2_format* src_fmt;
		} imf_cvt_cplx;
		struct {
			u32 src_width;
			u32 src_height;
			u32 top;
			u32 left;
			u32 dst_width;
			u32 dst_height;
		} crop;
	} params;
	struct v4l2_format* v4l_src_fmt;
};

struct v4lconvert_transformer {
	int (*convert) (struct v4lconvert_transformer* self, struct v4lconvert_buffer* buffer);
	u32 src_fmt;
	u32 dst_fmt;
	u32 num_transformations;
	struct v4lconvert_transformation** transformations;
};

v4lconvert_converter_t* v4lconvert_converter_getConverterById(u32 converterId);
int v4lconvert_converter_lookupConverterByConversion(u32 from, u32 to);
v4lconvert_converter_t* v4lconvert_converter_getConverterByConversion(u32 from, u32 to);

int v4lconvert_transformation_initSimple(struct v4lconvert_transformation* encoder, u32 converterId, u32 width, u32 height);

int v4lconvert_transformer_init(struct v4lconvert_encoder_series* self, u32 numTransformations, struct v4lconvert_transformation** transformations);
int v4lconvert_transformer_initWithIds(struct v4lconvert_transformer* transformer, u32 width, u32 height, u32 numConverters, u32 converterIds);
int v4lconvert_transformer_initForImfConversion(struct v4lconvert_encoder_series* self, u32 width, u32 height, u32 src_fmt, u32 dst_fmt, u32 maxIterations);
int v4lconvert_transformer_createBuffers(struct v4lconvert_transformer* transformer, u32 num_buffers, struct v4lconvert_buffer** buffers, int preserve_input, int doAllocate);
int v4lconvert_transformer_release(struct v4lconvert_transformer* transformer);

int v4lconvert_buffer_release(struct v4lconvert_buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif