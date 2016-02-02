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

#include <errno.h>
#include <sys/ioctl.h>
#include "libvideo.h"
#include "libvideo-err.h"
#include "log.h"

struct struct_node {
 	int id;
 	struct struct_node *next;
 };
typedef struct struct_node node;

static void add_node(node **list, int id) {
	node *t;
	if((t=*list)) {
		//create the subsequent nodes
		while(t->next) t = t->next;
		XMALLOC(t->next, node *, sizeof(node));
		t->next->id = id;
	} else {
		//create the first node
		XMALLOC((*list), node *, sizeof(node));
		(*list)->id = id;
	}
}

static  void empty_list(node *list){
	node *t;
	while(list) {
		t = list->next;
		XFREE(list);
		list = t;
	}
}
static int has_id(node *list, int id){
 	for(;list;list=list->next)
 		if(list->id==id) return 1;

 	return 0;
 }

 //returns the number of controls (standard and private V4L2 controls only)
int count_v4l2_controls(struct video_device *vdev) {
	struct v4l2_queryctrl qctrl;
	node *list=NULL;
	int i, count = 0, current = 0;

	CLEAR(qctrl);

	//std ctrls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1; i++) {
		qctrl.id = i;
		if(v4lconvert_vidioc_queryctrl(vdev->control->backend->convert, &qctrl) == 0) {
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
					qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS)
				continue;
			count++;
			add_node(&list, i);
		}
	}
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found %d std ctrls\n", count);

	//priv ctrls
	for (qctrl.id = V4L2_CID_PRIVATE_BASE;; qctrl.id++) {
		if(v4lconvert_vidioc_queryctrl(vdev->control->backend->convert, &qctrl) == 0) {
			if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
					qctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS)
				continue;
			count++;
			add_node(&list, qctrl.id);
		} else {
			if (errno == EINVAL)
				break;

			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: we shouldnt be here...\n");
		}
	}
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found %d std/priv ctrls\n", count);

	//checking extended controls
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	//while (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
	while(0 == v4lconvert_vidioc_queryctrl(vdev->control->backend->convert, &qctrl)) {
		if(!has_id(list,qctrl.id) && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				&& qctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: found unique ext ctrl\n");
			count++;
		} else {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
					"CTRL: found duplicate ext ctrl\n");
		}
		if(qctrl.id<=current){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: found buggy driver\n");
			qctrl.id++;
		}
		current = qctrl.id & ~V4L2_CTRL_FLAG_NEXT_CTRL;
    	qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	empty_list(list);
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: found %d std/priv/ext controls\n", count);
	return count;
}

static void set_menu(struct menu *q, int id, int idx, char *val){
	q->id = id;
	q->index = idx;
	snprintf((char *)q->name, 32, "%s", val);
}

static void set_query_menu(struct video_device *vd, struct control *c){
	int i, count = 0, idx = 0, id = c->id;
	struct menu qm, *q;
	CLEAR(qm);
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Setting menu for control %#x\n", c->id);

	//check for known menu controls and put in some sensible menu items
	if(id == V4L2_CID_POWER_LINE_FREQUENCY){
		//populate struct control->v4l2_querymenu
		XMALLOC(q, struct menu *, 3*sizeof(struct menu));
		set_menu(&q[count++], id, V4L2_CID_POWER_LINE_FREQUENCY_DISABLED, "Disabled");
		set_menu(&q[count++], id, V4L2_CID_POWER_LINE_FREQUENCY_50HZ, "50 Hz");
		set_menu(&q[count++], id, V4L2_CID_POWER_LINE_FREQUENCY_60HZ, "60 Hz");
	} else if(id == V4L2_CID_COLORFX){
		XMALLOC(q, struct menu *, 3*sizeof(struct menu));
		set_menu(&q[count++], id, V4L2_COLORFX_NONE, "None");
		set_menu(&q[count++], id, V4L2_COLORFX_BW, "Black/White");
		set_menu(&q[count++], id, V4L2_COLORFX_SEPIA, "Sepia");
	} else if(id == V4L2_CID_MPEG_STREAM_TYPE){
		XMALLOC(q, struct menu *, 6*sizeof(struct menu));
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_PS, "MPEG2 - Program stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_TS, "MPEG2 - Transport stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG1_SS, "MPEG1 - System stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_DVD, "MPEG2 - DVD-compatible stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG1_VCD, "MPEG1 - VCD-compatible stream");
		set_menu(&q[count++], id, V4L2_MPEG_STREAM_TYPE_MPEG2_SVCD, "MPEG2 - SVCD-compatible stream");
	} else if(id == V4L2_CID_EXPOSURE_AUTO){
		XMALLOC(q, struct menu *, 4*sizeof(struct menu));
		set_menu(&q[count++], id, V4L2_EXPOSURE_AUTO, "Auto");
		set_menu(&q[count++], id, V4L2_EXPOSURE_MANUAL, "Manual");
		set_menu(&q[count++], id, V4L2_EXPOSURE_SHUTTER_PRIORITY, "Shutter priority");
		set_menu(&q[count++], id, V4L2_EXPOSURE_APERTURE_PRIORITY, "Aperture priority");
		//TODO: finish this by addinng all the MPEG-specific menus
/*
	} else if(id == V4L2_CID_COLORFX){
		XMALLOC(q, struct v4l2_querymenu *, 3);
		set_menu(&q[count++], id, V4L2_COLORFX_NONE, "None");
		set_menu(&q[count++], id, V4L2_COLORFX_BW, "Black/White");
		set_menu(&q[count++], id, V4L2_COLORFX_SEPIA, "Sepia");
	} else if(id == V4L2_CID_COLORFX){
		XMALLOC(q, struct v4l2_querymenu *, 3);
		set_menu(&q[count++], id, V4L2_COLORFX_NONE, "None");
		set_menu(&q[count++], id, V4L2_COLORFX_BW, "Black/White");
		set_menu(&q[count++], id, V4L2_COLORFX_SEPIA, "Sepia");
*/
	} else {
		//Not defined in the videodev2 header

		//count how many menus there are
		qm.id = id;
		for(i = c->minimum; i==c->maximum; i++){
			qm.index = i;
			if(ioctl(vd->fd, VIDIOC_QUERYMENU, &qm) == 0){
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
						"CTRL: found menu item %s - %d\n", qm.name, qm.index);
				count++;
			}
		}

		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
				"CTRL: found %d menus\n", count);

		if(count>0){
			//populate struct control->menus
			XMALLOC(q, struct menu *, count * sizeof(struct menu));

			for(i = c->minimum; i==c->maximum; i++){
				q[idx].id = id;
				q[idx].index = i;
				if(ioctl(vd->fd, VIDIOC_QUERYMENU, &q[idx]) == 0)
					idx++;
			}
		} else {
			//sometimes, nothing is returned by the ioctl(VIDIOC_QUERYMENU),
			//but the menu still exist and is
			//made of contiguous values between minimum and maximum.
			count = (c->maximum - c->minimum)/
					c->step + 1;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
					"CTRL: creating %d menus\n", count);
			XMALLOC(q, struct menu *,count*sizeof(struct menu));
			for(i = c->minimum;
				i<=c->maximum;
				i+=c->step){
				dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG1,
						"CTRL: menu %d - val: %d\n", idx, i);
				q[idx].id = c->id;
				sprintf((char *) q[idx].name, "%d", i);
				q[idx++].index = i;
			}
		}
	}
	c->count_menu = count;
	c->menus = q;

}

static void fix_quirky_struct(struct control *c){
	if(c->type==V4L2_CTRL_TYPE_INTEGER) {
		if(c->step==0){
			c->step = 1;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for INTEGER control '%s' "
					"from 0 to 1\n", c->name);
		}
	} else if(c->type==V4L2_CTRL_TYPE_MENU) {
		//Not sure about this one...
		if(c->step==0){
			c->step = 1;
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for MENU control '%s' "
					"from 0 to 1\n", c->name);
		}
	} else if(c->type==V4L2_CTRL_TYPE_BOOLEAN) {
		if(c->step!=1){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for BOOLEAN control '%s' "
					"from %d to 1\n", c->name, c->step);
			c->step = 1;
		}
		if(c->minimum!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted minimum value for BOOLEAN control "
					"'%s' from %d to 0\n", c->name, c->minimum);
			c->minimum = 0;
		}
		if(c->maximum!=1){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted maximum value for BOOLEAN control "
					"'%s' from %d to 1\n", c->name, c->maximum);
			c->maximum = 1;
		}
	} else if(c->type==V4L2_CTRL_TYPE_BUTTON) {
		if(c->step!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted step value for BUTTON control "
					"'%s' from %d to 0\n", c->name, c->step);
			c->step = 1;
		}
		if(c->minimum!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted minimum value for BUTTON control "
					"'%s' from %d to 0\n", c->name, c->minimum);
			c->minimum = 0;
		}
		if(c->maximum!=0){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: QUIRK: adjusted maximum value for BUTTON control "
					"'%s' from %d to 0\n", c->name, c->maximum);
			c->maximum = 0;
		}
	}
}

