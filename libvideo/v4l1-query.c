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
#include "v4l1-input.h"
#include "libvideo-err.h"
#include "log.h"

static int check_palettes_v4l1(struct video_device *vdev){
	struct video_picture pic;
	int index = 0;
	struct device_info *di = vdev->info;
	di->palettes = NULL;

	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: Checking supported palettes.\n");

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_GREY;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying GREY\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = GREY;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: GREY supported\n");
	}


	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_HI240;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying HI240\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = HI240;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: HI240 supported\n");
	}

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB565;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying RGB565\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = RGB565;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: RGB565 supported\n");
	}

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB555;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying RGB555\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = RGB555;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: RGB555 supported\n");
	}
	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB24;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying RGB24\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = RGB24;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: RGB24 supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB32;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying RGB32\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = RGB32;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: RGB32 supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV422;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUV422\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUV422;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: YUV422 supported\n");
	};


	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUYV;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUYV\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUYV;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: YUYV supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_UYVY;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying UYVY\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = UYVY;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRy: UYVY supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV420;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUV420\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUV420;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: YUV420 supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV411;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUV411\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUV411;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRy: YUV411 supported\n");
	};


//skipping RAW palette
//	CLEAR(pic);
//	pic.palette = VIDEO_PALETTE_RAW;

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV422P;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUV422P\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUV422P;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: YUV422P supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV411P;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUV411P\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUV411P;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: YUV411P supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV420P;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUV420P\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUV420P;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: YUV420P supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV410P;
	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: trying YUV410P\n");
	if(ioctl(vdev->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(di->palettes, int *,(index +1) * sizeof(int));
		di->palettes[index++] = YUV410P;
		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: YUV410P supported\n");
	};

	return index;
}
static int query_tuner(struct video_input_info *vi, int fd){
	struct video_tuner t;
	CLEAR(t);
	t.tuner = 0;

	if (ioctl (fd, VIDIOCGTUNER, &t) != 0)
		return -1;

	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: Tuner: %s - low: %lu - high: %lu - unit: %s"
			" - PAL: %s - NTSC: %s - SECAM: %s\n", t.name, t.rangelow, t.rangehigh,
			t.flags & VIDEO_TUNER_LOW ? "kHz": "MHz", t.flags & VIDEO_TUNER_PAL ? "Yes" : "No",
			t.flags & VIDEO_TUNER_NTSC ? "Yes" : "No",t.flags & VIDEO_TUNER_SECAM ? "Yes" : "No");

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

	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG1, "QRY: querying inputs\n");

	if (-1 == ioctl( vd->fd, VIDIOCGCAP, &vc)) {
		info("Error checking capabilities of V4L1 video device %s\n", vd->file);
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}
	di->nb_inputs = vc.channels;

	XMALLOC(di->inputs, struct video_input_info *, vc.channels * sizeof(struct video_input_info ));

	for (i=0; i<vc.channels; i++) {
		CLEAR(chan);
		CLEAR(di->inputs[i]);
		chan.channel=i;
		if (-1 == ioctl(vd->fd, VIDIOCGCHAN, &chan)) {
			info("Failed to get details of input %d on device %s\n", i, vd->file);
			ret = LIBV4L_ERR_IOCTL;
			free_video_inputs(di->inputs,i);
			goto end;
		}

		dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: Found input %d - %s - %s - tuner: %d\n", i, chan.name, (chan.type == VIDEO_TYPE_TV) ? "Tuner" : "Camera",chan.tuners);

		//Quirky code: for sake of consistency, we assume there is only one tuner per input
		//(v4l2 says so, but not v4l1!!!
		if(chan.tuners > 1) {
			info("Your V4L1 device has more than one tuner connected to this input.\n");
			info("This is currently not supported by libv4l.\n");
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libv4l README file.\n");
			ret = LIBV4L_ERR_NOCAPS;
			free_video_inputs(di->inputs,i);
			goto end;
		}

		strncpy(di->inputs[i].name, chan.name, NAME_FIELD_LENGTH);
		di->inputs[i].index = 0;
		di->inputs[i].type = (chan.type == VIDEO_TYPE_TV) ? INPUT_TYPE_TUNER : INPUT_TYPE_CAMERA;

		//Quirky code: input standard can only be set from struct video_tuner in v4l1
		//which implies that there is a tuner. What if the input consists of an S-VIDEO
		//connector supporting multiple standards ? how do you set the standard in this case,
		//given that there are no tuner to invoke VIDIOCSTUNER(struct video_tuner) on ?

		if (chan.flags & VIDEO_VC_TUNER) {
			dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: Querying tuner\n");
			if (-1 == query_tuner(&di->inputs[i], vd->fd)) {
				info("Failed to get details of tuner on input %d of device %s\n", i, vd->file);
				ret = LIBV4L_ERR_IOCTL;
				free_video_inputs(di->inputs,i);
				goto end;
			}
		} else {
			dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: No tuner\n");
			di->inputs[i].nb_stds = 1;
			XMALLOC(di->inputs[i].supported_stds, int *, sizeof(int));
			di->inputs[i].supported_stds[0] = WEBCAM;
			di->inputs[i].tuner = NULL;
		}

	}
	end:
	return ret;
}

int query_device_v4l1(struct video_device *vdev){
	int ret = 0;
	struct video_capability caps;

	dprint(LIBV4L_LOG_SOURCE_QUERY, LIBV4L_LOG_LEVEL_DEBUG, "QRY: Querying V4L1 device.\n");

	if (check_v4l1(vdev->fd, &caps)==-1) {
		info("Error checking capabilities of V4L1 video device");
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}
	//fill name field
	strncpy(vdev->info->name, caps.name, NAME_FIELD_LENGTH);

	//fill input field
	if(check_inputs_v4l1(vdev)==-1){
		info("Error checking available inputs on V4L1 video device");
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}

	//fill palettes field
	vdev->info->nb_palettes = check_palettes_v4l1(vdev);

	end:
	return ret;

}

void free_video_device_v4l1(struct video_device *vd){
	free_video_inputs(vd->info->inputs, vd->info->nb_inputs);
	XFREE(vd->info->palettes);
}
