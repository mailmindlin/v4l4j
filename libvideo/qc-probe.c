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
#include <stdlib.h>			//required for videodev2.h
#include <stdbool.h>		//For bool type
#include "videodev2.h"

#include "videodev_additions.h"
#include "qc-probe.h"
/*
 * need quickam.h (tested with qc-0.6.6)
 */
#include "quickcam.h"
#include "log.h"
#include "libvideo-err.h"

#define NB_PRIV_IOCTL 5

struct qc_probe_private {
	bool ok;
};

//QC ioctl requests
static const unsigned long qc_getctrl_requests[] = {
		[0] = VIDIOCQCGSETTLE,
		[1] = VIDIOCQCGCOMPRESS,
		[2] = VIDIOCQCGQUALITY,
		[3] = VIDIOCQCGADAPTIVE,
		[4] = VIDIOCQCGEQUALIZE
	};
static const unsigned long qc_setctrl_requests[] = {
		[0] = VIDIOCQCSSETTLE,
		[1] = VIDIOCQCSCOMPRESS,
		[2] = VIDIOCQCSQUALITY,
		[3] = VIDIOCQCSADAPTIVE,
		[4] = VIDIOCQCSEQUALIZE
	};
static const char* const qc_ctrl_names[] = {
		[0] = "Brightness Settle",
		[1] = "Compression",
		[2] = "Interpolation",
		[3] = "Auto Brightness",
		[4] = "Equalize image"
	};
#define NUM_QC_CTRLS 5


int qc_driver_probe(struct video_device *vdev, void **data) {
	struct qc_probe_private *priv;
	struct qc_userlut default_ulut, our_ulut, check_ulut;
	
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "QC: probing Quickam\n");
	/*
	 * Probing qc ....
	 * qc has many private ioctls: among them, VIDIOCQCGCOMPATIBLE and VIDIOCQCSCOMPATIBLE set
	 * a 3-bit field. so writing any value <8 should read the same, writing anything >8 will read 0
	 * also, VIDIOCQCGVIDEONR should return the correct device number.
	 * VIDIOCQCSUSERLUT set a user lookup table, so reading from it, should return the same
	 */
	CLEAR(default_ulut);
	CLEAR(our_ulut);
	CLEAR(check_ulut);
	//get the default ulut
	default_ulut.flags |= QC_USERLUT_VALUES;
	default_ulut.flags |= QC_USERLUT_DEFAULT;
	if(ioctl(vdev->fd, VIDIOCQCGUSERLUT, &default_ulut) != 0)
		goto end;
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "..\n");
	
	//create a fake ulut
	for (unsigned int i = 0; i < QC_LUT_SIZE; i++)
		our_ulut.lut[i] = (unsigned char) (i % 3);
	
	//send it to QC driver
	our_ulut.flags |= QC_USERLUT_VALUES;
	our_ulut.flags |= QC_USERLUT_DEFAULT;
	if(ioctl(vdev->fd, VIDIOCQCSUSERLUT, &our_ulut) != 0)
		goto end;
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, ".. ..\n");
	
	//read it back and check it
	check_ulut.flags |= QC_USERLUT_VALUES;
	check_ulut.flags |= QC_USERLUT_DEFAULT;
	if(ioctl(vdev->fd, VIDIOCQCGUSERLUT, &check_ulut) != 0)
		goto end;
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, ".. .. ..\n");
	
	for (unsigned int i = 0; i < QC_LUT_SIZE; i++)
		if(check_ulut.lut[i] != our_ulut.lut[i])
			goto end;
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, ".. .. .. ..\n");
	
	//put default back
	default_ulut.flags |= QC_USERLUT_VALUES;
	default_ulut.flags |= QC_USERLUT_DEFAULT;
	if(ioctl(vdev->fd, VIDIOCQCSUSERLUT, &default_ulut) != 0)
		goto end;
	
	//do we need more checks ?
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "QC: found QC driver (%d controls)\n", NB_PRIV_IOCTL);
	XMALLOC(priv, struct qc_probe_private *, sizeof(struct qc_probe_private ));
	*data = (void *)priv;
	priv->ok = true;
	return NB_PRIV_IOCTL;

end:
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "QC: not found\n");
	return -1;
}

int qc_get_ctrl(struct video_device *vdev, struct v4l2_queryctrl *q, void *d, int *val) {
	UNUSED(d);
	
	if (q->id >= NUM_QC_CTRLS) {
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "QC: Cant identify control %d\n", q->id);
		return LIBVIDEO_ERR_IOCTL;
	}
	
	if (ioctl(vdev->fd, qc_getctrl_requests[q->id], val) != 0)
		return LIBVIDEO_ERR_IOCTL;
	return LIBVIDEO_ERR_SUCCESS;
}

int qc_set_ctrl(struct video_device *vdev, struct v4l2_queryctrl *q, int *val, void *data) {
	UNUSED(data);
	
	if (q->id >= NUM_QC_CTRLS) {
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "QC: Cant identify control %d\n", q->id);
		return LIBVIDEO_ERR_IOCTL;
	}
	
	int prev = 0;
	ioctl(vdev->fd, qc_getctrl_requests[q->id], &prev);//TODO check return?
	if (ioctl(vdev->fd, qc_setctrl_requests[q->id], val) != 0) {
		*val = prev;
		return LIBVIDEO_ERR_IOCTL;
	}
	return LIBVIDEO_ERR_SUCCESS;
}

int qc_list_ctrl(struct video_device *vdev, struct control *c, void *d) {
	UNUSED(vdev);
 	struct qc_probe_private *priv = (struct qc_probe_private *) d;
	
	if(priv->ok) {
		for (unsigned int i = 0; i < NUM_QC_CTRLS; i++) {
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "QC: Found quickcam private ioctl %s\n", qc_ctrl_names[i]);
			c[i].v4l2_ctrl->id = i;
			c[i].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
			strcpy((char *) c[i].v4l2_ctrl->name, qc_ctrl_names[i]);
			c[i].v4l2_ctrl->minimum = 0;
			//TODO check if this is right
			c[i].v4l2_ctrl->maximum = (i == 2) ? 5 : 1;
			c[i].v4l2_ctrl->step = 1;
			c[i].v4l2_ctrl->default_value = 0;
			c[i].v4l2_ctrl->reserved[0] = V4L2_PRIV_IOCTL;
			c[i].v4l2_ctrl->reserved[1] = QC_PROBE_INDEX;
		}
		return 5;
	} else {
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "QC: QC not found\n");
	}
	return 0;
}