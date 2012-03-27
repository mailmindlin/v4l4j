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
#include <fcntl.h>		//for open
#include <string.h>		//for strcpy
#include <sys/ioctl.h>	//for ioctl
#include <unistd.h>		//for write, close



#include "fps-param-probe.h"
#include "gspca-probe.h"
#include "libvideo.h"
#include "libvideo-err.h"
#include "log.h"
#include "libvideo-palettes.h"
#include "pwc-probe.h"
#include "qc-probe.h"
#include "version.h"
#include "v4l1-input.h"
#include "v4l1-query.h"
#include "v4l1-tuner.h"
#include "v4l2-input.h"
#include "v4l2-query.h"
#include "v4l2-tuner.h"
#include "videodev_additions.h"

/*
 * Copies the version info in the given char *
 * It must be allocated by caller. char [40] is enough
 */
char *get_libvideo_version(char * c) {
	snprintf(c, 39,"%d.%d-%s", VER_MAJ, VER_MIN, VER_REV);
	return c;
}

/*
 *
 * VIDEO DEVICE INTERFACE
 *
 */
struct video_device *open_device(char *file) {
	static int show_ver=0;
	struct video_device *vdev;
	int fd = -1;
	char version[40];
	if(show_ver==0){
		info("Using libvideo version %s\n", get_libvideo_version(version));
		fflush(stdout);
		show_ver=1;
	}

	//open device
	dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_DEBUG,
			"VD: Opening device file %s.\n", file);
	if ((strlen(file) == 0) || ((fd = open(file,O_RDWR )) < 0)) {
		info("V4L: unable to open device file %s. Check the name and "
				"permissions\n", file);
		return NULL;
	}

	XMALLOC(vdev, struct video_device *, sizeof(struct video_device));
	vdev->fd = fd;

	//Check v4l version (V4L2 first)
	dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_DEBUG,
			"VD: Checking V4L version on device %s\n", file);
	if(check_capture_capabilities_v4l2(fd, file)==0) {
		dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_INFO,
				"VD: device %s is V4L2\n", file);
		vdev->v4l_version=V4L2_VERSION;
	} else if(check_capture_capabilities_v4l1(fd, file)==0){
		dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_INFO,
				"VD: device %s is V4L1\n", file);
		vdev->v4l_version=V4L1_VERSION;
	} else {
		info("libvideo was unable to detect the version of V4L used by "
				"device %s\n", file);
		info("If it is a valid V4L device file & not currently used by any\n");
		info("other applications, let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");

		close_device(vdev);
		return NULL;
	}

	strncpy(vdev->file, file, FILENAME_LENGTH -1);

	return vdev;
}

//this method releases the video device. Provided that all other interfaces
//are released, it returns 0. if not, it returns LIBVIDEO_ERR_INFO_IN_USE,
//LIBVIDEO_ERR_CAPTURE_IN_USE, LIBVIDEO_ERR_CONTROL_IN_USE,
//LIBVIDEO_ERR_TUNER_IN_USE
int close_device(struct video_device *vdev) {
	//Close device file
	dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_DEBUG,
			"VD: closing device file %s.\n", vdev->file);

	//TODO: try and release info, capture and controls instead of failing
	//check that we have released the info, capture and controls stuff
	if(vdev->info) {
		dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_ERR,
				"VD: Cant close device file %s - device info data not released\n"
				, vdev->file);
		return LIBVIDEO_ERR_INFO_IN_USE;
	}
	if(vdev->capture) {
		dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_ERR,
				"VD: Cant close device file %s - capture interface not released\n"
				, vdev->file);
		return LIBVIDEO_ERR_CAPTURE_IN_USE;
	}
	if(vdev->control) {
		dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_ERR,
				"VD: Cant close device file %s - control interface not released\n"
				, vdev->file);
		return LIBVIDEO_ERR_CONTROL_IN_USE;
	}
	if(vdev->tuner_action) {
		dprint(LIBVIDEO_SOURCE_VIDDEV, LIBVIDEO_LOG_ERR,
				"VD: Cant close device file %s - tuner action not released\n"
				, vdev->file);
		return LIBVIDEO_ERR_TUNER_IN_USE;
	}

	close(vdev->fd);
	XFREE(vdev);
	return 0;
}

