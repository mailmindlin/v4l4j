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

#include <sys/ioctl.h>
#include "libvideo.h"
#include "log.h"

static void enum_image_fmt_v4l1(int fd) {
	struct video_picture pic;
	int i;
	CLEAR(pic);

	printf("============================================\n"
			"Querying image format\n\n");

	if(ioctl(fd, VIDIOCGPICT, &pic) >= 0) {
		printf("brightness: %d - hue: %d - colour: %d - contrast: %d - "
				"depth: %d (palette %d)\n",	pic.brightness, pic.hue, pic.colour,
				pic.contrast, pic.depth, pic.palette);
		i = pic.palette;

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_GREY;
		printf("Palette GREY: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_HI240;
		printf("Palette HI240: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB565;
		printf("Palette RGB565: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB555;
		printf("Palette RGB555: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB24;
		printf("Palette RGB24: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RGB32;
		printf("Palette RGB32: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV422;
		printf("Palette YUV422: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUYV;
		printf("Palette YUYV: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_UYVY;
		printf("Palette UYVY: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV420;
		printf("Palette YUV420: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV411;
		printf("Palette YUV411: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_RAW;
		printf("Palette RAW: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV422P;
		printf("Palette YUV422P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV411P;
		printf("Palette YUV411P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV420P;
		printf("Palette YUV420P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

		CLEAR(pic);
		pic.palette = VIDEO_PALETTE_YUV410P;
		printf("Palette YUV410P: ");
		if(ioctl(fd, VIDIOCSPICT, &pic) < 0)
			printf("NOT");
		printf(" supported (%d)", pic.palette);
		if(i==pic.palette) printf(" (current setting)");
		printf("\n");

	} else
		printf("Not supported ...\n");
	printf("\n");
}

static void query_current_image_fmt_v4l1(int fd) {
	struct video_window win;
	CLEAR(win);
	printf("============================================\n"
			"Querying current image size\n");

	if(-1 == ioctl(fd, VIDIOCGWIN, &win)){
		printf("Cannot get the image size\n");
		return;
	}
	printf("Current width: %d\n", win.width);
	printf("Current height: %d\n", win.height);
	printf("\n");
}

static void query_capture_intf_v4l1(int fd) {
	struct video_capability vc;
	struct video_channel chan;
	int i;
	CLEAR(vc);

	if (-1 == ioctl( fd, VIDIOCGCAP, &vc)) {
		printf("Failed to get capabilities.\n");
		return;
	}


	printf("============================================\n"
			"Querying capture interfaces\n");
	for (i=0; i<vc.channels; i++) {
		CLEAR(chan);
		chan.channel=i;
		if (-1 == ioctl(fd, VIDIOCGCHAN, &chan)) {
			printf("Failed to get input details.");
			return;
		}
		printf("Input number: %d\n", chan.channel);
		printf("Name: %s\n", chan.name);
		if(chan.flags & VIDEO_VC_TUNER) {
			printf("Has tuners\n");
			printf("\tNumber of tuners: (%d) ", chan.tuners);
			//TODO: list tuner using struct video_tuner and VIDIOCGTUNER
		} else
			printf("Doesnt have tuners\n");
		if(chan.flags & VIDEO_VC_AUDIO)
			printf("Has audio\n");

		printf("Type: ");
		if(chan.type & VIDEO_TYPE_TV) printf("TV\n");
		if(chan.type & VIDEO_TYPE_CAMERA) printf("Camera\n");
		printf("\n");
	}
	printf("\n");
}

static void query_frame_sizes_v4l1(int fd){
	struct video_capability vc;
	CLEAR(vc);

	if (-1 == ioctl( fd, VIDIOCGCAP, &vc)) {
		printf("Failed to get capabilities.");
		return;
	}

	printf("============================================\n"
			"Querying supported frame sizes\n\n");
	printf("Min width: %d - Min height %d\n", vc.minwidth, vc.minheight);
	printf("Max width: %d - Max height %d\n", vc.maxwidth, vc.maxheight);
	printf("\n");
}


//void query_control(struct capture_device *);
void list_cap_v4l1(int fd) {
	struct video_capability vc;
	CLEAR(vc);

	if (-1 == ioctl( fd, VIDIOCGCAP, &vc)) {
		printf("Failed to get capabilities.");
		return;
	}

	printf("============================================\n"
			"Querying general capabilities\n\n");

	//print capabilities
	printf("Driver name: %s\n",vc.name);
	if (vc.type & VID_TYPE_CAPTURE) printf("Has"); else printf("Does NOT have");
	printf(" capture capability\n");
	if (vc.type & VID_TYPE_TUNER) printf("Has"); else printf("Does NOT have");
	printf(" tuner\n");
	if (vc.type & VID_TYPE_TELETEXT) printf("Has"); else printf("Does NOT have");
	printf(" teletext capability\n");
	if (vc.type & VID_TYPE_OVERLAY) printf("Has"); else printf("Does NOT have");
	printf(" overlay capability\n");
	if (vc.type & VID_TYPE_CHROMAKEY) printf("Has"); else printf("Does NOT have");
	printf(" overlay chromakey capability\n");
	if (vc.type & VID_TYPE_CLIPPING) printf("Has"); else printf("Does NOT have");
	printf(" clipping capability\n");
	if (vc.type & VID_TYPE_FRAMERAM) printf("Has"); else printf("Does NOT have");
	printf(" frame buffer overlay capability\n");
	if (vc.type & VID_TYPE_SCALES) printf("Has"); else printf("Does NOT have");
	printf(" scaling capability\n");
	if (vc.type & VID_TYPE_MONOCHROME) printf("Has"); else printf("Does NOT have");
	printf(" monochrome only capture\n");
	if (vc.type & VID_TYPE_SUBCAPTURE) printf("Has"); else printf("Does NOT have");
	printf(" sub capture capability\n");

	query_capture_intf_v4l1(fd);
	enum_image_fmt_v4l1(fd);
	query_current_image_fmt_v4l1(fd);
	query_frame_sizes_v4l1(fd);
	//query_control(c);
}
