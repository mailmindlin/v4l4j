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
#include "rgbyuv.h" //For RGB2Y, RGB2U, RGB2V, UV2U1, UV2RG, UV2V1 macros

#ifdef CONVERT_ARM_NEON
#pragma message "NEON sez hi!"
#include "neon/rgbyuv_neon.h"
#else
#pragma message "NEON sez nope!"
#endif

void v4lconvert_rgb24_to_yuv420(const u8 *src, u8 *dest, unsigned int width, unsigned int height, unsigned int bytesperline, bool bgr, bool yvu) {
	/* Y */
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			if (bgr)
				*dest++ = RGB2Y(src[2], src[1], src[0]);
			else
				*dest++ = RGB2Y(src[0], src[1], src[2]);
			src += 3;
		}
		src += bytesperline - 3 * width;
	}
	src -= height * bytesperline;

	/* U + V */
	u8 *udest = dest;
	u8 *vdest = dest + width * height / 4;
	if (yvu) {
		//Swap udest/vdest
		u8 *tmp = udest;
		udest = vdest;
		vdest = tmp;
	}

	for (unsigned int y = 0; y < height / 2; y++) {
		//Pointer to line below the current one
		//TODO preserve pointer accross iterations
		const u8* nextLine = src + bytesperline;
		for (unsigned int x = 0; x < width / 2; x++) {
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
		src += 2 * bytesperline - 3 * width;
	}
}

void v4lconvert_yuv420_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height, bool yvu) {
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
			int y = FIX_Y(*ysrc++);
			int u = *usrc++ - 128;
			int v = *vsrc++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);
			
			*dest++ = CLIP_RGB(y + u1);
			*dest++ = CLIP_RGB(y - rg);
			*dest++ = CLIP_RGB(y + v1);

			y = FIX_Y(*ysrc++);
			*dest++ = CLIP_RGB(y + u1);
			*dest++ = CLIP_RGB(y - rg);
			*dest++ = CLIP_RGB(y + v1);
		}
		/* Rewind u and v for next line */
		if (!(i & 1)) {
			usrc -= width / 2;
			vsrc -= width / 2;
		}
	}
}

void v4lconvert_yuv420_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height, bool yvu) {
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
			int y = FIX_Y(*ysrc++);
			int u = *usrc++ - 128;
			int v = *vsrc++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP_RGB(y + v1);
			*dest++ = CLIP_RGB(y - rg);
			*dest++ = CLIP_RGB(y + u1);
			
			y = FIX_Y(*ysrc++);
			*dest++ = CLIP_RGB(y + v1);
			*dest++ = CLIP_RGB(y - rg);
			*dest++ = CLIP_RGB(y + u1);
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
			int y1 = FIX_Y(*src++);
			int u = *src++ - 128;
			int y2 = FIX_Y(*src++);
			int v = *src++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP_RGB(y1 + u1);
			*dest++ = CLIP_RGB(y1 - rg);
			*dest++ = CLIP_RGB(y1 + v1);

			*dest++ = CLIP_RGB(y2 + u1);
			*dest++ = CLIP_RGB(y2 - rg);
			*dest++ = CLIP_RGB(y2 + v1);
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
			int y1 = FIX_Y(*src++);
			int u = *src++ - 128;
			int y2 = FIX_Y(*src++);
			int v = *src++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP_RGB(y1 + v1);
			*dest++ = CLIP_RGB(y1 - rg);
			*dest++ = CLIP_RGB(y1 + u1);

			*dest++ = CLIP_RGB(y2 + v1);
			*dest++ = CLIP_RGB(y2 - rg);
			*dest++ = CLIP_RGB(y2 + u1);
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
void v4lconvert_yuyv_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height, bool yvu) {
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
			int y1 = FIX_Y(*src++);
			int v = *src++ - 128;
			int y2 = FIX_Y(*src++);
			int u = *src++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP_RGB(y1 + u1);
			*dest++ = CLIP_RGB(y1 - rg);
			*dest++ = CLIP_RGB(y1 + v1);

			*dest++ = CLIP_RGB(y2 + u1);
			*dest++ = CLIP_RGB(y2 - rg);
			*dest++ = CLIP_RGB(y2 + v1);
		}
	}
}

