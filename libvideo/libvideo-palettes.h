/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
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

#ifndef LIBVODEO_PALETTES_H
#define LIBVODEO_PALETTES_H

#include "videodev2.h"
#include "types.h"

/**
 * Struct containing information about a libvideo palette
 */
typedef struct ImagePalette {
	/**
	 * Palette ID
	 * -1 if undefined
	 */
	int libvideo_palette;
	u32 v4l1_palette;
	u32 v4l2_palette;
	/**
	 * Palette color depth. -1 for compressed formats
	 */
	int depth;
	/**
	 * Human-readable-ish palette name
	 */
	char name[15];
} ImagePalette;

#define VIDEO_PALETTE_UNDEFINED_V4L	 -1u
#define COMPRESSED_FORMAT_DEPTH      -1

extern const ImagePalette libvideo_palettes[];
extern const unsigned int libvideo_palettes_size;

/**
 * 
 */
LIBVIDEO_PUBLIC const ImagePalette* libvideo_mapV4L1Palette(u32 v4l1_palette);
LIBVIDEO_PUBLIC const ImagePalette* libvideo_mapV4L2Palette(u32 v4l2_palette);
LIBVIDEO_PUBLIC const ImagePalette* libvideo_getPaletteById(unsigned int id);


#endif //LIBVODEO_PALETTES_H