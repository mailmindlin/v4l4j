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

static int ac_count[2][16] = {
			{0, 0x2, 0x1, 0x3, 0x3, 0x2, 0x4, 0x3, 0x5, 0x5, 0x4, 0x4, 0, 0, 0x1, 0x7d},
			{0, 0x2, 0x1, 0x2, 0x4, 0x4, 0x3, 0x4, 0x7, 0x5, 0x4, 0x4, 0, 0x1, 0x2, 0x77}
		};
static int ac_symbol[2][256] = {
			{0x1, 0x2, 0x3, 0, 0x4, 0x11, 0x5, 0x12, 0x21, 0x31, 0x41, 0x6,
					0x13, 0x51, 0x61, 0x7, 0x22, 0x71, 0x14, 0x32, 0x81,
					0x91, 0xa1, 0x8, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52,
					0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x9, 0xa,
					0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
					0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,
					0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53,
					0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64,
					0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75,
					0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86,
					0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
					0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
					0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
					0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6,
					0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
					0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5,
					0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4,
					0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

			{0, 0x1, 0x2, 0x3, 0x11, 0x4, 0x5, 0x21, 0x31, 0x6, 0x12, 0x41,
					0x51, 0x7, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x8,
					0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x9, 0x23, 0x33,
					0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0xa, 0x16, 0x24,
					0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
					0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39,
					0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
					0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63,
					0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74,
					0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84,
					0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
					0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4,
					0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
					0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4,
					0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
					0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4,
					0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4,
					0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		};

static int dc_count[2][16] = {
		{0, 0x1, 0x5, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0, 0, 0, 0, 0, 0, 0},
		{0, 0x3, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0, 0, 0, 0, 0}
};
static int dc_symbol[2][256] = {
			{0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0},

			{0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0}
		};

/* Dummy JPEG methods */
static void init_source( j_decompress_ptr cinfo ){}
static boolean fill_intput_buffer( j_decompress_ptr cinfo ){return TRUE;}
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes) {}
static void term_source( j_decompress_ptr cinfo ){}
static void emit_msg(j_common_ptr cinfo, int msg_level){}


static void rgb24_encode_rgb24(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	memcpy(dst, src, d->len);
}

/* Converts a BGR24 frame of width "width and height "height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 */
static void rgb24_encode_bgr24(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	int size = d->r->nb_pixel, i=0;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	while(i++<size){
		*(dst++) = src[2];
		*(dst++) = src[1];
		*(dst++) = src[0];
		src += 3;
	}
}

/* Converts a RGB32 frame of width "width and height "height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 */
static void rgb24_encode_rgb32(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	int size = d->r->nb_pixel, i=0;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	while(i++<size){
		*(dst++) = src[1];
		*(dst++) = src[2];
		*(dst++) = src[3];
		src += 4;
	}
}

/* Converts a BGR32 frame of width "width and height "height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static void rgb24_encode_bgr32(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	int size = d->r->nb_pixel, i=0;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	while(i++<size){
		*(dst++) = src[2];
		*(dst++) = src[1];
		*(dst++) = src[0];
		src += 4;
	}
}

/* Converts a UYVY frame of width "width and height "height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 */
static void rgb24_encode_uyvy(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	int a=0, i=0, size = d->r->nb_pixel;
	int r, g, b;
	int y, u, v;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	while(i++<size){
		if (!a)
			y = src[1] << 8;
		else
			y = src[3] << 8;
		u = src[0] - 128;
		v = src[2] - 128;

		r = (y + (359 * v)) >> 8;
		g = (y - (88 * u) - (183 * v)) >> 8;
		b = (y + (454 * u)) >> 8;

		*(dst++) = CLIP(r);
		*(dst++) = CLIP(g);
		*(dst++) = CLIP(b);

		if (a++) {
			a = 0;
			src += 4;
		}
	}
}

/* Converts a YUYV frame of width "width and height "height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 */
static void rgb24_encode_yuyv(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	int a=0, i=0, size = d->r->nb_pixel;
	int r, g, b;
	int y, u, v;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	while(i++<size){
		if (!a)
			y = src[0] << 8;
		else
			y = src[2] << 8;
		u = src[1] - 128;
		v = src[3] - 128;

		r = (y + (359 * v)) >> 8;
		g = (y - (88 * u) - (183 * v)) >> 8;
		b = (y + (454 * u)) >> 8;

		*(dst++) = CLIP(r);
		*(dst++) = CLIP(g);
		*(dst++) = CLIP(b);

		if (a++) {
			a = 0;
			src += 4;
		}
	}
}

/* Converts a YVYU frame of width "width and height "height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 */
static void rgb24_encode_yvyu(struct v4l4j_device *d, unsigned char *src, unsigned char *dst){
	int a=0, i=0, size = d->r->nb_pixel;
	int r, g, b;
	int y, u, v;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	while(i++<size){
		if (!a)
			y = src[0] << 8;
		else
			y = src[2] << 8;
		u = src[3] - 128;
		v = src[1] - 128;

		r = (y + (359 * v)) >> 8;
		g = (y - (88 * u) - (183 * v)) >> 8;
		b = (y + (454 * u)) >> 8;

		*(dst++) = CLIP(r);
		*(dst++) = CLIP(g);
		*(dst++) = CLIP(b);

		if (a++) {
			a = 0;
			src += 4;
		}
	}
}

/* Converts a YUV planar frame of width "d->c->width and height "d->c->height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 * The following is taken from libv4l written by Hans de Goede <j.w.r.degoede@hhs.nl>
 * (See CREDITS)
 */
static void rgb24_encode_yuv420(struct v4l4j_device *d, unsigned char *src, unsigned char *dst) {
	int i,j, w = d->vdev->capture->width, h = d->vdev->capture->height, u1, rg, v1;
	unsigned char  *y = src, *u = src + w * h, *v = u + (w * h) / 4;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j += 2) {
			u1 = (((*u - 128) << 7) +  (*u - 128)) >> 6;
			rg = (((*u - 128) << 1) +  (*u - 128) +
					((*v - 128) << 2) + ((*v - 128) << 1)) >> 3;
			v1 = (((*v - 128) << 1) +  (*v - 128)) >> 1;

			*dst++ = CLIP(*y + v1);
			*dst++ = CLIP(*y - rg);
			*dst++ = CLIP(*y + u1);
			y++;

			*dst++ = CLIP(*y + v1);
			*dst++ = CLIP(*y - rg);
			*dst++ = CLIP(*y + u1);

			y++;
			u++;
			v++;
		}

		if (i&1) {
			u -= w / 2;
			v -= w / 2;
		}
	}
}


