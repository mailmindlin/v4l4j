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

#define DHT_SIZE		420
static unsigned char huffman_table[] =
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
static unsigned char *temp_buf;

/* Encodes a YUV planar frame of width "d->c->width and height "d->c->height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "d->len" is set to the
 * length of the compressed JPEG frame. "d->j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_yuv420(struct v4l4j_device *d, unsigned char *src, unsigned char *dst) {
	//Code for this function is taken from Motion
	//Credit to them !!!
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	JSAMPROW y[16],cb[16],cr[16];
	JSAMPARRAY data[3];
	int i, line, rgb_size, width, height;
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	width = d->vdev->capture->width;
	height = d->vdev->capture->height ;

	data[0] = y;
	data[1] = cb;
	data[2] = cr;

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo,d->j->jpeg_quality,TRUE);

	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	jpeg_start_compress(cinfo, TRUE );
	for (line=0; line<height; line+=16) {
		for (i=0; i<16; i++) {
			y[i] = (unsigned char *)src + width*(i+line);
			if (i%2 == 0) {
				cb[i/2] = (unsigned char *)src + width*height + width/2*((i+line)/2);
				cr[i/2] = (unsigned char *)src + width*height + width*height/4 + width/2*((i+line)/2);
			}
		}
		jpeg_write_raw_data(cinfo, data, 16);
	}
	jpeg_finish_compress(cinfo);
	d->len = rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

static void jpeg_encode_rgb24(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	JSAMPROW row_ptr[1];
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	int	width = d->vdev->capture->width;
	int height = d->vdev->capture->height ;
	int rgb_size = width * height * 3, stride = width * 3, bytes=0;

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

static void jpeg_encode_jpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	memcpy(dst, src, d->capture_len);
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->capture_len);
	d->len = d->capture_len;
}

static void jpeg_encode_mjpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	int has_dht=0, ptr=0, size;
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

/* Encodes a YUYV planar frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_yuyv(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	//Optimise me !!!
	//it should be possible to send a YUYV frame straight to the jpeg compressor without converting to RGB first
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	JSAMPROW row[1] = {temp_buf};
	int a=0, width = d->vdev->capture->width, height = d->vdev->capture->height, x, rgb_size;
	int r, g, b;
	int y, u, v;
	unsigned char *ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = temp_buf;

		for (x = 0; x < width; x++) {
			if (!a)
				y = src[0] << 8;
			else
				y = src[2] << 8;
			u = src[1] - 128;
			v = src[3] - 128;

			r = (y + (359 * v)) >> 8;
			g = (y - (88 * u) - (183 * v)) >> 8;
			b = (y + (454 * u)) >> 8;

			*(ptr++) = CLIP(r);
			*(ptr++) = CLIP(g);
			*(ptr++) = CLIP(b);

			if (a++) {
				a = 0;
				src += 4;
			}
		}
		jpeg_write_scanlines (cinfo, row, 1);
	}

	jpeg_finish_compress (cinfo);
	d->len =  rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

/* Encodes a YVYU planar frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_yvyu(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	//Optimise me !!!
	//it should be possible to send a YVYU frame straight to the jpeg compressor without converting to RGB first
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	JSAMPROW row[1] = {temp_buf};
	int a=0, width = d->vdev->capture->width, height = d->vdev->capture->height, x, rgb_size;
	int r, g, b;
	int y, u, v;
	unsigned char *ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = temp_buf;

		for (x = 0; x < width; x++) {

			if (!a)
				y = src[0] << 8;
			else
				y = src[2] << 8;
			v = src[1] - 128;
			u = src[3] - 128;

			r = (y + (359 * v)) >> 8;
			g = (y - (88 * u) - (183 * v)) >> 8;
			b = (y + (454 * u)) >> 8;

			*(ptr++) = CLIP(r);
			*(ptr++) = CLIP(g);
			*(ptr++) = CLIP(b);

			if (a++) {
				a = 0;
				src += 4;
			}
		}
		jpeg_write_scanlines (cinfo, row, 1);
	}

	jpeg_finish_compress (cinfo);
	d->len =  rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

/* Encodes a UYVY frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_uyvy(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	JSAMPROW row[1] = {temp_buf};
	int a=0, width = d->vdev->capture->width, height = d->vdev->capture->height, x, rgb_size;
	int r, g, b;
	int y, u, v;
	unsigned char *ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = temp_buf;

		for (x = 0; x < width; x++) {
			if (!a)
				y = src[1] << 8;
			else
				y = src[3] << 8;
			u = src[0] - 128;
			v = src[2] - 128;

			r = (y + (359 * v)) >> 8;
			g = (y - (88 * u) - (183 * v)) >> 8;
			b = (y + (454 * u)) >> 8;

			*(ptr++) = CLIP(r);
			*(ptr++) = CLIP(g);
			*(ptr++) = CLIP(b);

			if (a++) {
				a = 0;
				src += 4;
			}
		}
		jpeg_write_scanlines (cinfo, row, 1);
	}

	jpeg_finish_compress (cinfo);
	d->len =  rgb_size - cinfo->dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

/* Encodes a RGB32 frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_rgb32(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	int width = d->vdev->capture->width, height = d->vdev->capture->height, x, rgb_size;
	JSAMPROW row[1] = {temp_buf};
	unsigned char *ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = temp_buf;
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

/* Encodes a BGR24 frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_bgr24(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	int width = d->vdev->capture->width, height = d->vdev->capture->height, x, rgb_size;
	JSAMPROW row[1] = {temp_buf};
	unsigned char *ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = temp_buf;
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

/* Encodes a BGR32 frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void jpeg_encode_bgr32(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	struct jpeg_compress_struct *cinfo = d->j->cinfo;
	int width = d->vdev->capture->width, height = d->vdev->capture->height, x, rgb_size;
	JSAMPROW row[1] = {temp_buf};
	unsigned char *ptr;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j->destmgr->next_output_byte = dst;
	d->j->destmgr->free_in_buffer = rgb_size;
	jpeg_set_quality(cinfo, d->j->jpeg_quality,TRUE);

	jpeg_start_compress(cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->vdev->capture->imagesize);
	while (cinfo->next_scanline < height) {
		ptr = temp_buf;
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

		if(d->vdev->capture->palette == YUV420) {
			dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for YUV420\n");

			jpeg_set_defaults(d->j->cinfo);
			jpeg_set_colorspace(d->j->cinfo, JCS_YCbCr);
			d->j->cinfo->raw_data_in = TRUE; // supply downsampled data
			d->j->cinfo->comp_info[0].h_samp_factor = 2;
			d->j->cinfo->comp_info[0].v_samp_factor = 2;
			d->j->cinfo->comp_info[1].h_samp_factor = 1;
			d->j->cinfo->comp_info[1].v_samp_factor = 1;
			d->j->cinfo->comp_info[2].h_samp_factor = 1;
			d->j->cinfo->comp_info[2].v_samp_factor = 1;
			d->convert = jpeg_encode_yuv420;
		} else {
			d->j->cinfo->in_color_space = JCS_RGB;
			jpeg_set_defaults(d->j->cinfo) ;
			if(d->vdev->capture->palette == YUYV) {
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for YUYV\n");
				d->convert = jpeg_encode_yuyv;
				XMALLOC(temp_buf, unsigned char *, (d->vdev->capture->width*3));
			} else if (d->vdev->capture->palette == YVYU) {
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for YVYU\n");
				d->convert = jpeg_encode_yvyu;
				XMALLOC(temp_buf, unsigned char *, (d->vdev->capture->width*3));
			} else if (d->vdev->capture->palette == RGB24){
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for RGB24\n");
				d->convert = jpeg_encode_rgb24;
			} else if (d->vdev->capture->palette == RGB32){
				XMALLOC(temp_buf, unsigned char *, (d->vdev->capture->width*3));
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for RGB32\n");
				d->convert = jpeg_encode_rgb32;
			}  else if (d->vdev->capture->palette == BGR24){
				XMALLOC(temp_buf, unsigned char *, (d->vdev->capture->width*3));
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for BGR24\n");
				d->convert = jpeg_encode_bgr24;
			} else if (d->vdev->capture->palette == UYVY) {
				dprint(LOG_JPEG, "[JPEG] Setting jpeg compressor for UYVY\n");
				d->convert = jpeg_encode_uyvy;
				XMALLOC(temp_buf, unsigned char *, (d->vdev->capture->width*3));
			} else if (d->vdev->capture->palette == BGR32){
				XMALLOC(temp_buf, unsigned char *, (d->vdev->capture->width*3));
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
		jpeg_destroy_compress(d->j->cinfo);
		XFREE(d->j->cinfo);
		XFREE(d->j->jerr);
		XFREE(d->j->destmgr);
		if(d->vdev->capture->palette == YUYV || d->vdev->capture->palette == YVYU ||
				d->vdev->capture->palette == RGB32 || d->vdev->capture->palette == BGR24 ||
				d->vdev->capture->palette == UYVY || d->vdev->capture->palette == BGR32)
				XFREE(temp_buf);
	}
	XFREE(d->j);
}

//
//gcc -I ../libv4l jpeg.c -ggdb -DDEBUG -ljpeg -o jpeg
//
//Usage: ./jpeg *.raw

#ifdef MAKETEST
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
void *read_frame(void * d, int s, char *file){
	int f, l=0;

	//open file
	if ((f = open(file, O_RDONLY)) < 0) {
		printf( "FILE: can't open file\n");
		return NULL;
	}

	while((l += read(f, (d+l), 65536))<s);


	close(f);
	return d;
}

void write_frame(void *d, int size, char *file) {
	int outfile, len = 0;
	char filename[50];
	struct timeval tv;


	//Construct the filename
	gettimeofday(&tv, NULL);
	sprintf(filename,"%s.jpg", file);


	//open file
	if ((outfile = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
		printf( "FILE: can't open %s\n", filename);
		return;
	}

	while((len+=write(outfile, (d+len), (size-len)))<size);

	close(outfile);
}

int main(int argc, char **argv){
	int nb = 0;
	void *data, *jpeg;
	struct v4l4j_device d;
	struct video_device v;
	struct capture_device c;
	struct timeval start, now;
	d.vdev=&v;
	v.capture = &c;
	//Image format
	c.palette = RGB32;
	c.width = 640;
	c.height = 480;
	//size of v4l buffer
	c.imagesize = 1228800;
	//actual size of frame
	d.capture_len = c.imagesize;
	init_jpeg_compressor( &d, 80);
	//size of dest buffer (JPEG)
	jpeg = (void *) malloc(640*480*3);
	//size of source buffer - ADJUST ACCORDING TO FORMAT
	data = (void *) malloc(640*480*4);
	gettimeofday(&start, NULL);
	while(nb++<(argc-1)){
		read_frame(data, c.imagesize, argv[nb]);
		d.convert(&d, data, jpeg);
		write_frame(jpeg, d.len, argv[nb]);
	}
	gettimeofday(&now, NULL);
	printf("fps: %.1f\n", (nb/((now.tv_sec - start.tv_sec) + ((float) (now.tv_usec - start.tv_usec)/1000000))));
	free(data);
	free(jpeg);
	destroy_jpeg_compressor(&d);
	return 0;
}
#endif



