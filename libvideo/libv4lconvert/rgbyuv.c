/*

# RGB <-> YUV conversion routines
#             (C) 2008 Hans de Goede <hdegoede@redhat.com>

# RGB565 conversion routines
#             (C) 2009 Mauro Carvalho Chehab <mchehab@redhat.com>

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

#include <string.h>
#include <stdint.h>
#include "libv4lconvert-priv.h"

#ifdef CONVERT_ARM_NEON
#pragma message "NEON sez hi!"
#include "neon/rgbyuv_neon.h"
#else
#pragma message "NEON sez nope!"
#endif

#define RGB2Y(r, g, b) \
	((u8) ((8453 * (r) + 16594 * (g) + 3223 * (b) + 524288) >> 15))

#define RGB2U(r, g, b) \
	((u8) ((-4878 * (signed) (r) - 9578 * (signed) (g) + 14456 * (signed) (b) + 4210688) >> 15))

#define RGB2V(r, g, b) \
	((u8) ((14456 * (r) - 12105 * (g) - 2351 * (b) + 4210688) >> 15))

void v4lconvert_rgb24_to_yuv420(const u8 *src, u8 *dest, const struct v4l2_format *src_fmt, int bgr, int yvu) {
	u8 *udest, *vdest;

	/* Y */
	for (unsigned int y = 0; y < src_fmt->fmt.pix.height; y++) {
		for (unsigned int x = 0; x < src_fmt->fmt.pix.width; x++) {
			if (bgr)
				*dest++ = RGB2Y(src[2], src[1], src[0]);
			else
				*dest++ = RGB2Y(src[0], src[1], src[2]);
			src += 3;
		}
		src += src_fmt->fmt.pix.bytesperline - 3 * src_fmt->fmt.pix.width;
	}
	src -= src_fmt->fmt.pix.height * src_fmt->fmt.pix.bytesperline;

	/* U + V */
	if (yvu) {
		vdest = dest;
		udest = dest + src_fmt->fmt.pix.width * src_fmt->fmt.pix.height / 4;
	} else {
		udest = dest;
		vdest = dest + src_fmt->fmt.pix.width * src_fmt->fmt.pix.height / 4;
	}

	for (unsigned int y = 0; y < src_fmt->fmt.pix.height / 2; y++) {
		//Pointer to line below the current one
		//TODO preserve pointer accross iterations
		const u8* nextLine = src + src_fmt->fmt.pix.bytesperline;
		for (unsigned int x = 0; x < src_fmt->fmt.pix.width / 2; x++) {
			int avg_red = *src++ + *nextLine++;
			int avg_green = *src++ + *nextLine++;
			int avg_blue = *src++ + *nextLine++;
			
			avg_red += *src++ + *nextLine++;
			avg_green += *src++ + *nextLine++;
			avg_blue += *src++ + *nextLine++;
			
			avg_red = (avg_red + 2) / 4;
			avg_green = (avg_green + 2) / 4;
			avg_blue = (avg_blue + 2) / 4;
			if (bgr) {
				int tmp = avg_red;
				avg_red = avg_blue;
				avg_blue = tmp;
			}
			*udest++ = RGB2U(avg_red, avg_green, avg_blue);
			*vdest++ = RGB2V(avg_red, avg_green, avg_blue);
		}
		src += 2 * src_fmt->fmt.pix.bytesperline - 3 * src_fmt->fmt.pix.width;
	}
}

#ifdef GOOD_CONVERSION
	// Relatively slow conversion, but nice and accurate
	// If the processor has a fpu, these shouldn't be much worse
	#define UV2V1(u, v) (int) (0.00000f * (u) + 1.13983f * (v) + 0.5f)
	#define UV2RG(u, v) (int) (0.39465f * (u) + 0.58060f * (v) + 0.5f)
	#define UV2U1(u, v) (int) (2.03211f * (u) + 0.00000f * (v) + 0.5f)
