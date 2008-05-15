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

#ifndef H_LIBV4L_LOG
#define H_LIBV4L_LOG

#include <stdio.h>		//for fprintf
#include <stdlib.h>		//for malloc
#include <string.h>		//for memset

#define info(format, ...) do { fprintf (stderr, "[ %s:%d ] " format, __FILE__, __LINE__, ## __VA_ARGS__);\
								 fflush(stderr); } while(0)


//debug stuff
#ifdef DEBUG

#define LIBV4L_LOG_LEVEL_INFO 			1
#define LIBV4L_LOG_LEVEL_DEBUG 			2
#define LIBV4L_LOG_LEVEL_DEBUG1 		4
#define LIBV4L_LOG_LEVEL_DEBUG2 		8
#define LIBV4L_LOG_LEVEL_ERR 			16
#define LIBV4L_LOG_LEVEL_ALL			(LIBV4L_LOG_LEVEL_INFO | LIBV4L_LOG_LEVEL_DEBUG\
										 | LIBV4L_LOG_LEVEL_DEBUG1 | LIBV4L_LOG_LEVEL_DEBUG2 | LIBV4L_LOG_LEVEL_ERR)

#define LIBV4L_LOG_SOURCE_V4L			1
#define LIBV4L_LOG_SOURCE_V4L1			2
#define LIBV4L_LOG_SOURCE_V4L2			4
#define LIBV4L_LOG_SOURCE_MEMALLOC		8
#define LIBV4L_LOG_SOURCE_CONTROL		16
#define LIBV4L_LOG_SOURCE_CTRL_PROBE	32
#define LIBV4L_LOG_SOURCE_ALL 			(LIBV4L_LOG_SOURCE_V4L | LIBV4L_LOG_SOURCE_V4L1 | LIBV4L_LOG_SOURCE_V4L2 |\
							 			 LIBV4L_LOG_SOURCE_MEMALLOC | LIBV4L_LOG_SOURCE_CONTROL | LIBV4L_LOG_SOURCE_CTRL_PROBE)




//default log levels and sources
#ifndef LIBV4L_LOG_LEVEL
//#define LIBV4L2_LOG_LEVEL 		(LIBV4L2_LOG_LEVEL_ERR | LIBV4L2_LOG_LEVEL_INFO)
#define LIBV4L_LOG_LEVEL 		LIBV4L_LOG_LEVEL_ALL
#endif
#ifndef LIBV4L_LOG_SOURCE
#define LIBV4L_LOG_SOURCE 		LIBV4L_LOG_SOURCE_ALL
#endif

#define dprint(source, level,format, ...) do {if(((source) & LIBV4L_LOG_SOURCE) && ((level) & LIBV4L_LOG_LEVEL)) {\
				 fprintf (stderr, "[%s:%d %s] " format, __FILE__, __LINE__, __PRETTY_FUNCTION__, ## __VA_ARGS__); fflush(stderr);} } while(0) 
#define XMALLOC(var, type, size)	\
		do { \
			var = (type) malloc(size); \
			if (!var) dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ERR, "MEMALLOC: Cant allocate %d bytes.\n", size); \
			else { CLEAR(*var); \
				dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "MEMALLOC: allocating %d bytes of type %s for var %s (%p).\n", size, #type, #var, var); } \
		} while (0)

#define XFREE(var)					\
		do { dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "MEMALLOC: freeing memory for var %s (%p).\n", #var, var); \
			if (var) { free(var); } \
			else { dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "MEMALLOC: Trying to free a NULL pointer.\n");} \
		} while (0)


#else  //if not DEBUG
#define dprint(source, level, format, ...)
#define XMALLOC(var, type, size)	\
		do { \
			var = (type) malloc(size); \
			if (!var) {fprintf(stderr, "[%s:%d %s] MEMALLOC: OUT OF MEMORY. Cant allocate %d bytes.\n",\
					__FILE__, __LINE__, __PRETTY_FUNCTION__, size); fflush(stderr);}\
			else { CLEAR(*var);} \
		} while (0)

#define XFREE(var)	do { if (var) { free(var); } } while (0)

#endif // if DEBUG

#endif
