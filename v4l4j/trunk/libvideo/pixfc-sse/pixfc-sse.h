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
 *
 * (see pixfmt_description.h for a struct containing more
 * info about each format, including name and fill pattern)
 */
typedef enum {
	// YUV formats
	PixFcYUYV = 0,
	PixFcUYVY,
	PixFcYUV422P,
	PixFcYUV420P,

	// RGB formats
	PixFcARGB,	// 32-bit ARGB
	PixFcBGRA,	// 32-bit BGRA
	PixFcRGB24,	// 24-bit RGB
	PixFcBGR24,	// 24-bit BGR

	//
	PixFcFormatCount

} PixFcPixelFormat;


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
	ConversionBlockFn 			convert;	// Conversion function - call me to perform the conversion

	PixFcPixelFormat 			source_fmt;
	PixFcPixelFormat 			dest_fmt;
	uint32_t					pixel_count;
	uint32_t					width;
	uint32_t					height;
	uint32_t					uses_sse;	// set to 1 if the conversion function
											// uses SSE, 0 if not.
};

/*
 * Flags that can be passed to create_pixfc() to modify
 * the conversion routine selection process.
 */
typedef enum {
	PixFcFlag_Default = 0,

	//
	// Force the use of a non-SSE conversion routine.
	// (Using this flag implies PixFcFlag_NNbResamplingOnly).
	PixFcFlag_NoSSE	=				(1 << 0),
	// Force the use of a SSE2-only conversion routine
	PixFcFlag_SSE2Only =			(1 << 1),

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
	PixFcFlag_NNbResamplingOnly	=		(1 << 14),
} PixFcFlag;



/*
 * This function creates a struct PixFcSSE and sets it up
 * for a conversion from the given source format to the destination
 * one if supported. See macros further down for returned error codes.
 */
uint32_t		create_pixfc(struct PixFcSSE**,		// out - returns a struct PixFcSSE
								PixFcPixelFormat,  	// in  - source format
								PixFcPixelFormat, 	// in  - destination format
								uint32_t,			// in  - width
								uint32_t, 			// in  - height
								PixFcFlag			// in  - see enum above - tune the selection of the conversion function
);


/*
 * This function releases the given struct PixFcSSE
 */
void			destroy_pixfc(struct PixFcSSE*);

/*
 * Error codes
 */
// All good
#define			PIXFC_OK								0

// Generic error code
#define			PIXFC_ERROR								-1

// Conversion from source to destination format not supported
#define			PIXFC_CONVERSION_NOT_SUPPORTED			-2

// Out of memory
#define 		PIXFC_OOM								-3


#ifdef __cplusplus
}
#endif

#endif /* _PIXFC_SSE_H_ */
