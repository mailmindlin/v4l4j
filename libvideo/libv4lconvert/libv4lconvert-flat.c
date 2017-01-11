#include <errno.h>
#include <stdlib.h>
#include <collections/collections.h>
#include <collections/queue/queue.h>
#include "libv4lconvert-flat.h"
#include "jpeg_memsrcdest.h"
#include "../libvideo-palettes.h"
#include "../types.h"
#include "../log.h"

#ifndef __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_CPP
#define __LIBVIDEO_LIBV4LCONVERT_LIBV4LCONVERT_FLAT_CPP

#ifdef __cplusplus
extern "C" {
#endif

#ifndef N_A
#define N_A 0
#endif

#ifndef UNUSED
//Macro to hide unused parameter warnings
#define UNUSED(x) (void)(x)
#endif
static bool estimateCostPlaceholder(ImageTransformerPrototype* self, unsigned int* cpuCost, float* qualityCost, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, size_t options_len, void** options) __attribute__ ((nonnull (1, 2, 3, 4, 5, 7)));

#define GENERATE_CONVERTER(id, initFn, applyFn, costFn, src_fmt, dst_fmt, signature, flag1, flag2) \
	{\
		.id = (id),\
		.init = (initFn),\
		.estimateCost = (costFn),\
		.type = v4lconvert_conversion_type_imf,\
		.src_fmt = (src_fmt),\
		.dst_fmt = (dst_fmt),\
		.imf_params = {\
			.signature = v4lconvert_conversion_signature_##signature,\
			.target = {.cvt_##signature = (applyFn)},\
			.flag1 = (flag1),\
			.flag2 = (flag2),\
		}\
	}

#define GENERATE_CONVERTER_SDWH_0F(id, applyFn, costFn, src_fmt, dst_fmt)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_sdwh, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_0f, false, false)

#define GENERATE_CONVERTER_SDWH_1F(id, applyFn, costFn, src_fmt, dst_fmt, flag1)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_sdwh, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_1f, (flag1), false)

#define GENERATE_CONVERTER_SDWH_2F(id, applyFn, costFn, src_fmt, dst_fmt, flag1, flag2)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_sdwh, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_2f, (flag1), (flag2))

#define GENERATE_CONVERTER_SD_SF_0F(id, applyFn, costFn, src_fmt, dst_fmt)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_sd_sf, (applyFn), (costFn), (src_fmt), (dst_fmt), sd_sf_0f, false, false)

#define GENERATE_CONVERTER_SD_SF_1F(id, applyFn, costFn, src_fmt, dst_fmt, flag1)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_sd_sf, (applyFn), (costFn), (src_fmt), (dst_fmt), sd_sf_1f, (flag1), false)

#define GENERATE_CONVERTER_SD_SF_2F(id, applyFn, costFn, src_fmt, dst_fmt, flag1, flag2)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_sd_sf, (applyFn), (costFn), (src_fmt), (dst_fmt), sd_sf_2f, (flag1), (flag2))

#define GENERATE_CONVERTER_SPECIAL(id, src_fmt, dst_fmt, flag1, flag2) {(id), v4lconvert_conversion_signature_special, {.user_defined = NULL}, v4lconvert_conversion_type_unknown, (src_fmt), (dst_fmt), (flag1), (flag2)}

#define GENERATE_CONVERTER_SDWH_1F_x2(id, applyFn, costFn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SDWH_1F(id    , (applyFn), (costFn), (src_fmt_0), (dst_fmt_0), false),\
	GENERATE_CONVERTER_SDWH_1F(id + 1, (applyFn), (costFn), (src_fmt_1), (dst_fmt_1), true)

#define GENERATE_CONVERTER_SD_SF_1F_x2(id, applyFn, costFn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SD_SF_1F(id    , (applyFn), (costFn), (src_fmt_0), (dst_fmt_0), false),\
	GENERATE_CONVERTER_SD_SF_1F(id + 1, (applyFn), (costFn), (src_fmt_1), (dst_fmt_1), true)

#define GENERATE_CONVERTER_SD_SF_2F_x4(id, applyFn, costFn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SD_SF_2F(id    , (applyFn), (costFn), (src_fmt_0), (dst_fmt_0), false, false),\
	GENERATE_CONVERTER_SD_SF_2F(id + 1, (applyFn), (costFn), (src_fmt_1), (dst_fmt_0), true,  false),\
	GENERATE_CONVERTER_SD_SF_2F(id + 2, (applyFn), (costFn), (src_fmt_0), (dst_fmt_1), false, true),\
	GENERATE_CONVERTER_SD_SF_2F(id + 3, (applyFn), (costFn), (src_fmt_1), (dst_fmt_1), true,  true)

static v4lconvert_converter_t* v4lconvert_init_imf_sdwh(ImageTransformerPrototype* self, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, size_t options_len, void* options, char** errmsg);
static v4lconvert_converter_t* v4lconvert_init_imf_sd_sf(ImageTransformerPrototype* self, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, size_t options_len, void* options, char** errmsg);

static ImageTransformerPrototype const ImageTransformerPrototypes[][] = {
	[v4lconvert_conversion_type_identity] = {
		
	},
	[v4lconvert_conversion_type_imf] = {
		GENERATE_CONVERTER_SD_SF_2F_x4(0, v4lconvert_rgb24_to_yuv420, estimateCostPlaceholder, RGB24,  BGR24, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(4,  v4lconvert_yuv420_to_rgb24, estimateCostPlaceholder, YUV420, YVU420, RGB24, RGB24),
		GENERATE_CONVERTER_SDWH_1F_x2(6,  v4lconvert_yuv420_to_bgr24, estimateCostPlaceholder, YUV420, YVU420, BGR24, BGR24),
		GENERATE_CONVERTER_SDWH_0F(8,     v4lconvert_yuyv_to_rgb24,   estimateCostPlaceholder, YUYV,   RGB24),
		GENERATE_CONVERTER_SDWH_0F(9,     v4lconvert_yuyv_to_bgr24,   estimateCostPlaceholder, YUYV,   BGR24),
		GENERATE_CONVERTER_SDWH_1F_x2(10, v4lconvert_yuyv_to_yuv420,  estimateCostPlaceholder, YUYV,   YUYV, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_0F(12,    v4lconvert_yvyu_to_rgb24,   estimateCostPlaceholder, YUYV,   RGB24),
		GENERATE_CONVERTER_SDWH_0F(13,    v4lconvert_yvyu_to_bgr24,   estimateCostPlaceholder, YUYV,   BGR24),
		GENERATE_CONVERTER_SDWH_0F(14,    v4lconvert_uyvy_to_rgb24,   estimateCostPlaceholder, UYVY,   RGB24),
		GENERATE_CONVERTER_SDWH_0F(15,    v4lconvert_uyvy_to_bgr24,   estimateCostPlaceholder, UYVY,   BGR24),
		GENERATE_CONVERTER_SDWH_1F_x2(16, v4lconvert_uyvy_to_yuv420,  estimateCostPlaceholder, UYVY,   UYVY, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_0F(18,    v4lconvert_swap_rgb,        estimateCostPlaceholder, RGB24,  BGR24),
		GENERATE_CONVERTER_SDWH_0F(19,    v4lconvert_swap_rgb,        estimateCostPlaceholder, BGR24,  RGB24),
		GENERATE_CONVERTER_SD_SF_0F(20,   v4lconvert_swap_uv,         estimateCostPlaceholder, YUV420, YVU420),//TODO figure out if this is right...
		GENERATE_CONVERTER_SD_SF_0F(21,   v4lconvert_swap_uv,         estimateCostPlaceholder, YVU420, YUV420),
		GENERATE_CONVERTER_SDWH_0F(22,    v4lconvert_grey_to_rgb24,   estimateCostPlaceholder, GREY,   RGB24),
		GENERATE_CONVERTER_SD_SF_0F(23,   v4lconvert_grey_to_yuv420,  estimateCostPlaceholder, GREY,   YUV420),
		GENERATE_CONVERTER_SDWH_0F(24,    v4lconvert_rgb565_to_rgb24, estimateCostPlaceholder, RGB565, RGB24),
		GENERATE_CONVERTER_SDWH_0F(25,    v4lconvert_rgb565_to_bgr24, estimateCostPlaceholder, RGB565, BGR24),
		GENERATE_CONVERTER_SD_SF_1F_x2(26,v4lconvert_rgb565_to_yuv420,       estimateCostPlaceholder, RGB565,       RGB565,       YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(27, v4lconvert_spca501_to_yuv420,      estimateCostPlaceholder, SPCA501,      SPCA501,      YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(28, v4lconvert_spca505_to_yuv420,      estimateCostPlaceholder, SPCA505,      SPCA505,      YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(32, v4lconvert_spca508_to_yuv420,      estimateCostPlaceholder, SPCA508,      SPCA508,      YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(34, v4lconvert_cit_yyvyuy_to_yuv420,   estimateCostPlaceholder, CIT_YYVYUY,   CIT_YYVYUY,   YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(36, v4lconvert_konica_yuv420_to_yuv420,estimateCostPlaceholder, KONICA420,    KONICA420,    YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(38, v4lconvert_m420_to_yuv420,         estimateCostPlaceholder, M420,         M420,         YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_1F_x2(40, v4lconvert_sn9c20x_to_yuv420,      estimateCostPlaceholder, SN9C20X_I420, SN9C20X_I420, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_0F(42,    v4lconvert_decode_sn9c10x,  estimateCostPlaceholder, SN9C10X,  SBGGR8),
		GENERATE_CONVERTER_SDWH_0F(43,    v4lconvert_decode_sn9c2028, estimateCostPlaceholder, SN9C2028, SBGGR8),
		GENERATE_CONVERTER_SDWH_0F(44,    v4lconvert_decode_sq905c,   estimateCostPlaceholder, SQ905C,   SRGGB8),
		GENERATE_CONVERTER_SDWH_0F(45,    v4lconvert_decode_stv0680,  estimateCostPlaceholder, STV0680,  SRGGB8),
		GENERATE_CONVERTER_SDWH_0F(46,    v4lconvert_grey_to_rgb24,   estimateCostPlaceholder, GREY,     RGB24),
		GENERATE_CONVERTER_SDWH_0F(47,    v4lconvert_hm12_to_bgr24,   estimateCostPlaceholder, HM12,     BGR24),
		GENERATE_CONVERTER_SDWH_1F_x2(48, v4lconvert_hm12_to_yuv420,  estimateCostPlaceholder, HM12,     HM12, YUV420, YVU420),
#define GENERATE_CONVERTER_JPEG(id, costFn, src_fmt)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_jpeg, (costFn), (src_fmt), JPEG, special, false, false)
		//Virtual JPEG converters. Can't be used, but are placeholders because encoders can support them.
		GENERATE_CONVERTER_JPEG(50,       estimateCostPlaceholder, GREY),
		GENERATE_CONVERTER_JPEG(51,       estimateCostPlaceholder, RGB24),
		GENERATE_CONVERTER_JPEG(52,       estimateCostPlaceholder, YUV420),
		GENERATE_CONVERTER_JPEG(53,       estimateCostPlaceholder, YUYV),
		GENERATE_CONVERTER_JPEG(54,       estimateCostPlaceholder, YVYU),
		GENERATE_CONVERTER_JPEG(55,       estimateCostPlaceholder, UYVY),
		GENERATE_CONVERTER_JPEG(56,       estimateCostPlaceholder, VYUY),
#undef GENERATE_CONVERTER_JPEG
		//v4lconvert_y10b_to_rgb24(struct v4lconvert_data *data, const u8 *src, u8 *dest, u32 width, u32 height);
		//v4lconvert_y10b_to_yuv420(struct v4lconvert_data *data, const u8 *src, u8 *dest, u32 width, u32 height);
		//v4lconvert_cpia1_to_yuv420(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dst, u32 width, u32 height, int yvu);
		//v4lconvert_se401_to_rgb24(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dest, u32 width, u32 height);
		//v4lconvert_decode_jpeg_tinyjpeg(struct v4lconvert_data *data, u8 *src, int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt, int flags);
		//v4lconvert_decode_jpeg_libjpeg(struct v4lconvert_data *data, u8 *src, int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt);
		//v4lconvert_decode_jpgl(const u8 *src, u32 src_size, unsigned int dest_pix_fmt, u8 *dest, u32 width, u32 height);
		//v4lconvert_decode_spca561(const u8 *src, u8 *dst, u32 width, u32 height);
		//v4lconvert_decode_pac207(struct v4lconvert_data *data, const u8 *inp, int src_size, u8 *outp, u32 width, u32 height);
		//v4lconvert_decode_mr97310a(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dst, u32 width, u32 height);
		//v4lconvert_decode_jl2005bcd(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dest, u32 width, u32 height);
		//v4lconvert_bayer_to_rgb24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);
		//v4lconvert_bayer_to_bgr24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);
		//v4lconvert_bayer_to_yuv420(const u8 *bayer, u8 *yuv, u32 width, u32 height, unsigned int src_pixfmt, int yvu);
	}
};
#define COUNT_PROTOTYPES(type) [v4lconvert_conversion_type_##type] = ARRAY_SIZE(ImageTransformerPrototypes[v4lconvert_conversion_type_##type])
static const size_t v4lconvert_converter_num_prototypes[] = {
		COUNT_PROTOTYPES(identity),
		COUNT_PROTOTYPES(imf),
		COUNT_PROTOTYPES(crop),
		COUNT_PROTOTYPES(pad),
		COUNT_PROTOTYPES(scale),
		COUNT_PROTOTYPES(rotate),
		COUNT_PROTOTYPES(rotate90),
		COUNT_PROTOTYPES(rotate180),
		COUNT_PROTOTYPES(hflip),
		COUNT_PROTOTYPES(vflip)
	};
#undef COUNT_PROTOTYPES

static unsigned int VideoPipeline_apply(VideoPipeline* self, struct v4lconvert_buffer* buffer) __attribute__ ((nonnull (1, 2)));
static bool VideoPipeline_release(VideoPipeline* self) __attribute__ ((nonnull (1)));
//static bool VideoPipeline_releaseSelfOnly(VideoPipeline* self) __attribute__ ((nonnull (1)));
//static bool VideoPipeline_releaseConvertersOnly(VideoPipeline* self) __attribute__ ((nonnull (1)));
//static bool VideoPipeline_releaseSelfAndConverters(VideoPipeline* self) __attribute__ ((nonnull (1)));

static unsigned int ImageTransformer_applyIMF(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len);
static bool ImageTransformer_releaseIMF(ImageTransformer* self);

static unsigned int ImageTransformer_encodePixelJPEG(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len);
static unsigned int ImageTransformer_encodePlanarJPEG(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len);
static bool ImageTransformer_releaseJPEG(ImageTransformer* self);


//Placeholder for estimateCost methods
static bool estimateCostPlaceholder(ImageTransformerPrototype* self, unsigned int* cpuCost, float* qualityCost, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, size_t options_len, void** options) {
	*cpuCost = 10;
	*qualityCost = .5;
	return true;
}

static unsigned int ImageTransformer_applyIMF(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len) {
	UNUSED(src_len);
	v4lconvert_converter_t* converter = self->converter;
	switch (converter->signature) {
		case v4lconvert_conversion_signature_sdwh_0f:
			(*converter->target.cvt_sdwh_0f)(src, dst, self->src_width, self->src_height);
			break;
		case v4lconvert_conversion_signature_sdwh_1f:
			(*converter->target.cvt_sdwh_1f)(src, dst, self->src_width, self->src_height, converter->flag1);
			break;
		case v4lconvert_conversion_signature_sdwh_2f:
			(*converter->target.cvt_sdwh_2f)(src, dst, self->src_width, self->src_height, converter->flag1, converter->flag2);
			break;
		case v4lconvert_conversion_signature_sd_sf_0f:
			converter->target.cvt_sd_sf_0f(src, dst, self->imf_v4l2_src_fmt);
			break;
		case v4lconvert_conversion_signature_sd_sf_1f:
			converter->target.cvt_sd_sf_1f (src, dst, self->imf_v4l2_src_fmt, converter->flag1);
			break;
		case v4lconvert_conversion_signature_sd_sf_2f:
			converter->target.cvt_sd_sf_2f (src, dst, self->imf_v4l2_src_fmt, converter->flag1, converter->flag2);
			break;
		default:
			return 0;
	}
	return self->dst_len;
}

static bool v4lconvert_encoder_releaseIMF(ImageTransformer* self) {
	switch (self->prototype->signature) {
		case v4lconvert_conversion_signature_sdwh_0f:
		case v4lconvert_conversion_signature_sdwh_1f:
		case v4lconvert_conversion_signature_sdwh_2f:
			//Nothing to release
			return true;
		case v4lconvert_conversion_signature_sd_sf_0f:
		case v4lconvert_conversion_signature_sd_sf_1f:
		case v4lconvert_conversion_signature_sd_sf_2f:
			if (self->imf_v4l2_src_fmt) {
				free(self->imf_v4l2_src_fmt);
				self->imf_v4l2_src_fmt = NULL;
			}
			return true;
		default:
			return false;
	}
}

/**
 * v4lconvert_encoder::apply method for encoding pixel formats to JPEG.
 * e.g, RGB or GREY
 */
static size_t v4lconvert_encoder_encodePixelJPEG(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len) {
	struct jpeg_compress_struct* cinfo = self->params.jpeg_encode_params.cinfo;
	if (!cinfo)
		return 0;
	jpeg_set_quality(cinfo, self->params.jpeg_encode_params.quality, TRUE);
	
	// Configure the output to write to the destination buffer
	unsigned long dst_lencpy = (unsigned long) self->dst_len;
	jpeg_mem_dest(cinfo, &dst, &dst_lencpy);
	
	jpeg_start_compress(cinfo, TRUE);
	
	const JSAMPLE* row_ptr;
	const u32 row_stride  = self->params.jpeg_encode_params.row_stride;
	const u32 height = self->src_height;
	#ifdef SANITY_CHECK
		u32 max_scanline = src_len / row_stride;
	#else
		UNUSED(src_len);
	#endif
	while(cinfo->next_scanline < height) {
		#ifdef SANITY_CHECK
			if (cinfo->next_scanline > max_scanline) {
				jpeg_abort_compress(cinfo);
				return 0;
			}
		#endif
		row_ptr = src + cinfo->next_scanline * row_stride;
		jpeg_write_scanlines(cinfo, (JSAMPARRAY) &row_ptr, 1);
	}
	
	//Finish compressing the JPEG
	jpeg_finish_compress(cinfo);
	
	// Calculate the length of the resulting JPEG
	return self->dst_len - cinfo->dest->free_in_buffer;
}

static size_t v4lconvert_encoder_encodePlanarJPEG(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len) {
	struct jpeg_compress_struct* cinfo = self->params.jpeg_encode_params.cinfo;
	if (!cinfo)
		return 0;
	
	jpeg_set_quality(cinfo, self->params.jpeg_encode_params.quality, TRUE);
	
	// Configure the output to write to the destination buffer
	unsigned long dst_lencpy = (unsigned long) self->dst_len;
	jpeg_mem_dest(cinfo, &dst, &dst_lencpy);
	
	jpeg_start_compress(cinfo, TRUE);
	
	//TODO fix
	
	jpeg_finish_compress(cinfo);
	return 0;
}

static bool ImageTransformer_releaseJPEG(ImageTransformer* self) {
	if (self->params.jpeg_encode_params.cinfo) {
		jpeg_destroy_compress(self->params.jpeg_encode_params.cinfo);
		free(self->params.jpeg_encode_params.cinfo);
		self->params.jpeg_encode_params.cinfo = NULL;
	}
	if (self->params.jpeg_encode_params.cerr) {
		free(self->params.jpeg_encode_params.cerr);
		self->params.jpeg_encode_params.cerr = NULL;
	}
	return true;
}

size_t v4lconvert_estimateBufferSize(u32 fmt, unsigned int width, unsigned int height) {
	unsigned int pixels = width * height;
	switch (fmt) {
		case RGB32:
		case BGR32:
			return pixels * 4;
		case RGB24:
		case BGR24:
		case YUV444: //12 bytes per 4 pixels
			return pixels * 3;
		case YUV422:
		case YUV422P:
		case YUYV:
		case YYUV:
		case YVYU:
		case UYVY:
		case VYUY:
		case NV16:
		case NV61:
		case RGB555:
		case RGB555X:
		case RGB565:
		case RGB565X:
		case SBGGR10:
		case SGBRG10:
		case SGRBG10:
		case SRGGB10:
		case SBGGR16:
		case Y16:
			//8 bytes per 4 pixels
			return pixels * 2;
		case NV12:
		case NV21:
		case RGB444:
		case YUV411:
		case YUV420:
		case YVU420:
			//6 bytes per 4 pixels
			return pixels * 3 / 2;
		case Y10:
			return pixels * 5 / 4;
		case PAL8:
			return 256 + pixels * 1;
		case GREY:
		case RGB332:
		case SBGGR8:
		case SGBRG8:
		case SGRBG8:
		case SRGGB8:
		case SGRBG10DPCM8:
			return pixels * 1;
		case Y4:
			return (pixels + 1) / 2;
		default:
			//Estimate big. Really big.
			return pixels * 4;
	}
}

int v4lconvert_encoder_initWithConverter(struct v4lconvert_encoder* encoder, v4lconvert_converter_t* converter, u32 width, u32 height) {
	encoder->converter = converter;
	encoder->src_fmt = converter->src_fmt;
	encoder->dst_fmt = converter->dst_fmt;
	encoder->src_width = width;
	encoder->src_height = height;
	//Not necessarily true, but the default
	encoder->dst_width = width;
	encoder->dst_height = height;
	
	switch (converter->signature) {
		case v4lconvert_conversion_signature_unset:
			break;
		case v4lconvert_conversion_signature_sdwh_0f:
			encoder->apply = v4lconvert_encoder_applyIMF_sdwh;
			encoder->release = v4lconvert_encoder_releaseIMF;
			break;
		case v4lconvert_conversion_signature_sd_sf_0f:
		case v4lconvert_conversion_signature_sd_sf_1f:
		case v4lconvert_conversion_signature_sd_sf_2f:
			encoder->apply = v4lconvert_encoder_applyIMF_sd_sf;
			encoder->release = v4lconvert_encoder_releaseIMF;
			if (!(encoder->imf_v4l2_src_fmt = malloc(sizeof(struct v4l2_format))))
				return false;
			//TODO finish
			switch (encoder->imf_v4l2_src_fmt->type) {
				case V4L2_BUF_TYPE_VIDEO_CAPTURE:
					encoder->imf_v4l2_src_fmt->fmt.pix.width = width;
					encoder->imf_v4l2_src_fmt->fmt.pix.height = height;
					break;
				case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
					encoder->imf_v4l2_src_fmt->fmt.pix_mp.width = width;
					encoder->imf_v4l2_src_fmt->fmt.pix_mp.height = height;
					break;
				case V4L2_BUF_TYPE_VIDEO_OVERLAY:
				case V4L2_BUF_TYPE_VBI_CAPTURE:
				case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
					//Not sure how to handle these
				default:
					return false;
			}
			break;
		case v4lconvert_conversion_signature_special:
			//This gets tricky
			if (converter->dst_fmt == JPEG) {
				//JPEG encoder
				encoder->apply = v4lconvert_encoder_encodePixelJPEG;
				encoder->release = v4lconvert_encoder_releaseJPEG;
				
				//Default to quality of 100%
				encoder->jpeg_encode_params.quality = 100;
				
				//Create jpeg encoder
				if (!(encoder->jpeg_encode_params.cinfo = malloc(sizeof(struct jpeg_compress_struct))))
					return false;
				
				if (!(encoder->jpeg_encode_params.cerr = malloc(sizeof(struct jpeg_error_mgr)))) {
					free(encoder->jpeg_encode_params.cinfo);
					return false;
				}
				
				struct jpeg_compress_struct* cinfo = encoder->jpeg_encode_params.cinfo;
				//Initialize the error buffer
				cinfo->err = jpeg_std_error(encoder->jpeg_encode_params.cerr);
				//Initialize the compressor
				jpeg_create_compress(cinfo);
				//Set the image dimensions
				cinfo->image_width = width;
				cinfo->image_height = height;
				switch (converter->src_fmt) {
					case GREY:
						cinfo->input_components = 1;
						encoder->jpeg_encode_params.row_stride = width;
						cinfo->in_color_space = JCS_GRAYSCALE;
						jpeg_set_defaults(cinfo);
						break;
					case RGB24:
						cinfo->input_components = 3;
						encoder->jpeg_encode_params.row_stride = width * 3;
						cinfo->in_color_space = JCS_RGB;
						jpeg_set_defaults(cinfo);
						break;
					case YUV420:
					case YUYV:
					case YVYU:
					case UYVY:
					case VYUY:
						//Different apply method
						encoder->apply = v4lconvert_encoder_encodePlanarJPEG;
						cinfo->input_components = 3;
						encoder->jpeg_encode_params.row_stride = width * 3;
						jpeg_set_defaults(cinfo);
						jpeg_set_colorspace(cinfo, JCS_YCbCr);
						cinfo->raw_data_in = TRUE; // Supply downsampled data
						cinfo->comp_info[0].v_samp_factor = converter->src_fmt == YUV420 ? 2 : 1;
						cinfo->comp_info[0].h_samp_factor = 2;
						// cinfo->comp_info[0].v_samp_factor set below depending on source format
						cinfo->comp_info[1].h_samp_factor = 1;
						cinfo->comp_info[1].v_samp_factor = 1;
						cinfo->comp_info[2].h_samp_factor = 1;
						cinfo->comp_info[2].v_samp_factor = 1;
						return false;
					default:
						//TODO support more formats (esp. YUV420)
						return false;
				}
			} else {
				return false;
			}
			break;
		default:
			return false;
	}
	encoder->src_len = v4lconvert_estimateBufferSize(converter->src_fmt, width, height);
	encoder->dst_len = v4lconvert_estimateBufferSize(converter->dst_fmt, width, height);
	return true;
}

static unsigned int binaryGcd(unsigned int a, unsigned int b) {
	if (a == 0)
		return b;
	if (b == 0)
		return a;
	int shift;
	for (shift = 0; ((a | b) & 1) == 0; shift++) {
		a >>= 1;
		b >>= 1;
	}
	while ((a & 1) == 0)
		a >>= 1;
	do {
		while ((b & 1) == 0)
			b >>= 1;
		if (a > b) {
			unsigned int tmp = a;
			a = b;
			b = tmp;
		}
		v -= u;
	} while (v);
	return u << shift;
}

static ImageTransformerPrototype* lookupPrototype(v4lconvert_conversion_type type, u32 src_fmt) {
	const size_t max = v4lconvert_converter_num_prototypes[type];
	ImageTransformerPrototype* prototypes = ImageTransformerPrototypes[type];
	//Linear search
	for (unsigned int i = 0; i < max, i++) {
		ImageTransformerPrototype* prototype = &prototypes[i];
		if (prototype->src_fmt == current_fmt)
			return prototype;
	}
	return NULL;
}

typedef struct converter_node {
	/**
	 * Total cost to get to the current node.
	 * 0 if undefined (infinity)
	 */
	size_t cpuCost;
	float quality;
	u32 fmt;
	unsigned int flags;
	struct converter_node* prev;
	ImageTransformerPrototype* prototype;
} converter_node;

#define QUALITY_CPU_FACTOR 10000

static int converterNodeComparator(void* a, void* b) {
	converter_node* nodeA = (converter_node*) a;
	converter_node* nodeB = (converter_node*) b;
	//Lower cost has higher priority
	return (b->cost + (1 - b->quality) * QUALITY_CPU_FACTOR) - (a->cost + (1 - a->quality) * QUALITY_CPU_FACTOR);
}

bool getDataFromV4L2Format(struct v4l2_format* data, u32* fmt, unsigned int* width, unsigned int* height) {
	switch(data->type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			*fmt = data->fmt.pix.pixelformat;
			*width = data->fmt.pix.width;
			*height = data->fmt.pix.height;
			return true;
		case V4L2_BUF_TYPE_VIDEO_OVERLAY:
			//TODO get format
			*width = data->fmt.win.w.width;
			*height = data->fmt.win.w.height;
			return true;
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
			*fmt = data->fmt.pix_mp.pixelformat;
			*width = data->fmt.pix_mp.width;
			*height = data->fmt.pix_mp.height;
			return true;
		//Not sure how to deal with these
		case V4L2_BUF_TYPE_VBI_CAPTURE:
		case V4L2_BUF_TYPE_VBI_OUTPUT:
		case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
		//Undefined behavior of below
		case V4L2_BUF_TYPE_VBI_OUTPUT:
		case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		case V4L2_BUF_TYPE_PRIVATE:
			//TODO finish
		default:
			return false;
	}
}

/**
 * Populate a v4l2_format structure.
 */
static bool createV4L2Format(struct v4l2_format* data, u32 fmt, unsigned int width, unsigned int height) {
	//Map paletteID from libvideo to v4l2
	unsigned int v4l2Fmt = libvideo_palettes[fmt].v4l2_palette;
	
	switch (fmt) {
		case RGB332:
		case RGB444:
		case RGB555:
		case RGB565:
			//Pixel format
			data->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			data->fmt.pix.width = width;
			data->fmt.pix.height = height;
			data->fmt.pix.pixelformat = v4l2Fmt;
			return true;
		case YUV422P:
		case YUV411P:
		case Y41P:
			data->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			//TODO finish
			return true;
	}
	return false;
}

static size_t countNodesInPath(node* last) {
	size_t len = 1;
	node* tmp = last;
	while ((tmp = tmp->prev) != NULL)
		len++;
	return len;
}

/**
 * Traverse the nodes backwards and build an array of transformers
 */
static size_t VideoPipeline_buildTransformersFromPath(node* lastNode, ImageTransformer*** transformers, v4lconvert_conversion_request* request, unsigned int srcWidth, unsigned int srcHeight, unsigned int scaledWidth, unsigned int scaledHeight, unsigned int dstWidth, unsigned int dstHeight, char** errmsg) {
	//Count # of nodes in path
	size_t pathLength = countNodesInPath(lastNode);
	
	ImageTransformer** result = (ImageTransformer**) calloc(pathLength, sizeof(ImageTransformer*));
	if (result == NULL) {
		errno = ENOMEM;
		return 0;
	}
	
	node* current = lastNode;
	do {
		ImageTransformerPrototype* proto = current->prototype;
		ImageTransformer* transformer;
		switch (proto->type) {
			case v4lconvert_conversion_type_rotate:
				transformer = proto->init(proto, srcFmt, dstFmt, errmsg, 1, request->rotation);
				break;
			case v4lconvert_conversion_type_scale:
				transformer = proto->init(proto, srcFmt, dstFmt, errmsg, 2, request->scaleNumerator, request->scaleDenominator);
				break;
			case v4lconvert_conversion_type_hflip:
			case v4lconvert_conversion_type_vflip:
			case v4lconvert_conversion_type_identity:
			case v4lconvert_conversion_type_imf:
				transformer = proto->init(proto, srcFmt, dstFmt, errmsg, 0);
				break;
			default:
				//Unknown type
		}
		if (transformer == NULL) {
			//TODO cleanup
			return 0;
		}
	} while ((current = current->prev) != NULL);
	
	*transformers = result;
	return pathLength;
}

static size_t VideoPipeline_computeConverters(ImageTransformer*** transformers, struct v4lconvert_conversion_request* request, char** errmsg) {
	#define FAIL(msg) do {\
			if (errmsg) \
				*errmsg = msg;\
		} while (0);
	unsigned int rotation;
	{
		int _rotation = request->rotation % 360;
		rotation = (unsigned) (_rotation > 0 ? _rotation : (_rotation + 360));
	}
	
	bool flipHorizontal = request->flipHorizontal;
	bool flipVertical = request->flipVertical;
	
	if ((request->rotation % 180) == 0 && flipHorizontal && flipVertical) {
		request->rotation = (request->rotation + 180) % 180;
		flipHorizontal = false;
		flipVertical = false;
	}
	
	dprint(LIBVIDEO_SOURCE_CONVERT, LIBVIDEO_LOG_DEBUG, "Final rotation: %u\n", rotation);
	
	unsigned int scaleNumerator = request->scaleNumerator;
	unsigned int scaleDenominator = request->scaleDenominator;
	{
		//Reduce scale numerator/denominator
		unsigned int gcd = binaryGcd(scaleNumerator, scaleDenominator);
		scaleNumerator /= gcd;
		scaleDenominator /= gcd;
		if (scaleNumerator == 0 || scaleDenominator == 0) {
			FAIL("Scalar must be nonzero");
			errno = EINVAL;
			return 0;
		}
	}
	
	u32 src_fmt, dst_fmt;
	unsigned int src_width, dst_width, src_height, dst_height;
	if (!getDataFromV4L2Format(&request->src_fmt, &src_fmt, &src_width, &src_height) || !getDataFromV4L2Format(&request->dst_fmt, &dst_fmt, &dst_width, &dst_height)) {
		FAIL("Unable to understand format");
		errno = EINVAL;
		return 0;
	}
	
	//Calculate the size of the image after scaling
	unsigned int scaled_width = src_width * scaleNumerator / scaleDenominator;
	unsigned int scaled_height = src_height * scaleNumerator / scaleDenominator
	
	
	//Compute crop/pad offsets, in pixels from the edge (going in)
	//Cropping is applied AFTER scaling.
	signed int top_offset = request->top_offset;
	signed int left_offset = request->left_offset;
	signed int right_offset = dst_width - ((signed) scaled_width - left_offset);
	signed int bottom_offset = dst_height - ((signed) scaled_height - top_offset);
	
	
	//Create flags. This is to save space (memory), b/c we won't need all of them every time
	unsigned int shift = 0;
	const unsigned int closed_mask = 1;
	
	unsigned int rotate_mask = 0;
	if (rotation != 0)
		rotate_mask = 1 << ++shift;
	
	unsigned int scale_mask = 0;
	if (scaleNumerator != scaleDenominator)
		scale_mask = 1 << ++shift;
	
	unsigned int crop_mask = 0;
	if (top_offset || left_offset || right_offset || bottom_offset)
		crop_mask = 1 << ++shift;
	
	unsigned int hflip_mask = 0;
	if (flipHorizontal)
		hflip_mask = 1 << ++shift;
	
	unsigned int vflip_mask = 0;
	if (flipVertical)
		vflip_mask = 1 << ++shift;
	
	//Sanity check for odd processors with small word sizes
	if (sizeof(unsigned int) * 8 < shift) {
		//We have too many flags to fit in a word.
		FAIL("Word size is too small");
		return 0;
	}
	
	if (shift == 0 && src_fmt == dst_fmt) {
		//No transformations needed. Find an identity method
		dprint(LIBVIDEO_SOURCE_CONVERT, LIBVIDEO_LOG_DEBUG, "Finding identity transformation\n");
		ImageTransformerPrototype* identityPrototype = lookupPrototype(v4lconvert_conversion_type_identity, request->src_fmt, request->dst_fmt);
		if (!identityPrototype) {
			FAIL("Unable to lookup identity prototype");
			return 0;
		}
		ImageTransformer* identityConverter = identityPrototype->init(identityPrototype, request->src_fmt, request->dst_fmt, errmsg, 0);
		if (identityConverter == NULL)
			//Error message passed by init()
			return 0;
		*converters = calloc(1, sizeof(ImageTransformer**));
		if (*converters == NULL) {
			FAIL("Error allocating memory");
			errno = ENOMEM;
			return 0;
		}
		(*converters)[0] = identityConverter;
		return 1;
	}
	
	//Let's do an A* search now
	
	//Number of tiers needed
	const size_t numTiers = 1 << shift;
	//Size of each tier (# of nodes/tier)
	const size_t tierSize = libvideo_palettes_size;
	
	node** closedSet = calloc(numTiers * tierSize, sizeof(node*));
	
	//Get the origin node
	node* startNode = malloc(sizeof(node));
	startNode->cost = 1;
	startNode->fmt = src_fmt;
	startNode->flags = rotate_mask | scale_mask | crop_mask | hflip_mask | vflip_mask;
	startNode->prototype = NULL;
	closedSet[startNode->flags * tierSize + src_fmt] = startNode;
	
	Queue openSet;
	InitRelativePriorityQueue(&openSet, PairingRPQ, &converterNodeComparator);
	openSet.push(&openSet, (void*) startNode);
	
	//Whether anything happened in this iteration
	while (!openSet.isEmpty(&openSet)) {
		//Pop a node from the queue
		node* current = openSet.pop(&openSet);
		
		unsigned int currentTier = current->flags & ~closed_mask;
		if (currentTier == 0 && current->fmt == dst_fmt) {
			//We found a viable (good) path
			
			
			*transformers = result;
			return pathLength;
		}
		
		//Calculate what the format should be passing through this node
		struct v4l2_format currentSourceFormat = request->src_fmt;
		
		unsigned int currentSourceWidth, currentSourceHeight;
		if (currentTier & scale_mask) {
			//The image hasn't been scaled or cropped (yet), but will be
			currentSourceWidth = src_width;
			currentSourceHeight = src_height;
		} else if (currentTier & crop_mask) {
			//We can assume that the image has been scaled so far
			currentSourceWidth = scaled_width;
			currentSourceHeight = scaled_height;
		} else {
			currentSourceWidth = dst_width;
			currentSourceHeight = dst_height;
		}
		createV4L2Format(&currentSourceFormat, current->fmt, currentSourceWidth, currentSourceHeight);
		
		if (currentTier & rotate_mask) {
			//Look for rotations
			const unsigned int newTier = currentTier & ~rotate_mask;
			const size_t newTierOffset = newTier * tierSize;
			
			ImageTransformerPrototype* prototypes = ImageTransformerPrototype[v4lconvert_conversion_type_rotate];
			
			for (unsigned int i = 0; i < v4lconvert_converter_num_prototypes[v4lconvert_conversion_type_rotate], i++) {
				ImageTransformerPrototype* prototype = &prototypes[i];
				if (prototype->src_fmt != current_fmt)
					continue;
				
				//Create destination format
				struct v4l2_format targetFormat = currentSourceFormat;
				createV4L2Format(&targetFormat, prototype->dst_fmt, currentSourceWidth, currentSourceHeight);
				
				//Lookup cost of conversion
				unsigned int cpuCost;
				float quality;
				if (!prototype->estimateCost(prototype, currentSourceFormat, cpuCost, quality, currentSourceFormat, targetFormat, 1, rotation))
					continue;
				const unsigned int newCpuCost = current->cpuCost + cpuCost;
				const float newQuality = current->quality * quality;
				
				//Find/create node
				
				node* target = closedSet[newTierOffset + prototype->dst_fmt];
				if (target == NULL) {
					//No target existed before
					target = malloc(sizeof(node));
					target->fmt = prototype->dst_fmt;
					closedSet[newTierOffset + prototype->dst_fmt] = target;
				} else {
					if (target->cpuCost + (1 - target->quality) * QUALITY_CPU_FACTOR < newCpuCost + (1 - newQuality) * QUALITY_CPU_FACTOR)
						//The current path to that node was more efficient
						continue;
				}
				target->cpuCost = newCpuCost;
				target->quality = newQuality;
				target->flags = newTier;//Note that this clears the closed_mask bit
				target->prev = current;
				target->prototype = prototype;
				
				//Add to queue
				//TODO decrease-key operation
				openSet.push(&openSet, target);
			}
		} else {
			//Look for flips
			if (currentTier & hflip_mask) {
				const unsigned int newTier = currentTier & ~hflip_mask;
				size_t newTierOffset = newTier * tierSize;
				
				ImageTransformerPrototype* prototypes = ImageTransformerPrototype[v4lconvert_conversion_type_hflip];
				
				for (unsigned int i = 0; i < v4lconvert_converter_num_prototypes[v4lconvert_conversion_type_hflip], i++) {
					ImageTransformerPrototype* prototype = &prototypes[i];
					if (prototype->src_fmt == current_fmt) {
						node* target = &nodes[newTierOffset + prototyper->dst_fmt];
						target->cost = current->cost + prototype->estimateCost(prototype, ...);
						target->prev = current_idx;
					}
				}
			}
			if (currentTier & vflip_mask) {
				size_t newTierOffset = (currentTier & ~vflip_mask) * tierSize;
				ImageTransformerPrototype* prototypes = ImageTransformerPrototype[v4lconvert_conversion_type_rotate];
				for (unsigned int i = 0; i < v4lconvert_converter_num_prototypes[v4lconvert_conversion_type_vflip], i++) {
					ImageTransformerPrototype* prototype = &prototypes[i];
					if (prototype->src_fmt == current_fmt) {
						node* target = &nodes[newTierOffset + prototyper->dst_fmt];
						target->cost = current->cost + prototype->estimateCost(prototype, ...);
						target->prev = current_idx;
					}
				}
			}
			/*
			if ((currentTier & hflip_mask) && (currentTier & vflip_mask)) {
				//180deg rotation is the same as hflip + vflip
				size_t newTierOffset = (currentTier & ~hflip_mask & ~vflip_mask) * tierSize;
				for (unsigned int i = 0; i < v4lconvert_converter_num_prototypes[v4lconvert_conversion_type_rotate180], i++) {
					ImageTransformerPrototype* prototype = &ImageTransformerPrototypes[v4lconvert_conversion_type_rotate180][i];
					if (prototype->src_fmt == current_fmt) {
						node* target = &nodes[newTierOffset + prototyper->dst_fmt];
						target->cost = current->cost + prototype->estimateCost(prototype, ...);
						target->prev = current_idx;
					}
				}
			}*/
		}
		if (currentTier & scale_mask) {
			
		} else if (currentTier & crop_mask) {
			
		}
		//Look for IMF conversions
		
	}
	
	//TODO finish
	
	return 0;
	#undef FAIL
}


bool VideoPipeline_create(VideoPipeline* self, struct v4lconvert_conversion_request* request, char** errmsg) {
	size_t num_converters = v4lconvert_encoder_series_computeConverters(&self->converters, request, errmsg);
	if (num_converters == 0)
		return false;
	self->num_converters = num_converters;
	self->convert = &VideoPipeline_apply;
	self->release = &VideoPipeline_doRelease;
	self->src_fmt = self->converters[0]->prototype->src_fmt;
	self->dst_fmt = self->converters[num_converters - 1]->prototype->dst_fmt;
	//TODO finish src_len/dst_len
	return true;
}

static bool VideoPipeline_doRelease(VideoPipeline* self) {
	if (self != NULL) {
		if (self->encoders) {
			for (unsigned i = 0; i < self->num_encoders; i++) {
				self->encoders[i]->release(self->encoders[i]);
				free(self->encoders[i]);
			}
			free(self->encoders);
			self->encoders = NULL;
		}
	}
	return true;
}

static size_t VideoPipeline_apply(VideoPipeline* self, struct v4lconvert_buffer* buffer) {
	ImageTransformer* encoder = self->encoders[0];
	//We have to set up the buffers such that the data ends up being written into
	//the dst buffer.
	u8* bufA;
	u8* bufB;
	if (self->num_encoders & 1) {
		bufA = buffer->buf1;
		bufB = buffer->buf2;
	} else {
		bufA = buffer->buf2;
		bufB = buffer->buf1;
	}
	u32 src_len = buffer->buf0_len;
	//Apply the first encoder, copying the data from buf0 to bufA
	if ((src_len = encoder->apply(encoder, buffer->buf0, bufA, src_len)) == 0)
		return 0;
	for (unsigned int i = 1; i < (self->num_encoders & ~1u); i++) {
		//Apply a pair of encoders; this ensures that at the end of this loop,
		//the data will be written to bufA
		encoder = self->encoders[i];
		if ((src_len = encoder->apply(encoder, bufA, bufB, src_len)) == 0)
			return 0;
		encoder = self->encoders[++i];
		if ((src_len = encoder->apply(encoder, bufB, bufA, src_len)) == 0)
			return 0;
	}
	if (self->num_encoders & 1)
		return src_len;
	//Apply the last encoder (this is if there's an even number of encoders), writing to
	//the output buffer
	encoder = self->encoders[self->num_encoders - 1];
	return buffer->buf1_len = encoder->apply(encoder, bufA, buffer->buf1, src_len);
}

bool VideoPipeline_createBuffers(VideoPipeline* series, u32 num_buffers, struct v4lconvert_buffer** buffers, bool allocate) {
	u32 bufA_len = 0;
	u32 bufB_len = 0;
	for (unsigned int i = 0; i < (series->num_encoders / 2) * 2; i += 2) {
		u32 size = series->encoders[i]->dst_len;
		if (size > bufB_len)
			bufB_len = size;
		size = series->encoders[i + 1]->dst_len;
		if (size > bufA_len)
			bufA_len = size;
	}
	
	if (series->num_encoders & 1) {
		u32 tmp = bufA_len;
		u32 endsize = series->encoders[series->num_encoders - 1]->dst_len;
		bufA_len = bufB_len > endsize ? bufB_len : endsize;
		bufB_len = tmp;
	}
	
	for (unsigned i = 0; i < num_buffers; i++) {
		struct v4lconvert_buffer* buffer = buffers[i];
		if (!buffer) {
			if (allocate)
				buffers[i] = buffer = malloc(sizeof(struct v4lconvert_buffer));
			else
				return false;
		}
		
		buffer->buf0_len = 0;
		buffer->buf1_len = 0;
		buffer->buf0_cap = series->encoders[0]->src_len;
		buffer->buf1_cap = buffer->buf1_len = bufA_len;
		buffer->buf2_cap = bufB_len;
		if (allocate) {
			//Don't allocate buf0, because it might be used by the 
			buffer->buf1 = calloc(buffer->buf1_cap, sizeof(u8*));
			buffer->buf2 = calloc(buffer->buf2_cap, sizeof(u8*));
			if (!buffer->buf1 || !buffer->buf2) {
				free(buffer->buf1);
				free(buffer->buf2);
				buffer->buf1 = NULL;
				buffer->buf2 = NULL;
				//Zero out the remaining buffers
				for (unsigned j = i + 1; j < num_buffers; j++) {
					buffer = buffers[j];
					if (!buffer)
						continue;
					buffer->buf0_len = 0;
					buffer->buf1_len = 0;
					buffer->buf0_cap = 0;
					buffer->buf1_cap = 0;
					buffer->buf2_cap = 0;
					buffer->buf0 = NULL;
					buffer->buf1 = NULL;
					buffer->buf2 = NULL;
				}
				return false;
			}
		}
	}
	return true;
}

bool v4lconvert_buffer_release(struct v4lconvert_buffer* buffer) {
	free(buffer->buf0);
	buffer->buf0 = NULL;
	buffer->buf0_len = 0;
	buffer->buf0_cap = 0;
	
	free(buffer->buf1);
	buffer->buf1 = NULL;
	buffer->buf1_len = 0;
	buffer->buf1_cap = 0;
	
	free(buffer->buf2);
	buffer->buf2 = NULL;
	buffer->buf2_cap = 0;
	
	return true;
}

static inline bool computeEncoderPath(unsigned int* map, unsigned int* distances, u32 from, u32 to, unsigned int maxIterations) {
	distances[to] = 1;
	// Variation of Dijkstra's Algorithm, where each distance is 1
	// Works backwards from the 'to' format to the 'from' format
	for (unsigned int i = 0; i < maxIterations; i++) {
		int progress = 0;
		for (unsigned int j = 0; j < NUM_V4L_CONVERTERS; j++) {
			v4lconvert_converter_t* converter = &(v4lconvert_converters[j]);
			unsigned int distanceTo = distances[converter->dst_fmt];
			if (distanceTo < 1)
				continue;
			if (distances[converter->src_fmt] == 0) {
				// There was no path found to this node previously
				map[converter->src_fmt] = i;
				distances[converter->src_fmt] = distanceTo + 1;
				progress = 1;
				if (converter->src_fmt == from)
					return true;
			}
		}
		// If no progress has been made this iteration, we can conclude that no path exists
		if (!progress)
			return false;
	}
	// We took up too many iterations (the shortest path was too long)
	return false;
}

int v4lconvert_encoder_series_computeConversion(struct v4lconvert_encoder_series* self, u32 width, u32 height, u32 from, u32 to, unsigned int maxIterations) {
	if (from > NB_SUPPORTED_PALETTES || to > NB_SUPPORTED_PALETTES)
		return EXIT_FAILURE;
	unsigned int* map = calloc(NB_SUPPORTED_PALETTES, sizeof(unsigned int));
	unsigned int* distances = calloc(NB_SUPPORTED_PALETTES, sizeof(unsigned int));
	if (map == NULL || distances == NULL || !computeEncoderPath(map, distances, from, to, maxIterations)) {
		free(map);
		free(distances);
		return EXIT_FAILURE;
	}
	//Walk backwards along the map, building the a single path
	unsigned int distance = distances[from] - 1;
	unsigned int* route = calloc(distance, sizeof(unsigned int));
	for (unsigned int i = 0, fmt = from; i < distance; i++) {
		route[i] = map[fmt];
		fmt = v4lconvert_converters[fmt].dst_fmt;
	}
	free(distances);
	free(map);
	
	#if defined(DEBUG) || true
		dprint(LIBVIDEO_LOG_SOURCE, LIBVIDEO_LOG_DEBUG, "Computed conversion path from %d to %d (len %d): %d", from, to, distance, route[0]);
		for (unsigned i = 1; i < distance; i++)
			dprint(LIBVIDEO_LOG_SOURCE, LIBVIDEO_LOG_DEBUG, " => %d", route[i]);
		dprint(LIBVIDEO_LOG_SOURCE, LIBVIDEO_LOG_DEBUG, "\nFormats: %s", libvideo_palettes[route[0]].name);
		for (unsigned i = 0; i < distance; i++)
			dprint(LIBVIDEO_LOG_SOURCE, LIBVIDEO_LOG_DEBUG, " => %s", libvideo_palettes[route[i]].name);
		dprint(LIBVIDEO_LOG_SOURCE, LIBVIDEO_LOG_DEBUG, "\n");
	#endif
	
	//Now initialize the encoder series with the route just computed
	int result = v4lconvert_encoder_series_init(self, width, height, distance, route);
	free(route);
	return result;
}

#ifdef __cplusplus
}
#endif

#endif
