
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

LIBV4L_PUBLIC enum v4lconvert_conversion_type {
	/**
	 * Unknown transformation
	 */
	v4lconvert_conversion_type_unknown = 0,
	/**
	 * Identity conversion, such as memcpy
	 */
	v4lconvert_conversion_type_identity,
	/**
	 * Image format conversion
	 */
	v4lconvert_conversion_type_imf,
	/**
	 * Image crop
	 */
	v4lconvert_conversion_type_crop,
	/**
	 * Pad border around image
	 */
	v4lconvert_conversion_type_pad,
	/**
	 * Scale image
	 */
	v4lconvert_conversion_type_scale,
	/**
	 * Rotate image
	 */
	v4lconvert_conversion_type_rotate,
	/**
	 * Rotate image 90deg clockwise
	 */
	v4lconvert_conversion_type_rotate90,
	/**
	 * Rotate image 180deg
	 * Same as hflip + vflip
	 */
	v4lconvert_conversion_type_rotate180,
	/**
	 * Flip image horizontally
	 */
	v4lconvert_conversion_type_hflip,
	/**
	 * Flip image vertically
	 */
	v4lconvert_conversion_type_vflip,
	v4lconvert_num_conversion_types
};

#define V4LCONVERT_CONVERSION_TYPE_FLAG(type) (1u << v4lconvert_conversion_type_##type)

struct ImageTransformer;
typedef struct ImageTransformer ImageTransformer;
struct ImageTransformerPrototype;
typedef const struct ImageTransformerPrototype ImageTransformerPrototype;

struct ImageTransformerPrototype {
	size_t id;
	/**
	 * Allocates & initializes a converter
	 * @param info Pointer to info about this prototype
	 * @param src_fmt Source format requested
	 * @param dst_fmt Output format requested
	 * @param options_len Length (in bytes) of options
	 * @param options Additional parameters to use when converting
	 * @param errmsg Filled with message if this method fails. Does not need to be released in any case.
	 * @return Converter created, or NULL on error. On error, errno is set.
	 */
	ImageTransformer* (*init) (ImageTransformerPrototype* self, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, size_t options_len, void** options, char** errmsg);
	bool (*estimateCost) (ImageTransformerPrototype* self, unsigned int* cpuCost, float* qualityCost, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, size_t options_len, ...);
	enum v4lconvert_conversion_type type;
	/**
	 * Source format
	 */
	u32 src_fmt;
	/**
	 * Output format
	 */
	u32 dst_fmt;
	union {
		struct {
			enum v4lconvert_conversion_signature signature;
			union v4lconvert_conversion_fn {
				void (*cvt_sdwh_0f) (const u8* src, u8* dst, u32 width, u32 height);
				void (*cvt_sdwh_1f) (const u8* src, u8* dst, u32 width, u32 height, int flag1);
				void (*cvt_sdwh_2f) (const u8* src, u8* dst, u32 width, u32 height, int flag1, int flag2);
				void (*cvt_sd_sf_0f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt);
				void (*cvt_sd_sf_1f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1);
				void (*cvt_sd_sf_2f) (const u8* src, u8* dst, const struct v4l2_format* src_fmt, int flag1, int flag2);
				void* cvt_special;
			} target;
			int flag1;
			int flag2;
		} imf_params;
		u8 user_defined[sizeof(int*) * 4];
	};
};

struct v4lconvert_buffer {
	/**
	 * If true, buffer 0 should NOT be overwritten.
	 */
	bool preserve_buf0;
	/**
	 * Capacity of buffer 0
	 */
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

struct ImageTransformer {
	/**
	 * 
	 * @return number of bytes written to dst, or 0 if error
	 * If error, errno may be set.
	 */
	size_t (*apply) (ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len);
	/**
	 * Safely release memory used by encoder. Does not release the encoder itself.
	 * DO NOT call any methods on encoder after this method has been invoked.
	 */
	bool (*release) (ImageTransformer* self);
	
	u32 src_fmt;
	u32 dst_fmt;
	
	size_t src_len;
	/**
	 * Estimates the minimum size the destination buffer can be.
	 */
	size_t dst_len;
	
	//Private members. Please don't touch.
	v4lconvert_converter_prototype* prototype;
	
	union {
		struct {
			u32 row_stride;
			unsigned int quality;
			struct jpeg_compress_struct* cinfo;
			struct jpeg_error_mgr* cerr;
		} jpeg_encode_params;
		struct {
			signed int top;
			signed int left;
			//dst_width & dst_height are used to determine the other two sides
		} crop_params;
		struct {
			int flag1;
			int flag2;
		} imf_params;
		u8 user_defined[sizeof(int*) * 8];
	} params;
	struct control** controls;
};

typedef struct VideoPipeline {
	/**
	 * Method to invoke actual conversion
	 */
	u32 (*convert) (struct VideoPipeline* self, struct v4lconvert_buffer* buffers);
	/**
	 * Release memory owned by this struct.
	 */
	bool (*release) (struct VideoPipeline* self);
	/**
	 * Source format
	 */
	u32 src_fmt;
	/**
	 * Destination format (the format that this encoder series encodes to)
	 */
	u32 dst_fmt;
	
	
	size_t src_len;
	size_t dst_len;
	/**
	 * Number of encoders in series
	 */
	size_t numConverters;
	/**
	 * Array of encoders used.
	 * Please don't change it.
	 */
	ImageTransformer** converters;
} VideoPipeline;

struct v4lconvert_conversion_request {
	struct v4l2_format* src_fmt;
	struct v4l2_format* dst_fmt;
	unsigned int rotation;
	bool flipHorizontal;
	bool flipVertical;
	unsigned int scaleNumerator;
	unsigned int scaleDenominator;
	// Values for crop/pad
	signed int top_offset;
	signed int left_offset;
};

LIBV4L_PUBLIC size_t v4lconvert_estimateBufferSize(u32 fmt, unsigned int width, unsigned int height);

LIBV4L_PUBLIC bool ImageTransformer_initForIMF(ImageTransformer* transformer, u32 src_fmt, u32 dst_fmt, unsigned int width, unsigned int height);

/**
 * Create a VideoPipeline that will apply the given conversion request.
 */
LIBV4L_PUBLIC VideoPipeline* VideoPipeline_create(struct v4lconvert_conversion_request* request, char** errmsg) __attribute__((nonnull (1)));
/**
 * Initialize the 
 */
LIBV4L_PUBLIC VideoPipeline* VideoPipeline_init(VideoPipeline* pipeline, struct v4lconvert_conversion_request* request, char** errmsg) __attribute__((nonnull (1)));
/**
 * Create a VideoPipeline that will apply the given transformers in order.
 * @param pipeline
 *      Pipeline to 
 * @param cascadeRelease
 *      Whether, when the VideoPipeline is released, it also will release all of its transformers
 */
LIBV4L_PUBLIC VideoPipeline* VideoPipeline_wrap(size_t numTransformers, ImageTransformer* transformers[], bool cascadeRelease) __attribute__ ((nonnull (1, 3)));

/**
 * Allocate/compute buffers
 * @param series Series to create buffers for
 * @param num_buffers number of buffers to create (length of buffers array)
 * @param buffers Array of buffers to create
 * @param allocate boolean, whether to allocate the buffers themselves or not
 * @return 1 on success, 0 on failure
 */
LIBV4L_PUBLIC int v4lconvert_encoder_series_createBuffers(struct VideoPipeline* series, size_t num_buffers, struct v4lconvert_buffer* buffers[], bool allocate);
LIBV4L_PUBLIC int v4lconvert_buffer_release(struct v4lconvert_buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif