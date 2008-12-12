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

#ifndef H_ALLOC_DEBUG
#define H_ALLOC_DEBUG

#define CLEAR(x) memset(&x, 0x0, sizeof(x));

#define LOG_CALLS			1
#define LOG_LIBV4L			2
#define LOG_MEMALLOC		4
#define LOG_JPEG			8
#define LOG_V4L4J			16

#define LOG_ALL 			( LOG_CALLS | LOG_LIBV4L | LOG_MEMALLOC | LOG_JPEG | LOG_V4L4J)

#ifndef LOG_SOURCE
#define LOG_SOURCE			LOG_ALL
#endif 

#include <string.h>
#include <stdlib.h>


#ifdef DEBUG 
#define dprint(source, format, ...) do {if(((source) & (LOG_SOURCE))) {\
		fprintf (stderr, "[%s:%d] " format, __FILE__, __LINE__, ## __VA_ARGS__); fflush(stderr);} } while(0)
//		fprintf (stderr, "[%s:%d %s] " format, __FILE__, __LINE__, __PRETTY_FUNCTION__, ## __VA_ARGS__); fflush(stderr);} } while(0)
		 
#define XMALLOC(var, type, size)	\
		do { \
			var = (type) malloc(size); \
			if (!var) {dprint(LOG_MEMALLOC, "[MEMALLOC]: Cant allocate %d bytes.\n", size);} \
			else { CLEAR(*var); \
				dprint(LOG_MEMALLOC, "[MEMALLOC]: allocated %d bytes of type %s for var %s (%p).\n", size, #type, #var, var);}\
		} while (0)
		
#define XFREE(var)					\
		do { dprint(LOG_MEMALLOC, "[MEMALLOC]: freeing memory for var %s (%p).\n", #var, var);\
			if (var) { free(var); } \
			else { dprint(LOG_MEMALLOC, "[MEMALLOC]: Trying to free a NULL pointer.\n");}\
		} while (0)

#else
#define dprint(source, format, ...)

#define XMALLOC(var, type, size)	\
		do { \
			var = (type) malloc(size); \
			if (!var) {fprintf(stderr,"[%s:%d %s] MEMALLOC: OUT OF MEMORY !!! Cant allocate %d bytes.\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, size); fflush(stderr);} \
			else { CLEAR(*var);}\
		} while (0)

#define XFREE(var)					\
		do { if (var) { free(var); } } while (0)

#endif //#ifdef DEBUG

#endif
