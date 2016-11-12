/*
 * sq905c.c
 *
 * Here is the decompression function for the SQ905C cameras. The functions
 * used are adapted from the libgphoto2 functions for the same cameras,
 * which was
 * Copyright (c) 2005 and 2007 Theodore Kilgore <kilgota@auburn.edu>
 * This version for libv4lconvert is
 * Copyright (c) 2009 Theodore Kilgore <kilgota@auburn.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335  USA
 */

#include <stdlib.h>
#include <string.h>

#include "libv4lconvert-priv.h"


#define CLIP(x) ((u8) ((x) < 0 ? 0 : ((x) > 0xff) ? 0xff : (x)))


static int sq905c_first_decompress(u8 *output, const u8 *input, unsigned int outputsize) {
	u8 parity = 0;
	u8 nibble_to_keep[2];
	u8 temp1 = 0;
	u8 lookup = 0;
	unsigned int bytes_used = 0;
	unsigned int bytes_done = 0;
	unsigned int bit_counter = 8;
	unsigned int cycles = 0;
	int table[9] = { -1, 0, 2, 6, 0x0e, 0x0e, 0x0e, 0x0e, 0xfb};
	u8 lookup_table[16] = {
		0, 2, 6, 0x0e, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,
		0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb
	};
	u8 translator[16] = {
		8, 7, 9, 6, 10, 11, 12, 13,
		14, 15, 5, 4, 3, 2, 1, 0
	};

	nibble_to_keep[0] = 0;
	nibble_to_keep[1] = 0;

	while (bytes_done < outputsize) {
		while (parity < 2) {
			unsigned int temp2 = 0;
			while (lookup > table[cycles]) {
				if (bit_counter == 8) {
					temp1 = input[bytes_used++];
					bit_counter = 0;
				}
				u8 input_byte = temp1;
				temp2 = (temp2 << 1) & 0xFF;
				input_byte = input_byte >> 7;
				temp2 = temp2 | input_byte;
				temp1 = (u8) ((temp1 << 1u) & 0xFF);
				bit_counter++;
				cycles++;
				if (cycles > 8)
					return -1;
				lookup = temp2 & 0xff;
			}
			for (unsigned int i = 0; i < 17; i++) {
				if (i == 16)
					return -1;
				if (lookup == lookup_table[i]) {
					nibble_to_keep[parity] = translator[i];
					break;
				}
			}
			cycles = 0;
			parity++;
		}
		output[bytes_done] = (u8) ((nibble_to_keep[0] << 4) | nibble_to_keep[1]);
		bytes_done++;
		parity = 0;
	}
	return 0;
}
static int sq905c_second_decompress(u8 *uncomp, u8 *in, u32 width, u32 height) {
	unsigned int input_counter = 0;
	int delta_table[] = {
		-144, -110, -77, -53, -35, -21, -11, -3,
		2, 10, 20, 34, 52, 76, 110, 144
	};

	u8* templine_red = malloc(width);
	if (!templine_red) {
		free(templine_red);
		return -1;
	}
	
	u8* templine_green = malloc(width);
	if (!templine_green) {
		free(templine_green);
		return -1;
	}
	
	u8* templine_blue = malloc(width);
	if (!templine_blue) {
		free(templine_blue);
		return -1;
	}
	
	memset(templine_red, 0x80, width);
	memset(templine_green, 0x80, width);
	memset(templine_blue, 0x80, width);
	
	for (unsigned int m = 0; m < height / 2; m++) {
		/* First we do an even-numbered line */
		for (unsigned int i = 0; i < width / 2; i++) {
			u8 delta_right = in[input_counter] & 0x0f;
			u8 delta_left = (in[input_counter] >> 4) & 0xff;
			input_counter++;
			/* left pixel (red) */
			int diff = delta_table[delta_left];
			int tempval = (signed) templine_red[i];
			if (i)
				tempval = (tempval + (signed) uncomp[2 * m * width + 2 * i - 2]) / 2;
			tempval += diff;
			templine_red[i] = uncomp[2 * m * width + 2 * i] = CLIP(tempval);
			/* right pixel (green) */
			diff = delta_table[delta_right];
			if (!i)
				tempval = (signed) templine_green[1];
			else if (2 * i == width - 2)
				tempval = (signed) (templine_green[i] + uncomp[2 * m * width + 2 * i - 1]) / 2;
			else
				tempval = (signed) (templine_green[i + 1] + uncomp[2 * m * width + 2 * i - 1]) / 2;
			tempval += diff;
			templine_green[i] = uncomp[2 * m * width + 2 * i + 1] = CLIP(tempval);
		}
		/* then an odd-numbered line */
		for (unsigned int i = 0; i < width / 2; i++) {
			u8 delta_right = in[input_counter] & 0x0f;
			u8 delta_left = (in[input_counter] >> 4) & 0xff;
			input_counter++;
			/* left pixel (green) */
			int diff = delta_table[delta_left];
			int tempval = (signed) templine_green[i];
			if (i)
				tempval = (tempval + (signed) uncomp[(2 * m + 1) * width + 2 * i - 2])/2;
			tempval += diff;
			templine_green[i] = uncomp[(2 * m + 1) * width + 2 * i] = CLIP(tempval);
			/* right pixel (blue) */
			diff = delta_table[delta_right];
			tempval = (signed) templine_blue[i];
			if (i)
				tempval = (tempval + (signed) uncomp[(2 * m + 1) * width + 2 * i - 1]) / 2;
			tempval += diff;
			templine_blue[i] = uncomp[(2 * m + 1) * width + 2 * i + 1] = CLIP(tempval);
		}
	}
	free(templine_green);
	free(templine_red);
	free(templine_blue);
	return 0;
}

void v4lconvert_decode_sq905c(const u8 *src, u8 *dst, u32 width, u32 height) {
	/* here we get rid of the 0x50 bytes of header in src. */
	const u8* raw = src + 0x50;
	unsigned int size = width * height / 2;
	u8* temp_data = malloc(size);
	if (temp_data) {
		sq905c_first_decompress(temp_data, raw, size);
		sq905c_second_decompress(dst, temp_data, width, height);
	}
	free(temp_data);
}
