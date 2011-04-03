/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include "common.h"
#include "debug.h"
#include "libvideo.h"
#include "pixfmt-conv.h"

#define DHT_SIZE		420
static uint8_t huffman_table[] =
	{0xFF,0xC4,0x01,0xA2,0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01
        ,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02
        ,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x01,0x00,0x03
        ,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00
        ,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09
        ,0x0A,0x0B,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05
        ,0x05,0x04,0x04,0x00,0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04
        ,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22
        ,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15
        ,0x52,0xD1,0xF0,0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17
        ,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,0x35,0x36
        ,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A
        ,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66
        ,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A
        ,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95
        ,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8
        ,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2
        ,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5
        ,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7
        ,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9
        ,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05
        ,0x04,0x04,0x00,0x01,0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04
        ,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22
        ,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33
        ,0x52,0xF0,0x15,0x62,0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25
        ,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A,0x35,0x36
        ,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A
        ,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66
        ,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A
        ,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94
        ,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7
        ,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA
        ,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4
        ,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7
        ,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA};

/* Dummy JPEG methods */
static void init_destination( j_compress_ptr cinfo ){}
static boolean empty_output_buffer( j_compress_ptr cinfo ){return TRUE;}
static void term_destination( j_compress_ptr cinfo ){}

static void jpeg_encode_jpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	memcpy(dst, src, d->capture_len);
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->capture_len);
	d->len = d->capture_len;
}

static void jpeg_encode_mjpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	uint32_t has_dht = 0, ptr = 0, size;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	if(src[0]!=0xff && src[1]!=0xD8) {
		dprint(LOG_JPEG, "[JPEG] Invalid JPEG frame\n");
		return;
	}

	dprint(LOG_JPEG, "[JPEG] Adding Huffman tables\n");
	memcpy(dst,src,2);
	ptr += 2;

	while(!has_dht) {
		if(src[ptr]!=0xFF) {
			dprint(LOG_JPEG, "[JPEG] Invalid JPEG frame\n");
			return;
		}

		if(src[ptr+1] == 0xC4)
			has_dht=1;
		else if (src[ptr+1] == 0xDA)
			break;

		size = (src[ptr+2] << 8) + src[ptr+3];
		memcpy((dst+ptr), (src+ptr), 2+size);
		ptr += (2+size);
	}

	if(!has_dht) {
		memcpy((dst+ptr), huffman_table, DHT_SIZE);
		memcpy((dst+ptr+DHT_SIZE), (src+ptr), (d->capture_len-ptr));
		ptr += (DHT_SIZE+d->capture_len-ptr);
	} else {
		memcpy((dst+ptr), (src+ptr), (d->capture_len-ptr));
		ptr += (d->capture_len-ptr);
	}

	dprint(LOG_JPEG, "[JPEG] Frame now has %d bytes\n", ptr);
	d->len = ptr;
}

static void jpeg_encode_yuv420(struct v4l4j_device *d, unsigned char *src, unsigned char *dst) {
	//Code for this function is adapted from a similar one found in Motion
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	struct jpeg_compress_struct*	cinfo = d->j->cinfo;
	int 							i, line, rgb_size, width, height;

	width = d->vdev->capture->width;
	height = d->vdev->capture->height ;

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo,d->j->jpeg_quality,TRUE);

	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	jpeg_start_compress(cinfo, TRUE );
	for (line = 0; line < height; line += d->j->mcu) {
		// setup pointers in the JSAMPIMAGE array
		for (i = 0; i < d->j->mcu; i++) {
			d->j->y[i] = src + width * (i + line);
			if (i % 2 == 0) {
				d->j->cb[i/2] = src + width * height + width / 2 * ((i + line) / 2);
				d->j->cr[i/2] = d->j->cb[i/2] + width * height / 4;
			}
		}

		jpeg_write_raw_data(cinfo, d->j->data, d->j->mcu);
	}
	jpeg_finish_compress(cinfo);
	d->len = rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

static inline void jpeg_encode_yuv422p(struct v4l4j_device *d, unsigned char *dst){
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	uint32_t 					line = 0, i;
	uint32_t					width = d->vdev->capture->width;
	uint32_t					height = d->vdev->capture->height;

	// init JPEG dest mgr
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = d->vdev->capture->imagesize;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);

	jpeg_start_compress(cinfo, TRUE );

	while(line < d->vdev->capture->height){
		// setup pointers in the JSAMPIMAGE array
		for (i = 0; i < d->j->mcu; i++) {
			d->j->y[i] = d->conversion_buffer + width * (i + line);
			d->j->cb[i] = d->conversion_buffer + width * height + width * ((i + line) / 2);
			d->j->cr[i] = d->j->cb[i] + width * height / 2;
		}

		// pass the YUV planes to the jpeg compressor
		jpeg_write_raw_data(cinfo, d->j->data, d->j->mcu);

		// move on to next block of lines
		line += d->j->mcu;
	}
	jpeg_finish_compress (cinfo);
	d->len = d->vdev->capture->imagesize - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

