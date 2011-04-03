/*
 * pixfmt-conv.h
 *
 * Copyright (C) 2011 Gilles Gigan (gilles.gigan@gmail.com)
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


#ifndef PIXFMT_CONV_H_
#define PIXFMT_CONV_H_


#include <stdint.h>
#include <stdio.h>
#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

// Function prototype for all conversions from YUV422 to YUV422P
//typedef void	(*convert_to_yuv422p_fn)(uint8_t* yuyv_src, uint8_t* yuv422p_dst, uint32_t width, uint32_t height);

/*
 * converts 'height' lines in YUV formats into Y, U, V planes.
 */
extern inline void	convert_yuyv_to_yuv422p(uint8_t* yuyv_src, uint8_t* yuv422p_dst, uint32_t width, uint32_t height) {
	uint32_t 	line, col;
	uint8_t		*y, *u, *v;

	y = yuv422p_dst;
	u = y + width * height;
	v = u + width * height / 2;

	for(line = 0; line < height; line++) {
		for (col = 0; col < width; col += 2) {
			*y++ = *yuyv_src++;
			*u++ = *yuyv_src++;
			*y++ = *yuyv_src++;
			*v++ = *yuyv_src++;
		}
	}
}

#ifdef __SSSE3__
static void print_m128(char *name, __m128i *reg) {
	char*   array = (char *)reg;
	        printf("%s:\n%4hhu %4hhu %4hhu %4hhu \t %4hhu %4hhu %4hhu %4hhu \t "
	                        "%4hhu %4hhu %4hhu %4hhu \t %4hhu %4hhu %4hhu %4hhu\n", name,
	                        array[0], array[1], array[2], array[3],
	                        array[4], array[5], array[6], array[7],
	                        array[8], array[9], array[10], array[11],
	                        array[12], array[13], array[14], array[15]);

}
extern inline void	convert_yuyv_to_yuv422p_ssse3(uint8_t* yuyv_src, uint8_t* yuv422p_dst, uint32_t width, uint32_t height) {
	uint32_t	nb_pixels = width * height;
	__m128i*	src = (__m128i *) yuyv_src;
	__m128i*	y_dst = (__m128i *) yuv422p_dst;
	__m128i*	u_dst = (__m128i *) ((uint8_t *)yuv422p_dst + nb_pixels);
	__m128i*	v_dst = (__m128i *) ((uint8_t *)yuv422p_dst + nb_pixels * 3 / 2);

	__m128i		temp = {0x0LL, 0x0LL};


	static const __m128i y_shuff1 = {0x0E0C0A0806040200LL, 0xFFFFFFFFFFFFFFFFLL};
	static const __m128i y_shuff2 = {0xFFFFFFFFFFFFFFFFLL, 0x0E0C0A0806040200LL};
	static const __m128i u_shuff1 = {0xFFFFFFFF0D090501LL, 0xFFFFFFFFFFFFFFFFLL};
	static const __m128i u_shuff2 = {0x0D090501FFFFFFFFLL, 0xFFFFFFFFFFFFFFFFLL};
	static const __m128i u_shuff3 = {0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFF0D090501LL};
	static const __m128i u_shuff4 = {0xFFFFFFFFFFFFFFFFLL, 0x0D090501FFFFFFFFLL};
	static const __m128i v_shuff1 = {0xFFFFFFFF0F0B0703LL, 0xFFFFFFFFFFFFFFFFLL};
	static const __m128i v_shuff2 = {0x0F0B0703FFFFFFFFLL, 0xFFFFFFFFFFFFFFFFLL};
	static const __m128i v_shuff3 = {0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFF0F0B0703LL};
	static const __m128i v_shuff4 = {0xFFFFFFFFFFFFFFFFLL, 0x0F0B0703FFFFFFFFLL};

	while(nb_pixels > 0) {
		*y_dst = _mm_shuffle_epi8 (*src, y_shuff1);
		temp = _mm_shuffle_epi8 (*(src + 1), y_shuff2);
		*y_dst = _mm_or_si128(*y_dst, temp);

		y_dst++;

		*y_dst = _mm_shuffle_epi8 (*(src + 2), y_shuff1);
		temp = _mm_shuffle_epi8 (*(src + 3), y_shuff2);
		*y_dst = _mm_or_si128(*y_dst, temp);

		y_dst++;

		//print_m128("yuyv in: ", src);
		*u_dst = _mm_shuffle_epi8 (*src, u_shuff1);
		//print_m128("u_dst: ", u_dst);
		//print_m128("yuyv in: ", src+1);
		temp = _mm_shuffle_epi8 (*(src + 1), u_shuff2);
		//print_m128("temp: ", &temp);
		*u_dst = _mm_or_si128(*u_dst, temp);
		//print_m128("u_dst: ", u_dst);

		//print_m128("yuyv in: ", (src+2));
		temp = _mm_shuffle_epi8 (*(src + 2), u_shuff3);
		//print_m128("u_dst: ", temp);
		*u_dst = _mm_or_si128(*u_dst, temp);
		//print_m128("yuyv in: ", (src+3));
		temp = _mm_shuffle_epi8 (*(src + 3), u_shuff4);
		//print_m128("temp: ", &temp);
		*u_dst = _mm_or_si128(*u_dst, temp);
		//print_m128("u_dst: ", u_dst);

		u_dst++;

		*v_dst = _mm_shuffle_epi8 (*src, v_shuff1);
		temp = _mm_shuffle_epi8 (*(src + 1), v_shuff2);
		*v_dst = _mm_or_si128(*v_dst, temp);

		temp = _mm_shuffle_epi8 (*(src + 2), v_shuff3);
		*v_dst = _mm_or_si128(*v_dst, temp);
		temp = _mm_shuffle_epi8 (*(src + 3), v_shuff4);
		*v_dst = _mm_or_si128(*v_dst, temp);

		v_dst++;

		// 32 pixels are processed in each iterations
		src += 4;
		nb_pixels -= 32;
	}
}
#endif

extern inline void	convert_uyvy_to_yuv422p(uint8_t* uyvy_src, uint8_t* yuv422p_dst, uint32_t width, uint32_t height) {
	uint32_t 	line, col;
	uint8_t		*y, *u, *v;

	y = yuv422p_dst;
	u = y + width * height;
	v = u + width * height / 2;

	for(line = 0; line < height; line++) {
		for (col = 0; col < width; col += 2) {
			*u++ = *uyvy_src++;
			*y++ = *uyvy_src++;
			*v++ = *uyvy_src++;
			*y++ = *uyvy_src++;
		}
	}
}

extern inline void	convert_yvyu_to_yuv422p(uint8_t* yvyu_src, uint8_t* yuv422p_dst, uint32_t width, uint32_t height) {
	uint32_t 	line, col;
	uint8_t		*y, *u, *v;

	y = yuv422p_dst;
	u = y + width * height;
	v = u + width * height / 2;

	for(line = 0; line < height; line++) {
		for (col = 0; col < width; col += 2) {
			*y++ = *yvyu_src++;
			*v++ = *yvyu_src++;
			*y++ = *yvyu_src++;
			*u++ = *yvyu_src++;
		}
	}
}

#endif /* PIXFMT_CONV_H_ */
