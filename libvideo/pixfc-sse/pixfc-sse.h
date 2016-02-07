/*
 * pixfc.h
 *
 * Copyright (C) 2011 PixFC Team (pixelfc@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public  License as published by the
 * Free Software Foundation, either version 2 of the License, or
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

#ifndef _PIXFC_SSE_H_
#define _PIXFC_SSE_H_

#ifdef __cplusplus
	extern "C" {
#endif 

#include <stdint.h>

// Forward declaration
struct PixFcSSE;


/*
 * List of known pixel formats
 */
typedef enum {
	//
	// YUV formats

	// Interleaved YUV422    - 8-bit components
	// 4 bytes per 2 pixels  - row bytes is 2 * width
	PixFcYUYV = 0,

	// Interleaved YUV422    - 8-bit components
	// 4 bytes per 2 pixels  - row bytes is 2 * width
	PixFcUYVY,

	// Planar YUV422         - 8-bit components
	// 3 planes (Y, U, V) in a single contiguous memory area
	// 4 bytes per 2 pixels  - row bytes is 2 * width
	PixFcYUV422P,

	// Planar YUV420         - 8-bit components
	// 3 planes (Y, U, V) in a single contiguous memory area
	// 4 bytes per 2 pixels  - row bytes is 2 * width
	PixFcYUV420P,

	// Interleaved YUV422    - 10-bit components
	// 16 bytes per 6 pixels - row bytes is (width + 47) / 48 * 128 bytes long (integer math !!)
	// see https://developer.apple.com/quicktime/icefloe/dispatch019.html#v210
	PixFcV210,


	//
	// RGB formats

	// Interleaved 32-bit ARGB  - 8-bit components
	// 4 bytes per pixel        - row bytes is width * 4
	PixFcARGB,

	// Interleaved 32-bit BGRA  - 8-bit components
	// 4 bytes per pixel        - row bytes is width * 4
	PixFcBGRA,

	// Interleaved 24-bit RGB   - 8-bit components
	// 3 bytes per pixel        - row bytes is width * 3
	PixFcRGB24,

	// Interleaved 24-bit BGR   - 8-bit components
	// 3 bytes per pixel        - row bytes is width * 3
	PixFcBGR24,

  	// Interleaved 10-bit RGB   - 10-bit components
	// 4 bytes per pixel        - row bytes is (width + 63) / 64 * 256 bytes long (integer math !!)
	// see http://www.bitjazz.com/en/products/sheervideo/faq/formats/pixel_formats.php#r210
	PixFcR210,

	// Interleaved 10-bit RGB   - 10-bit components
	// 4 bytes per pixel        - row bytes is (width + 63) / 64 * 256 bytes long (integer math !!)
	// see http://www.bitjazz.com/en/products/sheervideo/faq/formats/pixel_formats.php#r10k
	PixFcR10k,
	
	//
	//
	PixFcFormatCount

} PixFcPixelFormat;



/*
 * Flags that can be passed to create_pixfc() to modify
 * the conversion routine selection process.
 */
typedef enum {
	//
	// Using this flag, PixFC will perform a full-range, average-resampling
	// conversion using the fastest routine supported by the platform.
	PixFcFlag_Default = 0,

	//
	// Force the use of a non-SSE integer conversion routine.
	PixFcFlag_NoSSE	=				(1 << 0),
	// Force the use of a non-SSE float conversion routine. If no float
	// conversion routine is available, an integer one is returned instead,
	// as if PixFcFlag_NoSSE was used.
	PixFcFlag_NoSSEFloat =			(1 << 1),
	// Force the use of a SSE2-only conversion routine
	// (ie. exclude Non-SSE, SSSE3 and SSE41 conversion routines)
	PixFcFlag_SSE2Only =			(1 << 2),
	// Force the use of a SSE2 and SSSE3-only conversion routine
	// (ie. exclude Non-SSE, SSE2-only and SSE41 conversion routines)
	PixFcFlag_SSE2_SSSE3Only =		(1 << 3),
	// Force the use of a SSE2, SSSE3 and SSE41 conversion routine
	// (ie. exclude Non-SSE, SSE2-only and SSE2/SSSE3-only conversion routines)
	// Note this flag is redundant as it is implied by PixFcFlag_Default.
	PixFcFlag_SSE2_SSSE3_SSE41Only=	(1 << 4),

	//
	// By default, PixFC performs a full-range conversion, unless one of the
	// following two flags is used.
	//
	// Force the use of conversion equations suitable for SD image formats,
	// as defined in ITU-R Rec. BT.601
	PixFcFlag_BT601Conversion =		(1 << 7),
	// Force the use of conversion equations suitable for HD image formats,
	// as defined in ITU-R Rec. BT.709
	PixFcFlag_BT709Conversion =		(1 << 8),

	//
	// Force the use of a conversion routine which uses nearest neighbour
	// resampling. This means:
	// - upsampling creates missing chromas by duplicating existing ones, and
	// - downsampling simply drops unused chromas.
	// This is the fastest form of resampling, but converted images can be
	// of lower quality and prone to conversion artifacts (aliasing).
	PixFcFlag_NNbResamplingOnly	=	(1 << 14),
} PixFcFlag;


