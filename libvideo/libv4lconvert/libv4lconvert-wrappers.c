#include <errno.h>
#include "libv4lconvert-wrappers.h"

static bool getDataFromV4L2Format(struct v4l2_format* data, u32* fmt, unsigned int* width, unsigned int* height) {
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

static void initCommonStuff(ImageTransformer* transformer, ImageTransformerPrototype* self, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt) {
	transformer->src_fmt = self->src_fmt;
	transformer->dst_fmt = self->dst_fmt;
	u32 scratch;
	getDataFromV4L2Format(src_fmt, scratch, &transformer->src_width, &transformer->src_height);
	getDataFromV4L2Format(dst_fmt, scratch, &transformer->dst_width, &transformer->dst_height);
	transformer->src_len = v4lconvert_estimateBufferSize(self->src_fmt, transformer->src_width, transformer->src_height);
	transformer->dst_len = v4lconvert_estimateBufferSize(self->dst_fmt, transformer->dst_width, transformer->dst_height);
	transformer->prototype = self;
}

static size_t ImageTransformer_imf_apply(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len) {
	UNUSED(src_len);
	ImageTransformerPrototype* proto = self->prototype;
	const unsigned int bytesPerLine = self->params.imf_params.bytesPerLine;
	const unsigned int width = self->src_width;
	const unsigned int height = self->src_height;
	switch (proto->signature) {
		case v4lconvert_conversion_signature_sdwh:
			proto->target.cvt_sdwh_0f(src, dst, width, height);
			break;
		case v4lconvert_conversion_signature_sdwh_f:
			proto->target.cvt_sdwh_1f(src, dst, width, height, proto->flag1);
			break;
		case v4lconvert_conversion_signature_sdwh_ff:
			proto->target.cvt_sdwh_2f(src, dst, width, height, proto->flag1, proto->flag2);
			break;
		case v4lconvert_conversion_signature_sdwh_b:
			proto->target.cvt_sdwh_b(src, dst, width, height, bytesPerLine);
			break;
		case v4lconvert_conversion_signature_sdwh_bf:
			proto->target.cvt_sdwh_bf(src, dst, width, height, bytesPerLine, proto->flag1);
			break;
		case v4lconvert_conversion_signature_sdwh_bff:
			proto->target.cvt_sdwh_bff(src, dst, width, height, bytesPerLine, proto->flag1, proto->flag2);
			break;
		default:
			return 0;
	}
	return self->dst_len;
}

static bool ImageTransformer_imf_release(ImageTransformer* self) {
	free(self);
	return true;
}

ImageTransformer* ImageTransformer_imf_init(ImageTransformerPrototype* self, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, char** errmsg, size_t options_len, ...) {
	ImageTransformer* result = malloc(sizeof(ImageTransformer));
	if (result == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	
	initCommonStuff(result, self, src_fmt, dst_fmt);
	
	result->apply = &ImageTransformer_imf_apply;
	result->release = &ImageTransformer_imf_release;
	return result;
}


/**
 * v4lconvert_encoder::apply method for encoding pixel formats to JPEG.
 * e.g, RGB or GREY
 */
static size_t ImageTransformer_encodePixelJPEG(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len) {
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

static size_t ImageTransformer_encodePlanarJPEG(ImageTransformer* self, const u8* src, size_t src_len, u8* dst, size_t dst_len) {
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

static bool ImageTransformer_jpeg_release(ImageTransformer* self) {
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

ImageTransformer* ImageTransformer_jpeg_init(ImageTransformerPrototype* self, struct v4l2_format* src_fmt, struct v4l2_format* dst_fmt, char** errmsg, size_t options_len, ...) {
	ImageTransformer* result = malloc(sizeof(ImageTransformer));
	if (result == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	
	initCommonStuff(result, self, src_fmt, dst_fmt);
	
	//JPEG encoder
	result->release = ImageTransformer_jpeg_release;
	
	//Default to quality of 100%
	result->jpeg_encode_params.quality = 100;
	
	//Create jpeg encoder
	if (!(result->jpeg_encode_params.cinfo = malloc(sizeof(struct jpeg_compress_struct)))) {
		errno = ENOMEM;
		free(result->jpeg_encode_params.cinfo);
		free(result);
		return false;
	}
	
	struct jpeg_compress_struct* cinfo = result->params.jpeg_encode_params.cinfo;
	//Initialize the error buffer
	cinfo->err = jpeg_std_error(result->params.jpeg_encode_params.cerr);
	//Initialize the compressor
	jpeg_create_compress(cinfo);
	//Set the image dimensions
	cinfo->image_width = width;
	cinfo->image_height = height;
	switch (converter->src_fmt) {
		case GREY:
			result->apply = ImageTransformer_encodePixelJPEG;
			cinfo->input_components = 1;
			result->params.jpeg_encode_params.row_stride = width;
			cinfo->in_color_space = JCS_GRAYSCALE;
			jpeg_set_defaults(cinfo);
			return result;
		case RGB24:
			result->apply = ImageTransformer_encodePixelJPEG;
			cinfo->input_components = 3;
			result->jpeg_encode_params.row_stride = width * 3;
			cinfo->in_color_space = JCS_RGB;
			jpeg_set_defaults(cinfo);
			return result;
		case YUV420:
		case YUYV:
		case YVYU:
		case UYVY:
		case VYUY:
			//Different apply method
			result->apply = v4lconvert_encoder_encodePlanarJPEG;
			cinfo->input_components = 3;
			result->jpeg_encode_params.row_stride = width * 3;
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
			return result;
		default:
			//TODO support more formats (esp. YUV420)
			return NULL;
	}
}