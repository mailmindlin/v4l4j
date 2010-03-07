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
#include <string.h>
#include <sys/ioctl.h>		//for ioctl

#include "libvideo.h"
#include "libvideo-palettes.h"
#include "v4l1-input.h"
#include "libvideo-err.h"
#include "log.h"

static void set_palette_info(struct device_info *di, int idx, int palette,
		struct frame_size_continuous *cont){
	XREALLOC(di->palettes,
			struct palette_info *,(idx+1) * sizeof(struct palette_info));
	di->palettes[idx].index = palette;
	di->palettes[idx].raw_palettes = NULL;
	di->palettes[idx].size_type = FRAME_SIZE_CONTINUOUS;
	di->palettes[idx].continuous = cont;
	di->palettes[idx].continuous->interval_type_max_res = FRAME_INTV_UNSUPPORTED;
	di->palettes[idx].continuous->interval_type_min_res = FRAME_INTV_UNSUPPORTED;
	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG, "QRY: %s (%d) supported\n",
			libvideo_palettes[palette].name, palette);
}

static int check_palettes_v4l1(struct video_device *vdev){
	struct video_picture pic;
	struct video_capability vc;
	struct frame_size_continuous *cont;

	int palette = 0, index=0;
	struct device_info *di = vdev->info;
	di->palettes = NULL;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				"QRY: Checking frame size.\n");
	if (-1 == ioctl( vdev->fd, VIDIOCGCAP, &vc)) {
		info("Error checking frame size of V4L1 video device %s\n",
				vdev->file);
		return 0;
	}

	XMALLOC(cont, struct frame_size_continuous *,
			sizeof(struct frame_size_continuous ));
	cont->max_height = vc.maxheight;
	cont->min_height = vc.minheight;
	cont->max_width = vc.maxwidth;
	cont->min_width = vc.minwidth;

	//can anyone think of a better value ?
	cont->step_width = 1;
	cont->step_height = 1;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Checking supported palettes.\n");
	for(palette = 0; palette < libvideo_palettes_size-3; palette++){
		if(libvideo_palettes[palette].v4l1_palette!=VIDEO_PALETTE_UNDEFINED_V4L1){
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
					"QRY: trying %s\n", libvideo_palettes[palette].name);
			CLEAR(pic);
			pic.palette = libvideo_palettes[palette].v4l1_palette;
			pic.depth = libvideo_palettes[palette].depth;
			if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0)
				set_palette_info(di, index++, palette, cont);
		}
	}

	//v4l1 weirdness
	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
			"QRY: trying %s\n", libvideo_palettes[YUV420P].name);
	CLEAR(pic);
	pic.palette = libvideo_palettes[YUV420P].v4l1_palette;
	pic.depth = libvideo_palettes[YUV420P].depth;
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0)
		set_palette_info(di, index++, YUV420, cont);

	//v4l1 weirdness
	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
			"QRY: trying %s\n", libvideo_palettes[YUV422].name);
	CLEAR(pic);
	pic.palette = libvideo_palettes[YUV422].v4l1_palette;
	pic.depth = libvideo_palettes[YUV422].depth;
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0)
		set_palette_info(di, index++, YUYV, cont);

	//v4l1 weirdness
	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
			"QRY: trying %s\n", libvideo_palettes[YUV411].name);
	CLEAR(pic);
	pic.palette = libvideo_palettes[YUV411].v4l1_palette;
	pic.depth = libvideo_palettes[YUV411].depth;
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0)
		set_palette_info(di, index++, YUV411P, cont);


	return index;
}
static int query_tuner(struct video_input_info *vi, int fd){
	struct video_tuner t;
	CLEAR(t);
	t.tuner = 0;

	if (ioctl (fd, VIDIOCGTUNER, &t) != 0)
		return -1;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Tuner: %s - low: %lu - high: %lu - unit: %s"
			" - PAL: %s - NTSC: %s - SECAM: %s\n", t.name, t.rangelow,
			t.rangehigh, t.flags & VIDEO_TUNER_LOW ? "kHz": "MHz",
			t.flags & VIDEO_TUNER_PAL ? "Yes" : "No",
			t.flags & VIDEO_TUNER_NTSC ? "Yes" : "No",
			t.flags & VIDEO_TUNER_SECAM ? "Yes" : "No");

	XMALLOC(vi->tuner, struct tuner_info *, sizeof(struct tuner_info));
	strncpy(vi->tuner->name, t.name, NAME_FIELD_LENGTH);
	vi->tuner->index = 0;
	vi->tuner->unit = t.flags & VIDEO_TUNER_LOW ? KHZ_UNIT : MHZ_UNIT;
	vi->tuner->rssi = t.signal;
	//Quirky: no way to know whether it is radio or TV
	vi->tuner->type =  TV_TYPE;
	vi->tuner->rangehigh = t.rangehigh;
	vi->tuner->rangelow = t.rangelow;

	if(t.flags & VIDEO_TUNER_PAL) {
		++vi->nb_stds;
		XREALLOC(vi->supported_stds, int *,vi->nb_stds * sizeof(int));
		vi->supported_stds[vi->nb_stds - 1] = PAL;
	}
	if(t.flags & VIDEO_TUNER_NTSC) {
		++vi->nb_stds;
		XREALLOC(vi->supported_stds, int *,vi->nb_stds * sizeof(int));
		vi->supported_stds[vi->nb_stds - 1] = NTSC;
	}
	if(t.flags & VIDEO_TUNER_SECAM) {
		++vi->nb_stds;
		XREALLOC(vi->supported_stds, int *,vi->nb_stds * sizeof(int));
		vi->supported_stds[vi->nb_stds - 1] = SECAM;
	}
	return 0;
}

