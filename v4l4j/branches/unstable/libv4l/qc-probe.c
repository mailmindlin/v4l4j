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
#include "qc-probe.h"
/*
 * need quickam.h (tested with qc-0.6.6)
 */
#include "quickcam.h"
#include "log.h"

#define NB_PRIV_IOCTL 5

struct qc_probe_private {
	int ok;
};


int qc_driver_probe(struct capture_device *c, void **data){
	struct qc_probe_private *priv;
	int i=-1;
	struct qc_userlut default_ulut, our_ulut, check_ulut;

	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: probing Quickam\n");
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
	if(ioctl(c->fd, VIDIOCQCGUSERLUT, &default_ulut)!=0)
		goto end;
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "..\n");

	//create a fake ulut
	while(++i<QC_LUT_SIZE){
		if(i%3==0) our_ulut.lut[i] = 0;
		else if(i%3==1) our_ulut.lut[i] = 1;
		else our_ulut.lut[i] = 2;
	}

	//send it to QC driver
	our_ulut.flags |= QC_USERLUT_VALUES;
	our_ulut.flags |= QC_USERLUT_DEFAULT;
	if(ioctl(c->fd, VIDIOCQCSUSERLUT, &our_ulut)!=0)
		goto end;
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, ".. ..\n");

	//read it back and check it
	check_ulut.flags |= QC_USERLUT_VALUES;
	check_ulut.flags |= QC_USERLUT_DEFAULT;
	if(ioctl(c->fd, VIDIOCQCGUSERLUT, &check_ulut)!=0)
		goto end;
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, ".. .. ..\n");

	i=0;
	while(++i<QC_LUT_SIZE)
		if(check_ulut.lut[i]!=our_ulut.lut[i])
			goto end;
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, ".. .. .. ..\n");

	//put default back
	default_ulut.flags |= QC_USERLUT_VALUES;
	default_ulut.flags |= QC_USERLUT_DEFAULT;
	if(ioctl(c->fd, VIDIOCQCSUSERLUT, &default_ulut)!=0)
		goto end;

	//do we need more checks ?
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: found QC driver\n");
	XMALLOC(priv, struct qc_probe_private *, sizeof(struct qc_probe_private ));
	*data = (void *)priv;
	priv->ok = 1;
	return NB_PRIV_IOCTL;

end:
	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "QC: not found\n");
	return -1;
}

int qc_get_ctrl(struct capture_device *c, struct v4l2_queryctrl *q, void *d){
	int val;
	switch (q->id) {
		case 0:
			if(ioctl(c->fd, VIDIOCQCGSETTLE, &val)==0)
				return val;
			break;
		case 1:
			if(ioctl(c->fd, VIDIOCQCGCOMPRESS, &val)==0)
				return val;
			break;
		case 2:
			if(ioctl(c->fd, VIDIOCQCGQUALITY, &val)==0)
				return val;
			break;
		case 3:
			if(ioctl(c->fd, VIDIOCQCGADAPTIVE, &val)==0)
				return val;
			break;
		case 4:
			if(ioctl(c->fd, VIDIOCQCGEQUALIZE, &val)==0)
				return val;
			break;
		default:
			dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "QC: Cant identify control %d\n",q->id);
			return -1;
	}

	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "QC: Cant get value of control %s\n",q->name);
	return -1;
}

int qc_set_ctrl(struct capture_device *c, struct v4l2_queryctrl *q, int val, void *d){
	switch (q->id) {
		case 0:
			if(ioctl(c->fd, VIDIOCQCSSETTLE, &val)==0)
				return val;
			break;
		case 1:
			if(ioctl(c->fd, VIDIOCQCSCOMPRESS, &val)==0)
				return val;
			break;
		case 2:
			if(ioctl(c->fd, VIDIOCQCSQUALITY, &val)==0)
				return val;
			break;
		case 3:
			if(ioctl(c->fd, VIDIOCQCSADAPTIVE, &val)==0)
				return val;
			break;
		case 4:
			if(ioctl(c->fd, VIDIOCQCSEQUALIZE, &val)==0)
				return val;
			break;
		default:
			dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "QC: Cant identify control %d\n",q->id);
			return 0;
	}

	dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_ERR, "QC: Cant set value of control %s\n",q->name);
	return 0;
}

int qc_list_ctrl(struct capture_device *c, struct v4l2_queryctrl *q, void *d){
	int i=0;
 	struct qc_probe_private *priv = (struct qc_probe_private *) d;
	if(priv->ok==1) {

		//
		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: Found quickcam private ioctl Brightness Settle\n");
		q[i].id=i;
		q[i].type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) q[i].name,"Brightness Settle");
		q[i].minimum =0;
		q[i].maximum = 1;
		q[i].step = 0;
		q[i].default_value = 0;
		q[i].reserved[0]=V4L2_PRIV_IOCTL;
		q[i].reserved[1]=QC_PROBE_INDEX;
		i++;

		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: Found quickcam private ioctl Compression mode\n");
		q[i].id=i;
		q[i].type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) q[i].name,"Compression");
		q[i].minimum =0;
		q[i].maximum = 1;
		q[i].step = 0;
		q[i].default_value = 0;
		q[i].reserved[0]=V4L2_PRIV_IOCTL;
		q[i].reserved[1]=QC_PROBE_INDEX;
		i++;

		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: Found quickcam private ioctl Interpolation\n");
		q[i].id=i;
		q[i].type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) q[i].name,"Interpolation");
		q[i].minimum =0;
		q[i].maximum = 5;
		q[i].step = 0;
		q[i].default_value = 0;
		q[i].reserved[0]=V4L2_PRIV_IOCTL;
		q[i].reserved[1]=QC_PROBE_INDEX;
		i++;

		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: Found quickcam private ioctl Auto Brightness\n");
		q[i].id=i;
		q[i].type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) q[i].name,"Auto Brightness");
		q[i].minimum =0;
		q[i].maximum = 1;
		q[i].step = 0;
		q[i].default_value = 0;
		q[i].reserved[0]=V4L2_PRIV_IOCTL;
		q[i].reserved[1]=QC_PROBE_INDEX;
		i++;

		dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: Found quickcam private ioctl Equalise image\n");
		q[i].id=i;
		q[i].type = V4L2_CTRL_TYPE_INTEGER;
		strcpy((char *) q[i].name,"Equalise image");
		q[i].minimum =0;
		q[i].maximum = 1;
		q[i].step = 0;
		q[i].default_value = 0;
		q[i].reserved[0]=V4L2_PRIV_IOCTL;
		q[i].reserved[1]=QC_PROBE_INDEX;
		i++;
	} else{
			dprint(LIBV4L_LOG_SOURCE_CTRL_PROBE, LIBV4L_LOG_LEVEL_DEBUG, "QC: QC not found\n");
	}
	return i;
}