#elif defined(FAST_CONVERSION)
	// Optimized for speed, at the expense of precision.
	// Might not be worth it nowdays if you have a co-processor or FPU
	#define UV2V1(u, v) ((((v) << 1) + (v)) >> 1)
	#define UV2RG(u, v) (((u << 1) + (u) + ((v) << 2) + ((v) << 1)) >> 3)
	#define UV2U1(u, v) ((((u) << 7) + (u)) >> 6)
#else
	// Integer approximation. Middling speed
	#define UV2V1(u, v) (((v) * 1436) >> 10)
	#define UV2RG(u, v) (((u) * 352 + ((v) * 731)) >> 10)
	#define UV2U1(u, v) (((u) * 1814) >> 10)
#endif
#define CLIP(color) (u8)(((color) > 0xFF) ? 0xFF : (((color) < 0) ? 0 : (color)))

void v4lconvert_yuv420_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height, int yvu) {
	const u8 *ysrc = src;
	const u8 *usrc, *vsrc;

	if (yvu) {
		vsrc = src + width * height;
		usrc = vsrc + (width * height) / 4;
	} else {
		usrc = src + width * height;
		vsrc = usrc + (width * height) / 4;
	}

	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int u = *usrc++ - 128;
			int v = *vsrc++ - 128;
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);
			*dest++ = CLIP(*ysrc + u1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + v1);
			ysrc++;

			*dest++ = CLIP(*ysrc + u1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + v1);
			ysrc++;
		}
		/* Rewind u and v for next line */
		if (!(i & 1)) {
			usrc -= width / 2;
			vsrc -= width / 2;
		}
	}
}

void v4lconvert_yuv420_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height, int yvu) {
	const u8 *ysrc = src;
	const u8 *usrc, *vsrc;

	if (yvu) {
		vsrc = src + width * height;
		usrc = vsrc + (width * height) / 4;
	} else {
		usrc = src + width * height;
		vsrc = usrc + (width * height) / 4;
	}

	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int u = *usrc++ - 128;
			int v = *vsrc++ - 128;
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP(*ysrc + v1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + u1);
			ysrc++;

			*dest++ = CLIP(*ysrc + v1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + u1);
			ysrc++;
		}
		/* Rewind u and v for next line */
		if (!(i&1)) {
			usrc -= width / 2;
			vsrc -= width / 2;
		}
	}
}

void v4lconvert_yuyv_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int y1 = *(src++);
			int u = *(src++) - 128;
			int y2 = *(src++);
			int v = *(src++) - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP(y1 + u1);
			*dest++ = CLIP(y1 - rg);
			*dest++ = CLIP(y1 + v1);

			*dest++ = CLIP(y2 + u1);
			*dest++ = CLIP(y2 - rg);
			*dest++ = CLIP(y2 + v1);
		}
	}
}

void v4lconvert_yuyv_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
#ifdef CONVERT_ARM_NEON
	//TODO handle overflow
	v4lconvert_neon_yuyv_to_rgb24(src, dest, height * (width / 2));
#else
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int y1 = *(src++);
			int u = *(src++) - 128;
			int y2 = *(src++);
			int v = *(src++) - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP(y1 + v1);
			*dest++ = CLIP(y1 - rg);
			*dest++ = CLIP(y1 + u1);

			*dest++ = CLIP(y2 + v1);
			*dest++ = CLIP(y2 - rg);
			*dest++ = CLIP(y2 + u1);
		}
	}
#endif
}

/**
 * @param src source buffer
 * @param dest output buffer
 * @param width width of frame
 * @param height height of frame
 * @param yvu whether to output in yuv420 or yvu420
 */
void v4lconvert_yuyv_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height, int yvu) {
	u8 *udest, *vdest;

	/* copy the Y values */
	const u8* src1 = src;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			*dest++ = src1[0];
			*dest++ = src1[2];
			src1 += 4;
		}
	}

	/* copy the U and V values */
	src++;				/* point to V */
	src1 = src + width * 2;		/* next line */
	if (yvu) {
		vdest = dest;
		udest = dest + width * height / 4;
	} else {
		udest = dest;
		vdest = dest + width * height / 4;
	}
	for (unsigned int i = 0; i < height; i += 2) {
		for (unsigned int j = 0; j < width; j += 2) {
			*udest++ = (u8) (((int) src[0] + src1[0]) / 2);	/* U */
			*vdest++ = (u8) (((int) src[2] + src1[2]) / 2);	/* V */
			src += 4;
			src1 += 4;
		}
		src = src1;
		src1 += width * 2;
	}
}

