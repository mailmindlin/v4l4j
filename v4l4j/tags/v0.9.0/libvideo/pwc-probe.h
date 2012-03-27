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

#ifndef H_PWC_PROBE
#define H_PWC_PROBE

#include "libvideo.h"

//index of the pwc probe in the struct probe_v4l_driver probe_drivers[] in v4l-probe.h
#define PWC_PROBE_INDEX	0
int pwc_driver_probe(struct video_device *, void **);
int pwc_list_ctrl(struct video_device *, struct control *, void *);
int pwc_get_ctrl(struct video_device *,  struct v4l2_queryctrl *, void *, int *);
int pwc_set_ctrl(struct video_device *, struct v4l2_queryctrl *, int *, void *);

#endif /*H_PWC_PROBE*/
