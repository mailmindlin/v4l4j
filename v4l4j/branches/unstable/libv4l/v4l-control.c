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
#include <sys/ioctl.h>
#include "libv4l.h"
#include "v4l-control.h"
#include "pwc-probe.h"
#include "qc-probe.h"
#include "gspca-probe.h"
#include "fps-param-probe.h"
#include "log.h"
#include "v4l1-input.h"
#include "v4l2-input.h"
#include "videodev_additions.h"
#include "libv4l-err.h"


static struct v4l_driver_probe known_driver_probes[] = {
	{
		.probe 		= pwc_driver_probe,
		.list_ctrl 	= pwc_list_ctrl,
		.get_ctrl	= pwc_get_ctrl,
		.set_ctrl	= pwc_set_ctrl,
		.priv = NULL,
	},
	{
		.probe 		= gspca_driver_probe,
		.list_ctrl 	= gspca_list_ctrl,
		.get_ctrl	= gspca_get_ctrl,
		.set_ctrl	= gspca_set_ctrl,
		.priv = NULL,
	},
	{
		.probe 		= qc_driver_probe,
		.list_ctrl 	= qc_list_ctrl,
		.get_ctrl	= qc_get_ctrl,
		.set_ctrl	= qc_set_ctrl,
		.priv = NULL,
	},
	{
		.probe 		= fps_param_probe,
		.list_ctrl 	= fps_param_list_ctrl,
		.get_ctrl	= fps_param_get_ctrl,
		.set_ctrl	= fps_param_set_ctrl,
		.priv = NULL,
	},
};

#define PROBE_NB 4


static void add_node(driver_probe **list, struct v4l_driver_probe *probe) {
	driver_probe *t;
	if((t=*list)) {
		//create the subsequent nodes
		while(t->next) t = t->next;
		XMALLOC(t->next, driver_probe *, sizeof(driver_probe));
		t->next->probe = probe;
	} else {
		//create the first node
		XMALLOC((*list), driver_probe *, sizeof(driver_probe));
		(*list)->probe = probe;
	}
 }
static void empty_list(driver_probe *list){
	driver_probe *t;
 	while(list) {
		t = list->next;
		XFREE(list);
		list = t;
 	}
 }

// ****************************************
// Control methods
// ****************************************
struct control_list *list_control(struct capture_device *c){
	struct v4l2_control ctrl;
	int probe_id = 0, count = 0, priv_ctrl_count = 0, nb=0;
	driver_probe *e = NULL;
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
	 * First libv4l will probe and detect the underlying video driver. Then, it will create fake V4L controls for every private ioctls so
	 * that the application can call these private ioctls through normal V4L controls.In struct v4l2_query, libv4l will use the reserved[0]
	 * field  and set it to a special unused value V4L2_PRIV_IOCTL (currently in kernel 2.6.25, only values from 1 to 6 are used by v4l2).
	 * The following code attempts to probe the underlying driver (pwc, bttv, gspca, ...) and create fake v4l2_ctrl based on supported
	 * ioctl (static list which must be updated manually after inspecting the code for each driver => ugly but there is no other option until all
	 * drivers make their private ioctl available through a control (or control class like the camera control class added to 2.6.25))
	 */
	//go through all probes
	while ( probe_id<PROBE_NB ){
		if ( (nb = known_driver_probes[probe_id].probe(c, &known_driver_probes[probe_id].priv)) != -1) {
			//if the probe is successful, add the nb of private controls detected to the grand total
			priv_ctrl_count += nb;
			add_node(&c->probes,&known_driver_probes[probe_id]);
		}
		probe_id++;
	}


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

		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: listing private controls (found %d)...\n", priv_ctrl_count);
		//probe the driver for private ioctl and turn them into fake V4L2 controls
		//if(priv_ctrl_count>0)

		for(e = c->probes;e;e=e->next)
		 		e->probe->list_ctrl(c, &l->ctrl[count], e->probe->priv);
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: done listing controls\n");

	} else {
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: No controls found...\n");
	}

	return l;
}

int get_control_value(struct capture_device *c, struct v4l2_queryctrl *ctrl, int *val){
	int ret = 0;
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: getting value for control %s\n", ctrl->name);
	if(ctrl->reserved[0]==V4L2_PRIV_IOCTL){
		struct v4l_driver_probe *s = &known_driver_probes[ctrl->reserved[1]];
		ret = s->get_ctrl(c, ctrl, s->priv, val);
	} else {
		if(c->v4l_version==V4L2_VERSION)
			ret = get_control_value_v4l2(c, ctrl, val);
		else if(c->v4l_version==V4L1_VERSION)
			ret =  get_control_value_v4l1(c, ctrl, val);
		else {
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: Weird V4L version (%d)...\n", c->v4l_version);
			ret =  LIBV4L_ERR_WRONG_VERSION;
		}
	}
	return ret;
}

int set_control_value(struct capture_device *c, struct v4l2_queryctrl *ctrl, int i){
	int ret = 0;
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: setting value (%d) for control %s\n",i, ctrl->name);
	if(i<ctrl->minimum || i > ctrl->maximum){
		dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: control value out of range\n");
		return LIBV4L_ERR_OUT_OF_RANGE;
	}

	if(ctrl->reserved[0]==V4L2_PRIV_IOCTL){
		struct v4l_driver_probe *s = &known_driver_probes[ctrl->reserved[1]];
		ret = s->set_ctrl(c, ctrl, i,s->priv);
	} else {
		if(c->v4l_version==V4L2_VERSION)
			ret = set_control_value_v4l2(c, ctrl, i);
		else if(c->v4l_version==V4L1_VERSION)
			ret = set_control_value_v4l1(c, ctrl, i);
		else {
			dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_ERR, "CTRL: Weird V4L version (%d)...\n", c->v4l_version);
			ret = LIBV4L_ERR_WRONG_VERSION;
		}

	}
	return ret;
}

void free_control_list(struct capture_device *c){
	dprint(LIBV4L_LOG_SOURCE_CONTROL, LIBV4L_LOG_LEVEL_DEBUG, "CTRL: Freeing controls \n");
	driver_probe *e;
	if (c->ctrls->ctrl)
		XFREE(c->ctrls->ctrl);

	for(e = c->probes; e; e = e->next)
		if (e->probe->priv)
			XFREE(e->probe->priv);

	empty_list(c->probes);

	if (c->ctrls)
		XFREE(c->ctrls);
}


//lists all extended controls
void query_ext_controls(struct capture_device *cdev) {
	struct v4l2_queryctrl qctrl;

	printf("============================================\nQuerying available extended contols\n\n");
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl (cdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
		printf("Control class :0x%x - type: %d - %s (class ID: 0x%lx)\n", qctrl.id, qctrl.type,  qctrl.name, V4L2_CTRL_ID2CLASS(qctrl.id));
    	qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
}

//lists all supported controls
void query_control(struct capture_device *cdev){
	int i;

	printf("============================================\nQuerying available contols\n\n");
	struct control_list *l = cdev->ctrls;
	for(i=0; i<l->count; i++) {
		printf("Control %d: Name %s (Min: %d Max: %d Step: %d)\n",\
				i, (char *) l->ctrl[i]. name, l->ctrl[i].minimum, l->ctrl[i].maximum, l->ctrl[i].step);
	}
	query_ext_controls(cdev);
}


