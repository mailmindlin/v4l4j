/*
	copyright 2006 Gilles GIGAN (gilles.gigan@gmail.com)
			
	This file is part of light_cap.

   	light_cap is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    light_cap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with light_cap; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef H_LOG
#define H_LOG

#include <stdio.h>

#define LOG_NONE				0
#define LOG_INFO				1
#define LOG_ERR					2
#define LOG_ALL					( LOG_ERR | LOG_INFO )
extern int verbosity;
#define info(type, format, ...) do { if(type & verbosity)  {fprintf (stderr, format, ## __VA_ARGS__); } } while(0)

//DEBUG stuff
#ifdef DEBUG

#define NB_ARGS					(DEFAULT_NB_ARGS + 2) 

#define LOG_LEVEL_DEBUG 		1
#define LOG_LEVEL_DEBUG1 		2
#define LOG_LEVEL_DEBUG2 		4
#define LOG_LEVEL_ALL			LOG_LEVEL_DEBUG | LOG_LEVEL_DEBUG1 | LOG_LEVEL_DEBUG2

#define LOG_SOURCE_JPEG			1
#define LOG_SOURCE_HTTP			2
#define LOG_SOURCE_MEMALLOC		4
#define LOG_SOURCE_ALL			LOG_SOURCE_MEMALLOC | LOG_SOURCE_HTTP | LOG_SOURCE_JPEG

#define DEFAULT_LOG_LEVEL 		LOG_LEVEL_ALL
#define DEFAULT_LOG_SOURCE 		LOG_SOURCE_ALL

extern int log_level;
extern int log_source;
#define dprint(source, level,format, ...) do { if(((source) & log_source) && ((level) & log_level)) {\
				 							fprintf (stderr, "[%s:%d %s] " format, __FILE__, __LINE__,\
				 							 __PRETTY_FUNCTION__, ## __VA_ARGS__); fflush(stderr);}\
											} while(0) 


#else //no DEBUG
#define NB_ARGS					DEFAULT_NB_ARGS
#define dprint(source, level, format, ...)
#endif


#endif
