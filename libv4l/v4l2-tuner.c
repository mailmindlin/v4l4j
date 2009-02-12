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
#include <sys/ioctl.h>
#include "libv4l-err.h"
#include "log.h"

int set_tuner_freq_v4l2(struct video_device *vdev, int idx, unsigned int f){
	struct v4l2_frequency freq;
	CLEAR(freq);

	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "TUN: Setting frequency to %u for tuner %d on device %s\n", f, idx, vdev->file);

	freq.tuner = idx;
	if(-1 == ioctl(vdev->fd, VIDIOC_G_FREQUENCY, &freq)){
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "TUN: Failed to set frequency for tuner %d on device %s\n", idx, vdev->file);
		return LIBV4L_ERR_IOCTL;
	}
	freq.frequency = f;
	if(-1 == ioctl(vdev->fd, VIDIOC_S_FREQUENCY, &freq)){
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "TUN: Failed to set frequency for tuner %d on device %s\n", idx, vdev->file);
		return LIBV4L_ERR_IOCTL;
	}
	return 0;
}

int get_tuner_freq_v4l2(struct video_device *vdev, int idx, unsigned int *f){
	struct v4l2_frequency freq;
	CLEAR(freq);

	freq.tuner = idx;
	if(-1 == ioctl(vdev->fd, VIDIOC_G_FREQUENCY, &freq)){
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "TUN: Failed to get tuner frequency for tuner %d on device %s\n", idx, vdev->file);
		return LIBV4L_ERR_IOCTL;
	}
	*f = freq.frequency;
	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "TUN: Got frequency %u for tuner %d on device %s\n", *f, idx, vdev->file);
	return 0;
}

int get_rssi_afc_v4l2(struct video_device *vdev, int idx, int *r, int *a){
	struct v4l2_tuner t;
	CLEAR(t);
	t.index = idx;

	if(-1 == ioctl (vdev->fd, VIDIOC_G_TUNER, &t)){
		dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_ERR, "TUN: Failed to get tuner info for tuner %d on device %s\n", idx, vdev->file);
		return LIBV4L_ERR_IOCTL;
	}
	dprint(LIBV4L_LOG_SOURCE_CAPTURE, LIBV4L_LOG_LEVEL_DEBUG, "TUN: Got RSSI %d & AFC %d for tuner %d on device %s\n", t.signal, t.afc, idx, vdev->file);
	*r = t.signal;
	*a = t.afc;
	return 0;
}