static void jpeg_encode_yuyv(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	// reorganise YUYV pixels into YUV420 suitable to give to the jpeg compressor
	convert_yuyv_to_yuv422p_ssse3(src, d->conversion_buffer, d->vdev->capture->width, d->vdev->capture->height);

	jpeg_encode_yuv422p(d, dst);
}

static void jpeg_encode_uyvy(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	// reorganise UYVY pixels into YUV420 suitable to give to the jpeg compressor
	convert_uyvy_to_yuv422p(src, d->conversion_buffer, d->vdev->capture->width, d->vdev->capture->height);

	jpeg_encode_yuv422p(d, dst);
}

static void jpeg_encode_yvyu(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	// reorganise YVYU pixels into YUV420 suitable to give to the jpeg compressor
	convert_yvyu_to_yuv422p(src, d->conversion_buffer, d->vdev->capture->width, d->vdev->capture->height);

	jpeg_encode_yuv422p(d, dst);
}

static void jpeg_encode_rgb32(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	struct jpeg_compress_struct*	cinfo = d->j->cinfo;
	uint32_t 						width = d->vdev->capture->width;
	uint32_t						height = d->vdev->capture->height;
	uint32_t 						x, rgb_size = width * height * 3;
	JSAMPROW 						row[1] = {d->conversion_buffer};
	uint8_t*						ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = d->conversion_buffer;
			for (x = 0; x < width; x++) {
				*(ptr++) = src[0];
				*(ptr++) = src[1];
				*(ptr++) = src[2];
				src += 4;
			}
		jpeg_write_scanlines (cinfo, row, 1);
	}
	jpeg_finish_compress (cinfo);
	d->len =  rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

static void jpeg_encode_bgr32(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	struct jpeg_compress_struct*	cinfo = d->j->cinfo;
	uint32_t 						width = d->vdev->capture->width;
	uint32_t						height = d->vdev->capture->height;
	uint32_t						x, rgb_size = width * height * 3;
	JSAMPROW 						row[1] = {d->conversion_buffer};
	uint8_t*						ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = d->conversion_buffer;
			for (x = 0; x < width; x++) {
				*(ptr++) = src[2];
				*(ptr++) = src[1];
				*(ptr++) = src[0];
				src += 4;
			}
		jpeg_write_scanlines (cinfo, row, 1);
	}

	jpeg_finish_compress (cinfo);
	d->len =  rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}


static void jpeg_encode_rgb24(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	JSAMPROW 						row_ptr[1];
	struct jpeg_compress_struct*	cinfo = d->j->cinfo;
	uint32_t						width = d->vdev->capture->width;
	uint32_t 						height = d->vdev->capture->height ;
	uint32_t						rgb_size = width * height * 3, stride = width * 3, bytes=0;

	//init JPEG dest mgr
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);
	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while(cinfo->next_scanline < height ) {
		bytes += stride;
		row_ptr[0] = src + cinfo->next_scanline * stride;
		jpeg_write_scanlines(cinfo, row_ptr, 1 );
	}
	jpeg_finish_compress(cinfo );

	d->len = rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

