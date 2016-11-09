/*
 * RGB <-> YUV conversion routines, optimized for ARM NEON
 * (C) 2016 mailmindlin
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 */
#ifndef __RGB_YUV_NEON_H
#define __RGB_YUV_NEON_H

#include <stdint.h>

/**
 * Quickly converts YUYV to RGB24
 * @param src Pointer to source buffer
 * @param dst Pointer to output buffer
 * @param len Number of (macro)pixels to convert. Optimal if multiple of 16.
 */
extern void v4lconvert_neon_yuyv_to_rgb24(const u8* src, u8* dest, u32 len);

#endif
