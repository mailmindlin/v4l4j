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

#ifndef H_LIBV4L2_LOG
#define H_LIBV4L2_LOG

#include <stdio.h>

#define LIBV4L2_LOG_LEVEL_INFO 			1
#define LIBV4L2_LOG_LEVEL_WARN 			2
#define LIBV4L2_LOG_LEVEL_ERR 			4
#define LIBV4L2_LOG_LEVEL_DEBUG 		8
#define LIBV4L2_LOG_LEVEL_DEBUG1 		16
#define LIBV4L2_LOG_LEVEL_DEBUG2 		32
#define LIBV4L2_LOG_LEVEL_ALL			(LIBV4L2_LOG_LEVEL_INFO | LIBV4L2_LOG_LEVEL_WARN | LIBV4L2_LOG_LEVEL_ERR |\
										LIBV4L2_LOG_LEVEL_DEBUG | LIBV4L2_LOG_LEVEL_DEBUG1 | LIBV4L2_LOG_LEVEL_DEBUG2)

#define LIBV4L2_LOG_SOURCE_V4L2			1
#define LIBV4L2_LOG_SOURCE_FRAME		2
#define LIBV4L2_LOG_SOURCE_MEMALLOC		4
#define LIBV4L2_LOG_SOURCE_CONTROL		8
#define LIBV4L2_LOG_SOURCE_CTRL_PROBE	16

#define LIBV4L2_LOG_SOURCE_ALL 			(LIBV4L2_LOG_SOURCE_V4L2 | LIBV4L2_LOG_SOURCE_FRAME | LIBV4L2_LOG_SOURCE_MEMALLOC |\
										LIBV4L2_LOG_SOURCE_CONTROL | LIBV4L2_LOG_SOURCE_CTRL_PROBE)

//debug stuff
#ifdef DEBUG

//default log levels and sources
#ifndef LIBV4L2_LOG_LEVEL
//#define LIBV4L2_LOG_LEVEL 		(LIBV4L2_LOG_LEVEL_ERR | LIBV4L2_LOG_LEVEL_INFO)
#define LIBV4L2_LOG_LEVEL 		LIBV4L2_LOG_LEVEL_ALL
#endif
#ifndef LIBV4L2_LOG_SOURCE
#define LIBV4L2_LOG_SOURCE 		LIBV4L2_LOG_SOURCE_ALL
#endif
// FIXME: would be good to allow app using libv4l2 to control debug output of v4l2
//by assigning proper values to the following var:
//extern int libv4l2_log_level;
//extern int libv4l2_log_source;
#define dprint(source, level,format, ...) do {if(((source) & LIBV4L2_LOG_SOURCE) && ((level) & LIBV4L2_LOG_LEVEL)) {\
				 fprintf (stderr, "[%s:%d %s] " format, __FILE__, __LINE__, __PRETTY_FUNCTION__, ## __VA_ARGS__); fflush(stderr);} } while(0) 

#else
#define dprint(source, level, format, ...)
#endif

#endif
