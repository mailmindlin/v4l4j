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
# Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335  USA
 */

#ifndef __LIBV4LCONVERT_PRIV_H
#define __LIBV4LCONVERT_PRIV_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdbool.h>
#include "types.h"
#include "control/libv4lcontrol.h"
#include "processing/libv4lprocessing.h"
#include "tinyjpeg.h"
#include "pixfc-sse/pixfc-sse.h"

#define V4LCONVERT_ERROR_MSG_SIZE 256
#define V4LCONVERT_MAX_FRAMESIZES 256

#define V4LCONVERT_ERR(...) \
	snprintf(data->error_msg, V4LCONVERT_ERROR_MSG_SIZE, "v4l-convert: error " __VA_ARGS__)

/* Card flags */
#define V4LCONVERT_IS_UVC                0x01
#define V4LCONVERT_USE_TINYJPEG          0x02

struct v4lconvert_data {
	int fd;
	int flags; /* bitfield */
	int control_flags; /* bitfield */
	unsigned int no_formats;
	int64_t supported_src_formats; /* bitfield */
	char error_msg[V4LCONVERT_ERROR_MSG_SIZE];
	struct jdec_private *tinyjpeg;
	struct jpeg_error_mgr jerr;
	int jerr_errno;
	jmp_buf jerr_jmp_state;
	struct jpeg_decompress_struct cinfo;
	bool cinfo_initialized;
	struct PixFcSSE *pixfc;
	struct v4l2_frmsizeenum framesizes[V4LCONVERT_MAX_FRAMESIZES];
	unsigned int no_framesizes;
	unsigned int bandwidth;
	unsigned int fps;
	unsigned int convert1_buf_size;
	unsigned int convert2_buf_size;
	unsigned int rotate90_buf_size;
	unsigned int flip_buf_size;
	unsigned int convert_pixfmt_buf_size;
	u8 *convert1_buf;
	u8 *convert2_buf;
	u8 *rotate90_buf;
	u8 *flip_buf;
	u8 *convert_pixfmt_buf;
	struct v4lcontrol_data *control;
	struct v4lprocessing_data *processing;

	/* Data for external decompression helpers code */
	pid_t decompress_pid;
	int decompress_in_pipe[2];  /* Data from helper to us */
	int decompress_out_pipe[2]; /* Data from us to helper */

	/* For mr97310a decoder */
	int frames_dropped;

	/* For cpia1 decoder */
	u8 *previous_frame;
};

struct v4lconvert_pixfmt {
	unsigned int fmt;	/* v4l2 fourcc */
	unsigned int bpp;		/* bits per pixel, 0 for compressed formats */
	int rgb_rank;		/* rank for converting to rgb32 / bgr32 */
	int yuv_rank;		/* rank for converting to yuv420 / yvu420 */
	int needs_conversion;
};

#define LIBV4LCONVERT_LOCAL __attribute__((visibility ("hidden")))
#define LIBV4LCONVERT_PUBLIC __attribute__((visibility ("default")))
#pragma GCC visibility push(hidden)
LIBV4LCONVERT_LOCAL void v4lconvert_fixup_fmt(struct v4l2_format *fmt);

LIBV4LCONVERT_LOCAL u8 *v4lconvert_alloc_buffer(unsigned int needed, u8 **buf, unsigned int *buf_size);

LIBV4LCONVERT_LOCAL int v4lconvert_oom_error(struct v4lconvert_data *data);

//OMX functions
LIBV4LCONVERT_LOCAL void v4lconvert_omx_init();

LIBV4LCONVERT_LOCAL void v4lconvert_omx_deinit();

//Conversion functions
LIBV4LCONVERT_LOCAL void v4lconvert_rgb24_to_yuv420(const u8 *src, u8 *dest, unsigned int width, unsigned int height, unsigned int bytesperline, bool bgr, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_yuv420_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_yuv420_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_cvt_yuv420_to_bgr24(const u8 *ysrc, const u8* usrc, const u8* vsrc, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_yuyv_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_yuyv_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_yuyv_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_yvyu_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_yvyu_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_uyvy_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_uyvy_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_uyvy_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_swap_rgb(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_swap_uv(const u8 *src, u8 *dst, const struct v4l2_format *src_fmt);

LIBV4LCONVERT_LOCAL void v4lconvert_grey_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_grey_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL int v4lconvert_y10b_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL int v4lconvert_y10b_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_rgb565_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_rgb565_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_rgb565_to_yuv420(const u8 *src, u8 *dest, unsigned int width, unsigned int height, unsigned int bytesperline, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_spca501_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_spca505_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_spca508_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_cit_yyvyuy_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_konica_yuv420_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_m420_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL int v4lconvert_cpia1_to_yuv420(struct v4lconvert_data *data, const u8 *src, unsigned int src_size, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_sn9c20x_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL int v4lconvert_se401_to_rgb24(struct v4lconvert_data *data, const u8 *src, unsigned int src_size, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL int v4lconvert_decode_jpeg_tinyjpeg(struct v4lconvert_data *data, u8 *src, unsigned int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt, unsigned int flags);

LIBV4LCONVERT_LOCAL int v4lconvert_decode_jpeg_libjpeg(struct v4lconvert_data *data, u8 *src, unsigned int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt);

LIBV4LCONVERT_LOCAL int v4lconvert_decode_jpgl(const u8 *src, u32 src_size, unsigned int dest_pix_fmt, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_decode_spca561(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_decode_sn9c10x(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL int v4lconvert_decode_pac207(const u8 *inp, unsigned int src_size, u8 *outp, u32 width, u32 height);

LIBV4LCONVERT_LOCAL int v4lconvert_decode_mr97310a(struct v4lconvert_data *data, const u8 *src, unsigned int src_size, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL int v4lconvert_decode_jl2005bcd(struct v4lconvert_data *data, const u8 *src, unsigned int src_size, u8 *dest, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_decode_sn9c2028(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_decode_sq905c(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_decode_stv0680(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_bayer_to_rgb24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);

LIBV4LCONVERT_LOCAL void v4lconvert_bayer_to_bgr24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);

LIBV4LCONVERT_LOCAL void v4lconvert_bayer_to_yuv420(const u8 *bayer, u8 *yuv, u32 width, u32 height, unsigned int src_pixfmt, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_hm12_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_hm12_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

LIBV4LCONVERT_LOCAL void v4lconvert_hm12_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, bool yvu);

LIBV4LCONVERT_LOCAL void v4lconvert_rotate90(u8 *src, u8 *dest, struct v4l2_format *fmt);

LIBV4LCONVERT_LOCAL void v4lconvert_flip(u8 *src, u8 *dest, struct v4l2_format *fmt, bool hflip, bool vflip);

LIBV4LCONVERT_LOCAL void v4lconvert_crop(u8 *src, u8 *dest, const struct v4l2_format *src_fmt, const struct v4l2_format *dest_fmt);

LIBV4LCONVERT_LOCAL int v4lconvert_helper_decompress(struct v4lconvert_data *data, const char *helper, const u8 *src, unsigned int src_size, u8 *dest, unsigned int dest_size, u32 width, u32 height, int command);

LIBV4LCONVERT_LOCAL void v4lconvert_helper_cleanup(struct v4lconvert_data *data);
#pragma GCC visibility pop
#endif