/*
 *
 * CAPTURE INTERFACE
 *
 */

static void setup_capture_actions(struct video_device *vdev) {
	struct capture_device *c = vdev->capture;
	XMALLOC(c->actions, struct capture_actions *,sizeof(struct capture_actions));

	if(vdev->v4l_version == V4L1_VERSION) {
		c->actions->set_cap_param = set_cap_param_v4l1;
		c->actions->init_capture = init_capture_v4l1;
		c->actions->set_frame_interval = set_frame_intv_v4l1;
		c->actions->get_frame_interval = get_frame_intv_v4l1;
		c->actions->set_video_input_std = set_video_input_std_v4l1;
		c->actions->get_video_input_std = get_video_input_std_v4l1;
		c->actions->start_capture = start_capture_v4l1;
		c->actions->dequeue_buffer = dequeue_buffer_v4l1;
		c->actions->convert_buffer = NULL;
		c->actions->enqueue_buffer = enqueue_buffer_v4l1;
		c->actions->stop_capture = stop_capture_v4l1;
		c->actions->free_capture = free_capture_v4l1;
		c->actions->list_cap = list_cap_v4l1;
	} else {
		c->actions->set_cap_param = set_cap_param_v4l2;
		c->actions->init_capture = init_capture_v4l2;
		c->actions->set_frame_interval = set_frame_intv_v4l2;
		c->actions->get_frame_interval = get_frame_intv_v4l2;
		c->actions->set_video_input_std = set_video_input_std_v4l2;
		c->actions->get_video_input_std = get_video_input_std_v4l2;
		c->actions->start_capture = start_capture_v4l2;
		c->actions->dequeue_buffer = dequeue_buffer_v4l2;
		c->actions->convert_buffer = convert_buffer_v4l2;
		c->actions->enqueue_buffer = enqueue_buffer_v4l2;
		c->actions->stop_capture = stop_capture_v4l2;
		c->actions->free_capture = free_capture_v4l2;
		c->actions->list_cap = list_cap_v4l2;
	}
}


//device file, width, height, channel, std, nb_buf
struct capture_device *init_capture_device(struct video_device *vdev,
		int w, int h, int ch, int s, int nb_buf){

	if(vdev->capture!=NULL)
		return vdev->capture;
	//create capture device
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Initialising capture interface\n");
	XMALLOC(vdev->capture,struct capture_device *,sizeof(struct capture_device));
	XMALLOC(vdev->capture->mmap, struct mmap *, sizeof(struct mmap));

	//fill in cdev struct
	vdev->capture->mmap->req_buffer_nr = nb_buf;
	vdev->capture->width = w;
	vdev->capture->height = h;
	vdev->capture->channel = ch;
	vdev->capture->std = s;

	setup_capture_actions(vdev);

	if(vdev->v4l_version == V4L2_VERSION) {
		XMALLOC(vdev->capture->convert, struct convert_data *,\
				sizeof(struct convert_data));
		vdev->capture->convert->priv = v4lconvert_create(vdev->fd);
	}

	return vdev->capture;
}

//counterpart of init_capture_device, must be called if
//init_capture_device was successful
void free_capture_device(struct video_device *vdev){
	dprint(LIBVIDEO_SOURCE_CAP, LIBVIDEO_LOG_DEBUG,
			"CAP: Freeing capture device on %s.\n", vdev->file);

	if(vdev->v4l_version == V4L2_VERSION){
		v4lconvert_destroy(vdev->capture->convert->priv);
		XFREE(vdev->capture->convert);
	}

	XFREE(vdev->capture->actions);
	XFREE(vdev->capture->mmap);
	XFREE(vdev->capture);
}

static void print_frame_intv_cont(struct frame_intv_continuous *c){
	printf("\t\t\tMin: %d / %d", c->min.numerator, c->min.denominator);
	printf(" - Max: %d / %d", c->max.numerator, c->max.denominator);
	printf(" - Step: %d / %d\n", c->step.numerator, c->step.denominator);
}