/*
 * A conversion block function converts pixels from an input buffer in a specific
 * format to a different format and places them in an output buffer. Both in- and
 * out-buffers must be allocated by the called. To achieve higher conversion speeds,
 * both buffers should be 16-byte aligned, but they do not have to.
 *
 * Prototype for conversion block functions
 */
typedef		void (*ConversionBlockFn)(const struct PixFcSSE *pixfc, void *inBuffer, void *outBuffer);


/*
 *  This structure is the entry point for pixel conversion.
 *  - You obtain one by calling by create_pixfc(),
 *  - Convert images by calling the convert() function pointer,
 *  - and release with destroy_pixfc().
 *  All members in this structure are read-only.
 */
struct PixFcSSE{
	// Conversion function - call me to perform the conversion
	ConversionBlockFn 			convert;

	// Values below are read-only & identical to those passed to create_pixfc()
	PixFcPixelFormat 			source_fmt;
	PixFcPixelFormat 			dest_fmt;
	uint32_t					pixel_count;
	uint32_t					width;
	uint32_t					height;
	uint32_t					source_row_bytes;
	uint32_t					dest_row_bytes;

	// PixFC will set this member to reflect the flags actually used by the conversion function
	PixFcFlag					flags;
};


/*
 * This function creates a struct PixFcSSE and sets it up
 * for a conversion from the given source format to the destination
 * one if supported. See macros further down for returned error codes.
 */
uint32_t		create_pixfc(
	struct PixFcSSE**,	// out - returns a struct PixFcSSE
	PixFcPixelFormat,  	// in  - source format
	PixFcPixelFormat, 	// in  - destination format
	uint32_t,			// in  - width
	uint32_t, 			// in  - height
	uint32_t,			// in  - source buffer row bytes: length of one image row (in bytes).
						//		 For most input formats, this is 
						//		 width * number_of_bytes_per_pixel. However some
						//		 formats have alignment requirements. For instance,
						//		 v210 requires the size of each line to be multiple
						//		 of 128 bytes.
	uint32_t,			// in  - destination buffer row bytes.
	PixFcFlag			// in  - tune the selection of the conversion function. See
						//		 enum definition above.
);


/*
 * This function releases the given struct PixFcSSE
 */
void			destroy_pixfc(struct PixFcSSE*);

/*
 * Error codes
 */
enum {
	// The SSE features required are not available on the executing CPU.
	PixFc_NoCPUSupport = -7,

	// Destination image buffer has an invalid row size
	PixFc_InvalidDestBufferRowSize = -6,

	// Source image buffer has an invalid row size
	PixFc_InvalidSourceBufferRowSize = -5,
	
	// Source image dimensions prevent SSE conversions from being used.
	PixFc_UnsupportedSourceImageDimension = -4,
		
	// Out of memory
	PixFc_OOMError = -3, 
	
	// Conversion from source to destination format not supported
	PixFc_UnsupportedConversionError = -2,
		
	// Generic error code
	PixFc_Error = -1,
	
	//
	PixFc_OK = 0,
};

#ifdef __cplusplus
}
#endif

#endif /* _PIXFC_SSE_H_ */
