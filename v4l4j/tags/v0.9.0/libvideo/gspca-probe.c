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

#include "libvideo.h"
#include "libvideo-err.h"
#include "videodev_additions.h"
#include "gspca-probe.h"

/*
 * need gspca.h (tested with gspca-20071224)
 */
struct video_param {
	int chg_para;
#define CHGABRIGHT   1
#define CHGQUALITY   2
#define CHGLIGHTFREQ 3
#define CHGTINTER    4
	__u8 autobright;
	__u8 quality;
	__u16 time_interval;
	__u8 light_freq;
};
/* Our private ioctl */
#define SPCAGVIDIOPARAM _IOR('v',BASE_VIDIOCPRIVATE + 1,struct video_param)
#define SPCASVIDIOPARAM _IOW('v',BASE_VIDIOCPRIVATE + 2,struct video_param)
/*
 * taken from gspca.h
 */

#include "log.h"

#define NB_PRIV_IOCTL 4

struct gspca_probe_private {
	int ok;
};


int gspca_driver_probe(struct video_device *vdev, void **data){
	struct gspca_probe_private *priv;
	struct video_param p;

	/*
	 * Probing gspca .... hmm, ugly hack is the first thing that comes in mind
	 * gspca has two private ioctls: SPCAGVIDIOPARAM & SPCASVIDIOPARAM to set 4 parameters:
	 * the autobrigthness, the quality (not sure what it is, only vimmicro supports it according
	 * to the gspca.c source file), the interval (to change the fps i presume), and the light frequency
	 * this hack tries to set the light frequency to all allowed values( 60, 50, 0) and to a wrong one in turn.
	 * if reading each value after setting them returns the value just set succeeds, we assume gspca is behind it.
	 */
	CLEAR(p);
	p.chg_para = CHGLIGHTFREQ;
	p.light_freq = 60;
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: probing GSPCA\n");
	if(ioctl(vdev->fd, SPCASVIDIOPARAM, &p)==0) {
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "..\n");
		CLEAR(p);
		if(ioctl(vdev->fd, SPCAGVIDIOPARAM, &p)!=0)
			goto end;

		if(p.light_freq!=60) {
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: light_freq: %d\n", p.light_freq);
			goto end;
		}

		CLEAR(p);
		p.chg_para = CHGLIGHTFREQ;
		p.light_freq = 50;
		if(ioctl(vdev->fd, SPCASVIDIOPARAM, &p)==0) {
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, ".. ..\n");
			CLEAR(p);
			if(ioctl(vdev->fd, SPCAGVIDIOPARAM, &p)!=0)
				goto end;

			if(p.light_freq!=50){
				dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: light_freq: %d\n", p.light_freq);
				goto end;
			}

			CLEAR(p);
			p.chg_para = CHGLIGHTFREQ;
			p.light_freq = 0;
			if(ioctl(vdev->fd, SPCASVIDIOPARAM, &p)==0) {
				dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, ".. .. ..\n");
					CLEAR(p);
					if(ioctl(vdev->fd, SPCAGVIDIOPARAM, &p)!=0)
						goto end;

					if(p.light_freq!=0) {
						dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: light_freq: %d\n", p.light_freq);
						goto end;
					}
					CLEAR(p);
					p.chg_para = CHGLIGHTFREQ;
					p.light_freq = 90;
					//weird: this ioctl should fail. but instead it succeed...
					if(ioctl(vdev->fd, SPCASVIDIOPARAM, &p)==0) {
						CLEAR(p);
						if(ioctl(vdev->fd, SPCAGVIDIOPARAM, &p)!=0)
							goto end;
						if(p.light_freq==0) {
							dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: found GSPCA driver (%d controls)\n", NB_PRIV_IOCTL);
							XMALLOC(priv, struct gspca_probe_private *, sizeof(struct gspca_probe_private ));
							*data = (void *)priv;
							priv->ok = 1;
							return NB_PRIV_IOCTL;
						}
					}
			}
		}
	}

end:
	dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "GSPCA: not found\n");
	return -1;
}

int gspca_get_ctrl(struct video_device *vdev, struct v4l2_queryctrl *q, void *d, int *val){
	int ret = 0;
	struct video_param p;
	if(ioctl(vdev->fd, SPCAGVIDIOPARAM, &p)<0) {
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "GSPCA: Cant get value of control %s\n",q->name);
		return ret;
	}
	switch (q->id) {
		case 0:
			*val = p.autobright;
			break;
		case 1:
			*val = p.quality;
			break;
		case 2:
			*val = p.time_interval;
			break;
		case 3:
			*val = p.light_freq;
			break;
		default:
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "GSPCA: Cant identify control %d\n",q->id);
			ret = LIBVIDEO_ERR_IOCTL;
	}
	return ret;
}

