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

#ifndef H_PALETTES
#define H_PALETTES

#include "videodev2.h"

#ifndef V4L2_PIX_FMT_SGBRG8
#define V4L2_PIX_FMT_SGBRG8 v4l2_fourcc('G','B','R','G')
#endif

#ifndef V4L2_PIX_FMT_SGRBG8
#define V4L2_PIX_FMT_SGRBG8 v4l2_fourcc('G','R','B','G')
#endif

#ifndef V4L2_PIX_FMT_SRGGB8
#define V4L2_PIX_FMT_SRGGB8 v4l2_fourcc('R','G','G','B')
#endif

#ifndef V4L2_PIX_FMT_SN9C20X_I420
#define V4L2_PIX_FMT_SN9C20X_I420  v4l2_fourcc('S', '9', '2', '0')
#endif

#ifndef V4L2_PIX_FMT_OV511
#define V4L2_PIX_FMT_OV511 v4l2_fourcc('O', '5', '1', '1')
#endif

#ifndef V4L2_PIX_FMT_OV518
#define V4L2_PIX_FMT_OV518 v4l2_fourcc('O', '5', '1', '8') /* ov518 JPEG */
#endif

struct libvideo_palette{
	int libvideo_palette;
	int v4l1_palette;
	int v4l2_palette;
	int depth;
	char name[15];
};

#define VIDEO_PALETTE_UNDEFINED_V4L1	-1
#define COMPRESSED_FORMAT_DEPTH			-1

extern const struct libvideo_palette libvideo_palettes[];
extern const int libvideo_palettes_size;


#endif //H_PALETTES
