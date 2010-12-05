/*
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <emmintrin.h>
#include "libvideo.h"
#include "common.h"


#define WIDTH		640
#define HEIGHT		480
#define YUYV_FILE	"/home/gilles/Downloads/raw_frame-2-yuyv.raw"
#define PPM_FILE	"/home/gilles/Downloads/raw_frame-2-yuyv.raw.ppm"


int get_buffer_from_file(unsigned char **out)
{
	unsigned char*	buffer = NULL;
	struct stat		file_stat;
	int 			fd;
	int				bytes_read = 0;

	// open file
	fd = open(YUYV_FILE, O_RDONLY);
	if (fd == -1)
		return 0;

	// get file size
	if (fstat(fd, &file_stat) != 0)
		goto bail;

	// ensure file size is multiple of 16
	if ((file_stat.st_size & 0x0F) != 0)
	{
		printf("file size not multiple of 16 (%d %% 16 = %ld)\n", (int)file_stat.st_size, (file_stat.st_size & 0x0F));
		goto bail;
	}

	// allocate buffer
	if (posix_memalign((void **) &buffer, 16, file_stat.st_size * sizeof(char)) != 0)
		goto bail;

	// place data in buffer
	while(bytes_read < file_stat.st_size)
		bytes_read += read(fd, (buffer + bytes_read), (file_stat.st_size - bytes_read));

	*out = buffer;

	bail:
	close(fd);
	return bytes_read;
}

void write_to_ppm(unsigned char *rgb_buffer, int rgb_buf_size)
{
	char header[1024] = {0};
	int fd, i, j;

	// open file
	fd = open(PPM_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	if (fd == -1)
		return;

	// write header
	sprintf(header, "P3\n%d %d\n255\n", WIDTH, HEIGHT);
	write(fd, header, strlen(header));

	// write pixels
	for(i=0; i<HEIGHT; i++)
	{
		for(j=0; j<WIDTH; j++)
		{
			char pixel[64] = {0};
			int count =	sprintf(pixel, "%hhu %hhu %hhu ", rgb_buffer[0], rgb_buffer[1], rgb_buffer[2]);
			//printf("got %d bytes\n", count);
			write(fd, pixel, count);
			rgb_buffer += 3;
		}
		write(fd, "\n", 1);
	}

	close(fd);
}

int main(int argc, char **argv) {

	__m128i*		yuyv_buffer;
	__m128i*		rgba_buffer;
	int				file_size;
	int 			pixel_count;
	int				runs = 1;
	int 			w = WIDTH, h = HEIGHT;
	ticks			total_sse = 0, total_non_sse = 0;

	__v16qi yuyv_8pix_in[] =
	{ {76, 84, 75, 255,	// 2 red pix  	255 	0		0
			149, 43, 148, 21,	// 2 green pix	0		255		0
			29, 255, 28, 107,	// 2 blue pix 	0  		0 		255
			29, 255, 29, 107,},
			{76, 84, 75, 255,	// 2 red pix  	255 	0		0
					149, 43, 148, 21,	// 2 green pix	0		255		0
					29, 255, 28, 107,	// 2 blue pix 	0  		0 		255
					255, 128, 254, 0,}
	};

//	yuyv_buffer = (__m128i*) yuyv_8pix_in;
//	file_size = 32;
//	w = 8;
//	h = 2;


	// Read raw yuyv file
	file_size = get_buffer_from_file((unsigned char **)&yuyv_buffer);
	if (file_size == 0)
		return 1;

	pixel_count = file_size / 2;

	printf("file size: %d - nb pixels:%d\n", file_size, pixel_count);

	// allocate destination buffer
	if (posix_memalign((void **) &rgba_buffer, 16, pixel_count * 4 * sizeof(char)) != 0)
	{
		printf("Error allocating output buffer\n");
		return 1;
	}

	// create struct pixconv
	struct pixconv* pixconv;
	if (create_pixconv(&pixconv, YUYV, RGB24, w, h) != 0)
	{
		printf("error creating pixconv\n");
		free(yuyv_buffer);
		free(rgba_buffer);
		return 1;
	}

	int i;
	for (i = 0; i< runs; i++)
	{
		ticks start, end;

		getticks(start);
		pixconv->convert(pixconv, (unsigned char *)yuyv_buffer, (unsigned char *)rgba_buffer);
		getticks(end);

		total_sse += (end - start);
	}

	printf("Avg SSE: %lld\nAvg non SSE: %lld\n", (total_sse / runs), (total_non_sse / runs));

	destroy_pixconv(pixconv);

	free(rgba_buffer);
	free(yuyv_buffer);

	return 0;
}