/* Encodes a BGR24 frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_bgr24(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	struct jpeg_compress_struct*	cinfo = d->j->cinfo;
	uint32_t						width = d->vdev->capture->width;
	uint32_t						height = d->vdev->capture->height;
	uint32_t						x, rgb_size = width * height * 3;
	JSAMPROW 						row[1] = {d->conversion_buffer};
	uint8_t*						ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = d->conversion_buffer;
			for (x = 0; x < width; x++) {
				*(ptr++) = src[2];
				*(ptr++) = src[1];
				*(ptr++) = src[0];
				src +=3;
			}
		jpeg_write_scanlines (cinfo, row, 1);
	}

	jpeg_finish_compress (cinfo);
	d->len =  rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}


int init_jpeg_compressor(struct v4l4j_device *d, int q){
	dprint(LOG_JPEG, "[JPEG] Initialising the JPEG compressor\n");
	XMALLOC(d->j, struct jpeg_data *, sizeof(struct jpeg_data));

	if(d->vdev->capture->palette == YUV420 || d->vdev->capture->palette == YUYV ||
			d->vdev->capture->palette == YVYU || d->vdev->capture->palette == RGB24 ||
			d->vdev->capture->palette == RGB32 || d->vdev->capture->palette == BGR24 ||
			d->vdev->capture->palette == UYVY || d->vdev->capture->palette == BGR32) {
		//JPEG param common to YUV420, YUYV, YVYU, RGB24, RGB32, BGR24, UYVY & BGR32
		XMALLOC(d->j->cinfo, struct jpeg_compress_struct *, sizeof(struct jpeg_compress_struct));
		XMALLOC(d->j->jerr, struct jpeg_error_mgr *, sizeof(struct jpeg_error_mgr));
		XMALLOC(d->j->destmgr, struct jpeg_destination_mgr *, sizeof(struct jpeg_destination_mgr));

		d->j->cinfo->err = jpeg_std_error(d->j->jerr);
		jpeg_create_compress(d->j->cinfo);
		d->j->destmgr->init_destination = init_destination;
		d->j->destmgr->empty_output_buffer = empty_output_buffer;
		d->j->destmgr->term_destination = term_destination;
		d->j->cinfo->dest = d->j->destmgr;

		d->j->cinfo->image_width = d->vdev->capture->width;
		d->j->cinfo->image_height = d->vdev->capture->height;
		d->j->cinfo->input_components = 3;
		d->j->cinfo->dct_method = JDCT_FASTEST;
		d->j->jpeg_quality = q;

		if ((d->vdev->capture->palette == YUV420) || (d->vdev->capture->palette == YUYV)
				|| (d->vdev->capture->palette == YVYU) || (d->vdev->capture->palette == UYVY)){

			// Set up the JPEG converter for YUV ->JPEG conversion
			jpeg_set_defaults(d->j->cinfo);
			jpeg_set_colorspace(d->j->cinfo, JCS_YCbCr);

			d->j->cinfo->raw_data_in = TRUE; // supply downsampled data
			d->j->cinfo->comp_info[0].h_samp_factor = 2;
			// d->j->cinfo->comp_info[0].v_samp_factor set below depending on source format
			d->j->cinfo->comp_info[1].h_samp_factor = 1;
			d->j->cinfo->comp_info[1].v_samp_factor = 1;
			d->j->cinfo->comp_info[2].h_samp_factor = 1;
			d->j->cinfo->comp_info[2].v_samp_factor = 1;

			if (d->vdev->capture->palette == YUV420) {
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for YUV420\n");
				d->j->cinfo->comp_info[0].v_samp_factor = 2;
				d->j->mcu = 16;
				d->convert = jpeg_encode_yuv420;
			} else {
				d->j->cinfo->comp_info[0].v_samp_factor = 1;
				d->j->mcu = 8;
				XMALLOC_ALIGNED(d->conversion_buffer, unsigned char *, (d->vdev->capture->width * d->vdev->capture->height * 2));
				switch (d->vdev->capture->palette) {
				case YUYV:
					dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for YUYV\n");
					d->convert = jpeg_encode_yuyv;
					break;
				case YVYU:
					dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for YVYU\n");
					d->convert = jpeg_encode_yvyu;
					break;
				case UYVY:
					dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for UYVY\n");
					d->convert = jpeg_encode_uyvy;
					break;
				}
			}

			// Allocate JSAMPROW arrays
			XMALLOC(d->j->y, JSAMPROW *, sizeof(JSAMPROW) * d->j->mcu);
			XMALLOC(d->j->cb, JSAMPROW *, sizeof(JSAMPROW) * d->j->mcu);
			XMALLOC(d->j->cr, JSAMPROW *, sizeof(JSAMPROW) * d->j->mcu);

			// set up JSAMPIMAGE
			d->j->data[0] = d->j->y;
			d->j->data[1] = d->j->cb;
			d->j->data[2] = d->j->cr;

		} else {
			d->j->cinfo->in_color_space = JCS_RGB;
			jpeg_set_defaults(d->j->cinfo) ;
			if (d->vdev->capture->palette == RGB24){
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for RGB24\n");
				d->convert = jpeg_encode_rgb24;
			} else if (d->vdev->capture->palette == RGB32){
				XMALLOC(d->conversion_buffer, unsigned char *, (d->vdev->capture->width*3));
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for RGB32\n");
				d->convert = jpeg_encode_rgb32;
			}  else if (d->vdev->capture->palette == BGR24){
				XMALLOC(d->conversion_buffer, unsigned char *, (d->vdev->capture->width*3));
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for BGR24\n");
				d->convert = jpeg_encode_bgr24;
			} else if (d->vdev->capture->palette == BGR32){
				XMALLOC(d->conversion_buffer, unsigned char *, (d->vdev->capture->width*3));
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for BGR32\n");
				d->convert = jpeg_encode_bgr32;
			}
		}
		jpeg_set_quality(d->j->cinfo, d->j->jpeg_quality,TRUE);
	} else if(d->vdev->capture->palette == MJPEG) {
		dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for MJPEG\n");
		d->convert = jpeg_encode_mjpeg;
	} else if(d->vdev->capture->palette == JPEG) {
		dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for JPEG\n");
		d->convert = jpeg_encode_jpeg;
	} else {
		info("[JPEG] Image format %d cannot be JPEG encoded\n", d->vdev->capture->palette);
		XFREE(d->j);
		return -1;
	}
	return 0;
}

void destroy_jpeg_compressor(struct v4l4j_device *d){
	dprint(LOG_JPEG, "[JPEG] Destroying JPEG compressor\n");
	if(d->vdev->capture->palette == YUV420 || d->vdev->capture->palette == YUYV ||
		d->vdev->capture->palette == YVYU || d->vdev->capture->palette == RGB24 ||
		d->vdev->capture->palette == RGB32 || d->vdev->capture->palette == BGR24 ||
		d->vdev->capture->palette == UYVY || d->vdev->capture->palette == BGR32) {

		XFREE(d->conversion_buffer);
		XFREE(d->j->y);
		XFREE(d->j->cb);
		XFREE(d->j->cr);

		// free JPEG compressor & data structs
		jpeg_destroy_compress(d->j->cinfo);
		XFREE(d->j->destmgr);
		XFREE(d->j->jerr);
		XFREE(d->j->cinfo);
	}

	XFREE(d->j);
}

//
//
//
//Usage: ./jpeg *.raw

#ifdef MAKETEST
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdint.h>
void *read_frame(struct capture_device *c, char *file){
	int fd, len=0;
	void *data;
	struct stat file_stat;

	//open file
	if ((fd = open(file, O_RDONLY)) < 0) {
		printf( "FILE: can't open file\n");
		return NULL;
	}

	// get file size
	if (fstat(fd, &file_stat) != 0) {
		printf("Error stat'ing file\n");
		close(fd);
		return NULL;
	}

	c->imagesize = file_stat.st_size;

	// allocate buffer
	posix_memalign(&data, 16, file_stat.st_size);
	if (data == NULL) {
		printf("Error allocating image buffer\n");
		close(fd);
		return NULL;
	}

	// read file contents
	while((len += read(fd, (data + len), (c->imagesize - len))) < c->imagesize);

	close(fd);

	return data;
}

void write_frame(void *d, int size, char *file) {
	int outfile, len = 0;
	char filename[200];
	
	//Construct the filename
	if (file != NULL)
		sprintf(filename,"%s.jpg", file);
	else {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		sprintf(filename, "%d-%d.jpg", (int) tv.tv_sec, (int) tv.tv_usec);
	}

	//open file
	if ((outfile = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
		printf( "FILE: can't open %s\n", filename);
		return;
	}

	while((len += write(outfile, (d + len), (size - len))) < size);

	close(outfile);
}

static const uint32_t  yuyv_pattern[] = {
	0xFF4B544C,	// 2 red pixels		255 0	0
	0xB6B21BB2, // 2 orange pixels	255	175	0
	0x15942B95, // 2 green pixels	0	255 0
	0x07A983A9, // 2 blue-green pix	0	255	175
	0x6B1CFF1D, // 2 blue pixels	0	0	255
	0xC552E152, // 2 purple pixels	180	0	255
	0x80FE80FF, // 2 white pixels	255 255 255
	0x80508050, // 2 grey pixels	80	80	80
};

static const uint32_t  uyvy_pattern[] = {
	0x4BFF4C54,	// 2 red pixels		255 0	0
	0xB2B6B21B, // 2 orange pixels	255	175	0
	0x9415952B, // 2 green pixels	0	255 0
	0xA907A983, // 2 blue-green pix	0	255	175
	0x1C6B1DFF, // 2 blue pixels	0	0	255
	0x52C552E1, // 2 purple pixels	180	0	255
	0xFE80FF80, // 2 white pixels	255 255 255
	0x50805080, // 2 grey pixels	80	80	80
};
static const uint32_t  yvyu_pattern[] = {
	0x544BFF4C,	// 2 red pixels		255 0	0
	0x1BB2B6B2, // 2 orange pixels	255	175	0
	0x2B941595, // 2 green pixels	0	255 0
	0x83A907A9, // 2 blue-green pix	0	255	175
	0xFF1C6B1D, // 2 blue pixels	0	0	255
	0xE152C552, // 2 purple pixels	180	0	255
	0x80FE80FF, // 2 white pixels	255 255 255
	0x80508050, // 2 grey pixels	80	80	80
};

void *generate_buffer(struct capture_device *c) {
	void 		*buffer = NULL;
	uint32_t 	*ptr = NULL, *pattern = NULL;
	uint32_t 	x, y, num_elem;

	// select the correct palette and image size
	switch (c->palette) {
		case YUV420:
			c->imagesize = c->width * c->height * 3 / 2;
			break;
		case YUYV:
			pattern = (uint32_t *)yuyv_pattern;
			num_elem = sizeof(yuyv_pattern) / sizeof(yuyv_pattern[0]);
			c->imagesize = c->width * c->height * 2;
			break;
		case UYVY:
			pattern = (uint32_t *)uyvy_pattern;
			c->imagesize = c->width * c->height * 2;
			num_elem = sizeof(uyvy_pattern) / sizeof(uyvy_pattern[0]);
			break;
		case YVYU:
			pattern = (uint32_t *)yvyu_pattern;
			c->imagesize = c->width * c->height * 2;
			num_elem = sizeof(yvyu_pattern) / sizeof(yvyu_pattern[0]);
			break;
		default:
			printf("Unknown palette %d\n", c->palette);
			fflush(stdout);
			return NULL;
	}

	// allocate buffer
	posix_memalign(&buffer, 16, c->imagesize);
	ptr = buffer;

	// fill buffer
	if (c->palette == YUYV || c->palette == UYVY || c->palette == YVYU) {
		for (y = 0; y < c->height; y++)
			for (x = 0; x < c->width ; x+=2)
				*ptr++ = pattern[num_elem * x / c->width];
	} else if (c->palette == YUV420) {
		unsigned char *py = (unsigned char *)ptr;
		unsigned char *pu = (unsigned char *)py + c->width * c->height;
		unsigned char *pv = (unsigned char *)pu + c->width * c->height / 4;
		unsigned char *src;

		for (y = 0; y < c->height; y++)
			for (x = 0; x < c->width ; x+=2) {
				// select the right pattern in the array
				pattern = (uint32_t *)&yuyv_pattern[sizeof(yuyv_pattern) / sizeof(yuyv_pattern[0]) * x / c->width];
				src = (unsigned char *)pattern;

				// copy first and third bytes (Y1 Y2)
				*py++ = *src;
				*py++ = *(src + 2);

				// copy second and fourth byte (U & V) if line number is even
				if ((y & 0x1) == 0) {
					*pu++ = *(src + 1);
					*pv++ = *(src + 3);
				}
			}
	
	}
 
	return buffer;
}

int main(int argc, char **argv){
	struct v4l4j_device 	d;
	struct video_device 	v;
	struct capture_device 	c;
	struct timeval 			start, now;
	char 					*filename = NULL;
	int 					iter;
	uint64_t				total_us = 0;
	void 					*input = NULL, *jpeg;

	// make sure we have required args
	if ((argc != 5) && (argc != 6)){
		printf("Usage: %s <num_iter> <width> <height> <format> [input_file]\n", argv[0]);
		printf("Formats: YUYV: %d - UYVY: %d - YVYU: %d - YUV420: %d\n", YUYV, UYVY, YVYU, YUV420);
		return -1;
	}

	// parse args:
	iter = atoi(argv[1]);
	c.width = atoi(argv[2]);
	c.height = atoi(argv[3]);
	c.palette = atoi(argv[4]);
	if (argc == 6)
		filename = argv[5];

	// struct setup
	d.vdev=&v;
	v.capture = &c;

	// fill buffer either from file or with generated pattern
	if (filename != NULL)
		input = read_frame(&c, filename);
	else
		input = generate_buffer(&c);

	// setup JPEG compressor
	init_jpeg_compressor( &d, 80);

	//size of dest buffer (JPEG) - use the same size as uncompressed frame - should be enough !
	posix_memalign(&jpeg, 16, c.imagesize);

	while(iter-- > 0){
		// time conversion
		gettimeofday(&start, NULL);
		d.convert(&d, input, jpeg);
		gettimeofday(&now, NULL);

		total_us += (now.tv_sec - start.tv_sec)*1000000 + (now.tv_usec - start.tv_usec);
	}

	printf("Average conversion time: %.1f ms\n", (float)total_us / (1000.0 * atoi(argv[1])));
	// save converted frame
	write_frame(jpeg, d.len, filename);

	free(input);
	free(jpeg);
	destroy_jpeg_compressor(&d);
	return 0;
}
#endif



