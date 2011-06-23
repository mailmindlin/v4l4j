/*
#             (C) 2008 Hans de Goede <hdegoede@redhat.com>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LIBV4LCONVERT_PRIV_H
#define __LIBV4LCONVERT_PRIV_H

#include <stdio.h>
#include <sys/types.h>
#include "libv4lconvert.h"
#include "control/libv4lcontrol.h"
#include "processing/libv4lprocessing.h"
#include "tinyjpeg.h"
#include "pixfc-sse.h"

#define ARRAY_SIZE(x) ((int)sizeof(x)/(int)sizeof((x)[0]))

#define V4LCONVERT_ERROR_MSG_SIZE 256
#define V4LCONVERT_MAX_FRAMESIZES 256

#define V4LCONVERT_ERR(...) \
	snprintf(data->error_msg, V4LCONVERT_ERROR_MSG_SIZE, \
			"v4l-convert: error " __VA_ARGS__)

/* Card flags */
#define V4LCONVERT_IS_UVC                0x01

/* Pixformat flags */
#define V4LCONVERT_COMPRESSED            0x01 /* Compressed format */
#define V4LCONVERT_NEEDS_CONVERSION      0x02 /* Apps likely wont know this */
#define V4LCONVERT_COMPRESSED_AND_NEEDS_CONVERSION 0x03

struct v4lconvert_data {
	int fd;
	int flags; /* bitfield */
	int control_flags; /* bitfield */
	int supported_src_formats; /* bitfield */
	unsigned int no_formats;
	char error_msg[V4LCONVERT_ERROR_MSG_SIZE];
	struct PixFcSSE	*pixfc;
	struct jdec_private *jdec;
	struct v4l2_frmsizeenum framesizes[V4LCONVERT_MAX_FRAMESIZES];
	unsigned int no_framesizes;
	int convert1_buf_size;
	int convert2_buf_size;
	int rotate90_buf_size;
	int flip_buf_size;
	int convert_pixfmt_buf_size;
	unsigned char *convert1_buf;
	unsigned char *convert2_buf;
	unsigned char *rotate90_buf;
	unsigned char *flip_buf;
	unsigned char *convert_pixfmt_buf;
	struct v4lcontrol_data *control;
	struct v4lprocessing_data *processing;

	/* Data for external decompression helpers code */
	pid_t decompress_pid;
	int decompress_in_pipe[2];  /* Data from helper to us */
	int decompress_out_pipe[2]; /* Data from us to helper */

	/* For mr97310a decoder */
	int frames_dropped;

	/* For cpia1 decoder */
	unsigned char *previous_frame;
};

struct v4lconvert_pixfmt {
	unsigned int fmt;
	int flags;
};

void v4lconvert_fixup_fmt(struct v4l2_format *fmt);

unsigned char *v4lconvert_alloc_buffer(int needed,
		unsigned char **buf, int *buf_size);

void v4lconvert_rgb24_to_yuv420(const unsigned char *src, unsigned char *dest,
		const struct v4l2_format *src_fmt, int bgr, int yvu);

void v4lconvert_yuv420_to_rgb24(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_yuv420_to_bgr24(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_yuyv_to_rgb24(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_yuyv_to_bgr24(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_yuyv_to_yuv420(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_yvyu_to_rgb24(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_yvyu_to_bgr24(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_uyvy_to_rgb24(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_uyvy_to_bgr24(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_uyvy_to_yuv420(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_swap_rgb(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_swap_uv(const unsigned char *src, unsigned char *dst,
		const struct v4l2_format *src_fmt);

void v4lconvert_rgb565_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height);

void v4lconvert_rgb565_to_bgr24(const unsigned char *src, unsigned char *dest,
		int width, int height);

void v4lconvert_rgb565_to_yuv420(const unsigned char *src, unsigned char *dest,
		const struct v4l2_format *src_fmt, int yvu);

void v4lconvert_spca501_to_yuv420(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_spca505_to_yuv420(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_spca508_to_yuv420(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_cit_yyvyuy_to_yuv420(const unsigned char *src,
		unsigned char *ydest,
		int width, int height, int yvu);

void v4lconvert_konica_yuv420_to_yuv420(const unsigned char *src,
		unsigned char *ydest,
		int width, int height, int yvu);

int v4lconvert_cpia1_to_yuv420(struct v4lconvert_data *data,
		const unsigned char *src, int src_size,
		unsigned char *dst, int width, int height, int yvu);

void v4lconvert_sn9c20x_to_yuv420(const unsigned char *src, unsigned char *dst,
		int width, int height, int yvu);

void v4lconvert_decode_spca561(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_decode_sn9c10x(const unsigned char *src, unsigned char *dst,
		int width, int height);

int v4lconvert_decode_pac207(struct v4lconvert_data *data,
		const unsigned char *inp, int src_size, unsigned char *outp,
		int width, int height);

int v4lconvert_decode_mr97310a(struct v4lconvert_data *data,
		const unsigned char *src, int src_size, unsigned char *dst,
		int width, int height);

void v4lconvert_decode_sn9c2028(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_decode_sq905c(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_decode_stv0680(const unsigned char *src, unsigned char *dst,
		int width, int height);

void v4lconvert_bayer_to_rgb24(const unsigned char *bayer,
		unsigned char *rgb, int width, int height, unsigned int pixfmt);

void v4lconvert_bayer_to_bgr24(const unsigned char *bayer,
		unsigned char *rgb, int width, int height, unsigned int pixfmt);

void v4lconvert_bayer_to_yuv420(const unsigned char *bayer, unsigned char *yuv,
		int width, int height, unsigned int src_pixfmt, int yvu);

void v4lconvert_hm12_to_rgb24(const unsigned char *src,
		unsigned char *dst, int width, int height);

void v4lconvert_hm12_to_bgr24(const unsigned char *src,
		unsigned char *dst, int width, int height);

void v4lconvert_hm12_to_yuv420(const unsigned char *src,
		unsigned char *dst, int width, int height, int yvu);

void v4lconvert_rotate90(unsigned char *src, unsigned char *dest,
		struct v4l2_format *fmt);

void v4lconvert_flip(unsigned char *src, unsigned char *dest,
		struct v4l2_format *fmt, int hflip, int vflip);

void v4lconvert_crop(unsigned char *src, unsigned char *dest,
		const struct v4l2_format *src_fmt, const struct v4l2_format *dest_fmt);

int v4lconvert_helper_decompress(struct v4lconvert_data *data,
		const char *helper, const unsigned char *src, int src_size,
		unsigned char *dest, int dest_size, int width, int height, int command);

void v4lconvert_helper_cleanup(struct v4lconvert_data *data);

#endif