/* Converts a JPEG planar frame of width "d->c->width and height "d->c->height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
  */
static void rgb24_encode_jpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst) {
	struct jpeg_decompress_struct *c = d->r->cinfo;
	int line_size;
	JSAMPROW row[1] = {dst};
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	d->r->srcmgr->next_input_byte = src;
	d->r->srcmgr->bytes_in_buffer = d->capture_len;
	jpeg_read_header(c, TRUE);
	d->r->cinfo->dct_method = JDCT_FASTEST;
	d->r->cinfo->do_fancy_upsampling = FALSE;
	d->r->cinfo->out_color_space = JCS_RGB;

	dprint(LOG_RGB, "[RGB] Decompressing JPEG image %d\n", d->capture_len);
	//set decomp param

	jpeg_start_decompress(c);
	line_size = c->output_width * c->out_color_components * sizeof(JSAMPLE);

	while (c->output_scanline < c->output_height) {
		jpeg_read_scanlines(c, row, 1);
		row[0] += line_size;
	}
	jpeg_finish_decompress(c);

}

static void add_huffman_tables(struct v4l4j_device *d){
	int j, i;
	JHUFF_TBL *huff_ptr;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	for(j=0; j<2;j++){
		if (d->r->cinfo->ac_huff_tbl_ptrs[j] == NULL) {
			d->r->cinfo->ac_huff_tbl_ptrs[j] = jpeg_alloc_huff_table((j_common_ptr) d->r->cinfo);
			huff_ptr = d->r->cinfo->ac_huff_tbl_ptrs[j];
			for (i = 1; i <= 16; i++)
				huff_ptr->bits[i] = ac_count[j][(i-1)];

			for (i = 0; i < 256; i++)
				huff_ptr->huffval[i] = ac_symbol[j][i];
		}
	}

	for(j=0; j<2;j++){
		if (d->r->cinfo->dc_huff_tbl_ptrs[j] == NULL){
			d->r->cinfo->dc_huff_tbl_ptrs[j] = jpeg_alloc_huff_table((j_common_ptr) d->r->cinfo);
			huff_ptr = d->r->cinfo->dc_huff_tbl_ptrs[j];
			for (i = 1; i <= 16; i++)
				huff_ptr->bits[i] = dc_count[j][(i-1)];

			for (i = 0; i < 256; i++)
				huff_ptr->huffval[i] = dc_symbol[j][i];
		}
	}

}

int init_rgb_converter(struct v4l4j_device *d){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	XMALLOC(d->r, struct rgb_data *, sizeof(struct rgb_data));

	if(d->vdev->capture->palette == RGB24){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for RGB24\n");
		d->convert = rgb24_encode_rgb24;
	} else if(d->vdev->capture->palette == BGR24){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for BGR24\n");
		d->convert = rgb24_encode_bgr24;
	} else if(d->vdev->capture->palette == RGB32){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for RGB32\n");
		d->convert = rgb24_encode_rgb32;
	} else if(d->vdev->capture->palette == BGR32){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for BGR32\n");
		d->convert = rgb24_encode_bgr32;
	} else if(d->vdev->capture->palette == UYVY){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for UYVY\n");
		d->convert = rgb24_encode_uyvy;
	} else if(d->vdev->capture->palette == YUYV){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for YUYV\n");
		d->convert = rgb24_encode_yuyv;
	} else if(d->vdev->capture->palette == YVYU){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for YVYU\n");
		d->convert = rgb24_encode_yvyu;
	} else if(d->vdev->capture->palette == YUV420){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for YUV420\n");
		d->convert = rgb24_encode_yuv420;
	} else if(d->vdev->capture->palette == JPEG || d->vdev->capture->palette == MJPEG){

		//setup JPEG decomp
		XMALLOC(d->r->jerr, struct jpeg_error_mgr *, sizeof(struct jpeg_error_mgr));
		XMALLOC(d->r->cinfo, struct jpeg_decompress_struct *, sizeof(struct jpeg_decompress_struct));
		XMALLOC(d->r->srcmgr, struct jpeg_source_mgr *, sizeof(struct jpeg_source_mgr));

		d->r->cinfo->err = jpeg_std_error(d->r->jerr);
		d->r->jerr->emit_message = emit_msg;
		jpeg_create_decompress(d->r->cinfo);
		d->r->srcmgr->init_source = init_source;
		d->r->srcmgr->fill_input_buffer = fill_intput_buffer;
		d->r->srcmgr->skip_input_data = skip_input_data;
		d->r->srcmgr->resync_to_restart = jpeg_resync_to_restart;
		d->r->srcmgr->term_source = term_source;
		d->r->cinfo->src = d->r->srcmgr;

		if(d->vdev->capture->palette == JPEG){
			dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for JPEG\n");
		} else {
			dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for MJPEG\n");
			add_huffman_tables(d);
		}
		d->convert = rgb24_encode_jpeg;

	} else {
		info("[RGB] Image format %d cannot be transformed to RGB24\n", d->vdev->capture->palette);
		XFREE(d->r);
		return -1;
	}
	d->len = d->vdev->capture->width * d->vdev->capture->height * 3;
	d->r->nb_pixel = d->vdev->capture->width * d->vdev->capture->height;
	return 0;
}

