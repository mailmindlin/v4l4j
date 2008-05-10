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
#include "v4l2-input.h"
#include "libv4l2_log.h"
#include "v4l2-probe.h"
#include "videodev_additions.h"


#define XMALLOC(var, type, size)	\
		do { \
			var = (type) malloc(size); \
			if (!var) dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ERR, "MEMALLOC: Cant allocate %d bytes.\n", size); \
			else { CLEAR(*var); \
				dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ALL, "MEMALLOC: allocating %d bytes of type %s for var %s (%p).\n", size, #type, #var, var); } \
		} while (0)

#define XFREE(var)					\
		do { dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ALL, "MEMALLOC: freeing memory for var %s (%p).\n", #var, var); \
			if (var) { free(var); } \
			else { dprint(LIBV4L2_LOG_SOURCE_MEMALLOC, LIBV4L2_LOG_LEVEL_ALL, "MEMALLOC: Trying to free a NULL pointer.\n");} \
		} while (0)


// ****************************************
// Control methods
// ****************************************
struct control_list *list_control(struct capture_device *c){
	struct v4l2_queryctrl qctrl;
	struct v4l2_querymenu qmenu;
	struct v4l2_control ctrl;
	int i, count = 0;
	struct control_list *l; 
	XMALLOC(l, struct control_list *, sizeof(struct control_list));
	
	memset(&qctrl, 0, sizeof(struct v4l2_queryctrl));
	memset(&qmenu, 0, sizeof(struct v4l2_querymenu));
	memset(&ctrl, 0, sizeof(struct v4l2_control));

	//dry run to see how many control we have
	//std ctrls
	for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1; i++) {
		qctrl.id = i;
		if((ioctl(c->fd, VIDIOC_QUERYCTRL, &qctrl) == 0) && ( ! (qctrl.flags & V4L2_CTRL_FLAG_DISABLED)))
			count++;
	}
	
	//priv ctrls
	for (qctrl.id = V4L2_CID_PRIVATE_BASE;; qctrl.id++) {
        if (0 == ioctl (c->fd, VIDIOC_QUERYCTRL, &qctrl)) {
                if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                        continue;
            	count++;
        } else {
                if (errno == EINVAL)
                        break;
                        
                dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_ERR, "V4L2: we shouldnt be here...\n");
        }
	}
	
	/*
	 *  The following is an attempt to support driver private (custom) ioctls.
	 * In struct v4l2_query, the reserved[0] field  is set to a special unused value V4L2_PRIV_IOCTL (currently in kernel 2.6.25, 
	 * only values from 1 to 6 are used by v4l2).
	 * The following code attempts to probe the underlying driver (pwc, bttv, gspca, ...) and create fake v4l2_ctrl based on supported
	 * ioctl (static list which must be updated manually after inspecting the code for each driver => ugly but there is no other option until all
	 * drivers make their private ioctl available through a control (or control class like the camera control class added to 2.6.25)) 
	 */
	 int priv_ctrl[PROBE_NB];
	 for(i=0; i<PROBE_NB; i++) {
	 	priv_ctrl[i] = probe_drivers[i].probe(c->fd);
	 	count += priv_ctrl[i]; 
	 }
	
	l->count = count;
	if(count>0) {
		XMALLOC( l->ctrl , struct v4l2_queryctrl *, (l->count * sizeof(struct v4l2_queryctrl)) );
		
		dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: listing controls (found %d)...\n", count);
		//fill in controls
		count = 0;
		for( i = V4L2_CID_BASE; i< V4L2_CID_LASTP1 && count < l->count; i++) {
			l->ctrl[count].id = i;
			if((ioctl(c->fd, VIDIOC_QUERYCTRL, &l->ctrl[count]) == 0) && ( ! (l->ctrl[count].flags & V4L2_CTRL_FLAG_DISABLED))) {
				dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: found control(id: %d - name: %s - min: %d -max: %d - val: %d)\n", \
				l->ctrl[count].id, (char *) &l->ctrl[count].name, l->ctrl[count].minimum, l->ctrl[count].maximum, get_control_value(c, &l->ctrl[count]));
				count++;
			}
		}
		for (i = V4L2_CID_PRIVATE_BASE;count < l->count; i++) {
			l->ctrl[count].id = i;
			if ((0 == ioctl (c->fd, VIDIOC_QUERYCTRL, &l->ctrl[count])) && ( ! (l->ctrl[count].flags & V4L2_CTRL_FLAG_DISABLED))) {
					dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: found control(id: %d - name: %s - min: %d -max: %d - val: %d)\n", \
					l->ctrl[count].id, (char *) &l->ctrl[count].name, l->ctrl[count].minimum, l->ctrl[count].maximum, get_control_value(c, &l->ctrl[count]));
	            	count++;
	        } else {
	                if (errno == EINVAL)
	                        break;
	               
	                dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_ERR, "V4L2: we shouldnt be here...\n");
	        }
		}
		
		for(i=0; i<PROBE_NB; i++)
		 		probe_drivers[i].list_ctrl(c->fd, &l->ctrl[count]);
	} else {
		dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: No controls found...\n");
	}
	
	return l;
}

int get_control_value(struct capture_device *cdec, struct v4l2_queryctrl *c){
	if(c->reserved[0]!=V4L2_PRIV_IOCTL){
		struct v4l2_control vc;
		memset(&vc, 0, sizeof(struct v4l2_control));
		vc.id = c->id;
		if(ioctl(cdec->fd, VIDIOC_G_CTRL, &vc) == 0 )
			return vc.value;
		else
			return 0;
	} else {
		return probe_drivers[c->reserved[1]].get_ctrl(cdec->fd, c);
	}
}

void set_control_value(struct capture_device *cdev, struct v4l2_queryctrl *c, int i){
	if(c->reserved[0]!=V4L2_PRIV_IOCTL){ 
		struct v4l2_control vc;
		int u;
		vc.id = c->id;
		vc.value = i;
		//sanity check
		if ( (i>=c->minimum) && (i<=c->maximum) ) {
				if(ioctl(cdev->fd, VIDIOC_S_CTRL, &vc)!= 0) {
					dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_ERR, "ioctl error: ");
					if(errno == EINVAL) dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_ERR, "einval\n"); 
					else if(errno == ERANGE) dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_ERR, "erange\n");
					else dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_ERR, "unknown error %d\n", errno);
				}
		}
		if ((u = get_control_value(cdev, c)) != i)
			dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_ERR,"Error setting control '%s' to value '%d' (current value '%d')\n", c->name, i, u);
	} else {
		probe_drivers[c->reserved[1]].set_ctrl(cdev->fd, c, i);
	}
}

void free_control_list(struct control_list *l){
	dprint(LIBV4L2_LOG_SOURCE_CONTROL, LIBV4L2_LOG_LEVEL_DEBUG, "V4L2: Freeing controls \n");
	if (l->ctrl)
		XFREE(l->ctrl);

	if (l)
		XFREE(l);
}
