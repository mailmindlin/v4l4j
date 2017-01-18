#include "libv4lconvert-flat.h"
#include "libv4lconvert-wrappers.h"
#include "libv4lconvert-prototypes.h"
#include <collections/iterator/iterator.h>
#include <collections/list/list.h>

//Placeholder for estimateCost methods
static bool estimateCostPlaceholder(ImageTransformerPrototype* self, unsigned int* cpuCost, float* qualityCost, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, size_t options_len, void** options) {
	*cpuCost = 10;
	*qualityCost = .5;
	return true;
}

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

#define GENERATE_CONVERTER_SDWH(id, applyFn, costFn, src_fmt, dst_fmt)\
	GENERATE_CONVERTER((id), ImageTransformer_imf_init, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh, false, false)

#define GENERATE_CONVERTER_SDWH_F(id, applyFn, costFn, src_fmt, dst_fmt, flag1)\
	GENERATE_CONVERTER((id), ImageTransformer_imf_init, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_f, (flag1), false)

#define GENERATE_CONVERTER_SDWH_FF(id, applyFn, costFn, src_fmt, dst_fmt, flag1, flag2)\
	GENERATE_CONVERTER((id), ImageTransformer_imf_init, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_ff, (flag1), (flag2))

#define GENERATE_CONVERTER_SDWH_B(id, applyFn, costFn, src_fmt, dst_fmt, flag1, flag2)\
	GENERATE_CONVERTER((id), ImageTransformer_imf_init, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_b, false, false)

#define GENERATE_CONVERTER_SDWH_BF(id, applyFn, costFn, src_fmt, dst_fmt, flag1, flag2)\
	GENERATE_CONVERTER((id), ImageTransformer_imf_init, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_bf, (flag1), false)

#define GENERATE_CONVERTER_SDWH_BFF(id, applyFn, costFn, src_fmt, dst_fmt, flag1, flag2)\
	GENERATE_CONVERTER((id), ImageTransformer_imf_init, (applyFn), (costFn), (src_fmt), (dst_fmt), sdwh_bff, (flag1), false)

#define GENERATE_CONVERTER_SDWH_F_x2(id, applyFn, costFn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SDWH_F(id    , (applyFn), (costFn), (src_fmt_0), (dst_fmt_0), false),\
	GENERATE_CONVERTER_SDWH_F(id + 1, (applyFn), (costFn), (src_fmt_1), (dst_fmt_1), true)

#define GENERATE_CONVERTER_SDWH_BFF_x4(id, applyFn, costFn, src_fmt_0, src_fmt_1, dst_fmt_0, dst_fmt_1) \
	GENERATE_CONVERTER_SDWH_BFF(id    , (applyFn), (costFn), (src_fmt_0), (dst_fmt_0), false, false),\
	GENERATE_CONVERTER_SDWH_BFF(id + 1, (applyFn), (costFn), (src_fmt_1), (dst_fmt_0), true,  false),\
	GENERATE_CONVERTER_SDWH_BFF(id + 2, (applyFn), (costFn), (src_fmt_0), (dst_fmt_1), false, true),\
	GENERATE_CONVERTER_SDWH_BFF(id + 3, (applyFn), (costFn), (src_fmt_1), (dst_fmt_1), true,  true)