static void print_frame_intv_disc(struct frame_intv_discrete *c){
	int k = -1;
	while(c[++k].numerator!=0)
		printf("%d/%d - ", c[k].numerator, c[k].denominator);

	printf("\n");
}

void print_device_info(struct video_device *v){
	int j,k;
	struct device_info *i = v->info;
	printf("============================================\n\n");
	printf("Printing device info\n\n");
	printf("Device name: %s\n",i->name);
	printf("Device file: %s\n",v->file);
	printf("Supported image formats (Name - Index):\n");
	//for each palette
	for(j=0; j<i->nb_palettes; j++){

		//print it name and index
		printf("\t%s (%d)", libvideo_palettes[i->palettes[j].index].name,
				i->palettes[j].index);

		if(i->palettes[j].raw_palettes!=NULL){
			//if it s a converted palette, print the native palettes
			//which can be used to obtain this converted palette
			k=-1;
			printf(" ( converted from ");
			while(i->palettes[j].raw_palettes[++k]!=-1)
				printf("%s (%d) - ",
						libvideo_palettes[i->palettes[j].raw_palettes[k]].name,
						i->palettes[j].raw_palettes[k]
				);
			printf(" )\n");
		} else {
			//if it is a native palette, print the frame sizes and intervals
			//if available
			printf("\n");

			//if the frame size is a continuous one, print its details
			if(i->palettes[j].size_type==FRAME_SIZE_CONTINUOUS){
				printf("\t\tResolution - "
						"Min: %d x %d - Max: %d x %d"
						" - Step: %d x %d\n",
						i->palettes[j].continuous->min_width,
						i->palettes[j].continuous->min_height,
						i->palettes[j].continuous->max_width,
						i->palettes[j].continuous->max_height,
						i->palettes[j].continuous->step_width,
						i->palettes[j].continuous->step_height
				);

				//print frame interval details
				if(i->palettes[j].continuous->interval_type_min_res==FRAME_INTV_CONTINUOUS){
					printf("\t\tContinuous frame intervals for minimum resolution: ");
					print_frame_intv_cont(i->palettes[j].continuous->intv_min_res.continuous);
				}else if(i->palettes[j].continuous->interval_type_min_res==FRAME_INTV_DISCRETE){
					printf("\t\tDiscrete frame intervals for minimum resolution: ");
					print_frame_intv_disc(i->palettes[j].continuous->intv_min_res.discrete);
				} else
					printf("\t\tFrame interval enumeration not supported\n");

				if(i->palettes[j].continuous->interval_type_max_res==FRAME_INTV_CONTINUOUS){
					printf("\t\tContinuous frame intervals for maximum resolution: ");
					print_frame_intv_cont(i->palettes[j].continuous->intv_max_res.continuous);
				}else if(i->palettes[j].continuous->interval_type_max_res==FRAME_INTV_DISCRETE){
					printf("\t\tDiscrete frame intervals for maximum resolution: ");
					print_frame_intv_disc(i->palettes[j].continuous->intv_max_res.discrete);
				} else
					printf("\t\tFrame interval enumeration not supported\n");
			}else if(i->palettes[j].size_type==FRAME_SIZE_DISCRETE){
				//frame size type is discrete
				k = -1;

				//print frame size & interval info
				while(i->palettes[j].discrete[++k].width!=0) {
					printf("\t\t%d x %d\n",
							i->palettes[j].discrete[k].width,
							i->palettes[j].discrete[k].height
					);
					if(i->palettes[j].discrete[k].interval_type==FRAME_INTV_CONTINUOUS){
						printf("\t\tContinuous frame intervals: ");
						print_frame_intv_cont(i->palettes[j].discrete[k].intv.continuous);
					}else if(i->palettes[j].discrete[k].interval_type==FRAME_INTV_DISCRETE){
						printf("\t\tDiscrete frame intervals: ");
						print_frame_intv_disc(i->palettes[j].discrete[k].intv.discrete);
					} else
						printf("\t\tFrame interval enumeration not supported\n");
				}

			}else
				printf("\t\tFrame interval enumeration not supported\n");
		}
	}

	//print the input detail
	printf("Inputs:\n");
	for(j=0; j<i->nb_inputs; j++){
		printf("\tName: %s\n", i->inputs[j].name);
		printf("\tNumber: %d\n", i->inputs[j].index);
		printf("\tType: %d (%s)\n", i->inputs[j].type,
				i->inputs[j].type==INPUT_TYPE_TUNER ? "Tuner" : "Camera");
		printf("\tSupported standards:\n");
		for(k=0; k<i->inputs[j].nb_stds; k++)
			printf("\t\t%d (%s)\n",i->inputs[j].supported_stds[k],
					i->inputs[j].supported_stds[k]==WEBCAM?"Webcam":
					i->inputs[j].supported_stds[k]==PAL?"PAL":
					i->inputs[j].supported_stds[k]==SECAM?"SECAM":"NTSC");
		if(i->inputs[j].tuner!=NULL){
			printf("\tTuner\n");
			printf("\t\tName: %s\n",i->inputs[j].tuner->name);
			printf("\t\tIndex: %d\n", i->inputs[j].tuner->index);
			printf("\t\tRange low: %lu\n", i->inputs[j].tuner->rangelow);
			printf("\t\tRange high: %lu\n", i->inputs[j].tuner->rangehigh);
			printf("\t\tUnit: %d (%s)\n", i->inputs[j].tuner->unit,
					i->inputs[j].tuner->unit==KHZ_UNIT?"KHz":"MHz");
			printf("\t\tType: %d (%s)\n", i->inputs[j].tuner->type,
					i->inputs[j].tuner->type==RADIO_TYPE?"Radio":"TV");

		}
	}
}

/*
 *
 * QUERY INTERFACE
 *
 */
struct device_info *get_device_info(struct video_device *vdev){
	int ret;
	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Querying device %s.\n", vdev->file);

	XMALLOC(vdev->info, struct device_info *, sizeof(struct device_info));

	if(vdev->v4l_version == V4L2_VERSION) {
		//v4l2 device
		ret = query_device_v4l2(vdev);
	} else if (vdev->v4l_version == V4L1_VERSION) {
		//v4l1 device
		ret = query_device_v4l1(vdev);
	} else {
		info("libvideo was unable to detect the version of V4L used by "
				"device %s\n", vdev->file);
		info("If this device is not currently used by any other application,\n");
		info("please let the author know about this issue.\n");
		info("See the ISSUES section in libvideo README file.\n");
		XFREE(vdev->info);
		vdev->info = NULL;
	}

	if(ret!=0){
		info("libvideo was unable to gather information on device %s\n",
				vdev->file);
		info("If this device is not currently used by any other application,\n");
		info("please let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
		XFREE(vdev->info);
		vdev->info = NULL;
	}

	return vdev->info;
}

void release_device_info(struct video_device *vdev){
	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Releasing device info for device %s.\n", vdev->file);
	if(vdev->v4l_version == V4L2_VERSION) {
		//v4l2 device
		free_video_device_v4l2(vdev);
	} else if (vdev->v4l_version == V4L1_VERSION) {
		//v4l1 device
		free_video_device_v4l1(vdev);
	} else {
		info("libvideo was unable to detect the version of V4L used by "
				"device %s\n", vdev->file);
		info("Please let the author know about this error.\n");
		info("See the ISSUES section in the libvideo README file.\n");
		return;
	}

	XFREE(vdev->info);
}


/*
 *
 * CONTROL INTERFACE
 *
 */
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
	{NULL, NULL, NULL, NULL, NULL}
};

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
struct control_list *get_control_list(struct video_device *vdev){
	struct v4l2_control ctrl;
	int probe_id = 0,  v4l_count = 0, priv_ctrl_count = 0, nb=0;
	driver_probe *e = NULL;
	struct control_list *l;

	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG, "CTRL: Listing controls\n");

	XMALLOC(vdev->control, struct control_list *, sizeof(struct control_list));
	l = vdev->control;


	CLEAR(ctrl);

	//dry run to see how many control we have
	if(vdev->v4l_version==V4L2_VERSION){
		l->priv=v4lconvert_create(vdev->fd);
		v4l_count = count_v4l2_controls(vdev);
	} else if(vdev->v4l_version==V4L1_VERSION)
		//4 basic controls in V4L1
		v4l_count = count_v4l1_controls(vdev);
	else {
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
				"CTRL: Weird V4L version (%d)...\n", vdev->v4l_version);
		l->count=0;
		return l;
	}


	/*
	 *  The following is an attempt to support driver private (custom) ioctls.
	 * First libvideo will probe and detect the underlying video driver. Then, it
	 * will create fake V4L controls for every private ioctls so
	 * that the application can call these private ioctls through normal V4L
	 * controls.In struct v4l2_query, libvideo will use the reserved[0]
	 * field  and set it to a special unused value V4L2_PRIV_IOCTL (currently in
	 * kernel 2.6.25, only values from 1 to 6 are used by v4l2).
	 * The following code attempts to probe the underlying driver (pwc, bttv,
	 * gspca, ...) and create fake v4l2_ctrl based on supported
	 * ioctl (static list which must be updated manually after inspecting the
	 * code for each driver => ugly but there is no other option until all
	 * drivers make their private ioctl available through a control (or control
	 * class like the camera control class added to 2.6.25))
	 */
	//go through all probes
	while ( known_driver_probes[probe_id].probe!=NULL ){
		if ( (nb = known_driver_probes[probe_id].probe(
					vdev,
					&known_driver_probes[probe_id].priv)
				) != -1) {
			//if the probe is successful, add the nb of private controls
			//detected to the grand total
			priv_ctrl_count += nb;
			add_node(&l->probes,&known_driver_probes[probe_id]);
		}
		probe_id++;
	}


	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: Got %d v4l controls and %d driver probe controls \n",
			v4l_count, priv_ctrl_count);

	l->count = v4l_count + priv_ctrl_count;
	if(l->count>0) {
		XMALLOC(l->controls, struct control *,l->count * sizeof(struct control));
		for(nb = 0; nb<l->count; nb++)
			XMALLOC( l->controls[nb].v4l2_ctrl ,
					struct v4l2_queryctrl *, sizeof(struct v4l2_queryctrl) );

		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
				"CTRL: Creating v4l controls (found %d)...\n", v4l_count);

		//fill in controls
		if(vdev->v4l_version==V4L2_VERSION)
			v4l_count = create_v4l2_controls(vdev, l->controls, l->count);
		else if(vdev->v4l_version==V4L1_VERSION)
			v4l_count = create_v4l1_controls(vdev, l->controls, l->count);

		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
				"CTRL: (got %d)\n", v4l_count);

		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
				"CTRL: listing private controls (found %d)...\n",
				priv_ctrl_count);
		//Get the driver probes to look for private ioctls
		//and turn them into fake V4L2 controls
		for(e = l->probes;e;e=e->next)
		 		e->probe->list_ctrl(vdev,&l->controls[v4l_count],e->probe->priv);

		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
				"CTRL: done listing controls\n");

	} else {
		dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
				"CTRL: No controls found...\n");
	}

	return l;
}