void v4lconvert_yvyu_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int y1 = *(src++);
			int v = *(src++) - 128;
			int y2 = *(src++);
			int u = *(src++) - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP(y1 + u1);
			*dest++ = CLIP(y1 - rg);
			*dest++ = CLIP(y1 + v1);

			*dest++ = CLIP(y2 + u1);
			*dest++ = CLIP(y2 - rg);
			*dest++ = CLIP(y2 + v1);
		}
	}
}

void v4lconvert_yvyu_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int y1 = *(src++);
			int v = *(src++) - 128;
			int y2 = *(src++);
			int u = *(src++) - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);
			
			*dest++ = CLIP(y1 + v1);
			*dest++ = CLIP(y1 - rg);
			*dest++ = CLIP(y1 + u1);

			*dest++ = CLIP(y2 + v1);
			*dest++ = CLIP(y2 - rg);
			*dest++ = CLIP(y2 + u1);
		}
	}
}

void v4lconvert_uyvy_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int u = *(src++) - 128;
			int y1 = *(src++);
			int v = *(src++) - 128;
			int y2 = *(src++);
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP(y1 + u1);
			*dest++ = CLIP(y1 - rg);
			*dest++ = CLIP(y1 + v1);

			*dest++ = CLIP(y2 + u1);
			*dest++ = CLIP(y2 - rg);
			*dest++ = CLIP(y2 + v1);
		}
	}
}

void v4lconvert_uyvy_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int u = *(src++) - 128;
			int y1 = *(src++);
			int v = *(src++) - 128;
			int y2 = *(src++);
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP(y1 + v1);
			*dest++ = CLIP(y1 - rg);
			*dest++ = CLIP(y1 + u1);

			*dest++ = CLIP(y2 + v1);
			*dest++ = CLIP(y2 - rg);
			*dest++ = CLIP(y2 + u1);
		}
	}
}

void v4lconvert_uyvy_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height, int yvu) {
	const u8 *src1;
	u8 *udest, *vdest;

	/* copy the Y values */
	src1 = src;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			*dest++ = src1[1];
			*dest++ = src1[3];
			src1 += 4;
		}
	}

	/* copy the U and V values */
	src1 = src + width * 2;		/* next line */
	if (yvu) {
		vdest = dest;
		udest = dest + width * height / 4;
	} else {
		udest = dest;
		vdest = dest + width * height / 4;
	}
	for (unsigned int i = 0; i < height; i += 2) {
		for (unsigned int j = 0; j < width; j += 2) {
			*udest++ = (u8) ((int) src[0] + src1[0]) / 2;	/* U */
			*vdest++ = (u8) ((int) src[2] + src1[2]) / 2;	/* V */
			src += 4;
			src1 += 4;
		}
		src = src1;
		src1 += width * 2;
	}
}

void v4lconvert_swap_rgb(const u8 *src, u8 *dst, u32 width, u32 height) {
	for (unsigned int i = 0; i < (width * height); i++) {
		u8 tmp0, tmp1;
		tmp0 = *src++;
		tmp1 = *src++;
		*dst++ = *src++;
		*dst++ = tmp1;
		*dst++ = tmp0;
	}
}

