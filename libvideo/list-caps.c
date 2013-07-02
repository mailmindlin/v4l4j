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


// This program list capture parameters from /dev/video0


#include <stdio.h>
#include <stdlib.h>

#include "libvideo.h"

int main(int argc, char** argv) {
	struct capture_device *c;
	struct control_list *l;
	struct v4l2_queryctrl *qc;
	struct video_device *v;
	int std=0, channel=0, i, j;

	if(argc!=2 && argc!=4) {
		printf("This program requires the path to the video device file to be tested.\n");
		printf("The optional second and third arguments are a video standard and channel.\n");
		printf("Usage: %s <video_device_file> [standard channel]\n", argv[0]);
		printf("Video standards: webcam:0 - PAL:1 - SECAM:2 - NTSC:3\n");
		return -1;
	}


	if (argc==4){
		std = atoi(argv[2]);
		channel = atoi(argv[3]);
		printf("Using standard %d, channel %d\n",std, channel);
	}

	v = open_device(argv[1]);
	if(v==NULL){
		printf("Error opening device\n");
		return -1;
	}
	c = init_capture_device(v,MAX_WIDTH,MAX_HEIGHT,channel,std,3);
	c->actions->list_cap(v->fd);
	free_capture_device(v);
	l = get_control_list(v);
	printf("Listing available controls (%d)\n", l->count);
	for(i=0;i<l->count; i++){
		qc = l->controls[i].v4l2_ctrl;
		printf("Control: id: 0x%x - name: %s - min: %d -max: %d - step: %d - type: %d(%s) - flags: %d (%s%s%s%s%s%s)\n",
				qc->id, (char *) &qc->name, qc->minimum, qc->maximum, qc->step, qc->type,
				qc->type == V4L2_CTRL_TYPE_INTEGER ? "Integer" :
				qc->type == V4L2_CTRL_TYPE_BOOLEAN ? "Boolean" :
				qc->type == V4L2_CTRL_TYPE_MENU ? "Menu" :
				qc->type == V4L2_CTRL_TYPE_BUTTON ? "Button" :
				qc->type == V4L2_CTRL_TYPE_INTEGER64 ? "Integer64" :
				qc->type == V4L2_CTRL_TYPE_CTRL_CLASS ? "Class" :
				qc->type == V4L2_CTRL_TYPE_STRING ? "String" :
				qc->type == V4L2_CTRL_TYPE_BITMASK ? "Bitmask" :
				qc->type == V4L2_CTRL_TYPE_INTEGER_MENU ? "IntMenu" : "",
				qc->flags,
				qc->flags & V4L2_CTRL_FLAG_DISABLED ? "Disabled " : "",
				qc->flags & V4L2_CTRL_FLAG_GRABBED ? "Grabbed " : "",
				qc->flags & V4L2_CTRL_FLAG_READ_ONLY ? "ReadOnly " : "",
				qc->flags & V4L2_CTRL_FLAG_UPDATE ? "Update " : "",
				qc->flags & V4L2_CTRL_FLAG_INACTIVE ? "Inactive " : "",
				qc->flags & V4L2_CTRL_FLAG_SLIDER ? "slider " : "",
				qc->flags & V4L2_CTRL_FLAG_WRITE_ONLY ? "Write only" : "");

		if(l->controls[i].count_menu!=0){
			printf("Menu items (%d) %s\n", l->controls[i].count_menu, l->controls[i].v4l2_ctrl->step==1?"contiguous":"non-contiguous");
			for(j=0; j<l->controls[i].count_menu; j++)
				printf("\tMenu item: %s - %d\n", l->controls[i].v4l2_menu[j].name, l->controls[i].v4l2_menu[j].index);

		}
	}

	get_device_info(v);
	print_device_info(v);
	release_device_info(v);

	release_control_list(v);

	close_device(v);

	return 0;
}
