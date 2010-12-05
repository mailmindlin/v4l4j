/*
 * common.h
 *
 * Copyright (C) 2010 Gilles Gigan (gilles.gigan@gmail.com)
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

#ifndef COMMON_H_

#include <time.h>
#include "pixconv.h"

struct converter {
	int (*check_conversion)(struct pixconv*);
};

#define CLIP_PIXEL(x) ((x>255) ? 255 : (x<0) ? 0 : (x))

#define	support_sse2(result)	\
			do{	\
				unsigned long 	eax = 1;\
				unsigned long 	edx;\
				__asm__ __volatile__ (\
						"pushl %%ebx;\n"\
						"cpuid;\n"\
						"popl %%ebx;"\
						: "=d" (edx)\
						: "a" (eax)\
						: "%ecx"\
				);\
				(result) = (edx & 0x4000000);\
			} while(0)


typedef unsigned long long	ticks;
#define		getticks(t)	\
		do{\
				struct timespec ts;\
				clock_gettime(CLOCK_MONOTONIC, &ts);\
				t = (ticks) (ts.tv_sec * 1000000000 + ts.tv_nsec);\
		} while(0)
	//	struct timeval tv;
	//	gettimeofday(&tv, NULL);
	//	return (ticks) (tv.tv_sec * 1000000 + tv.tv_usec);
	//
	//	unsigned long a, d;
	//	__asm__ __volatile__ (
	//			"xor %%eax, %%eax;\n"
	//			"cpuid;\n"
	//			"rdtsc;\n"
	//			: "=a" (a), "=d" (d)
	//			  :	// no input
	//			  : "%ebx", "%ecx");
	//
	//	return (ticks) (((ticks)a) | ( ((ticks)d) << 32ULL ) );


#define COMMON_H_


#endif /* COMMON_H_ */
