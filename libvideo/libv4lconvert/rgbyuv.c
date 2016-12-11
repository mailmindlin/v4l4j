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
	u8 *udest = dest;
	u8 *vdest = dest + src_fmt->fmt.pix.width * src_fmt->fmt.pix.height / 4;
	if (yvu) {
		//Swap udest/vdest
		u8 *tmp = udest;
		udest = vdest;
		vdest = udest;
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

#define SCALEBITS 		10
#define ONE_HALF		(1UL << (SCALEBITS - 1))	
#define CLIP256(color)	(u8)({int _color = (color);(_color > 0xFF) ? 0xFF : ((_color < 0) ? 0 : _color);})
#define CLIP(color)		CLIP256(color)
#if defined(CONVERSION_QUALITY) && (CONVERSION_QUALITY < 0 || CONVERSION_QUALITY > 3)
	#undef CONVERSION_QUALITY
#endif
#ifndef CONVERSION_QUALITY
	#pragma message "Defaulting CONVERSION_QUALITY to 3 (full float)"
	#define CONVERSION_QUALITY 3
#endif
#if CONVERSION_QUALITY == 0
	// Optimized for speed, at the expense of precision.
	// Might not be worth it nowdays if you have a co-processor or FPU
	#define FIX(x)			(x)
	#define UV2V1(u, v)		((((v) << 1) + (v)) >> 1)
	#define UV2RG(u, v)		(((u << 1) + (u) + ((v) << 2) + ((v) << 1)) >> 3)
	#define UV2U1(u, v)		((((u) << 7) + (u)) >> 6)
#elif CONVERSION_QUALITY == 1
	// Integer approximation. Middling speed
	#define FIX(x)			(x)
	#define UV2V1(u, v)		(((v) * 1436) >> 10)
	#define UV2RG(u, v)		(((u) * 352 + ((v) * 731)) >> 10)
	#define UV2U1(u, v)		(((u) * 1814) >> 10)
#elif CONVERSION_QUALITY == 2
	// Fixed-point arithmetic. Kinda fast, and should get you the precision you need
	// Can still be beat out by some FPU's, just because it requires more instructions
	#define FIX(x)			((int)((x) * (1UL << SCALEBITS) + 0.5))
	#define UV2V1(u, v)		(FIX(1.40200) * v + ONE_HALF)
	#define UV2RG(u, v)		(FIX(0.34414) * u + FIX(0.71414) * v - ONE_HALF)
	#define UV2U1(u, v)		(FIX(1.77200) * u + ONE_HALF)
	#define CLIP(color)		CLIP256((color) >> SCALEBITS)
#elif CONVERSION_QUALITY == 3
	// Relatively slow conversion, but nice and accurate
	// If the processor has a fpu, these shouldn't be much worse
	#define FIX(x)			(x)
	#define UV2V1(u, v)		(int) (0.00000f * (float) (u) + 1.13983f * (float) (v) + 0.5f)
	#define UV2RG(u, v)		(int) (0.39465f * (float) (u) + 0.58060f * (float) (v) - 0.5f)
	#define UV2U1(u, v)		(int) (2.03211f * (float) (u) + 0.00000f * (float) (v) + 0.5f)
#endif

void v4lconvert_yuv420_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height, int yvu) {
	const u8 *ysrc = src;
	const u8 *usrc = src + width * height;
	const u8 *vsrc = usrc + (width * height) / 4;
	
	if (yvu) {
		const u8* tmp = usrc;
		usrc = vsrc;
		vsrc = tmp;
	}
	
	v4lconvert_cvt_yuv420_to_bgr24(ysrc, usrc, vsrc, dest, width, height);
}

