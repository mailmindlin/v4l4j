
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

#define GENERATE_CONVERTER_SDWH_0F(id, fn, src_fmt, dst_fmt) {(id), v4lconvert_conversion_signature_sdwh_0f, {.cvt_sdwh_0f = (fn)}, (src_fmt), (dst_fmt), N_A, N_A}

#define GENERATE_CONVERTER_SDWH_1F(id, fn, src_fmt, dst_fmt, flag1) {(id), v4lconvert_conversion_signature_sdwh_1f, {.cvt_sdwh_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), N_A}

#define GENERATE_CONVERTER_SDWH_2F(id, fn, src_fmt, dst_fmt, flag1, flag2) {(id), v4lconvert_conversion_signature_sdwh_1f, {.cvt_sdwh_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), (flag2)}

#define GENERATE_CONVERTER_SD_SF_0F(id, fn, src_fmt, dst_fmt) {(id), v4lconvert_conversion_signature_sd_sf_0f, {.cvt_sd_sf_0f = (fn)}, (src_fmt), (dst_fmt), N_A, N_A}

#define GENERATE_CONVERTER_SD_SF_1F(id, fn, src_fmt, dst_fmt, flag1) {(id), v4lconvert_conversion_signature_sd_sf_1f, {.cvt_sd_sf_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), N_A}

#define GENERATE_CONVERTER_SD_SF_2F(id, fn, src_fmt, dst_fmt, flag1, flag2) {(id), v4lconvert_conversion_signature_sd_sf_2f, {.cvt_sd_sf_2f = (fn)}, (src_fmt), (dst_fmt), (flag1), (flag2)}

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

#define NUM_CONVERTERS 28
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
	GENERATE_CONVERTER_SDWH_0F(19, v4lconvert_swap_rgb, BGR24, RGB24),
	GENERATE_CONVERTER_SD_SF_0F(20, v4lconvert_swap_uv, YUV420, YVU420),//TODO figure out if this is right...
	GENERATE_CONVERTER_SDWH_0F(21, v4lconvert_grey_to_rgb24, GREY, RGB24),
	GENERATE_CONVERTER_SD_SF_0F(22, v4lconvert_grey_to_yuv420, GREY, YUV420),
	//v4lconvert_y10b_to_rgb24(struct v4lconvert_data *data, const u8 *src, u8 *dest, u32 width, u32 height);
	//v4lconvert_y10b_to_yuv420(struct v4lconvert_data *data, const u8 *src, u8 *dest, u32 width, u32 height);
	//v4lconvert_rgb565_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height);
	GENERATE_CONVERTER_SDWH_0F(23, v4lconvert_rgb565_to_bgr24, RGB565, BGR24),
	GENERATE_CONVERTER_SD_SF_1F_x2(24, v4lconvert_rgb565_to_yuv420, RGB565, RGB565, YUV420, YVU420),
	GENERATE_CONVERTER_SDWH_1F_x2(26, v4lconvert_spca501_to_yuv420, SPCA501, SPCA501, YUV420, YVU420),
	//v4lconvert_spca505_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);
	//v4lconvert_spca508_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);
	//v4lconvert_cit_yyvyuy_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, int yvu);
	//v4lconvert_konica_yuv420_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, int yvu);
	//v4lconvert_m420_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, int yvu);
	//int v4lconvert_cpia1_to_yuv420(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dst, u32 width, u32 height, int yvu);
	//v4lconvert_sn9c20x_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);
	//int v4lconvert_se401_to_rgb24(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dest, u32 width, u32 height);

	//int v4lconvert_decode_jpeg_tinyjpeg(struct v4lconvert_data *data, u8 *src, int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt, int flags);

	//int v4lconvert_decode_jpeg_libjpeg(struct v4lconvert_data *data, u8 *src, int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt);

	//int v4lconvert_decode_jpgl(const u8 *src, u32 src_size, unsigned int dest_pix_fmt, u8 *dest, u32 width, u32 height);

	//v4lconvert_decode_spca561(const u8 *src, u8 *dst, u32 width, u32 height);

	//v4lconvert_decode_sn9c10x(const u8 *src, u8 *dst, u32 width, u32 height);

	//int v4lconvert_decode_pac207(struct v4lconvert_data *data, const u8 *inp, int src_size, u8 *outp, u32 width, u32 height);

	//int v4lconvert_decode_mr97310a(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dst, u32 width, u32 height);

	//int v4lconvert_decode_jl2005bcd(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dest, u32 width, u32 height);

	//v4lconvert_decode_sn9c2028(const u8 *src, u8 *dst, u32 width, u32 height);

	//v4lconvert_decode_sq905c(const u8 *src, u8 *dst, u32 width, u32 height);

	//v4lconvert_decode_stv0680(const u8 *src, u8 *dst, u32 width, u32 height);

	//v4lconvert_bayer_to_rgb24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);

	//v4lconvert_bayer_to_bgr24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);

	//v4lconvert_bayer_to_yuv420(const u8 *bayer, u8 *yuv, u32 width, u32 height, unsigned int src_pixfmt, int yvu);

	//v4lconvert_hm12_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

	//v4lconvert_hm12_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

	//v4lconvert_hm12_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);
	//TODO add other converters
};

