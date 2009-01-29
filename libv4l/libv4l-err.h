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

#ifndef H_LIBV4LERR_LOG
#define H_LIBV4LERR_LOG

//Cant set channel (set_cap_param_v4l2)
#define LIBV4L_ERR_CHANNEL					-1

//Cant set standard (set_cap_param_v4l2)
#define LIBV4L_ERR_STD						-2

//Cant set standard (set_cap_param_v4l1)
#define LIBV4L_ERR_CHANNEL_SETUP			-3

//cant get capabilities (set_cap_param_v4l1, query_device_v4l1)
#define LIBV4L_ERR_NOCAPS					-4

//
//AVAILABLE									-5

//wrong height (set_cap_param_v4l1)
#define LIBV4L_ERR_DIMENSIONS				-6

//generic IOCTL error (check_inputs_v4l1)
#define LIBV4L_ERR_IOCTL					-7

//cant set the image format, palette, width, height, ... (set_cap_param_v4l1 & 2)
#define LIBV4L_ERR_FORMAT					-8

//cant set the image cropping(set_cap_param_v4l2)
#define LIBV4L_ERR_CROP						-9

//cant request mmap info(init_cap_param_v4l1 & 2)
#define LIBV4L_ERR_REQ_MMAP					-10

//cant get mmap buffer (init_cap_param_v4l2)
#define LIBV4L_ERR_REQ_MMAP_BUF				-11

//unsupported number of mmap buffers (init_cap_param_v4l1)
#define LIBV4L_ERR_INVALID_BUF_NB			-12

//Error mmap'ing buffers (init_cap_param_v4l1 & 2)
#define LIBV4L_ERR_MMAP_BUF					-13

//Wrong palette stored in struct capture_device, most likely changed by user (start_cap_param_v4l1)
#define LIBV4L_ERR_PALETTE					-14

#endif

