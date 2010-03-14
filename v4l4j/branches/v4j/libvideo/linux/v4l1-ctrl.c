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

#include <sys/ioctl.h>
#include "libvideo.h"
#include "libvideo-err.h"
#include "log.h"

 //returns the number of controls (standard and private V4L1 controls only)
int count_v4l1_controls(struct video_device *vdev) {
	//4 basic controls in V4L1
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG, "CTRL: found 4 controls\n");
	return 4;
}

//Populate the control_list with fake V4L2 controls matching V4L1 video
//controls and returns how many fake controls were created
int create_v4l1_controls(struct video_device *vdev, struct control *controls,
		int max){
	int count = 0;

	//list standard V4L controls
	//brightness
	controls[count].v4l2_ctrl->id = V4L2_CID_BRIGHTNESS;
	controls[count].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)controls[count].v4l2_ctrl->name, "Brightness\0");
	controls[count].v4l2_ctrl->minimum = 0;
	controls[count].v4l2_ctrl->maximum = 65535;
	controls[count].v4l2_ctrl->step = 1;
	controls[count].v4l2_ctrl->default_value = 32768;
	controls[count].v4l2_ctrl->flags = 0;
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found control(id: %d - name: %s - min: %d - max: %d - "
			"step: %d)\n", controls[count].v4l2_ctrl->id,
			(char *) &controls[count].v4l2_ctrl->name,
			controls[count].v4l2_ctrl->minimum,
			controls[count].v4l2_ctrl->maximum,
			controls[count].v4l2_ctrl->step);
	count++;

	//hue
	controls[count].v4l2_ctrl->id = V4L2_CID_HUE;
	controls[count].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)controls[count].v4l2_ctrl->name, "Hue\0");
	controls[count].v4l2_ctrl->minimum = 0;
	controls[count].v4l2_ctrl->maximum = 65535;
	controls[count].v4l2_ctrl->step = 1;
	controls[count].v4l2_ctrl->default_value = 32768;
	controls[count].v4l2_ctrl->flags = 0;
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found control(id: %d - name: %s - min: %d -max: %d - "
			"step: %d)\n", controls[count].v4l2_ctrl->id,
			(char *) &controls[count].v4l2_ctrl->name,
			controls[count].v4l2_ctrl->minimum,
			controls[count].v4l2_ctrl->maximum,
			controls[count].v4l2_ctrl->step);
	count++;

	//color
	controls[count].v4l2_ctrl->id = V4L2_CID_SATURATION;
	controls[count].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)controls[count].v4l2_ctrl->name, "Saturation\0");
	controls[count].v4l2_ctrl->minimum = 0;
	controls[count].v4l2_ctrl->maximum = 65535;
	controls[count].v4l2_ctrl->step = 1;
	controls[count].v4l2_ctrl->default_value = 32768;
	controls[count].v4l2_ctrl->flags = 0;
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found control(id: %d - name: %s - min: %d -max: %d - "
			"step: %d)\n", controls[count].v4l2_ctrl->id,
			(char *) &controls[count].v4l2_ctrl->name,
			controls[count].v4l2_ctrl->minimum,
			controls[count].v4l2_ctrl->maximum,
			controls[count].v4l2_ctrl->step);
	count++;

	//contrast
	controls[count].v4l2_ctrl->id = V4L2_CID_CONTRAST;
	controls[count].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
	strcpy((char *)controls[count].v4l2_ctrl->name, "Contrast\0");
	controls[count].v4l2_ctrl->minimum = 0;
	controls[count].v4l2_ctrl->maximum = 65535;
	controls[count].v4l2_ctrl->step = 1;
	controls[count].v4l2_ctrl->default_value = 32768;
	controls[count].v4l2_ctrl->flags = 0;
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found control(id: %d - name: %s - min: %d -max: %d - "
			"step: %d)\n", controls[count].v4l2_ctrl->id,
			(char *) &controls[count].v4l2_ctrl->name,
			controls[count].v4l2_ctrl->minimum,
			controls[count].v4l2_ctrl->maximum,
			controls[count].v4l2_ctrl->step);
	count++;

	return count;
}

//returns the value of a control
int get_control_value_v4l1(struct video_device *vdev,
		struct v4l2_queryctrl *ctrl, int *val){
	struct video_picture pict;
	CLEAR(pict);
	//query the current image format
	if(-1 == ioctl(vdev->fd, VIDIOCGPICT, &pict)) {
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: cannot get the value for control %s\n",
				(char *) &ctrl->name );
		return LIBVIDEO_ERR_IOCTL;
	}
	switch(ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
			*val = pict.brightness;
			break;
		case V4L2_CID_HUE:
			*val = pict.hue;
			break;
		case V4L2_CID_SATURATION:
			*val = pict.colour;
			break;
		case V4L2_CID_CONTRAST:
			*val = pict.contrast;
			break;
		default:
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: unknown control %s (id: %d)\n",
					(char *) &ctrl->name, ctrl->id);
			return LIBVIDEO_ERR_IOCTL;
	}
	return 0;
}

//sets the value of a control
int set_control_value_v4l1(struct video_device *vdev,
		struct v4l2_queryctrl *ctrl, int *v){
	struct video_picture pict;
	int prev = 0;
	CLEAR(pict);
	//query the current image format
	if(-1 == ioctl(vdev->fd, VIDIOCGPICT, &pict)) {
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: cannot get the current value for control %s\n",
				(char *) &ctrl->name );
		return LIBVIDEO_ERR_IOCTL;
	}

	switch(ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
			prev = pict.brightness;
			pict.brightness = *v;
			break;
		case V4L2_CID_HUE:
			prev = pict.hue;
			pict.hue = *v;
			break;
		case V4L2_CID_SATURATION:
			prev = pict.colour;
			pict.colour = *v;
			break;
		case V4L2_CID_CONTRAST:
			prev = pict.contrast;
			pict.contrast = *v;
			break;
		default:
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: unknown control %s (id: %d)\n",
					(char *) &ctrl->name, ctrl->id);
			return LIBVIDEO_ERR_IOCTL;
	}

	//set the new value
	if((-1 == ioctl(vdev->fd, VIDIOCSPICT, &pict))) {
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: Error setting the new value (%d) for control %s\n",
				*v, (char *) &ctrl->name );
		*v = prev;
		return LIBVIDEO_ERR_IOCTL;
	}

	return 0;
}