ImageTransformerPrototype const ImageTransformerPrototypes[][] = {
	[v4lconvert_conversion_type_identity] = {
		
	},
	[v4lconvert_conversion_type_imf] = {
		GENERATE_CONVERTER_SDWH_BFF_x4(0, v4lconvert_rgb24_to_yuv420, estimateCostPlaceholder, RGB24,  BGR24, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(4,  v4lconvert_yuv420_to_rgb24, estimateCostPlaceholder, YUV420, YVU420, RGB24, RGB24),
		GENERATE_CONVERTER_SDWH_F_x2(6,  v4lconvert_yuv420_to_bgr24, estimateCostPlaceholder, YUV420, YVU420, BGR24, BGR24),
		GENERATE_CONVERTER_SDWH(8,     v4lconvert_yuyv_to_rgb24,   estimateCostPlaceholder, YUYV,   RGB24),
		GENERATE_CONVERTER_SDWH(9,     v4lconvert_yuyv_to_bgr24,   estimateCostPlaceholder, YUYV,   BGR24),
		GENERATE_CONVERTER_SDWH_F_x2(10, v4lconvert_yuyv_to_yuv420,  estimateCostPlaceholder, YUYV,   YUYV, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH(12,    v4lconvert_yvyu_to_rgb24,   estimateCostPlaceholder, YUYV,   RGB24),
		GENERATE_CONVERTER_SDWH(13,    v4lconvert_yvyu_to_bgr24,   estimateCostPlaceholder, YUYV,   BGR24),
		GENERATE_CONVERTER_SDWH(14,    v4lconvert_uyvy_to_rgb24,   estimateCostPlaceholder, UYVY,   RGB24),
		GENERATE_CONVERTER_SDWH(15,    v4lconvert_uyvy_to_bgr24,   estimateCostPlaceholder, UYVY,   BGR24),
		GENERATE_CONVERTER_SDWH_F_x2(16, v4lconvert_uyvy_to_yuv420,  estimateCostPlaceholder, UYVY,   UYVY, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH(18,    v4lconvert_swap_rgb,        estimateCostPlaceholder, RGB24,  BGR24),
		GENERATE_CONVERTER_SDWH(19,    v4lconvert_swap_rgb,        estimateCostPlaceholder, BGR24,  RGB24),
		GENERATE_CONVERTER_SDWH_B(20,   v4lconvert_swap_uv,         estimateCostPlaceholder, YUV420, YVU420),//TODO figure out if this is right...
		GENERATE_CONVERTER_SDWH_B(21,   v4lconvert_swap_uv,         estimateCostPlaceholder, YVU420, YUV420),
		GENERATE_CONVERTER_SDWH(22,    v4lconvert_grey_to_rgb24,   estimateCostPlaceholder, GREY,   RGB24),
		GENERATE_CONVERTER_SDWH(23,   v4lconvert_grey_to_yuv420,  estimateCostPlaceholder, GREY,   YUV420),
		GENERATE_CONVERTER_SDWH(24,    v4lconvert_rgb565_to_rgb24, estimateCostPlaceholder, RGB565, RGB24),
		GENERATE_CONVERTER_SDWH(25,    v4lconvert_rgb565_to_bgr24, estimateCostPlaceholder, RGB565, BGR24),
		GENERATE_CONVERTER_SDWH_BF_x2(26,v4lconvert_rgb565_to_yuv420,       estimateCostPlaceholder, RGB565,       RGB565,       YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(27, v4lconvert_spca501_to_yuv420,      estimateCostPlaceholder, SPCA501,      SPCA501,      YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(28, v4lconvert_spca505_to_yuv420,      estimateCostPlaceholder, SPCA505,      SPCA505,      YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(32, v4lconvert_spca508_to_yuv420,      estimateCostPlaceholder, SPCA508,      SPCA508,      YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(34, v4lconvert_cit_yyvyuy_to_yuv420,   estimateCostPlaceholder, CIT_YYVYUY,   CIT_YYVYUY,   YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(36, v4lconvert_konica_yuv420_to_yuv420,estimateCostPlaceholder, KONICA420,    KONICA420,    YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(38, v4lconvert_m420_to_yuv420,         estimateCostPlaceholder, M420,         M420,         YUV420, YVU420),
		GENERATE_CONVERTER_SDWH_F_x2(40, v4lconvert_sn9c20x_to_yuv420,      estimateCostPlaceholder, SN9C20X_I420, SN9C20X_I420, YUV420, YVU420),
		GENERATE_CONVERTER_SDWH(42,    v4lconvert_decode_sn9c10x,  estimateCostPlaceholder, SN9C10X,  SBGGR8),
		GENERATE_CONVERTER_SDWH(43,    v4lconvert_decode_sn9c2028, estimateCostPlaceholder, SN9C2028, SBGGR8),
		GENERATE_CONVERTER_SDWH(44,    v4lconvert_decode_sq905c,   estimateCostPlaceholder, SQ905C,   SRGGB8),
		GENERATE_CONVERTER_SDWH(45,    v4lconvert_decode_stv0680,  estimateCostPlaceholder, STV0680,  SRGGB8),
		GENERATE_CONVERTER_SDWH(46,    v4lconvert_grey_to_rgb24,   estimateCostPlaceholder, GREY,     RGB24),
		GENERATE_CONVERTER_SDWH(47,    v4lconvert_hm12_to_bgr24,   estimateCostPlaceholder, HM12,     BGR24),
		GENERATE_CONVERTER_SDWH_F_x2(48,v4lconvert_hm12_to_yuv420, estimateCostPlaceholder, HM12,     HM12, YUV420, YVU420),
#define GENERATE_CONVERTER_JPEG(id, costFn, src_fmt)\
	GENERATE_CONVERTER((id), v4lconvert_init_imf_jpeg, (costFn), (src_fmt), JPEG, special, false, false)
		//Virtual JPEG converters. Can't be used, but are placeholders because encoders can support them.
		GENERATE_CONVERTER_JPEG(50,    estimateCostPlaceholder, GREY),
		GENERATE_CONVERTER_JPEG(51,    estimateCostPlaceholder, RGB24),
		GENERATE_CONVERTER_JPEG(52,    estimateCostPlaceholder, YUV420),
		GENERATE_CONVERTER_JPEG(53,    estimateCostPlaceholder, YUYV),
		GENERATE_CONVERTER_JPEG(54,    estimateCostPlaceholder, YVYU),
		GENERATE_CONVERTER_JPEG(55,    estimateCostPlaceholder, UYVY),
		GENERATE_CONVERTER_JPEG(56,    estimateCostPlaceholder, VYUY),
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
const size_t v4lconvert_converter_num_prototypes[] = {
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

static List imageTransformerRegistry = {0};

unsigned int ImageTransformerPrototype_register(ImageTransformerPrototype* prototype) {
	enum v4lconvert_conversion_type type = prototype->type;
	if (type >= v4lconvert_num_conversion_types) {
		errno = EINVAL;
		return -1u;
	}
	if (imageTransformerRegistry.type == 0)
		ArrayList_create(&imageTransformerRegistry);
	return imageTransformerRegistry.add(&imageTransformerRegistry, prototype);
}

ImageTransformerPrototype* ImageTransformerPrototype_lookupById(unsigned int id) {
	return NULL;
}