static int v4lconvert_transformation_applyIMFSimple(struct v4lconvert_transformation* self, const u8* src, u8* dst) {
	v4lconvert_converter_t* converter = self->converter;
	
}

static int v4lconvert_transformation_apply(struct v4lconvert_transformation* self, const u8* src, u8* dst) {
	v4lconvert_converter_t* converter = &(v4lconvert_converters[self->converter_id]);
	
	switch (converter->signature) {
		case v4lconvert_conversion_signature_unset:
			//TODO maybe throw error or something
			return;
		case v4lconvert_conversion_signature_sdwh_0f:
			converter->target.cvt_sdwh_0f (src, dst, self->width, self->height);
			return;
		case v4lconvert_conversion_signature_sdwh_1f:
			converter->target.cvt_sdwh_1f (src, dst, self->width, self->height, converter->flag1);
			return;
		case v4lconvert_conversion_signature_sdwh_2f:
			converter->target.cvt_sdwh_2f (src, dst, self->width, self->height, converter->flag1, converter->flag2);
			return;
		case v4lconvert_conversion_signature_sd_sf_1f:
			converter->target.cvt_sd_sf_1f (src, dst, self->v4l_src_fmt, converter->flag1);
			return;
		case v4lconvert_conversion_signature_sd_sf_2f:
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

int v4lconvert_transformer_createBuffers(struct v4lconvert_transformer* transformer, u32 num_buffers, struct v4lconvert_buffer** buffers, int preserve_input, int doAllocate) {
	u32 bufA_len;
	u32 bufB_len;
	for (unsigned int i = 0; i < (transformer->num_transformations & ~1); i += 2) {
		u32 size = transformer->transformations[i]->dst_size;
		if (size > bufA_len)
			bufA_len = size;
		size = transformer->transformations[i + 1]->dst_size;
		if (size > bufB_len)
			bufB_len = size;
	}
	
	if (transformer->num_transformations & 1) {
		u32 tmp = bufB_len;
		u32 endsize = transformer->transformations[transformer->num_transformations - 1]->dst_size;
		bufB_len = bufA_len > endsize ? bufA_len : endsize;
		bufA_len = bufB_len;
	}
	
	u32 bufC_len = 0;
	if (preserve_input) {
		bufC_len = bufA_len;
		bufA_len = 0;
	}
	
	for (unsigned int i = 0; i < num_buffers; i++) {
		struct v4lconvert_buffer* buffer = buffers[i];
		if (!buffer)
			buffers[i] = buffer = malloc(sizeof(v4lconvert_buffer));
		buffer->preserve_input = preserve_input;
		buffer->buf0_len = bufA_len;
		buffer->buf1_len = bufB_len;
		buffer->buf2_len = bufC_len;
		if (doAllocate) {
			//TODO test if allocations failed
			buffer->buf0 = calloc(bufA_len, sizeof(u8*));
			buffer->buf1 = calloc(bufB_len, sizeof(u8*));
			buffer->buf2 = calloc(bufC_len, sizeof(u8*));
		}
	}
	return EXIT_SUCCESS;
}

v4lconvert_converter_t* v4lconvert_converter_getConverterById(unsigned int converterId) {
	return &(v4lconvert_converters[converterId]);
}

/**
 * 
 */
unsigned int v4lconvert_converter_lookupConverterByConversion(unsigned int from, unsigned int to) {
	int i;
	v4lconvert_converter_t* converter;
	
	//TODO use better lookup algorithm than O(n)
	for (i = 0; i < NUM_CONVERTERS; i++) {
		converter = &(v4lconvert_converters[i]);
		if ((converter->src_fmt == from) && (converter->dst_fmt == to))
			return i;
	}
	return -1;
}
/**
 * Find a converter that does the conversion that you want
 * @return the converter requested, or NULL
 */
v4lconvert_converter_t* v4lconvert_converter_getConverterByConversion(unsigned int from, unsigned int to) {
	int i;
	v4lconvert_converter_t* converter;
	
	//TODO use better lookup algorithm than O(n)
	for (i = 0; i < NUM_CONVERTERS; i++) {
		converter = &(v4lconvert_converters[i]);
		if ((converter->src_fmt == from) && (converter->dst_fmt == to))
			return converter;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif
