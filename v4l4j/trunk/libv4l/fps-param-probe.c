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

#include <sys/ioctl.h>		//for ioctl
#include <string.h>
#include <stdlib.h>	//required for videodev2.h
#include "videodev2.h"

#include "videodev_additions.h"
#include "fps-param-probe.h"
#include "log.h"

#define NB_PRIV_IOCTL 1


int fps_param_probe(struct capture_device *c, void **data){
	struct v4l2_streamparm *param;

	XMALLOC(param, struct v4l2_streamparm *, sizeof(struct v4l2_streamparm ));
	
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "FPS-PARAM: probing FPS adjust caps ...\n");

	if(c->v4l_version!=V4L2_VERSION) 
		goto end;
	
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "FPS-PARAM: .. \n");
	
	CLEAR(*param);
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 != ioctl(c->fd, VIDIOC_G_PARM, param)) 
		goto end;
	
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "FPS-PARAM: .. .. \n");
	
	if (0 != ioctl(c->fd, VIDIOC_S_PARM, param)) 
		goto end;
	
	*data = (void *) param;
	
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "FPS-PARAM: FPS adujstable !\n");
	return NB_PRIV_IOCTL;

end:
	XFREE(param);
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "FPS-PARAM: NOT found\n");
	return -1;
}

int fps_param_get_ctrl(struct capture_device *c, struct v4l2_queryctrl *q, void *d){
	struct v4l2_streamparm *param = (struct v4l2_streamparm *) d;
	
	CLEAR(*param);
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(c->fd, VIDIOC_G_PARM, param)) 
		return param->parm.capture.timeperframe.denominator;

	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "FPS-PARAM: Error getting current FPS\n");
	return -1;
}

int fps_param_set_ctrl(struct capture_device *c, struct v4l2_queryctrl *q, int val, void *d){
	struct v4l2_streamparm *param = (struct v4l2_streamparm *) d;
	
	CLEAR(*param);
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(c->fd, VIDIOC_G_PARM, param)) {
		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "FPS-PARAM: Error getting current FPS\n");
		return -1;
	}
	
	param->parm.capture.timeperframe.numerator = 1;
	param->parm.capture.timeperframe.denominator = val;
	
	if (0 == ioctl(c->fd, VIDIOC_S_PARM, param)) {
		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "FPS-PARAM: Error setting new FPS %d\n",val);
		return -1;
	}
	return 0;
}

int fps_param_list_ctrl(struct capture_device *c, struct v4l2_queryctrl *q, void *d){
	struct v4l2_streamparm *param = (struct v4l2_streamparm *) d;
	if(param != NULL) {		
		CLEAR(*param);
		param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (0 != ioctl(c->fd, VIDIOC_G_PARM, param))
			dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "FPS-PARAM: Error getting current FPS\n");
	
		//Set FPS
		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "FPS-PARAM: Found FPS adjust ioctl\n");
		q[0].id=0;
		q[0].type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) q[0].name,"Frame rate");
		q[0].minimum = 0;
		q[0].maximum = 255;
		q[0].step = 0;
		q[0].default_value = param->parm.capture.timeperframe.denominator;
		q[0].reserved[0]=V4L2_PRIV_IOCTL;
		q[0].reserved[1]=FPS_PARAM_PROBE_INDEX;
	}
	return NB_PRIV_IOCTL;
}