int get_control_value(struct video_device *vdev,
		struct v4l2_queryctrl *ctrl, void *val, int size){

	int ret = 0;
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: getting value for control %s\n", ctrl->name);

	if(ctrl->reserved[0]==V4L2_PRIV_IOCTL){
		struct v4l_driver_probe *s = &known_driver_probes[ctrl->reserved[1]];
		ret = s->get_ctrl(vdev, ctrl, s->priv, val);
	} else {
		if(vdev->v4l_version==V4L2_VERSION)
			ret = get_control_value_v4l2(vdev, ctrl, val, size);
		else if(vdev->v4l_version==V4L1_VERSION)
			ret =  get_control_value_v4l1(vdev, ctrl, val);
		else {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: Weird V4L version (%d)...\n", vdev->v4l_version);
			ret =  LIBVIDEO_ERR_WRONG_VERSION;
		}
	}
	return ret;
}

int set_control_value(struct video_device *vdev,
		struct v4l2_queryctrl *ctrl, void *value, int size){

	int ret = 0;
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,
			"CTRL: setting value for control %s\n", ctrl->name);

	// Ensure the value is within the bounds
	if ((ctrl->type == V4L2_CTRL_TYPE_INTEGER) || (ctrl->type == V4L2_CTRL_TYPE_BOOLEAN) ||
			(ctrl->type == V4L2_CTRL_TYPE_MENU) || (ctrl->type == V4L2_CTRL_TYPE_BUTTON)) {
		if(*(int32_t*)value<ctrl->minimum || *(int32_t*)value > ctrl->maximum){
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: control value (%d) out of range (%d - %d)\n",
					*(int32_t*)value, ctrl->minimum, ctrl->maximum);
			return LIBVIDEO_ERR_OUT_OF_RANGE;
		}
	} else if (ctrl->type == V4L2_CTRL_TYPE_STRING) {
		// Ensure the string size is within the bounds
		if (size <= ctrl->minimum || size > (ctrl->maximum + 1)) {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: string control length (%d) out of range (%d - %d)\n",
					size, ctrl->minimum, ctrl->maximum + 1);
			return LIBVIDEO_ERR_OUT_OF_RANGE;
		}
	}

	if(ctrl->reserved[0]==V4L2_PRIV_IOCTL){
		struct v4l_driver_probe *s = &known_driver_probes[ctrl->reserved[1]];
		ret = s->set_ctrl(vdev, ctrl, value,s->priv);
	} else {
		if(vdev->v4l_version==V4L2_VERSION)
			ret = set_control_value_v4l2(vdev, ctrl, value, size);
		else if(vdev->v4l_version==V4L1_VERSION)
			ret = set_control_value_v4l1(vdev, ctrl, (int *)value);
		else {
			dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_ERR,
					"CTRL: Weird V4L version (%d)...\n", vdev->v4l_version);
			ret = LIBVIDEO_ERR_WRONG_VERSION;
		}

	}
	return ret;
}