__attribute__((inline)) void v4lconvert_cvt_yuv420_to_bgr24(const u8 *ysrc, const u8* usrc, const u8* vsrc, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int y = FIX(*ysrc++);
			int u = *usrc++ - 128;
			int v = *vsrc++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);
			
			*dest++ = CLIP(y + u1);
			*dest++ = CLIP(y - rg);
			*dest++ = CLIP(y + v1);

			y = FIX(*ysrc++);
			*dest++ = CLIP(y + u1);
			*dest++ = CLIP(y - rg);
			*dest++ = CLIP(y + v1);
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
			int y = FIX(*ysrc++);
			int u = *usrc++ - 128;
			int v = *vsrc++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP(y + v1);
			*dest++ = CLIP(y - rg);
			*dest++ = CLIP(y + u1);
			
			y = FIX(*ysrc++);
			*dest++ = CLIP(y + v1);
			*dest++ = CLIP(y - rg);
			*dest++ = CLIP(y + u1);
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
			int y1 = FIX(*src++);
			int u = *src++ - 128;
			int y2 = FIX(*src++);
			int v = *src++ - 128;
			
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
			int y1 = FIX(*src++);
			int u = *src++ - 128;
			int y2 = FIX(*src++);
			int v = *src++ - 128;
			
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
	
	u8 *udest = dest;
	u8 *vdest = dest + width * height / 4;
	if (yvu) {
		u8 *tmp = udest;
		udest = vdest;
		vdest = tmp;
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
			int y1 = FIX(*src++);
			int v = *src++ - 128;
			int y2 = FIX(*src++);
			int u = *src++ - 128;
			
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
			int y1 = FIX(*src++);
			int v = *src++ - 128;
			int y2 = FIX(*src++);
			int u = *src++ - 128;
			
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
			int u = *src++ - 128;
			int y1 = FIX(*src++);
			int v = *src++ - 128;
			int y2 = FIX(*src++);
			
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
			int u = *src++ - 128;
			int y1 = FIX(*src++);
			int v = *src++ - 128;
			int y2 = FIX(*src++);
			
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
	/* copy the Y values */
	const u8 *src1 = src;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			*dest++ = src1[1];
			*dest++ = src1[3];
			src1 += 4;
		}
	}

	/* copy the U and V values */
	src1 = src + width * 2;		/* next line */
	u8 *udest = dest;
	u8 *vdest = dest + width * height / 4;
	if (yvu) {
		u8 *tmp = udest;
		udest = vdest;
		vdest = udest;
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
		u8 tmp0 = *src++;
		u8 tmp1 = *src++;
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
	/* Y */
	for (unsigned int y = 0; y < src_fmt->fmt.pix.height; y++) {
		for (unsigned int x = 0; x < src_fmt->fmt.pix.width; x++) {
			unsigned short tmp = *(unsigned short *)src;
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
	u8* udest = dest;
	u8* vdest = dest + src_fmt->fmt.pix.width * src_fmt->fmt.pix.height / 4;
	if (yvu) {
		//Swap buffers
		u8 *tmp = udest;
		udest = vdest;
		vdest = tmp;
	}

	for (unsigned int y = 0; y < src_fmt->fmt.pix.height / 2; y++) {
		for (unsigned int x = 0; x < src_fmt->fmt.pix.width / 2; x++) {
			unsigned short tmp = *(unsigned short *)src;
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

void v4lconvert_grey_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height) {
	/* Y */
	//TODO replace w/ memcpy?
	for (unsigned int y = 0; y < height; y++)
		for (unsigned int x = 0; x < width; x++)
			*dest++ = *src++;

	/* Clear U/V */
	memset(dest, 0x80, width * height / 2);
}

int v4lconvert_y10b_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
	uint32_t buffer = 0;
	unsigned int bitsIn = 0;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			//Get some bits to put in the buffer.
			while (bitsIn < 10) {
				buffer = (buffer << 8) | *(src++);
				bitsIn += 8;
			}
			bitsIn -= 10;
			// Only 10 useful bits, so we discard the LSBs
			//int mask10 = (1 << 10) - 1;
			//uint16_t unpacked = (buffer >> bitsIn) & mask10;//Unpacked value
			//uint8_t value = (uint8_t) ((unpacked >> 2) & 0xFF);//Discard LSB
			uint8_t value = (uint8_t) ((buffer >> (bitsIn + 2)) & 0xFF);
			*dest++ = value;
			*dest++ = value;
			*dest++ = value;
		}
	}
	return 0;
}

int v4lconvert_y10b_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height) {
	/* Y */
	uint32_t buffer = 0;
	unsigned int bitsIn = 0;
	for (unsigned int y = 0; y < height; y++)
		for (unsigned int x = 0; x < width; x++) {
			while (bitsIn < 10) {
				buffer = (buffer << 8) | *(src++);
				bitsIn += 8;
			}
			bitsIn -= 10;
			// Only 10 useful bits, so we discard the LSBs
			//int mask10 = (1 << 10) - 1;
			//uint16_t unpacked = (buffer >> bitsIn) & mask10;//Unpacked value
			//uint8_t value = (uint8_t) ((unpacked >> 2) & 0xFF);//Discard LSB
			uint8_t value = (uint8_t) ((buffer >> (bitsIn + 2)) & 0xFF);
			*dest++ = value;
		}

	/* Clear U/V */
	memset(dest, 0x80, width * height / 2);

	return 0;
}

#undef RGB2Y