
#include "../types.h"
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

LIBV4L_PUBLIC enum v4lconvert_conversion_signature {
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
	v4lconvert_conversion_signature_sd_sf_2f,
	v4lconvert_conversion_signature_special
};

LIBV4L_PUBLIC union v4lconvert_conversion_fn {
	void (*cvt_sdwh_0f) (const u8* src, u8* dst, u32 width, u32 height);
	void (*cvt_sdwh_1f) (const u8* src, u8* dst, u32 width, u32 height, int flag1);
	void (*cvt_sdwh_2f) (const u8* src, u8* dst, u32 width, u32 height, int flag1, int flag2);
	void (*cvt_sd_sf_0f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt);
	void (*cvt_sd_sf_1f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1);
	void (*cvt_sd_sf_2f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1, int flag2);
	void* user_defined;
};

LIBV4L_PUBLIC struct v4lconvert_converter {
	u32 id;
	enum v4lconvert_conversion_signature signature;
	union v4lconvert_conversion_fn target;
	u32 src_fmt;
	u32 dst_fmt;
	int flag1;
	int flag2;
};

typedef const struct v4lconvert_converter v4lconvert_converter_t;

LIBV4L_PUBLIC struct v4lconvert_buffer {
	int preserve_buf0;
	u32 buf0_cap;
	u32 buf1_cap;
	u32 buf2_cap;
	u32 buf0_len;
	u32 buf1_len;
	/**
	 * Input buffer
	 */
	u8* buf0;
	/**
	 * Output buffer
	 */
	u8* buf1;
	/**
	 * Intermediate buffer. May be null.
	 */
	u8* buf2;
};

LIBV4L_PUBLIC struct v4lconvert_encoder {
	/**
	 * 
	 * @return number of bytes written to dst, or 0 if error
	 */
	u32 (*apply) (struct v4lconvert_encoder* self, const u8* src, u8* dst, u32 src_len);
	int (*release) (struct v4lconvert_encoder* self);
	u32 src_fmt;
	u32 dst_fmt;
	
	u32 src_len;
	u32 src_width;
	u32 src_height;
	/**
	 * Estimates the minimum size the destination buffer can be.
	 */
	u32 dst_len;
	u32 dst_width;
	u32 dst_height;
	const struct v4lconvert_converter* converter;
	union {
		struct v4l2_format* imf_v4l2_src_fmt;
		struct {
			u32 row_stride;
			int quality;
			struct jpeg_compress_struct* cinfo;
			struct jpeg_error_mgr* cerr;
		} jpeg_encode_params;
		struct {
			u32 top;
			u32 left;
		} crop_params;
	};
};

LIBV4L_PUBLIC struct v4lconvert_encoder_series {
	/**
	 * Method to invoke actual conversion
	 */
	u32 (*convert) (struct v4lconvert_encoder_series* self, struct v4lconvert_buffer* buffers);
	/**
	 * Source format
	 */
	u32 src_fmt;
	/**
	 * Destination format (the format that this encoder series encodes to)
	 */
	u32 dst_fmt;
	u32 dst_size;
	/**
	 * Number of encoders in series
	 */
	u32 num_encoders;
	/**
	 * Array of encoders used
	 */
	struct v4lconvert_encoder** encoders;
};

LIBV4L_PUBLIC u32 v4lconvert_estimateBufferSize(u32 fmt, u32 width, u32 height);

LIBV4L_PUBLIC int v4lconvert_encoder_initWithConverter(struct v4lconvert_encoder* encoder, v4lconvert_converter_t* converter, u32 width, u32 height);
LIBV4L_PUBLIC int v4lconvert_encoder_initForIMF(struct v4lconvert_encoder* encoder, u32 src_fmt, u32 dst_fmt, u32 width, u32 height);

LIBV4L_PUBLIC int v4lconvert_encoder_series_init(struct v4lconvert_encoder_series* self, u32 width, u32 height, u32 numConverters, u32* converterIds);
LIBV4L_PUBLIC int v4lconvert_encoder_series_computeConversion(struct v4lconvert_encoder_series* self, u32 width, u32 height, u32 from, u32 to, unsigned int maxIterations);
LIBV4L_PUBLIC int v4lconvert_encoder_series_doRelease(struct v4lconvert_encoder_series* self);

/**
 * Allocate/compute buffers
 * @param series Series to create buffers for
 * @param num_buffers number of buffers to create (length of buffers array)
 * @param buffers Array of buffers to create
 * @param allocate boolean, whether to allocate the buffers themselves or not
 * @return 1 on success, 0 on failure
 */
LIBV4L_PUBLIC int v4lconvert_encoder_series_createBuffers(struct v4lconvert_encoder_series* series, u32 num_buffers, struct v4lconvert_buffer** buffers, int allocate);
LIBV4L_PUBLIC int v4lconvert_buffer_release(struct v4lconvert_buffer* buffer);

LIBV4L_PUBLIC v4lconvert_converter_t* v4lconvert_converter_getConverterById(unsigned int converterId);
LIBV4L_PUBLIC int v4lconvert_converter_lookupConverterByConversion(u32 from, u32 to);
LIBV4L_PUBLIC v4lconvert_converter_t* v4lconvert_converter_getConverterByConversion(u32 from, u32 to);

#ifdef __cplusplus
}
#endif

#endif