void release_control_list(struct video_device *vdev){
	dprint(LIBVIDEO_SOURCE_CTRL, LIBVIDEO_LOG_DEBUG,"CTRL: Freeing controls \n");
	driver_probe *e;
	int i;

	//free each individual v4l2_menu and v4l2_ctrl within a struct control
	for(i=0; i<vdev->control->count; i++){
		XFREE(vdev->control->controls[i].v4l2_ctrl);
		if(vdev->control->controls[i].v4l2_menu)
			XFREE(vdev->control->controls[i].v4l2_menu);
	}

	//free all struct control
	if (vdev->control->controls)
		XFREE(vdev->control->controls);

	//free all driver probe private data
	for(e = vdev->control->probes; e; e = e->next)
		if (e->probe->priv)
			XFREE(e->probe->priv);

	//empty driver probe linked list
	empty_list(vdev->control->probes);

	//free libv4lconvert struct
	if(vdev->control->priv)
		v4lconvert_destroy(vdev->control->priv);

	//free control_list
	if (vdev->control)
		XFREE(vdev->control);
}

/*
 *
 * TUNER INTERFACE
 *
 */
struct tuner_actions *get_tuner_actions(struct video_device *vdev) {
	dprint(LIBVIDEO_SOURCE_TUNER, LIBVIDEO_LOG_DEBUG,
			"TUN: Getting struct tuner actions\n");

	XMALLOC(vdev->tuner_action,
			struct tuner_actions *, sizeof(struct tuner_actions *));

	if(vdev->v4l_version==V4L2_VERSION){
		vdev->tuner_action->get_rssi_afc = get_rssi_afc_v4l2;
		vdev->tuner_action->get_tuner_freq = get_tuner_freq_v4l2;
		vdev->tuner_action->set_tuner_freq = set_tuner_freq_v4l2;
	} else {
		vdev->tuner_action->get_rssi_afc = get_rssi_afc_v4l1;
		vdev->tuner_action->get_tuner_freq = get_tuner_freq_v4l1;
		vdev->tuner_action->set_tuner_freq = set_tuner_freq_v4l1;
	}

	return vdev->tuner_action;
}


void release_tuner_actions(struct video_device *vdev){
	dprint(LIBVIDEO_SOURCE_TUNER, LIBVIDEO_LOG_DEBUG,
			"TUN: Releasing struct tuner actions\n");
	XFREE(vdev->tuner_action);
}