int gspca_set_ctrl(struct video_device *vdev, struct v4l2_queryctrl *q, int *val, void *d){
	struct video_param p;
	switch (q->id) {
		case 0:
			p.chg_para = CHGABRIGHT;
			p.autobright = *val;
			break;
		case 1:
			p.chg_para = CHGQUALITY;
			p.quality = *val;
			break;
		case 2:
			p.chg_para = CHGTINTER;
			p.time_interval = *val;
			break;
		case 3:
			p.chg_para = CHGLIGHTFREQ;
			p.light_freq = *val;
			break;
		default:
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "GSPCA: Cant identify control %d\n",q->id);
			return -1;
	}

	if(ioctl(vdev->fd, SPCASVIDIOPARAM, &p)<0) {
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_ERR, "GSPCA: Cant set control %s to value %d\n",q->name, *val);
		return LIBVIDEO_ERR_IOCTL;
	}

	return 0;
}

int gspca_list_ctrl(struct video_device *vdev, struct control *c, void *d){
	int i=0;
	struct gspca_probe_private *priv = (struct gspca_probe_private *) d;
	if(priv->ok==1) {

		//
		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: Found gspca private ioctl Auto-Brightness\n");
		c[i].v4l2_ctrl->id=i;
		c[i].v4l2_ctrl->type = V4L2_CTRL_TYPE_BOOLEAN;
		strcpy((char *) c[i].v4l2_ctrl->name,"Auto-Brightness");
		c[i].v4l2_ctrl->minimum =0;
		c[i].v4l2_ctrl->maximum = 1;
		c[i].v4l2_ctrl->step = 0;
		c[i].v4l2_ctrl->default_value = 0;
		c[i].v4l2_ctrl->reserved[0]=V4L2_PRIV_IOCTL;
		c[i].v4l2_ctrl->reserved[1]=GSPCA_PROBE_INDEX;
		i++;

		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: Found gspca private ioctl Quality\n");
		c[i].v4l2_ctrl->id=i;
		c[i].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) c[i].v4l2_ctrl->name,"Quality");
		c[i].v4l2_ctrl->minimum =0;
		c[i].v4l2_ctrl->maximum = 6;
		c[i].v4l2_ctrl->step = 0;
		c[i].v4l2_ctrl->default_value = 0;
		c[i].v4l2_ctrl->reserved[0]=V4L2_PRIV_IOCTL;
		c[i].v4l2_ctrl->reserved[1]=GSPCA_PROBE_INDEX;
		i++;

		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: Found gspca private ioctl Frame Interval\n");
		c[i].v4l2_ctrl->id=i;
		c[i].v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) c[i].v4l2_ctrl->name,"Frame Interval");
		c[i].v4l2_ctrl->minimum =0;
		c[i].v4l2_ctrl->maximum = 1000;
		c[i].v4l2_ctrl->step = 0;
		c[i].v4l2_ctrl->default_value = 0;
		c[i].v4l2_ctrl->reserved[0]=V4L2_PRIV_IOCTL;
		c[i].v4l2_ctrl->reserved[1]=GSPCA_PROBE_INDEX;
		i++;

		dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: Found gspca private ioctl Light Frequency\n");
		c[i].v4l2_ctrl->id=i;
		c[i].v4l2_ctrl->type = V4L2_CTRL_TYPE_MENU;
		strcpy((char *) c[i].v4l2_ctrl->name,"Light Frequency");
		c[i].v4l2_ctrl->minimum =0;
		c[i].v4l2_ctrl->maximum = 60;
		c[i].v4l2_ctrl->step = 0;
		c[i].v4l2_ctrl->default_value = 0;
		c[i].v4l2_ctrl->reserved[0]=V4L2_PRIV_IOCTL;
		c[i].v4l2_ctrl->reserved[1]=GSPCA_PROBE_INDEX;
		c[i].count_menu = 3;
		XMALLOC(c[i].v4l2_menu, struct v4l2_querymenu *, 3 * sizeof(struct v4l2_querymenu));
		c[i].v4l2_menu[0].id=i;
		c[i].v4l2_menu[0].index = 0;
		strcpy((char *) c[i].v4l2_menu[0].name,"Disable light frequency filter");
		c[i].v4l2_menu[1].id=i;
		c[i].v4l2_menu[1].index = 50;
		strcpy((char *) c[i].v4l2_menu[1].name,"50 Hz light frequency filter");
		c[i].v4l2_menu[2].id=i;
		c[i].v4l2_menu[2].index = 60;
		strcpy((char *) c[2].v4l2_menu[1].name,"60 Hz light frequency filter");
		i++;
	} else{
			dprint(LIBVIDEO_SOURCE_DRV_PROBE, LIBVIDEO_LOG_DEBUG, "GSPCA: GSPCA not found\n");
	}
	return i;
}