void v4lconvert_swap_uv(const u8 *src, u8 *dest, const struct v4l2_format *src_fmt) {
	/* Copy Y */
	for (unsigned int y = 0; y < src_fmt->fmt.pix.height; y++) {
		memcpy(dest, src, src_fmt->fmt.pix.width);
		dest += src_fmt->fmt.pix.width;
		src += src_fmt->fmt.pix.bytesperline;
	}

	/* Copy component 2 */
	src += src_fmt->fmt.pix.height * src_fmt->fmt.pix.bytesperline / 4;
	for (unsigned int y = 0; y < src_fmt->fmt.pix.height / 2; y++) {
		memcpy(dest, src, src_fmt->fmt.pix.width / 2);
		dest += src_fmt->fmt.pix.width / 2;
		src += src_fmt->fmt.pix.bytesperline / 2;
	}

	/* Copy component 1 */
	src -= src_fmt->fmt.pix.height * src_fmt->fmt.pix.bytesperline / 2;
	for (unsigned int y = 0; y < src_fmt->fmt.pix.height / 2; y++) {
		memcpy(dest, src, src_fmt->fmt.pix.width / 2);
		dest += src_fmt->fmt.pix.width / 2;
		src += src_fmt->fmt.pix.bytesperline / 2;
	}
}

void v4lconvert_rgb565_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			unsigned short tmp = *(unsigned short *)src;

			/* Original format: rrrrrggg gggbbbbb */
			*dest++ = (u8) (0xf8 & (tmp >> 8));
			*dest++ = (u8) (0xfc & (tmp >> 3));
			*dest++ = (u8) (0xf8 & (tmp << 3));

			src += 2;
		}
	}
}

void v4lconvert_rgb565_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			unsigned short tmp = *(unsigned short *)src;

			/* Original format: rrrrrggg gggbbbbb */
			*dest++ = (u8) (0xf8 & (tmp << 3));
			*dest++ = (u8) (0xfc & (tmp >> 3));
			*dest++ = (u8) (0xf8 & (tmp >> 8));

			src += 2;
		}
	}
}

void v4lconvert_rgb565_to_yuv420(const u8 *src, u8 *dest, const struct v4l2_format *src_fmt, int yvu) {
	unsigned short tmp;
	
	/* Y */
	for (unsigned int y = 0; y < src_fmt->fmt.pix.height; y++) {
		for (unsigned int x = 0; x < src_fmt->fmt.pix.width; x++) {
			tmp = *(unsigned short *)src;
			unsigned int r = 0xf8 & (tmp << 3);
			unsigned int g = 0xfc & (tmp >> 3);
			unsigned int b = 0xf8 & (tmp >> 8);
			*dest++ = (u8) RGB2Y(r, g, b);
			src += 2;
		}
		src += src_fmt->fmt.pix.bytesperline - 2 * src_fmt->fmt.pix.width;
	}
	src -= src_fmt->fmt.pix.height * src_fmt->fmt.pix.bytesperline;

	/* U + V */
	u8* udest;
	u8* vdest;
	if (yvu) {
		vdest = dest;
		udest = dest + src_fmt->fmt.pix.width * src_fmt->fmt.pix.height / 4;
	} else {
		udest = dest;
		vdest = dest + src_fmt->fmt.pix.width * src_fmt->fmt.pix.height / 4;
	}

	for (unsigned int y = 0; y < src_fmt->fmt.pix.height / 2; y++) {
		for (unsigned int x = 0; x < src_fmt->fmt.pix.width / 2; x++) {
			tmp = *(unsigned short *)src;
			unsigned int avg_red = 0xf8 & (tmp << 3);
			unsigned int avg_green = 0xfc & (tmp >> 3);
			unsigned int avg_blue = 0xf8 & (tmp >> 8);

			tmp = *(((unsigned short *)src) + 1);
			avg_red += 0xf8 & (tmp << 3);
			avg_green += 0xfc & (tmp >> 3);
			avg_blue += 0xf8 & (tmp >> 8);

			tmp = *(((unsigned short *)src) + src_fmt->fmt.pix.bytesperline);
			avg_red += 0xf8 & (tmp << 3);
			avg_green += 0xfc & (tmp >> 3);
			avg_blue += 0xf8 & (tmp >> 8);

			tmp = *(((unsigned short *)src) + src_fmt->fmt.pix.bytesperline + 1);
			avg_red += 0xf8 & (tmp << 3);
			avg_green += 0xfc & (tmp >> 3);
			avg_blue += 0xf8 & (tmp >> 8);
			
			//Add 2 because right shift is floor(avg/4), so it evens out the results
			avg_red = (avg_green + 2) / 4;
			avg_green = (avg_green + 2) / 4;
			avg_blue = (avg_blue + 2) / 4;
			
			*udest++ = (u8) RGB2U(avg_red, avg_green, avg_blue);
			*vdest++ = (u8) RGB2V(avg_red, avg_green, avg_blue);
			src += 4;
		}
		src += 2 * src_fmt->fmt.pix.bytesperline - 2 * src_fmt->fmt.pix.width;
	}
}

