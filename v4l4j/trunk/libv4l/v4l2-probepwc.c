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
#include "pwc_probe.h"
/*
 * need pwc-ioctl.h (tested with pwc 20080311-042701)
 */
#include "pwc-ioctl.h"
#include "libv4l2_log.h"

#define NB_PRIV_IOCTL 3

struct pwc_probe_private {
	int isPTZ;
};


int pwc_driver_probe(struct capture_device *c, struct control_list *l){
	struct pwc_probe p;
	struct pwc_probe_private *priv = (struct pwc_probe_private *) malloc(sizeof(struct pwc_probe_private ));
	l->probe_priv = (void *)priv;

	int i=3;
	dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: probing PWC ...\n");
	if(ioctl(c->fd, VIDIOCPWCPROBE, &p)>=0) {
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: found pwc driver\n");
		if(ioctl(c->fd, VIDIOCPWCMPTRESET, &i)>=0) {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: found PTZ-capable camera\n");
			priv->isPTZ=1;
			return NB_PRIV_IOCTL;
		} else {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: no PTZ camera found\n");
			priv->isPTZ=0;
			return 0;
		}
	} else {
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: pwc driver NOT found\n");
		return 0;
	}		
}

int pwc_get_ctrl(struct capture_device *c, struct v4l2_queryctrl *q){
	struct pwc_mpt_angles angles;
	int ret = -1;

	if(q->id==0) {
		//Pan/tilt reset
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Invoked get on Pan/Tilt reset button \n");
	} else if(q->id==1) {
		
		//Pan control
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Invoked get on Pan\n");
		if(ioctl(c->fd, VIDIOCPWCMPTGANGLE, &angles)==0){
			ret = angles.pan;
		} else {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error probing pan angle\n");
		}
		
	} else if(q->id==2) {
		
		//tilt control
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Invoked get on Tilt\n");
		if(ioctl(c->fd, VIDIOCPWCMPTGANGLE, &angles)==0){
			ret = angles.tilt;
		} else {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error probing tilt angle\n");
		}
		
	} else {
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Cant identify control %d\n",q->id);
	}
	return ret;
}

int pwc_set_ctrl(struct capture_device *c, struct v4l2_queryctrl *q, int val){
	struct pwc_mpt_angles angles;
	int ret = -1;

	if(q->id==0) {
		
		//Pan/tilt reset
		int i;
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Invoked set on Pan/Tilt reset\n");
		i=3;
		if(ioctl(c->fd, VIDIOCPWCMPTRESET, &i)==0){
			ret = 0;
		} else {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error resetting pan/tilt\n");
		}
		
	} else if(q->id==1) {
		
		//Pan control
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Invoked set on pan\n");
		
		//finds the previous values for pan and tilt
		if(ioctl(c->fd, VIDIOCPWCMPTGANGLE, &angles)!=0){
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error probing pan/tilt angle\n");
		}
		
		angles.pan=val;
		angles.absolute=1;
		
		if(ioctl(c->fd, VIDIOCPWCMPTSANGLE, &angles)==0){
			ret = 0;
		} else {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error setting pan angle\n");
		}
		
	} else if(q->id==2) {
		
		//tilt control
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Invoked set on tiltn\n");
		
		//finds the previous values for pan and tilt
		if(ioctl(c->fd, VIDIOCPWCMPTGANGLE, &angles)!=0){
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error probing pan/tilt angle\n");
		}
		
		angles.tilt=val;
		angles.absolute=1;
		
		if(ioctl(c->fd, VIDIOCPWCMPTSANGLE, &angles)==0){
			ret = 0;
		} else {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error setting tilt angle\n");
		}
		
	} else {
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Cant identify control %d\n",q->id);
	}
	return ret;
}

int pwc_list_ctrl(struct capture_device *c,struct control_list *l, struct v4l2_queryctrl *q){
	int i=0;
	struct pwc_probe_private *priv = (struct pwc_probe_private *) l->probe_priv;
	if(priv->isPTZ==1) {
		
		struct pwc_mpt_range range;
		
		//Pan/tilt reset
		dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Found pwc private ioctl Pan/Tilt reset\n");
		q[i].id=i;
		q[i].type = V4L2_CTRL_TYPE_BUTTON;
		strcpy((char *) q[i].name,"Pan/Tilt reset");
		q[i].minimum = q[i].maximum = q[i].step = q[i].default_value = 0;
		q[i].reserved[0]=V4L2_PRIV_IOCTL;
		q[i].reserved[1]=PWC_PROBE_INDEX;
		i++;
	
		//Pan/tilt control
		if(ioctl(c->fd, VIDIOCPWCMPTGRANGE, &range) ==0) {
			//Pan control
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Found pwc private ioctl Pan control\n");	
			q[i].id=i;
			q[i].type = V4L2_CTRL_TYPE_INTEGER;
			strcpy((char *) q[i].name,"Pan");
			q[i].minimum = range.pan_min;
			q[i].maximum = range.pan_max;
			q[i].step = 1;
			q[i].default_value = 0;
			q[i].reserved[0]=V4L2_PRIV_IOCTL;
			q[i].reserved[1]=PWC_PROBE_INDEX;
			i++;
			
			//tilt control
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Found pwc private ioctl Tilt control\n");	
			q[i].id=i;
			q[i].type = V4L2_CTRL_TYPE_INTEGER;
			strcpy((char *) q[i].name,"Tilt");
			q[i].minimum = range.tilt_min;
			q[i].maximum = range.tilt_max;
			q[i].step = 1;
			q[i].default_value = 0;
			q[i].reserved[0]=V4L2_PRIV_IOCTL;
			q[i].reserved[1]=PWC_PROBE_INDEX;
			
		} else {
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_ERR, "V4L2: Error probing Pan/tilt range\n");
		}
	} else{
			dprint(LIBV4L2_LOG_SOURCE_CTRL_PROBE, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: PTZ not supported\n");
	}
	return i;
}

