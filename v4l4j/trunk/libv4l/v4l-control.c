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

#include <errno.h>
#include "libv4l.h"
#include "log.h"
#include "v4l-probe.h"
#include "v4l1-input.h"
#include "v4l2-input.h"
#include "videodev_additions.h"

// ****************************************
// Control methods
// ****************************************
struct control_list *list_control(struct capture_device *c){
	struct v4l2_control ctrl;
	int probe_id = 0, count = 0;
	struct control_list *l;
	
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: Listing controls\n");
	 
	XMALLOC(l, struct control_list *, sizeof(struct control_list));
	CLEAR(ctrl);	

	//dry run to see how many control we have
	if(c->v4l_version==V4L2_VERSION)
		count = count_v4l2_controls(c);
	else if(c->v4l_version==V4L1_VERSION)
		//4 basic controls in V4L1
		count = count_v4l1_controls(c);
	else {
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: Weird V4L version (%d)...\n", c->v4l_version);
		l->count=0;
		return l;
	}

	
	/*
	 *  The following is an attempt to support driver private (custom) ioctls.
	 * In struct v4l2_query, the reserved[0] field  is set to a special unused value V4L2_PRIV_IOCTL (currently in kernel 2.6.25, 
	 * only values from 1 to 6 are used by v4l2).
	 * The following code attempts to probe the underlying driver (pwc, bttv, gspca, ...) and create fake v4l2_ctrl based on supported
	 * ioctl (static list which must be updated manually after inspecting the code for each driver => ugly but there is no other option until all
	 * drivers make their private ioctl available through a control (or control class like the camera control class added to 2.6.25)) 
	 */
	int priv_ctrl_count;
	while ( ((priv_ctrl_count = probe_drivers[probe_id].probe(c, l)) == -1) && (probe_id<PROBE_NB) )
		probe_id++;
	
	count += priv_ctrl_count;
	
	l->count = count;
	if(count>0) {
		XMALLOC( l->ctrl , struct v4l2_queryctrl *, (l->count * sizeof(struct v4l2_queryctrl)) );
		
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: listing controls (found %d)...\n", count);
		
		//fill in controls
		if(c->v4l_version==V4L2_VERSION)
			count = create_v4l2_controls(c, l);
		else if(c->v4l_version==V4L1_VERSION)
			count = create_v4l1_controls(c, l);
		
		//probe the driver for private ioctl and turn them into fake V4L2 controls
		if(priv_ctrl_count!=0)
	 		probe_drivers[probe_id].list_ctrl(c, l, &l->ctrl[count]);
	} else {
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: No controls found...\n");
	}
	
	return l;
}

int get_control_value(struct capture_device *c, struct v4l2_queryctrl *ctrl){
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: getting value for control %s\n", ctrl->name);
	if(ctrl->reserved[0]==V4L2_PRIV_IOCTL){
		return probe_drivers[ctrl->reserved[1]].get_ctrl(c, ctrl);
	} else {
		if(c->v4l_version==V4L2_VERSION)
			return get_control_value_v4l2(c, ctrl);
		else if(c->v4l_version==V4L1_VERSION)
			return get_control_value_v4l1(c, ctrl);
		else {
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: Weird V4L version (%d)...\n", c->v4l_version);
			return 0;
		}
	}
}

void set_control_value(struct capture_device *c, struct v4l2_queryctrl *ctrl, int i){
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: setting value (%d) for control %s\n",i, ctrl->name);
	i = (i<ctrl->minimum || i > ctrl->maximum) ? ctrl->minimum : i;
	if(ctrl->reserved[0]==V4L2_PRIV_IOCTL){
		probe_drivers[ctrl->reserved[1]].set_ctrl(c, ctrl, i);
	} else {
		if(c->v4l_version==V4L2_VERSION)
			set_control_value_v4l2(c, ctrl, i);
		else if(c->v4l_version==V4L1_VERSION)
			set_control_value_v4l1(c, ctrl, i);
		else
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: Weird V4L version (%d)...\n", c->v4l_version);
	}
}

void free_control_list(struct control_list *l){
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: Freeing controls \n");
	if (l->ctrl)
		XFREE(l->ctrl);

	if(l->probe_priv)
		XFREE(l->probe_priv);

	if (l)
		XFREE(l);
}


//lists all supported controls
void query_control(struct capture_device *cdev){
	int i;
	
	printf("============================================\nQuerying available contols\n\n");
	struct control_list *l = cdev->ctrls; 
	for(i=0; i<l->count; i++) {
		printf("Control %d: Name %s - Value: %d (Min: %d Max: %d Step: %d)\n",\
				i, (char *) l->ctrl[i]. name, get_control_value(cdev, &l->ctrl[i]), l->ctrl[i].minimum, l->ctrl[i].maximum, l->ctrl[i].step);
	}
	
/*	struct v4l2_queryctrl qctrl;
	struct v4l2_querymenu qmenu;
	struct v4l2_control ctrl;
	int fd = cdev->fd;
	
	memset(&qctrl, 0, sizeof(struct v4l2_queryctrl));
	memset(&qmenu, 0, sizeof(struct v4l2_querymenu));
	memset(&ctrl, 0, sizeof(struct v4l2_control));
	qctrl.id = V4L2_CID_BASE;	
  	if(cdev->v4l_version == V4L2_VERSION) {
		while(ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0) {
			if (qctrl.flags == V4L2_CTRL_FLAG_DISABLED) continue;
			if (qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS) { printf("Control class name: %s\n",qctrl.name); continue; }
			printf("Name: %s", qctrl.name);
			switch ((int)qctrl.type) {
				case V4L2_CTRL_TYPE_INTEGER:
					ctrl.id = qctrl.id;
					ioctl(fd, VIDIOC_G_CTRL, &ctrl);
					printf(" - Value: %d (Min: %d Max: %d Step: %d)\n", ctrl.value, qctrl.minimum, qctrl.maximum, qctrl.step);
					break;
				case V4L2_CTRL_TYPE_BOOLEAN:
					ctrl.id = qctrl.id;
					ioctl(fd, VIDIOC_G_CTRL, &ctrl);
					printf(" - Value: %d (On/Off)\n", ctrl.value);
					break;
				case V4L2_CTRL_TYPE_BUTTON:
					ctrl.id = qctrl.id;
					ioctl(fd, VIDIOC_G_CTRL, &ctrl);
					printf(" - Value: %d (button)\n", ctrl.value);
					break;
				case V4L2_CTRL_TYPE_MENU:
					qmenu.id = qctrl.id;
					for(qmenu.index=qctrl.minimum;qmenu.index<=qctrl.maximum;qmenu.index++) {
						if (0 == ioctl(fd,VIDIOC_QUERYMENU, &qmenu)) printf("Menu item (%d): %s\n", qmenu.index, qmenu.name);
					}
					memset(&qmenu, 0, sizeof(struct v4l2_querymenu));
					break;
			}
			qctrl.id++;
		}
	}*/
}
