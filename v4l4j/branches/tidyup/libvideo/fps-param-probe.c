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
#include <float.h>
#include <math.h>
#include "videodev2.h"

#include "videodev_additions.h"
#include "fps-param-probe.h"
#include "log.h"
#include "libvideo-err.h"

#define NB_PRIV_IOCTL 1

//the frame interval depends on an image format
//if the control interface is created before the capture one
//then controls created by this probe may be invalid because
//capture may use a different image format.

/**
 * Credit to Laurent Pinchart &&  Michel Xhaard for the float_to_fraction
 * & float_to_fraction_recursive code taken from luvcview
 */
static int float_to_fraction_recursive(double f, double p, int *num, int *den)
{
        int whole = (int)f;
        f = fabs(f - whole);

        if(f > p) {
                int n, d;
                int a = float_to_fraction_recursive(1 / f, p + p / f, &n, &d);
                *num = d;
                *den = d * a + n;
        }
        else {
                *num = 0;
                *den = 1;
        }
        return whole;
}

static void float_to_fraction(float f, int *num, int *den)
{
        int whole = float_to_fraction_recursive(f, FLT_EPSILON, num, den);
        *num += whole * *den;
}


int fps_param_probe(struct video_device *vdev, void **data){
	struct v4l2_streamparm *param;

	XMALLOC(param, struct v4l2_streamparm *, sizeof(struct v4l2_streamparm ));

	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "FPS-PARAM: probing FPS adjust caps ...\n");

	if(vdev->v4l_version!=V4L2_VERSION)
		goto end;

	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "FPS-PARAM: .. \n");

	CLEAR(*param);
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 != ioctl(vdev->fd, VIDIOC_G_PARM, param))
		goto end;

	if(!(param->parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
		goto end;

	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "FPS-PARAM: .. .. \n");

	if (0 != ioctl(vdev->fd, VIDIOC_S_PARM, param))
		goto end;

	*data = (void *) param;

	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "FPS-PARAM: FPS adjustable  (%d control)!\n", NB_PRIV_IOCTL);
	return NB_PRIV_IOCTL;

end:
	XFREE(param);
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "FPS-PARAM: NOT found\n");
	return -1;
}

int fps_param_get_ctrl(struct video_device *vdev, struct v4l2_queryctrl *q, void *d, int *val){
	struct v4l2_streamparm *param = (struct v4l2_streamparm *) d;
	int ret = LIBVIDEO_ERR_IOCTL;

	CLEAR(*param);
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(vdev->fd, VIDIOC_G_PARM, param)) {
		*val = (int) param->parm.capture.timeperframe.numerator / param->parm.capture.timeperframe.denominator;
		ret = 0;
	} else
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "FPS-PARAM: Error getting current FPS\n");

	return ret;
}

int fps_param_set_ctrl(struct video_device *vdev, struct v4l2_queryctrl *q, int *val, void *d){
	struct v4l2_streamparm *param = (struct v4l2_streamparm *) d;
	int n = 0, dd = 0;

	CLEAR(*param);
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	float_to_fraction((float) *val, &n, &dd);

	param->parm.capture.timeperframe.numerator = dd;
	param->parm.capture.timeperframe.denominator = n;


	if (-1 == ioctl(vdev->fd, VIDIOC_S_PARM, param)) {
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "FPS-PARAM: Error setting new FPS %d\n", *val);
		return LIBVIDEO_ERR_STREAMING;
	}

	return 0;
}

int fps_param_list_ctrl(struct video_device *vdev, struct control *c, void *d){
	struct v4l2_streamparm *param = (struct v4l2_streamparm *) d;
	if(param != NULL) {
		CLEAR(*param);
		param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (0 != ioctl(vdev->fd, VIDIOC_G_PARM, param))
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "FPS-PARAM: Error getting current FPS\n");

		//Set FPS
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "FPS-PARAM: Found FPS adjust ioctl\n");
		c[0].v4l2_ctrl->id=0;
		c[0].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) c[0].v4l2_ctrl->name,"Frame rate");
		c[0].v4l2_ctrl->minimum = 0;
		c[0].v4l2_ctrl->maximum = 255;
		c[0].v4l2_ctrl->step = 1;
		c[0].v4l2_ctrl->default_value = param->parm.capture.timeperframe.denominator;
		c[0].v4l2_ctrl->reserved[0]=V4L2_PRIV_IOCTL;
		c[0].v4l2_ctrl->reserved[1]=FPS_PARAM_PROBE_INDEX;
	}
	return NB_PRIV_IOCTL;
}

