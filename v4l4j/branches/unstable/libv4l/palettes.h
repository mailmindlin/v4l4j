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

#include "libv4l.h"

struct libv4l_palette{
	int libv4l_palette;
	int v4l1_palette;
	int v4l2_palette;
	int depth;
	char name[10];
};

static struct libv4l_palette libv4l_palettes[] = {
	{   
		YUV420,
		VIDEO_PALETTE_YUV420,
		V4L2_PIX_FMT_YUV420,
		12,		
		"YUV420"
	},
	{   
		YUYV,
		VIDEO_PALETTE_YUYV,
		V4L2_PIX_FMT_YUYV,
		16,
		"YUYV"
	},
		{   
		RGB24,
		VIDEO_PALETTE_RGB24,
		V4L2_PIX_FMT_RGB24,
		24,
		"RGB24"
	},
		{   
		RGB32,
		VIDEO_PALETTE_RGB32,
		V4L2_PIX_FMT_RGB32,
		32,
		"RGB32"
	},
		{   
		RGB555,
		VIDEO_PALETTE_RGB555,
		V4L2_PIX_FMT_RGB555,
		16,
		"RGB555"
	},
		{   
		RGB565,
		VIDEO_PALETTE_RGB565,
		V4L2_PIX_FMT_RGB565,
		16,
		"RGB565"
	},
		{   
		GREY,
		VIDEO_PALETTE_GREY,
		V4L2_PIX_FMT_GREY,
		8,
		"GREY"
	},
		{   
		MJPEG,
#define VIDEO_PALETTE_UNDEFINED_V4L1	-1
		VIDEO_PALETTE_UNDEFINED_V4L1,
		V4L2_PIX_FMT_MJPEG,
#define COMPRESSED_FORMAT_DEPTH			-1
		COMPRESSED_FORMAT_DEPTH,
		"MJPEG"
	},
		{   
		JPEG,
#define VIDEO_PALETTE_UNDEFINED_V4L1	-1
		VIDEO_PALETTE_UNDEFINED_V4L1,
		V4L2_PIX_FMT_JPEG,
#define COMPRESSED_FORMAT_DEPTH			-1
		COMPRESSED_FORMAT_DEPTH,
		"JPEG"
	},
//Dont use the following two, use YUV420 and YUYV instead !!
	{   
		YUV420P,
		VIDEO_PALETTE_YUV420P,
		0,
		12,
		"YUV420"
	},
		{   
		YUV422,
		VIDEO_PALETTE_YUV422,
		0,
		8,
		"YUYV"
	}
};

#endif //H_PALETTES
