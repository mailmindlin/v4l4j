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

#define CLIP(x) (unsigned char) ((x) > 255) ? 255 : (((x) < 0) ? 0 : (x));

static unsigned char *temp_buf;

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
		*(dst++) = src[0];
		*(dst++) = src[1];
		*(dst++) = src[2];
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
 * The following uses the tinyjpeg library written by Luc Saillard (See CREDITS)
 */
static void rgb24_encode_jpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst) {
	struct jdec_private *jdec = d->r->jdec;
	int res;
	if (tinyjpeg_parse_header(jdec, src, d->capture_len)) {
		info("Error parsing JPEG header: %s\n",	tinyjpeg_get_errorstring(jdec));
		return;
	}
	tinyjpeg_set_components(jdec, &src, 1);
	res = tinyjpeg_decode(jdec, TINYJPEG_FMT_RGB24);
	if(res){
		info("Error decoding JPEG header: %s\n",	tinyjpeg_get_errorstring(jdec));
		return;
	}
}

/* Converts a MJPEG planar frame of width "d->c->width and height "d->c->height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 * The following uses the tinyjpeg library written by Luc Saillard (See CREDITS)
 */
static void rgb24_encode_mjpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst) {
	int has_dht=0, ptr=0, size;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	if(src[0]!=0xff && src[1]!=0xD8) {
		dprint(LOG_JPEG, "[JPEG] Invalid JPEG frame\n");
		return;
	}

	dprint(LOG_JPEG, "[JPEG] Adding Huffman tables\n");
	memcpy(temp_buf,src,2);
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
		memcpy((temp_buf+ptr), (src+ptr), 2+size);
		ptr += (2+size);
	}

	if(!has_dht) {
		dprint(LOG_JPEG, "[JPEG] doesnt have DHT\n");
		memcpy((temp_buf+ptr), huffman_table, DHT_SIZE);
		memcpy((temp_buf+ptr+DHT_SIZE), (src+ptr), (d->capture_len-ptr));
		ptr += (DHT_SIZE+d->capture_len-ptr);
	} else {
		dprint(LOG_JPEG, "[JPEG] Has DHT\n");
		memcpy((temp_buf+ptr), (src+ptr), (d->capture_len-ptr));
		ptr += (d->capture_len-ptr);
	}

	dprint(LOG_JPEG, "[JPEG] Frame now has %d bytes\n", ptr);
	rgb24_encode_jpeg(d, temp_buf,dst);
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
	} else if(d->vdev->capture->palette == JPEG ){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for JPEG\n");
		d->convert = rgb24_encode_jpeg;
		d->r->jdec = tinyjpeg_init();
		//tinyjpeg_set_flags(d->r->jdec, TINYJPEG_FLAGS_MJPEG_TABLE);
	} else if(d->vdev->capture->palette == MJPEG ){
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for MJPEG\n");
		XMALLOC(temp_buf, unsigned char *, (d->vdev->capture->imagesize * sizeof(unsigned char)) + DHT_SIZE);
		d->convert = rgb24_encode_jpeg;
		d->r->jdec = tinyjpeg_init();
		tinyjpeg_set_flags(d->r->jdec, TINYJPEG_FLAGS_MJPEG_TABLE);
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
		unsigned char *c[3] = { NULL, NULL, NULL };
		tinyjpeg_set_components(d->r->jdec, c, 3);
		tinyjpeg_free(d->r->jdec);
		if(temp_buf!=NULL)
			XFREE(temp_buf);
	}

	XFREE(d->r);
}


//
//gcc -I ../libv4l -c jidctflt.c  -ggdb -DDEBUG &&  gcc -I ../libv4l -c rgb.c -ggdb -DDEBUG && gcc -I ../libv4l -c tinyjpeg.c -ggdb -DDEBUG && gcc rgb.o tinyjpeg.o  jidctflt.o -o rgb
//
//Usage: ./rgb *.raw

#ifdef MAKETEST

#include <stdio.h>
#include <string.h>
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
	snprintf(filename,49, "%s.jpg", file);


	//open file
	if ((outfile = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
		printf( "FILE: can't open %s\n", filename);
		return;
	}

	while((len+=write(outfile, (d+len), (size-len)))<size);

	close(outfile);
}
#define PPM_HEADER			"P3 %d %d 255 \n"

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
	void *data, *rgb;
	struct v4l4j_device d;
	struct video_device v;
	struct capture_device c;
	struct timeval start, now;
	d.vdev=&v;
	v.capture = &c;
	c.palette = MJPEG;
	c.width = 640;
	c.height = 480;
	c.imagesize = 204800;
	init_rgb_converter( &d);
	d.capture_len = c.imagesize;
	rgb = (void *) malloc(640*480*3);
	data = (void *) malloc(d.capture_len);
	gettimeofday(&start, NULL);
	while(nb++<(argc-1)){
		read_frame(data, d.capture_len, argv[nb]);
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



