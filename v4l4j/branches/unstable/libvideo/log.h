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

#include "libvideo.h"

#define info(format, ...) do { fprintf (stderr, "[ %s:%d ] " format, __FILE__, __LINE__, ## __VA_ARGS__);\
								 fflush(stderr); } while(0)

#define CLEAR_ARR(x,s) memset((x), 0x0, (s));
#define ARRAY_SIZE(x) ((int)sizeof(x)/(int)sizeof((x)[0]))


//debug stuff
#ifdef DEBUG

#define LIBV4L_LOG_LEVEL_INFO 			1
#define LIBV4L_LOG_LEVEL_DEBUG 			2	//once / twice per call
#define LIBV4L_LOG_LEVEL_DEBUG1 		4	//for / while loops
#define LIBV4L_LOG_LEVEL_DEBUG2 		8	//many times per seconds
#define LIBV4L_LOG_LEVEL_ERR 			16  //error / quirks
#define LIBV4L_LOG_LEVEL_ALL			(LIBV4L_LOG_LEVEL_INFO | LIBV4L_LOG_LEVEL_DEBUG\
										 | LIBV4L_LOG_LEVEL_DEBUG1 | LIBV4L_LOG_LEVEL_DEBUG2 | LIBV4L_LOG_LEVEL_ERR)

#define LIBV4L_LOG_SOURCE_VIDEO_DEVICE	1
#define LIBV4L_LOG_SOURCE_QUERY			2
#define LIBV4L_LOG_SOURCE_CAPTURE		4
#define LIBV4L_LOG_SOURCE_CONTROL		8
#define LIBV4L_LOG_SOURCE_MEMALLOC		16
#define LIBV4L_LOG_SOURCE_CTRL_PROBE	32
#define LIBV4L_LOG_SOURCE_TUNER			64
#define LIBV4L_LOG_SOURCE_ALL 			(LIBV4L_LOG_SOURCE_VIDEO_DEVICE | LIBV4L_LOG_SOURCE_QUERY | LIBV4L_LOG_SOURCE_CAPTURE |\
							 			 LIBV4L_LOG_SOURCE_CONTROL | LIBV4L_LOG_SOURCE_MEMALLOC | LIBV4L_LOG_SOURCE_CTRL_PROBE | \
										LIBV4L_LOG_SOURCE_TUNER)




//default log levels and sources
#ifndef LIBV4L_LOG_LEVEL
//#define LIBV4L2_LOG_LEVEL 		(LIBV4L2_LOG_LEVEL_ERR | LIBV4L2_LOG_LEVEL_INFO)
#define LIBV4L_LOG_LEVEL 		LIBV4L_LOG_LEVEL_ALL
#endif
#ifndef LIBV4L_LOG_SOURCE
#define LIBV4L_LOG_SOURCE 		LIBV4L_LOG_SOURCE_ALL
//#define LIBV4L_LOG_SOURCE 		LIBV4L_LOG_SOURCE_CTRL_PROBE
#endif

#define dprint(source, level,format, ...) do {if(((source) & LIBV4L_LOG_SOURCE) && ((level) & LIBV4L_LOG_LEVEL)) {\
				 fprintf (stderr, "[%s:%d %s] " format, __FILE__, __LINE__, __PRETTY_FUNCTION__, ## __VA_ARGS__); fflush(stderr);} } while(0)


#else  //if not DEBUG
#define dprint(source, level, format, ...)
#endif // if DEBUG

#define dprint_v4l2_control(qc) do { \
								dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG1, \
										"CTRL: control id: 0x%x - name: %s - min: %d - max: %d - step: %d - type: %d(%s) - flags: %d (%s%s%s%s%s%s)\n", \
										qc->id, (char *) &qc->name, qc->minimum, qc->maximum, qc->step, qc->type, \
										qc->type == V4L2_CTRL_TYPE_INTEGER ? "Integer" :  \
										qc->type == V4L2_CTRL_TYPE_BOOLEAN ? "Boolean" :  \
										qc->type == V4L2_CTRL_TYPE_MENU ? "Menu" :  \
										qc->type == V4L2_CTRL_TYPE_BUTTON ? "Button" : \
										qc->type == V4L2_CTRL_TYPE_INTEGER64 ? "Integer64" :  \
										qc->type == V4L2_CTRL_TYPE_CTRL_CLASS ? "Class" : "", \
										qc->flags, \
										qc->flags & V4L2_CTRL_FLAG_DISABLED ? "Disabled " : "", \
										qc->flags & V4L2_CTRL_FLAG_GRABBED ? "Grabbed " : "", \
										qc->flags & V4L2_CTRL_FLAG_READ_ONLY ? "ReadOnly " : "", \
										qc->flags & V4L2_CTRL_FLAG_UPDATE ? "Update " : "", \
										qc->flags & V4L2_CTRL_FLAG_INACTIVE ? "Inactive " : "", \
										qc->flags & V4L2_CTRL_FLAG_SLIDER ? "slider " : ""); \
							} while(0);

#define XMALLOC(var, type, size)	\
		do { \
			(var) = (type) malloc((size)); \
			if (!(var)) {fprintf(stderr, "[%s:%d %s] MEMALLOC: OUT OF MEMORY. Cant allocate %lu bytes.\n",\
					__FILE__, __LINE__, __PRETTY_FUNCTION__, (long unsigned int) (size)); fflush(stderr);}\
			else { CLEAR_ARR((var), (size)); \
				dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "MEMALLOC: allocating %lu bytes of type %s for var %s (%p).\n", (long unsigned int) (size), #type, #var, (var)); } \
		} while (0)

#define XREALLOC(var, type, size)	\
		do { \
			int should_clear = var == NULL ? 1 : 0;\
			var = (type) realloc(var, (size)); \
			if (!var) {fprintf(stderr, "[%s:%d %s] REALLOC: OUT OF MEMORY. Cant reallocate %lu bytes.\n",\
					__FILE__, __LINE__, __PRETTY_FUNCTION__,(long unsigned int)  (size)); fflush(stderr);}\
			else { \
					if (should_clear) {\
						CLEAR_ARR((var), (size)); \
						dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "REALLOC: Allocating %lu bytes of type %s for var %s (%p).\n", (long unsigned int) (size), #type, #var, (var)); \
					} else \
						{dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "REALLOC: re-allocating %lu bytes of type %s for var %s (%p).\n", (long unsigned int) (size), #type, #var, (var));} \
			}\
		} while (0)

#define XFREE(var)					\
		do { dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "MEMALLOC: freeing memory for var %s (%p).\n", #var, (var)); \
			if ((var)) { free((var)); } \
			else { dprint(LIBV4L_LOG_SOURCE_MEMALLOC, LIBV4L_LOG_LEVEL_ALL, "MEMALLOC: Trying to free a NULL pointer.\n");} \
			var = NULL;\
		} while (0)

#endif
