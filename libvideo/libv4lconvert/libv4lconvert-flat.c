
#include <stdlib.h>
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

#define GENERATE_CONVERTER_SD_SF_0F(id, fn, src_fmt, dst_fmt) {(id), v4lconvert_conversion_fingerprint_sd_sf_0f, {.cvt_sd_sf_0f = (fn)}, (src_fmt), (dst_fmt), N_A, N_A}

#define GENERATE_CONVERTER_SD_SF_1F(id, fn, src_fmt, dst_fmt, flag1) {(id), v4lconvert_conversion_fingerprint_sd_sf_1f, {.cvt_sd_sf_1f = (fn)}, (src_fmt), (dst_fmt), (flag1), N_A}

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

static int v4lconvert_encoder_doConvert(struct v4lconvert_encoder* self, const u8* src, u8* dst);
static int v4lconvert_encoder_estimateResultSize(struct v4lconvert_encoder* self);
static int v4lconvert_encoder_series_doConvert(struct v4lconvert_encoder_series* self, const u8* src, u8* dst);
static inline int computeEncoderPath(u32* map, u32* distances, u32 from, u32 to, u32 maxIterations);


int v4lconvert_encoder_init(struct v4lconvert_encoder* encoder, unsigned int converterId, unsigned int width, unsigned int height) {
	encoder->convert = &v4lconvert_encoder_doConvert;
	encoder->estimate_result_size = &v4lconvert_encoder_estimateResultSize;
	if (converterId > NUM_CONVERTERS)
		return EXIT_FAILURE;
	encoder->converter = &(v4lconvert_converters[converterId]);
	encoder->src_fmt = encoder->converter->src_fmt;
	encoder->dst_fmt = encoder->converter->dst_fmt;
	encoder->width = width;
	encoder->height = height;
	return EXIT_SUCCESS;
}

static int v4lconvert_encoder_doConvert(struct v4lconvert_encoder* self, const u8* src, u8* dst) {
	v4lconvert_converter_t* converter = self->converter;
	
	switch (converter->fingerprint) {
		case v4lconvert_conversion_fingerprint_unset:
			//TODO maybe throw error or something
			break;
		case v4lconvert_conversion_fingerprint_sdwh_0f:
			converter->target.cvt_sdwh_0f (src, dst, self->width, self->height);
			break;
		case v4lconvert_conversion_fingerprint_sdwh_1f:
			converter->target.cvt_sdwh_1f (src, dst, self->width, self->height, converter->flag1);
			break;
		case v4lconvert_conversion_fingerprint_sdwh_2f:
			converter->target.cvt_sdwh_2f (src, dst, self->width, self->height, converter->flag1, converter->flag2);
			break;
		case v4lconvert_conversion_fingerprint_sd_sf_1f:
			converter->target.cvt_sd_sf_1f (src, dst, self->v4l_src_fmt, converter->flag1);
			break;
		case v4lconvert_conversion_fingerprint_sd_sf_2f:
			converter->target.cvt_sd_sf_2f (src, dst, self->v4l_src_fmt, converter->flag1, converter->flag2);
			break;
		default:
			break;
	}
	
	//TODO better results
	return v4lconvert_encoder_estimateResultSize(self);
}

static int v4lconvert_encoder_estimateResultSize(struct v4lconvert_encoder* self) {
	int pixels = self->width * self->height;
	switch (self->dst_fmt) {
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

int v4lconvert_encoder_series_init(struct v4lconvert_encoder_series* self, u32 width, u32 height, u32 numConverters, u32* converterIds) {
	self->convert = &v4lconvert_encoder_series_doConvert;
	self->num_encoders = numConverters;
	self->num_buffers = numConverters - 1;
	if (numConverters == 0)
		return EXIT_FAILURE;
	
	self->encoders = calloc(numConverters, sizeof(struct v4lconvert_encoder));
	self->buffers = calloc(self->num_buffers, sizeof(void*));
	for (unsigned int i = 0; i < numConverters; i++) {
		struct v4lconvert_encoder* encoder = &(self->encoders[i]);
		if (!v4lconvert_encoder_init(encoder, converterIds[i], width, height)) {
			//Initialization failure
			v4lconvert_encoder_series_doRelease(self);
			return EXIT_FAILURE;
		}
		if (i != numConverters - 1) {
			u32 buffer_size = (*encoder->estimate_result_size)(encoder);
			if (!(self->buffers[i] = malloc(buffer_size))) {
				// Memory allocation failure
				v4lconvert_encoder_series_doRelease(self);
				return EXIT_FAILURE;
			}
		}
	}
	
	self->src_fmt = self->encoders[0].src_fmt;
	self->dst_fmt = self->encoders[numConverters - 1].dst_fmt;
	return EXIT_SUCCESS;
}

int v4lconvert_encoder_series_doRelease(struct v4lconvert_encoder_series* self) {
	if (self != NULL) {
		if (self->encoders != NULL) {
			free(self->encoders);
			self->encoders = NULL;
		}
		if (self->buffers != NULL) {
			for (unsigned int i = 0; i < self->num_buffers; i++)
				free(self->buffers[i]);
			free(self->buffers);
			self->buffers = NULL;
		}
	}
	return EXIT_SUCCESS;
}

static int v4lconvert_encoder_series_doConvert(struct v4lconvert_encoder_series* self, const u8* src, u8* dst) {
	const u8* origin = src;
	for (unsigned int i = 0; i < self->num_encoders - 1; i++) {
		struct v4lconvert_encoder* encoder = &(self->encoders[i]);
		u8* result = self->buffers[i];
		if (((*encoder->convert)(encoder, origin, result)) == 0)
			return 0;
		origin = result;
	}
	//The last encoder writes to the dst buffer
	struct v4lconvert_encoder* encoder = &(self->encoders[self->num_encoders - 1]);
	return (*encoder->convert)(encoder, origin, dst);
}

static inline int computeEncoderPath(unsigned int* map, unsigned int* distances, u32 from, u32 to, unsigned int maxIterations) {
	distances[to] = 1;
	// Variation of Dijkstra's Algorithm, where each distance is 1
	// Works backwards from the 'to' format to the 'from' format
	for (unsigned int i = 0; i < maxIterations; i++) {
		int progress = 0;
		for (int j = 0; j < NUM_CONVERTERS; j++) {
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
					return EXIT_SUCCESS;
			}
		}
		// If no progress has been made this iteration, we can conclude that no path exists
		if (!progress)
			return EXIT_FAILURE;
	}
	// We took up too many iterations (the shortest path was too long)
	return EXIT_FAILURE;
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
	
	//Now initialize the encoder series with the route just computed
	int result = v4lconvert_encoder_series_init(self, width, height, distance, route);
	free(route);
	return result;
}

v4lconvert_converter_t* v4lconvert_converter_getConverterById(unsigned int converterId) {
	return &(v4lconvert_converters[converterId]);
}

/**
 * 
 */
int v4lconvert_converter_lookupConverterByConversion(unsigned int from, unsigned int to) {
	v4lconvert_converter_t* converter;
	
	//TODO use better lookup algorithm than O(n)
	for (int i = 0; i < NUM_CONVERTERS; i++) {
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
	//TODO use better lookup algorithm than O(n)
	for (int i = 0; i < NUM_CONVERTERS; i++) {
		v4lconvert_converter_t* converter = &(v4lconvert_converters[i]);
		if ((converter->src_fmt == from) && (converter->dst_fmt == to))
			return converter;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif
