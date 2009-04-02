/*
	copyright 2006 Gilles GIGAN (gilles.gigan@gmail.com)

	This file is part of light_cap.

   	light_cap is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    light_cap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with light_cap; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "libvideo.h"
#include "utils.h"
#include "log.h"

extern int jpeg_quality;

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

/* Adds Huffman tables to MJPEG frames of width "width and height "height" at "src" straight
 * if they re not there already
 */

//static void write_frame_jpeg(int length, void *d) {
//	int outfile, len = 0;
//	char filename[50];
//	struct timeval tv;
//
//
//	//Construct the filename
//	gettimeofday(&tv, NULL);
//	sprintf(filename,"raw_frame-%d-%d.jpg", (int) tv.tv_sec, (int) tv.tv_usec);
//
//
//	//open file
//	if ((outfile = open(filename, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
//		printf( "FILE: can't open %s\n", filename);
//		return;
//	}
//	//printf( "FILE: writing %d bytes to file\n", c->imagesize);
//	while((len+=write(outfile, (d+len), (length-len)))<length);
//
//	close(outfile);
//}
//
//static void write_frame_raw(int length, void *d) {
//	int outfile, len = 0;
//	char filename[50];
//	struct timeval tv;
//
//
//	//Construct the filename
//	gettimeofday(&tv, NULL);
//	sprintf(filename,"raw_frame-%d-%d.raw", (int) tv.tv_sec, (int) tv.tv_usec);
//
//
//	//open file
//	if ((outfile = open(filename, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
//		printf( "FILE: can't open %s\n", filename);
//		return;
//	}
//
//	//printf( "FILE: writing %d bytes to file\n", c->imagesize);
//	while((len+=write(outfile, (d+len), (length-len)))<length);
//
//	close(outfile);
//}

static int jpeg_encode_mjpeg(void *src, int src_len, struct capture_device *cdev,  struct jpeg *j, void *dst) {
	int has_dht=0, ptr=0, size;
	unsigned char *source = src, *dest = dst;

	if(source[0]!=0xff && source[1]!=0xD8) {
		info(LOG_ERR, "Invalid JPEG frame\n");
		return -1;
	}

	memcpy(dest,source,2);
	ptr += 2;

	while(!has_dht) {
		if(source[ptr]!=0xFF) {
			info(LOG_ERR, "Invalid JPEG frame\n");
			return -1;
		}

		if(source[ptr+1] == 0xC4)
			has_dht=1;
		else if (source[ptr+1] == 0xDA)
			break;

		size = (source[ptr+2] << 8) + source[ptr+3];
		memcpy((dest+ptr), (source+ptr), 2+size);
		ptr += (2+size);
	}

	if(!has_dht) {
		memcpy((dest+ptr), huffman_table, DHT_SIZE);
		memcpy((dest+ptr+DHT_SIZE), (source+ptr), (src_len-ptr));
		ptr += (DHT_SIZE+src_len-ptr);
	} else {
		memcpy((dest+ptr), (source+ptr), (src_len-ptr));
		ptr += (src_len-ptr);
	}

	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Frame had %d bytes, now %d bytes\n", src_len, ptr);
	//write_frame_jpeg(ptr, dst);
	//write_frame_raw(l, src);
	return ptr;
}

/* Encodes a RGB24 frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static int jpeg_encode_rgb(void *src,int src_len,  struct capture_device *cdev, struct jpeg *j, void *dst) {
	//Code for this function is taken from Motion
	//Credit to them !!!

	JSAMPROW row_ptr[1];
	int rgb_size, stride, bytes=0;

	//init JPEG dest mgr
	rgb_size = cdev->width * cdev->height * 3;
	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Encoding a %dx%d frame (%d bytes)\n", cdev->width, cdev->height, rgb_size);

	j->destmgr.next_output_byte = dst;
	j->destmgr.free_in_buffer = rgb_size;
	jpeg_set_quality(&j->cinfo, jpeg_quality,TRUE);
	jpeg_start_compress( &j->cinfo, TRUE );
	stride = j->cinfo.image_width * 3;

	while(j->cinfo.next_scanline < j->cinfo.image_height ) {
		bytes += stride;
		row_ptr[0] = src + j->cinfo.next_scanline * stride;
		jpeg_write_scanlines( &j->cinfo, row_ptr, 1 );
	}
	jpeg_finish_compress( &j->cinfo );

	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Compressed %d bytes to %d bytes\n", rgb_size, (rgb_size - j->cinfo.dest->free_in_buffer)) ;
	return rgb_size - j->cinfo.dest->free_in_buffer;
}

/* Encodes a YUV planar frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static int jpeg_encode_yuv(void *src, int src_len, struct capture_device *cdev, struct jpeg *j, void *dst) {
	//Code for this function is taken from Motion
	//Credit to them !!!

	JSAMPROW y[16],cb[16],cr[16];
	JSAMPARRAY data[3];
	int i, line, rgb_size, width = cdev->width, height = cdev->height;

	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Encoding a %dx%d frame\n", width, height);

	data[0] = y;
	data[1] = cb;
	data[2] = cr;

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	j->destmgr.next_output_byte = dst;
	j->destmgr.free_in_buffer = rgb_size;
	jpeg_set_quality(&j->cinfo, jpeg_quality,TRUE);

	jpeg_start_compress( &j->cinfo, TRUE );
	for (line=0; line<height; line+=16) {
		for (i=0; i<16; i++) {
			y[i] = src + width*(i+line);
			if (i%2 == 0) {
				cb[i/2] = src + width*height + width/2*((i+line)/2);
				cr[i/2] = src + width*height + width*height/4 + width/2*((i+line)/2);
			}
		}
		jpeg_write_raw_data(&j->cinfo, data, 16);
	}
	jpeg_finish_compress(&j->cinfo);

	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Compressed %d bytes to %d bytes\n", rgb_size, (rgb_size - j->cinfo.dest->free_in_buffer));
	return rgb_size - j->cinfo.dest->free_in_buffer;
}


/* Encodes a YUYV planar frame of width "width and height "height" at "src" straight
 * into a JPEG frame at "dst" (must be allocated y caller). "len" is set to the
 * length of the compressed JPEG frame. "j" contains the JPEG compressor and
 * must be initialised correctly by the caller
 */
static unsigned char *temp_buf;
static int jpeg_encode_yuyv(void *source, int src_len, struct capture_device *cdev, struct jpeg *j, void *dest) {
	//Optimise me !!!
	//it should be possible to send a YUYV frame straight to the jpeg compressor without converting to RGB first

	JSAMPROW row[1] = {temp_buf};
	int a=0, width = cdev->width, height = cdev->height, x, rgb_size;
	unsigned char *src = source, *ptr, *dst = dest;

	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Encoding a %dx%d frame\n", width, height);

	//init JPEG dest mgr
	rgb_size = width * height * 3;
	j->destmgr.next_output_byte = dst;
	j->destmgr.free_in_buffer = rgb_size;
	jpeg_set_quality(&j->cinfo, jpeg_quality,TRUE);

	jpeg_start_compress( &j->cinfo, TRUE );
	while (j->cinfo.next_scanline < height) {
		ptr = temp_buf;

		for (x = 0; x < width; x++) {
			int r, g, b;
			int y, u, v;

			if (!a)
				y = src[0] << 8;
			else
				y = src[2] << 8;
			u = src[1] - 128;
			v = src[3] - 128;

			r = (y + (359 * v)) >> 8;
			g = (y - (88 * u) - (183 * v)) >> 8;
			b = (y + (454 * u)) >> 8;

			*(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
			*(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
			*(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

			if (a++) {
				a = 0;
				src += 4;
			}
		}
		jpeg_write_scanlines (&j->cinfo, row, 1);
	}

	jpeg_finish_compress (&j->cinfo);

	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Compressed %d bytes to %d bytes\n", rgb_size, (rgb_size - j->cinfo.dest->free_in_buffer));
	return rgb_size - j->cinfo.dest->free_in_buffer;
}

static int jpeg_encode_jpeg(void *source, int src_len, struct capture_device *cdev, struct jpeg *j, void *dest) {
	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Encoding a %dx%d frame\n", cdev->width, cdev->height);

	memcpy(dest, source, src_len);
	dprint(LOG_SOURCE_JPEG, LOG_LEVEL_DEBUG2, "Compressed to %d bytes\n", src_len);
	return src_len;
}

void release_jpeg_encoder(struct capture_device *cdev, struct jpeg *j) {
	if(cdev->palette == YUYV)
		XFREE(temp_buf);

	jpeg_destroy_compress(&j->cinfo);
}



//jpeg callbacks (dummy functions, do nothing)
void init_destination( j_compress_ptr );
boolean empty_output_buffer( j_compress_ptr );
void term_destination( j_compress_ptr );

int setup_jpeg_encoder(struct capture_device *cdev, struct jpeg *j){

	if(cdev->palette == YUV420 || cdev->palette == YUYV || cdev->palette == RGB24) {
		//JPEG param common to YUV420, YUYV and RGB24
		j->cinfo.err = jpeg_std_error(&j->jerr);
		jpeg_create_compress(&j->cinfo);
		j->destmgr.init_destination = init_destination;
		j->destmgr.empty_output_buffer = empty_output_buffer;
		j->destmgr.term_destination = term_destination;
		j->cinfo.dest = &j->destmgr;

		j->cinfo.image_width = cdev->width;
		j->cinfo.image_height = cdev->height;
		j->cinfo.input_components = 3;

		if(cdev->palette == YUV420) {
			dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "Setting jpeg compressor for YUV420\n");

			jpeg_set_defaults(&j->cinfo);
			jpeg_set_colorspace(&j->cinfo, JCS_YCbCr);
			j->cinfo.raw_data_in = TRUE; // supply downsampled data
			j->cinfo.comp_info[0].h_samp_factor = 2;
			j->cinfo.comp_info[0].v_samp_factor = 2;
			j->cinfo.comp_info[1].h_samp_factor = 1;
			j->cinfo.comp_info[1].v_samp_factor = 1;
			j->cinfo.comp_info[2].h_samp_factor = 1;
			j->cinfo.comp_info[2].v_samp_factor = 1;
			j->cinfo.dct_method = JDCT_FASTEST;
			j->jpeg_encode = jpeg_encode_yuv;
		} else {
			j->cinfo.in_color_space = JCS_RGB;
			jpeg_set_defaults(&j->cinfo) ;
			if(cdev->palette == YUYV) {
				dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "Setting jpeg compressor for YUYV\n");
				j->jpeg_encode = jpeg_encode_yuyv;
				XMALLOC(temp_buf, unsigned char *, (cdev->width*3));
			} else {
				dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "Setting jpeg compressor for RGB\n");
				j->jpeg_encode = jpeg_encode_rgb;
			}
		}
		jpeg_set_quality(&j->cinfo, jpeg_quality,TRUE);
	} else if(cdev->palette == MJPEG) {
		dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "Setting jpeg compressor for MJPEG\n");
		j->jpeg_encode = jpeg_encode_mjpeg;
	} else if(cdev->palette == JPEG) {
		dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "Setting jpeg compressor for JPEG\n");
		j->jpeg_encode = jpeg_encode_jpeg;
	} else {
		info(LOG_ERR, "Palette not supported %d\n", cdev->palette);
		return -1;
	}
	return 0;
}