void destroy_rgb_converter(struct v4l4j_device *d){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	if(d->vdev->capture->palette == JPEG || d->vdev->capture->palette == MJPEG) {
		jpeg_destroy_decompress(d->r->cinfo);
		XFREE(d->r->cinfo);
		XFREE(d->r->jerr);
		XFREE(d->r->srcmgr);
	}

	XFREE(d->r);
}


//
//Usage: ./rgb *.raw

#ifdef MAKETEST

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

void *read_frame(void * d, int *size, char *file){
	int f, l=0;
	*size=0;

	//open file
	if ((f = open(file, O_RDONLY)) < 0) {
		printf( "FILE: can't open file\n");
		return NULL;
	}

	while(( l= read(f, (d+*size), 65536))!=0)
		*size += l;

	printf("read %d bytes\n", *size);

	close(f);
	return d;
}

//void *read_frame(void * d, int s, char *file){
//	int f, l=0;
//
//	//open file
//	if ((f = open(file, O_RDONLY)) < 0) {
//		printf( "FILE: can't open file\n");
//		return NULL;
//	}
//
//	while((l += read(f, (d+l), 65536))<s);
//
//	close(f);
//	return d;
//}

void write_frame(void *d, int size, char *file) {
	int outfile, len = 0;
	char filename[50];
	struct timeval tv;


	//Construct the filename
	gettimeofday(&tv, NULL);
	snprintf(filename,49, "%s.jpg", file);


	//open file
	if ((outfile = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
		printf( "FILE: can't open %s\n", filename);
		return;
	}

	while((len+=write(outfile, (d+len), (size-len)))<size);

	close(outfile);
}
#define PPM_HEADER			"P6 %d %d 255 \n"

void write_frame_ppm(unsigned char *d, char *file, int size, int w, int h){
	int len = 0, i=0;
	char header[20], filename[50];
	FILE *f;
	struct timeval tv;


	//Construct the filename
	gettimeofday(&tv, NULL);
	snprintf(filename,49, "%s.ppm", file);

	sprintf(header, PPM_HEADER, w,h);


	//open file
	if ((f = fopen(filename, "w"))==NULL) {
		printf( "FILE: can't open %s\n", filename);
		return;
	}

	while((len+=fprintf(f,header))<strlen(header));
	while(i<size) {
		len+=fprintf(f, "%hhu %hhu %hhu ", d[i], d[i+1], d[i+2]);
		i +=3;
	}

	fclose(f);

}

int main(int argc, char **argv){
	int nb = 0;
	unsigned char *data, *rgb;
	struct v4l4j_device d;
	struct video_device v;
	struct capture_device c;
	struct timeval start, now;
	d.vdev=&v;
	v.capture = &c;
	//Image format
	c.palette = MJPEG;
	c.width = 640;
	c.height = 480;
	//size of v4l buffer
	c.imagesize = 204800;
	//actual size of frame
	d.capture_len = 27417;
	init_rgb_converter( &d);
	//size of dest buffer (RGB24)
	XMALLOC(rgb, void *, 640*480*3);
	//size of source buffer - ADJUST ACCORDING TO FORMAT
	XMALLOC(data, void *, d.capture_len);
	gettimeofday(&start, NULL);
	while(nb++<(argc-1)){
		read_frame(data, &d.capture_len, argv[nb]);
		d.convert(&d, data, rgb);
		write_frame_ppm(rgb, argv[nb], 640*480*3, 640, 480);
	}
	gettimeofday(&now, NULL);
	printf("fps: %.1f\n", (nb/((now.tv_sec - start.tv_sec) + ((float) (now.tv_usec - start.tv_usec)/1000000))));
	free(data);
	free(rgb);
	destroy_rgb_converter(&d);
	return 0;
}
#endif



