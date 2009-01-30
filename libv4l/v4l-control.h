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

#ifndef H_V4LCONTROL
#define H_V4LCONTROL

#include "libv4l.h"

struct control_list *list_control(struct capture_device *);
//returns 0, LIBV4L_ERR_WRONG_VERSION, LIBV4L_ERR_IOCTL
int get_control_value(struct capture_device *, struct v4l2_queryctrl *, int *);
//returns 0, LIBV4L_ERR_WRONG_VERSION, LIBV4L_ERR_IOCTL or LIBV4L_ERR_STREAMING
int set_control_value(struct capture_device *, struct v4l2_queryctrl *,  int);
void free_control_list(struct capture_device *);
void query_control(struct capture_device *);

#endif