void v4lconvert_grey_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			*dest++ = *src;
			*dest++ = *src;
			*dest++ = *src;
			src++;
		}
	}
}

void v4lconvert_grey_to_yuv420(const u8 *src, u8 *dest, const struct v4l2_format *src_fmt) {
	/* Y */
	//TODO replace w/ memcpy?
	for (unsigned int y = 0; y < src_fmt->fmt.pix.height; y++)
		for (unsigned int x = 0; x < src_fmt->fmt.pix.width; x++)
			*dest++ = *src++;

	/* Clear U/V */
	memset(dest, 0x80, src_fmt->fmt.pix.width * src_fmt->fmt.pix.height / 2);
}

/* Unpack buffer of (vw bit) data into padded 16bit buffer. */
static inline void convert_packed_to_16bit(uint8_t *raw, uint16_t *unpacked, unsigned int vw, unsigned int unpacked_len) {
	uint16_t mask = (uint16_t) ((1 << vw) - 1);
	uint32_t buffer = 0;
	unsigned int bitsIn = 0;
	while (unpacked_len--) {
		while (bitsIn < vw) {
			buffer = (buffer << 8) | *(raw++);
			bitsIn += 8;
		}
		bitsIn -= vw;
		*(unpacked++) = (uint16_t) ((buffer >> bitsIn) & mask);
	}
}

int v4lconvert_y10b_to_rgb24(struct v4lconvert_data *data, const u8 *src, u8 *dest, u32 width, u32 height) {
	u8 *unpacked_buffer;

	unpacked_buffer = v4lconvert_alloc_buffer(width * height * 2, &data->convert_pixfmt_buf, &data->convert_pixfmt_buf_size);
	if (!unpacked_buffer)
		return v4lconvert_oom_error(data);

	convert_packed_to_16bit((uint8_t *)src, (uint16_t *)unpacked_buffer, 10, width * height);

	unsigned short *tmp = (unsigned short *)unpacked_buffer;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {

			/* Only 10 useful bits, so we discard the LSBs */
			*dest++ = (u8) ((*tmp & 0x3ff) >> 2);
			*dest++ = (u8) ((*tmp & 0x3ff) >> 2);
			*dest++ = (u8) ((*tmp & 0x3ff) >> 2);

			/* +1 means two bytes as we are dealing with (unsigned short) */
			tmp += 1;
		}
	}
	return 0;
}

int v4lconvert_y10b_to_yuv420(struct v4lconvert_data *data,	const u8 *src, u8 *dest, u32 width, u32 height) {
	u8 *unpacked_buffer = v4lconvert_alloc_buffer(width * height * 2, &data->convert_pixfmt_buf, &data->convert_pixfmt_buf_size);
	if (!unpacked_buffer)
		return v4lconvert_oom_error(data);

	convert_packed_to_16bit((uint8_t *)src, (uint16_t *)unpacked_buffer, 10, width * height);

	unsigned short *tmp = (unsigned short *)unpacked_buffer;

	/* Y */
	for (unsigned int y = 0; y < height; y++)
		for (unsigned int x = 0; x < width; x++) {

			/* Only 10 useful bits, so we discard the LSBs */
			*dest++ = (u8) ((*tmp & 0x3ff) >> 2);

			/* +1 means two bytes as we are dealing with (unsigned short) */
			tmp += 1;
		}

	/* Clear U/V */
	memset(dest, 0x80, width * height / 2);

	return 0;
}

#undef RGB2Y