//Populate the control_list with reported V4L2 controls
//and returns how many controls were created
int create_v4l2_controls(struct video_device *vdev, struct control *controls,
		int max){
	struct v4l2_queryctrl qctrl;
	node *list=NULL;
	int count = 0, i, current = 0;

	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Creating std controls\n");
	//create standard V4L controls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1 && count < max; i++) {
		controls[count].id = i;
		if(v4lconvert_vidioc_queryctrl(vdev->control->backend->convert, (struct v4l2_queryctrl *)&controls[count]) == 0) {
			dprint_v4l2_control(&controls[count]);
			if ( !(controls[count].flags & V4L2_CTRL_FLAG_DISABLED)&&
					controls[count].type!=V4L2_CTRL_TYPE_CTRL_CLASS){
				fix_quirky_struct(&controls[count]);
				if(controls[count].type == V4L2_CTRL_TYPE_MENU)
					set_query_menu(vdev, &controls[count]);
				count++;
				add_node(&list, i);
			}
		}
	}

	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Creating priv controls (found %d std ctrl)\n", count);
	//create device-specific private V4L2 controls
	for (i = V4L2_CID_PRIVATE_BASE;count < max; i++) {
		controls[count].id = i;
		if(v4lconvert_vidioc_queryctrl(vdev->control->backend->convert, (struct v4l2_queryctrl *)&controls[count]) == 0) {
			dprint_v4l2_control(&controls[count]);
			if( ! (controls[count].flags & V4L2_CTRL_FLAG_DISABLED) &&
					controls[count].type!=V4L2_CTRL_TYPE_CTRL_CLASS){
				fix_quirky_struct(&controls[count]);
				if(controls[count].type == V4L2_CTRL_TYPE_MENU)
					set_query_menu(vdev, &controls[count]);
				count++;
				add_node(&list, i);
			}
    	} else {
            if (errno == EINVAL)
            	break;

            dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
            		"CTRL: we shouldnt be here...\n");
    	}
	}

	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Creating ext controls (created %d std/priv ctrl so far)\n",
			count);
	//create ext ctrls
	//TODO Add support for group-changes of extended controls. For now,
	//TODO reported ext ctrl can only be changed one at a time.
	//TODO add an extra method that list ext (so move the following to the new
	//TODO method) so apps are aware of which ctrls are
	//TODO extended ones, and can decide whether or not to change multiple ctrls
	//TODO at once or not
	//checking extended controls
	CLEAR(qctrl);
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	// while (0 == ioctl (vdev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
	while (v4lconvert_vidioc_queryctrl(vdev->control->backend->convert, &qctrl) == 0) {
		if(!has_id(list,qctrl.id ) && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				&& qctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS ){
			dprint_v4l2_control((&qctrl));
			CLEAR(controls[count]);
			memcpy(&controls[count], &qctrl,sizeof(struct control));
			fix_quirky_struct(&controls[count]);
			if(controls[count].type == V4L2_CTRL_TYPE_MENU)
				set_query_menu(vdev, &controls[count]);
			count++;
		} else {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
					"CTRL: duplicate ext ctrl\n");
		}
		if(qctrl.id<=current){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
					"CTRL: found buggy driver\n");
			qctrl.id++;
		}
		current = qctrl.id & ~V4L2_CTRL_FLAG_NEXT_CTRL;
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	empty_list(list);
	return count;
}
static int fix_quirky_values(struct control *c, int v){
	if(v < c->minimum) {
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: QUIRK: fixed quirky control value %d below minimum %d\n",
				v,c->minimum);
		return c->minimum;
	} else if (v>c->maximum) {
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: QUIRK: fixed quirky control value %d above maximum %d\n",
				v,c->maximum);
		return c->maximum;
	}
	return v;

}
//returns the value of a control
int get_control_value_v4l2(struct video_device *vdev,
		struct control *ctrl, int *val){
	struct v4l2_control vc;
	int ret = LIBVIDEO_ERR_IOCTL;
	CLEAR(vc);
	vc.id = ctrl->id;

	if( (ret = v4lconvert_vidioc_g_ctrl(vdev->control->backend->convert, &vc)) == 0 ){
		*val = fix_quirky_values(ctrl, vc.value);
		ret = 0;
	} else
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: Error getting current value (%d)\n", errno);

	return ret;
}

//sets the value of a control
int set_control_value_v4l2(struct video_device *vdev,
		struct control *ctrl, int *i) {
	struct v4l2_control vc;
	int prev = 0;

	get_control_value_v4l2(vdev,ctrl, &prev);
	vc.id = ctrl->id;
	vc.value = *i;

	if( v4lconvert_vidioc_s_ctrl(vdev->control->backend->convert, &vc) != 0 ){
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: Error setting value\n");
		if(errno == EINVAL)
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: einval\n");
		else if(errno == ERANGE)
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: erange\n");
		else
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: unknown error %d\n", errno);
		return LIBVIDEO_ERR_IOCTL;
	}
	*i=prev;
	return 0;
}
