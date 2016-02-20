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

#ifndef __LIBV4LCONVERT_H
#define __LIBV4LCONVERT_H

#define ARRAY_SIZE(x) ((int)sizeof(x)/(int)sizeof((x)[0]))

#define V4LCONVERT_ERROR_MSG_SIZE 256
#define V4LCONVERT_MAX_FRAMESIZES 256

#define V4LCONVERT_ERR(...) \
	snprintf(data->error_msg, V4LCONVERT_ERROR_MSG_SIZE, \
			"v4l-convert: error " __VA_ARGS__)

/* Card flags */
#define V4LCONVERT_IS_UVC                0x01
#define V4LCONVERT_USE_TINYJPEG          0x02

struct v4lconvert_data;

void v4lconvert_rgb24_to_yuv420(const u8 *src, u8 *dest, const struct v4l2_format *src_fmt, int bgr, int yvu);

void v4lconvert_yuv420_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_yuv420_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_yuyv_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_yuyv_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_yuyv_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_yvyu_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_yvyu_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_uyvy_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_uyvy_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_uyvy_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_swap_rgb(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_swap_uv(const u8 *src, u8 *dst, const struct v4l2_format *src_fmt);

void v4lconvert_grey_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height);

void v4lconvert_grey_to_yuv420(const u8 *src, u8 *dest, const struct v4l2_format *src_fmt);

int v4lconvert_y10b_to_rgb24(struct v4lconvert_data *data, const u8 *src, u8 *dest, u32 width, u32 height);

int v4lconvert_y10b_to_yuv420(struct v4lconvert_data *data, const u8 *src, u8 *dest, u32 width, u32 height);

void v4lconvert_rgb565_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height);

void v4lconvert_rgb565_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height);

void v4lconvert_rgb565_to_yuv420(const u8 *src, u8 *dest, const struct v4l2_format *src_fmt, int yvu);

void v4lconvert_spca501_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_spca505_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_spca508_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_cit_yyvyuy_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, int yvu);

void v4lconvert_konica_yuv420_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, int yvu);

void v4lconvert_m420_to_yuv420(const u8 *src, u8 *ydest, u32 width, u32 height, int yvu);

int v4lconvert_cpia1_to_yuv420(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_sn9c20x_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

int v4lconvert_se401_to_rgb24(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dest, u32 width, u32 height);

int v4lconvert_decode_jpeg_tinyjpeg(struct v4lconvert_data *data, u8 *src, int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt, int flags);

int v4lconvert_decode_jpeg_libjpeg(struct v4lconvert_data *data, u8 *src, int src_size, u8 *dest, struct v4l2_format *fmt, unsigned int dest_pix_fmt);

int v4lconvert_decode_jpgl(const u8 *src, u32 src_size, unsigned int dest_pix_fmt, u8 *dest, u32 width, u32 height);

void v4lconvert_decode_spca561(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_decode_sn9c10x(const u8 *src, u8 *dst, u32 width, u32 height);

int v4lconvert_decode_pac207(struct v4lconvert_data *data, const u8 *inp, int src_size, u8 *outp, u32 width, u32 height);

int v4lconvert_decode_mr97310a(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dst, u32 width, u32 height);

int v4lconvert_decode_jl2005bcd(struct v4lconvert_data *data, const u8 *src, int src_size, u8 *dest, u32 width, u32 height);

void v4lconvert_decode_sn9c2028(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_decode_sq905c(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_decode_stv0680(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_bayer_to_rgb24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);

void v4lconvert_bayer_to_bgr24(const u8 *bayer, u8 *rgb, u32 width, u32 height, unsigned int pixfmt);

void v4lconvert_bayer_to_yuv420(const u8 *bayer, u8 *yuv, u32 width, u32 height, unsigned int src_pixfmt, int yvu);

void v4lconvert_hm12_to_rgb24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_hm12_to_bgr24(const u8 *src, u8 *dst, u32 width, u32 height);

void v4lconvert_hm12_to_yuv420(const u8 *src, u8 *dst, u32 width, u32 height, int yvu);

void v4lconvert_rotate90(u8 *src, u8 *dest, struct v4l2_format *fmt);

void v4lconvert_flip(u8 *src, u8 *dest, struct v4l2_format *fmt, int hflip, int vflip);

void v4lconvert_crop(u8 *src, u8 *dest, const struct v4l2_format *src_fmt, const struct v4l2_format *dest_fmt);

int v4lconvert_helper_decompress(struct v4lconvert_data *data, const char *helper, const u8 *src, int src_size, u8 *dest, int dest_size, u32 width, u32 height, int command);

void v4lconvert_helper_cleanup(struct v4lconvert_data *data);

#endif