void v4lconvert_yvyu_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int y1 = FIX_Y(*src++);
			int v = *src++ - 128;
			int y2 = FIX_Y(*src++);
			int u = *src++ - 128;
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);
			
			*dest++ = CLIP_RGB(y1 + v1);
			*dest++ = CLIP_RGB(y1 - rg);
			*dest++ = CLIP_RGB(y1 + u1);

			*dest++ = CLIP_RGB(y2 + v1);
			*dest++ = CLIP_RGB(y2 - rg);
			*dest++ = CLIP_RGB(y2 + u1);
		}
	}
}

void v4lconvert_uyvy_to_bgr24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int u = *src++ - 128;
			int y1 = FIX_Y(*src++);
			int v = *src++ - 128;
			int y2 = FIX_Y(*src++);
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP_RGB(y1 + u1);
			*dest++ = CLIP_RGB(y1 - rg);
			*dest++ = CLIP_RGB(y1 + v1);

			*dest++ = CLIP_RGB(y2 + u1);
			*dest++ = CLIP_RGB(y2 - rg);
			*dest++ = CLIP_RGB(y2 + v1);
		}
	}
}

void v4lconvert_uyvy_to_rgb24(const u8 *src, u8 *dest, u32 width, u32 height) {
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j += 2) {
			int u = *src++ - 128;
			int y1 = FIX_Y(*src++);
			int v = *src++ - 128;
			int y2 = FIX_Y(*src++);
			
			int u1 = UV2U1(u, v);
			int rg = UV2RG(u, v);
			int v1 = UV2V1(u, v);

			*dest++ = CLIP_RGB(y1 + v1);
			*dest++ = CLIP_RGB(y1 - rg);
			*dest++ = CLIP_RGB(y1 + u1);

			*dest++ = CLIP_RGB(y2 + v1);
			*dest++ = CLIP_RGB(y2 - rg);
			*dest++ = CLIP_RGB(y2 + u1);
		}
	}
}

void v4lconvert_uyvy_to_yuv420(const u8 *src, u8 *dest, u32 width, u32 height, bool yvu) {
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
		vdest = tmp;
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

void v4lconvert_swap_uv(const u8 *src, u8 *dest, unsigned int width, unsigned int height, unsigned int bytesperline) {
	/* Copy Y */
	for (unsigned int y = 0; y < height; y++) {
		memcpy(dest, src, width);
		dest += width;
		src += bytesperline;
	}

	/* Copy component 2 */
	src += height * bytesperline / 4;
	for (unsigned int y = 0; y < height / 2; y++) {
		memcpy(dest, src, width / 2);
		dest += width / 2;
		src += bytesperline / 2;
	}

	/* Copy component 1 */
	src -= height * bytesperline / 2;
	for (unsigned int y = 0; y < height / 2; y++) {
		memcpy(dest, src, width / 2);
		dest += width / 2;
		src += bytesperline / 2;
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

void v4lconvert_rgb565_to_yuv420(const u8 *src, u8 *dest, unsigned int width, unsigned int height, unsigned int bytesperline, bool yvu) {
	/* Y */
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			unsigned short tmp = *(unsigned short *)src;
			unsigned int r = 0xf8 & (tmp << 3);
			unsigned int g = 0xfc & (tmp >> 3);
			unsigned int b = 0xf8 & (tmp >> 8);
			*dest++ = (u8) RGB2Y(r, g, b);
			src += 2;
		}
		src += bytesperline - 2 * width;
	}
	src -= height * bytesperline;

	/* U + V */
	u8* udest = dest;
	u8* vdest = dest + width * height / 4;
	if (yvu) {
		//Swap buffers
		u8 *tmp = udest;
		udest = vdest;
		vdest = tmp;
	}

	for (unsigned int y = 0; y < height / 2; y++) {
		for (unsigned int x = 0; x < width / 2; x++) {
			unsigned short tmp = *(unsigned short *)src;
			unsigned int avg_red = 0xf8 & (tmp << 3);
			unsigned int avg_green = 0xfc & (tmp >> 3);
			unsigned int avg_blue = 0xf8 & (tmp >> 8);

			tmp = *(((unsigned short *)src) + 1);
			avg_red += 0xf8 & (tmp << 3);
			avg_green += 0xfc & (tmp >> 3);
			avg_blue += 0xf8 & (tmp >> 8);

			tmp = *(((unsigned short *)src) + bytesperline);
			avg_red += 0xf8 & (tmp << 3);
			avg_green += 0xfc & (tmp >> 3);
			avg_blue += 0xf8 & (tmp >> 8);

			tmp = *(((unsigned short *)src) + bytesperline + 1);
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
		src += 2 * bytesperline - 2 * width;
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