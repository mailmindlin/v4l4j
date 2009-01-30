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

#include "libv4l.h"
#include "v4l1-input.h"
#include "libv4l-err.h"
#include "log.h"

static int check_palettes_v4l1(struct video_device *vd){
	struct video_picture pic;
	int index = 0;
	vd->palettes = NULL;

	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: Checking supported palettes.\n");

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_GREY;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying GREY\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = GREY;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: GREY supported\n");
	}


	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_HI240;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying HI240\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = HI240;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: HI240 supported\n");
	}

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB565;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying RGB565\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = RGB565;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: RGB565 supported\n");
	}

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB555;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying RGB555\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = RGB555;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: RGB555 supported\n");
	}
	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB24;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying RGB24\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = RGB24;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: RGB24 supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_RGB32;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying RGB32\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = RGB32;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: RGB32 supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV422;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUV422\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUV422;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUV422 supported\n");
	};


	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUYV;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUYV\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUYV;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUYV supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_UYVY;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying UYVY\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = UYVY;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: UYVY supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV420;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUV420\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUV420;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUV420 supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV411;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUV411\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUV411;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUV411 supported\n");
	};


//skipping RAW palette
//	CLEAR(pic);
//	pic.palette = VIDEO_PALETTE_RAW;

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV422P;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUV422P\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUV422P;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUV422P supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV411P;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUV411P\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUV411P;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUV411P supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV420P;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUV420P\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUV420P;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUV420P supported\n");
	};

	CLEAR(pic);
	pic.palette = VIDEO_PALETTE_YUV410P;
	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: trying YUV410P\n");
	if(ioctl(vd->fd, VIDIOCSPICT, &pic) >= 0) {
		XREALLOC(vd->palettes, int *,(index +1) * sizeof(int));
		vd->palettes[index++] = YUV410P;
		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: YUV410P supported\n");
	};

	return index;
}
static int query_tuner(struct video_input *vi, int fd){
	struct video_tuner t;
	CLEAR(t);
	t.tuner = 0;

	if (ioctl (fd, VIDIOCGTUNER, &t) > 0)
		return -1;

	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: Tuner: %s - low: %lu - high: %lu - unit: %s"
			" - PAL: %s - NTSC: %s - SECAM: %s\n", t.name, t.rangelow, t.rangehigh,
			t.flags & VIDEO_TUNER_LOW ? "kHz": "MHz", t.flags & VIDEO_TUNER_PAL ? "Yes" : "No",
			t.flags & VIDEO_TUNER_NTSC ? "Yes" : "No",t.flags & VIDEO_TUNER_SECAM ? "Yes" : "No");

	XMALLOC(vi->tuner, struct tuner *, sizeof(struct tuner));
	strncpy(vi->tuner->name, t.name, NAME_FIELD_LENGTH);
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

static void free_tuner(struct tuner *t){
	if (t)
		XFREE(t);
}

static void free_video_inputs(struct video_input *vi, int nb){
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
	vd->inputs = NULL;

	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: querying inputs\n");

	if (-1 == ioctl( vd->fd, VIDIOCGCAP, &vc)) {
		info("Error checking capabilities of V4L1 video device %s\n", vd->file);
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}
	vd->nb_inputs = vc.channels;

	XMALLOC(vd->inputs, struct video_input *, vc.channels * sizeof(struct video_input ));

	for (i=0; i<vc.channels; i++) {
		CLEAR(chan);
		chan.channel=i;
		if (-1 == ioctl(vd->fd, VIDIOCGCHAN, &chan)) {
			info("Failed to get details of input %d on device %s\n", i, vd->file);
			ret = LIBV4L_ERR_IOCTL;
			free_video_inputs(vd->inputs,i);
			goto end;
		}

		dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: Found input %d - %s - %s - tuner: %d\n", i, chan.name, (chan.type == VIDEO_TYPE_TV) ? "Tuner" : "Camera",chan.tuners);

		//Quirky code: for sake of consistency, we assume there is only one tuner per input
		//(v4l2 says so, but not v4l1!!!
		if(chan.tuners > 1) {
			info("Your V4L1 device has more than one tuner connected to this input.\n");
			info("This is currently not supported by libv4l.\n");
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libv4l README file.\n");
			ret = LIBV4L_ERR_NOCAPS;
			free_video_inputs(vd->inputs,i);
			goto end;
		}

		strncpy(vd->inputs[i].name, chan.name, NAME_FIELD_LENGTH);
		vd->inputs[i].type = (chan.type == VIDEO_TYPE_TV) ? INPUT_TYPE_TUNER : INPUT_TYPE_CAMERA;

		//Quirky code: input standard can only be set from struct video_tuner in v4l1
		//which implies that there is a tuner. What if the input consists of an S-VIDEO
		//connector supporting multiple standards ? how do you set the standard in this case,
		//given that there are no tuner to invoke VIDIOCSTUNER(struct video_tuner) on ?

		if (chan.flags & VIDEO_VC_TUNER) {
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: Querying tuner\n");
			if (-1 == query_tuner(&vd->inputs[i], vd->fd)) {
				info("Failed to get details of tuner on input %d of device %s\n", i, vd->file);
				ret = LIBV4L_ERR_IOCTL;
				free_video_inputs(vd->inputs,i);
				goto end;
			}
		} else {
			dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: No tuner\n");
			vd->inputs[i].nb_stds = 1;
			XMALLOC(vd->inputs[i].supported_stds, int *, sizeof(int));
			vd->inputs[i].supported_stds[0] = WEBCAM;
			vd->inputs[i].tuner = NULL;
		}

	}
	end:
	return ret;
}

int query_device_v4l1(struct video_device *vd){
	int ret = 0;
	struct video_capability caps;

	dprint(LIBV4L_LOG_SOURCE_V4L1, LIBV4L_LOG_LEVEL_DEBUG1, "V4L1: Querying V4L1 device.\n");

	if (check_v4l1(vd->fd, &caps)==-1) {
		info("Error checking capabilities of V4L1 video device %s", vd->file);
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}
	//fill name field
	strncpy(vd->name, caps.name, NAME_FIELD_LENGTH);

	//fill input field
	if(check_inputs_v4l1(vd)==-1){
		info("Error checking available inputs on V4L1 video device %s", vd->file);
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}

	//fill palettes field
	vd->nb_palettes = check_palettes_v4l1(vd);

	end:
	return ret;

}

void free_video_device_v4l1(struct video_device *vd){
	free_video_inputs(vd->inputs, vd->nb_inputs);
	XFREE(vd->palettes);
}