static void free_tuner(struct tuner_info *t){
	if (t)
		XFREE(t);
}

static void free_video_inputs(struct video_input_info *vi, int nb){
	int i;
	for(i=0;i<nb;i++) {
		free_tuner(vi[i].tuner);
		XFREE(vi[i].supported_stds);
	}
	XFREE(vi);
}

int check_inputs_v4l1(struct video_device *vd){
	struct video_capability vc;
	struct video_channel chan;
	int i, ret = 0;
	CLEAR(vc);
	struct device_info *di = vd->info;
	di->inputs = NULL;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1, "QRY: querying inputs\n");

	if (-1 == ioctl( vd->fd, VIDIOCGCAP, &vc)) {
		info("Error checking capabilities of V4L1 video device %s\n", vd->file);
		ret = LIBVIDEO_ERR_NOCAPS;
		goto end;
	}
	di->nb_inputs = vc.channels;

	XMALLOC(di->inputs, struct video_input_info *,
			vc.channels * sizeof(struct video_input_info ));

	for (i=0; i<vc.channels; i++) {
		CLEAR(chan);
		CLEAR(di->inputs[i]);
		chan.channel=i;
		if (-1 == ioctl(vd->fd, VIDIOCGCHAN, &chan)) {
			info("Failed to get details of input %d on device %s\n",i,vd->file);
			ret = LIBVIDEO_ERR_IOCTL;
			free_video_inputs(di->inputs,i);
			goto end;
		}

		dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				"QRY: Found input %d - %s - %s - tuner: %d\n", i, chan.name,
				(chan.type == VIDEO_TYPE_TV) ? "Tuner" : "Camera",chan.tuners);

		//Quirky code: for sake of consistency, we assume there is only one
		//tuner per input (v4l2 says so, but not v4l1!!!
		if(chan.tuners > 1) {
			info("Your V4L1 device has more than one tuner connected to this ");
			info("input.\nThis is currently not supported by libvideo.\n");
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libvideo README file.\n");
			ret = LIBVIDEO_ERR_NOCAPS;
			free_video_inputs(di->inputs,i);
			goto end;
		}

		strncpy(di->inputs[i].name, chan.name, NAME_FIELD_LENGTH);
		di->inputs[i].index = 0;
		di->inputs[i].type = (chan.type == VIDEO_TYPE_TV) ?
				INPUT_TYPE_TUNER : INPUT_TYPE_CAMERA;

		//Quirky code: input standard can only be set from struct video_tuner
		//in v4l1 which implies that there is a tuner. What if the input
		//consists of an S-VIDEO connector supporting multiple standards ? how
		//do you set the standard in this case, given that there are no tuner
		//to invoke VIDIOCSTUNER(struct video_tuner) on ?

		if (chan.flags & VIDEO_VC_TUNER) {
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
					"QRY: Querying tuner\n");
			if (-1 == query_tuner(&di->inputs[i], vd->fd)) {
				info("Failed to get details of tuner on input %d of device %s\n"
						, i, vd->file);
				ret = LIBVIDEO_ERR_IOCTL;
				free_video_inputs(di->inputs,i);
				goto end;
			}
		} else {
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG, "QRY: No tuner\n");
			di->inputs[i].nb_stds = 1;
			XMALLOC(di->inputs[i].supported_stds, int *, sizeof(int));
			di->inputs[i].supported_stds[0] = WEBCAM;
			di->inputs[i].tuner = NULL;
		}

	}
	end:
	return ret;
}

static int list_frame_intv(struct device_info *dinfo, int fmt, int width,
		int height, void **p){
	*p = NULL;
	return FRAME_INTV_UNSUPPORTED;
}

int query_device_v4l1(struct video_device *vdev){
	int ret = 0;
	struct video_capability caps;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Querying V4L1 device.\n");

	if (check_v4l1(vdev->fd, &caps)==-1) {
		info("Error checking capabilities of V4L1 video device");
		ret = LIBVIDEO_ERR_NOCAPS;
		goto end;
	}
	//fill name field
	strncpy(vdev->info->name, caps.name, NAME_FIELD_LENGTH);

	//fill input field
	if(check_inputs_v4l1(vdev)==-1){
		info("Error checking available inputs on V4L1 video device");
		ret = LIBVIDEO_ERR_NOCAPS;
		goto end;
	}

	//fill palettes field
	vdev->info->nb_palettes = check_palettes_v4l1(vdev);

	vdev->info->list_frame_intv = list_frame_intv;

	end:
	return ret;

}

void free_video_device_v4l1(struct video_device *vd){
	free_video_inputs(vd->info->inputs, vd->info->nb_inputs);
	XFREE(vd->info->palettes->continuous);
	XFREE(vd->info->palettes);
}
