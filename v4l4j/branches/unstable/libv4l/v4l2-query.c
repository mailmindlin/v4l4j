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
#include "v4l2-input.h"
#include "libv4l-err.h"
#include "log.h"
#include "palettes.h"


/*
 * this function takes an image format int returned by v4l2
 * and finds the matching libv4l id
 */
static int find_v4l2_palette(int v4l2_fmt){
	int i = 0;

	while(i<NB_SUPPORTED_PALETTE) {
		if(libv4l_palettes[i].v4l2_palette == v4l2_fmt)
			return i;
		i++;
	}

	return UNSUPPORTED_PALETTE;
}

static void add_supported_palette(struct video_device *vd, int fmt){
	vd->nb_palettes++;
	XREALLOC(vd->palettes, int *, vd->nb_palettes * sizeof(int));
	vd->palettes[(vd->nb_palettes - 1)] = fmt;
}

static int check_palettes_v4l2(struct video_device *vd){
	struct v4l2_fmtdesc fmtd;
	CLEAR(fmtd);
	vd->palettes = NULL;
	int p;

	dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG1, "V4L2: Checking supported palettes.\n");

	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;

	while(ioctl(vd->fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
		dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG2, "V4L2: looking for palette %d\n", fmtd.pixelformat);
		if ((p=find_v4l2_palette(fmtd.pixelformat)) == UNSUPPORTED_PALETTE) {
			info("libv4l has encountered an unsupported image format:\n");
			info("%s (%d)\n", fmtd.description, fmtd.pixelformat);
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libv4l README file.\n");
		} else {
			dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG1,
					"V4L2: %s supported (%d)\n", libv4l_palettes[p].name, p);
			add_supported_palette(vd, p);
		}
		fmtd.index++;
	}

	return fmtd.index;
}
static int query_tuner(struct video_input *vi, int fd, int index){
	struct v4l2_tuner t;
	CLEAR(t);
	t.index = index;

	if (ioctl (fd, VIDIOC_G_TUNER, &t) > 0)
		return -1;

	dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG2,
			"V4L2: Tuner: %s - low: %u - high: %u - unit: %s\n",
			t.name, t.rangelow, t.rangehigh,
			t.capability & V4L2_TUNER_CAP_LOW ? "kHz": "MHz");

	XMALLOC(vi->tuner, struct tuner *, sizeof(struct tuner));
	strncpy(vi->tuner->name, (char *) t.name, NAME_FIELD_LENGTH);
	vi->tuner->unit = t.capability & VIDEO_TUNER_LOW ? KHZ_UNIT : MHZ_UNIT;
	vi->tuner->rssi = t.signal;
	vi->tuner->type =  t.type == V4L2_TUNER_RADIO ? RADIO_TYPE : TV_TYPE;
	vi->tuner->rangehigh = t.rangehigh;
	vi->tuner->rangelow = t.rangelow;

	dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG2,
				"V4L2: Tuner: %s - low: %ld - high: %ld - unit: %d\n",
				vi->tuner->name, vi->tuner->rangelow, vi->tuner->rangehigh,
				vi->tuner->unit);

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
		if (vi[i].nb_stds) XFREE(vi[i].supported_stds);
	}
	XFREE(vi);
}

static void add_supported_std(struct video_input *vi, int std){
	dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG2, "V4L2: Adding standard %d\n", std);
	vi->nb_stds++;
	XREALLOC(vi->supported_stds, int *, vi->nb_stds * sizeof(int));
	vi->supported_stds[(vi->nb_stds - 1)] = std;
}

int check_inputs_v4l2(struct video_device *vd){
	struct v4l2_input vi;
	int i, ret = 0;
	CLEAR(vi);
	vd->inputs = NULL;

	dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG1, "V4L2: querying inputs\n");

	//Check how many inputs there are
	while (-1 != ioctl(vd->fd, VIDIOC_ENUMINPUT, &vi))
		vi.index++;

	vd->nb_inputs = vi.index;

	dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG1, "V4L2: found %d inputs\n", vd->nb_inputs );

	XMALLOC(vd->inputs, struct video_input *, vd->nb_inputs * sizeof(struct video_input ));

	for (i=0; i<vd->nb_inputs; i++) {
		CLEAR(vi);
		CLEAR(vd->inputs[i]);
		vi.index = i;
		if (-1 == ioctl(vd->fd, VIDIOC_ENUMINPUT, &vi)) {
			info("Failed to get details of input %d on device %s\n", i, vd->file);
			ret = LIBV4L_ERR_IOCTL;
			free_video_inputs(vd->inputs,i);
			goto end;
		}

		dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG2, "V4L2: input %d - %s - %s - tuner: %d\n", i, vi.name, (vi.type == V4L2_INPUT_TYPE_TUNER) ? "Tuner" : "Camera",vi.tuner);

		strncpy(vd->inputs[i].name, (char *) vi.name, NAME_FIELD_LENGTH);
		vd->inputs[i].type = (vi.type == V4L2_INPUT_TYPE_TUNER) ? INPUT_TYPE_TUNER : INPUT_TYPE_CAMERA;

		if (vi.type & V4L2_INPUT_TYPE_TUNER) {
			dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG1, "V4L2: Querying tuner\n");
			if (-1 == query_tuner(&vd->inputs[i], vd->fd, vi.tuner)) {
				info("Failed to get details of tuner on input %d of device %s\n", i, vd->file);
				ret = LIBV4L_ERR_IOCTL;
				free_video_inputs(vd->inputs,i);
				goto end;
			}
		} else {
			dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG1, "V4L2: No tuner\n");
			vd->inputs[i].tuner = NULL;
		}

		if(vi.std & V4L2_STD_PAL) add_supported_std(&vd->inputs[i], PAL);
		if(vi.std & V4L2_STD_NTSC) add_supported_std(&vd->inputs[i], NTSC);
		if(vi.std & V4L2_STD_SECAM) add_supported_std(&vd->inputs[i], SECAM);
		if(vi.std == V4L2_STD_UNKNOWN) add_supported_std(&vd->inputs[i], WEBCAM);

	}
	end:
	return ret;
}

int query_device_v4l2(struct video_device *vd){
	int ret = 0;
	struct v4l2_capability caps;

	dprint(LIBV4L_LOG_SOURCE_V4L2, LIBV4L_LOG_LEVEL_DEBUG1, "V4L2: Querying V4L2 device.\n");

	if (check_v4l2(vd->fd, &caps)==-1) {
		info("Error checking capabilities of V4L2 video device %s", vd->file);
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}
	//fill name field
	strncpy(vd->name, (char *) caps.card, NAME_FIELD_LENGTH);

	//fill input field
	if(check_inputs_v4l2(vd)==-1){
		info("Error checking available inputs on V4L2 video device %s", vd->file);
		ret = LIBV4L_ERR_NOCAPS;
		goto end;
	}

	//fill palettes field
	vd->nb_palettes = check_palettes_v4l2(vd);

	end:
	return ret;

}

void free_video_device_v4l2(struct video_device *vd){
	free_video_inputs(vd->inputs, vd->nb_inputs);
	XFREE(vd->palettes);
}
