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

#ifndef H_V4L2PROBE
#define H_V4L2PROBE

#include <stdlib.h>	//required for videodev2.h
#include "videodev2.h"
#include "pwc_probe.h"

struct probe_v4l2_driver {
	int (*probe) (int);
	int (*list_ctrl)(int, struct v4l2_queryctrl *);
	int (*get_ctrl)(int, struct v4l2_queryctrl *);
	int (*set_ctrl)(int, struct v4l2_queryctrl *, int);
};

#define PROBE_NB 1

static struct probe_v4l2_driver probe_drivers[] = {
	{
		.probe 		= pwc_probe,
		.list_ctrl 	= pwc_list_ctrl,
		.get_ctrl	= pwc_get_ctrl,
		.set_ctrl	= pwc_set_ctrl,
	}
};

#endif /*H_V4L2PROBE*/
