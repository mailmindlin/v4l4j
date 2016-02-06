/*
 * RGB <-> YUV conversion routines, optimized for ARM NEON
 * (C) 2016 mailmindlin
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 */
#include <types.h>

extern void v4lconvert_neon_yuyv_to_rgb24(u8* src, u8* dest, u32 length);