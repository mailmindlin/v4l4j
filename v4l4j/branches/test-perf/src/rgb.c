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

/* Converts a MJPEG planar frame of width "d->c->width and height "d->c->height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 * The following uses the tinyjpeg library written by Luc Saillard (See CREDITS)
 */
static void rgb24_encode_mjpeg(struct v4l4j_device *d, unsigned char *src, unsigned char *dst) {
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
		dprint(LOG_RGB, "[RGB] Setting RGB24 conversion for JPEG\n");
		d->convert = rgb24_encode_mjpeg;
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
	}

	XFREE(d->r);
}


//
//gcc -I ../libv4l rgb.c -ggdb -DDEBUG -ljpeg -o rgb
//
//Usage: ./jpeg *.raw


//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/time.h>
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
//
//	close(f);
//	return d;
//}
//
//void write_frame(void *d, int size, char *file) {
//	int outfile, len = 0;
//	char filename[50];
//	struct timeval tv;
//
//
//	//Construct the filename
//	gettimeofday(&tv, NULL);
//	sprintf(filename,"%s.jpg", file);
//
//
//	//open file
//	if ((outfile = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
//		printf( "FILE: can't open %s\n", filename);
//		return;
//	}
//
//	while((len+=write(outfile, (d+len), (size-len)))<size);
//
//	close(outfile);
//}
//
//int main(int argc, char **argv){
//	int s, nb = 0;
//	void *data, *jpeg;
//	struct v4l4j_device d;
//	struct video_device v;
//	struct capture_device c;
//	struct timeval start, now;
//	d.vdev=&v;
//	v.capture = &c;
//	c.palette = RGB32;
//	c.width = 640;
//	c.height = 480;
//	c.imagesize = 640*480*4;
//	init_jpeg_compressor( &d, 80);
//	jpeg = (void *) malloc(c.imagesize);
//	data = (void *) malloc(c.imagesize);
//	gettimeofday(&start, NULL);
//	while(nb++<(argc-1)){
//		read_frame(data, c.imagesize, argv[nb]);
//		jpeg_encode_rgb32(&d, data, jpeg);
//		write_frame(jpeg, d.len, argv[nb]);
//	}
//	gettimeofday(&now, NULL);
//	printf("fps: %.1f\n", (nb/((now.tv_sec - start.tv_sec) + ((float) (now.tv_usec - start.tv_usec)/1000000))));
//	free(data);
//	free(jpeg);
//	destroy_jpeg_compressor(&d);
//	return 0;
//}




