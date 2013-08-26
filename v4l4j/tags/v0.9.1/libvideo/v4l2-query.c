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
#include "v4l2-input.h"
#include "libvideo-err.h"
#include "log.h"
#include "libvideo-palettes.h"

//forward declarations
static int try_format(int, int, int, struct v4l2_format *,
		struct v4l2_format *, struct v4lconvert_data *);

/*
 * this function takes an image format int returned by v4l2
 * and finds the matching libvideo id
 * returns -1 if not found
 */
static int find_v4l2_palette(int v4l2_fmt){
	int i = -1;
	while(++i<libvideo_palettes_size)
		if(libvideo_palettes[i].v4l2_palette == v4l2_fmt)
			return i;

	info("Error looking up V4L2 format %x. Please submit\n"
			"a bug report on the v4l4j mailing list.", v4l2_fmt);
	return -1;
}

//this function enumerates the frame intervals for a given v4l2 format fmt
//image width and height and modifies the pointer at p to point to either
//NULL, an array of struct frame_intv_discrete or struct frame_intv_continuous.
//It returns FRAME_INTV_UNSUPPORTED, DISCRETE or CONTINUOUS respectively.
static int lookup_frame_intv(struct v4lconvert_data *conv, int fmt, int width,
		int height, void **p){

	struct v4l2_frmivalenum intv;
	int intv_type = FRAME_INTV_UNSUPPORTED;
	struct frame_intv_discrete *d = NULL;
	struct frame_intv_continuous *c = NULL;

	CLEAR(intv);

	intv.index = 0;
	intv.pixel_format = fmt;
	intv.width = width;
	intv.height = height;

	//while we have a valid frame interval...
	while (v4lconvert_enum_frameintervals(conv, &intv) == 0) {
		//check its type (discrete, continuous / stepwise)
		if(intv.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
				if(intv_type == FRAME_INTV_UNSUPPORTED){
					intv_type = FRAME_INTV_DISCRETE;
					dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
							"QRY: Found discrete frame intv:\n");
				}

				dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
							"QRY: %d/%d\n", intv.discrete.numerator ,
							intv.discrete.denominator);

				//increase the array size by one for the extra
				//discrete frame interval
				XREALLOC(d,struct frame_intv_discrete *,
						(intv.index+1) * sizeof(struct frame_intv_discrete));

				//fill in the values of the new element
				d[intv.index].numerator = intv.discrete.numerator;
				d[intv.index].denominator = intv.discrete.denominator;

				//prepare the values for the next iteration
				intv.index++;
				intv.pixel_format = fmt;
				intv.width = width;
				intv.height = height;
		} else if((intv.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) || 
					(intv.type == V4L2_FRMIVAL_TYPE_STEPWISE)) {
				dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
					"QRY: Found %s frame intv:\n",
					(intv.type==V4L2_FRMIVAL_TYPE_CONTINUOUS)?"continuous":
					"stepwise");

				// WORKAROUND FOR BROKEN DRIVER:
				// reported on ML:
				// http://groups.google.com/group/v4l4j/browse_thread/thread/a80cb345876acf76?hl=en_US#
				// in this instance, ioctl(VIDIOC_ENUM_FRAMEINTERVALS) returns
				// stepwise frame intervals where
				// min frame interval == max frame interval and a weird value
				// for the step (1/10000000). So instead of reporting this
				// stepwise interval, we report it as a discrete one with only
				// a single value.
				if( (intv.stepwise.min.numerator == intv.stepwise.max.numerator)
				&& (intv.stepwise.min.denominator == intv.stepwise.max.denominator) ) {

					dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
						"QRY: using workaround for broken frame interval\n"
						"QRY: Reporting discrete frame intv:\n"
						"QRY: %d/%d\n", intv.stepwise.min.numerator ,
						intv.stepwise.min.denominator);

					intv_type = FRAME_INTV_DISCRETE;

					//increase the array size by one for the extra
					//discrete frame interval
					XREALLOC(d,struct frame_intv_discrete *,
							(intv.index+1) * sizeof(struct frame_intv_discrete));

					//fill in the values of the new element
					d[intv.index].numerator = intv.stepwise.min.numerator;
					d[intv.index].denominator = intv.stepwise.min.denominator;

					intv.index++;
				} else {
					//allocate memory for the continuous intv struct
					XMALLOC(c, struct frame_intv_continuous *,
							sizeof(struct frame_intv_continuous));
					intv_type = FRAME_INTV_CONTINUOUS;

					//copy the values
					dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
						"min - max - step : %d/%d - %d/%d - %d/%d\n",
						intv.stepwise.min.numerator, intv.stepwise.min.denominator,
						intv.stepwise.max.numerator, intv.stepwise.max.denominator,
						intv.stepwise.step.numerator, intv.stepwise.step.denominator
					);
					c->min.numerator = intv.stepwise.min.numerator;
					c->min.denominator = intv.stepwise.min.denominator;
					c->max.numerator = intv.stepwise.max.numerator;
					c->max.denominator = intv.stepwise.max.denominator;
					c->step.numerator = intv.stepwise.step.numerator;
					c->step.denominator = intv.stepwise.step.denominator;
				}
				
				// exit the while loop
				break;
		}
	}

	//store the appropriate result in void** arg
	if(intv_type==FRAME_INTV_DISCRETE){
		//add a struct frame_intv_discrete with null values at the end of
		//the list
		XREALLOC(d, struct frame_intv_discrete *,
				(intv.index+1)*sizeof(struct frame_intv_discrete));
		CLEAR(d[intv.index]);
		*p = d;
	} else if(intv_type==FRAME_INTV_CONTINUOUS)
		*p = c;
	else
		*p = NULL;

	return intv_type;
}

//this function enumerates the frame sizes for a given v4l2 format fmt
//and populates the struct palette_info with these sizes
static void lookup_frame_sizes(struct v4lconvert_data *conv, int fmt,
		struct palette_info *p){

	struct v4l2_frmsizeenum s;
	CLEAR(s);

	s.index = 0;
	s.pixel_format = fmt;
	while(v4lconvert_enum_framesizes(conv, &s)==0){
		if(s.type==V4L2_FRMSIZE_TYPE_DISCRETE){
			if(p->size_type==FRAME_SIZE_UNSUPPORTED){
				dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
					"QRY: Found discrete supported resolution:\n");
				p->size_type = FRAME_SIZE_DISCRETE;
			}
			XREALLOC(p->discrete, struct frame_size_discrete *,
					(s.index+1)*sizeof(struct frame_size_discrete));
			CLEAR(p->discrete[s.index]);
			p->discrete[s.index].height = s.discrete.height;
			p->discrete[s.index].width = s.discrete.width;
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
				"QRY: %d x %d\n",
				p->discrete[s.index].width, p->discrete[s.index].height);

			//fill in frame intv for this res
			p->discrete[s.index].interval_type =
					lookup_frame_intv(conv, fmt,
							s.discrete.width, s.discrete.height,
							(void **)&p->discrete[s.index].intv.discrete );

			s.index++;
		} else {
			//continuous & stepwise end up here
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
							"QRY: Found %s supported resolution:\n",
							s.type==V4L2_FRMSIZE_TYPE_CONTINUOUS?"continuous":
								"stepwise"
					);
			p->size_type=FRAME_SIZE_CONTINUOUS;
			//copy data
			XMALLOC(p->continuous, struct frame_size_continuous *,
					sizeof(struct frame_size_continuous ));
			p->continuous->max_height = s.stepwise.max_height;
			p->continuous->min_height = s.stepwise.min_height;
			p->continuous->max_width = s.stepwise.max_width;
			p->continuous->min_width= s.stepwise.min_width;
			p->continuous->step_height = s.stepwise.step_height;
			p->continuous->step_width = s.stepwise.step_width;

			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
					"QRY: Width x height (min/max/step) : %d,%d,%d x %d,%d,%d\n",
					p->continuous->min_width,
					p->continuous->max_width,
					p->continuous->step_width,
					p->continuous->min_height,
					p->continuous->max_height,
					p->continuous->step_height
				);

			// fill in frame intv for min / max res
			p->continuous->interval_type_min_res =
					lookup_frame_intv(conv, fmt,s.stepwise.min_width,
							s.stepwise.min_height,
							(void **) &p->continuous->intv_min_res.continuous);
			p->continuous->interval_type_max_res =
					lookup_frame_intv(conv, fmt,s.stepwise.max_width,
							s.stepwise.max_height,
							(void **) &p->continuous->intv_max_res.continuous);
			break;
		}
	}
	//add a struct frame_size_discrete with null values at the end of
	//the list
	if(p->size_type==FRAME_SIZE_DISCRETE){
		XREALLOC(p->discrete, struct frame_size_discrete *,
				(s.index+1)*sizeof(struct frame_size_discrete));
		CLEAR(p->discrete[s.index]);
	}

}

/*
 * this function checks if the given raw native format can be used for capture.
 * If yes, it adds the given raw palette (fmt) to the end of the list of raw
 * palettes at 'p->raw_palettes' of current size 'size', increment 'size' &
 * returns 0.
 * If the palette cannot be used for capture because libv4lconvert doesnt support
 * it, the matching converted palette is advertised as native if there are no
 * raw formats for it yet (if there are, this step is skipped)and -1 is returned
 */
static int add_raw_format(struct v4lconvert_data *conv, int width, int height,
		struct palette_info *p, int fmt, int *size){
	if(fmt!=-1){
		//test the given native format fmt to see if it can be used
		//by v4lconvert. Sometimes, the native format that must be used to
		//obtain a converted format CANNOT be used for capture in native format
		//(for instance with SPCA561 webcams). See GC issue 7.
		struct v4l2_format src, dst;
		try_format(fmt, width, height, &dst, &src, conv);
		if(dst.fmt.pix.pixelformat != libvideo_palettes[fmt].v4l2_palette){
			//the given native format is not supported by libv4lconvert
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
					"QRY: raw palette %d (%s - %#x) cannot be used for capture"
					"- Advertising this 'converted' palette (%s - %d) as a "
					"native one\n",
					fmt,
					libvideo_palettes[fmt].name,
					libvideo_palettes[fmt].v4l2_palette,
					libvideo_palettes[p->index].name,
					p->index);

			if(*size==0){
				//advertise this converted format as a native one
				//ONLY IF we havent already added a raw format (*size==0)
				p->raw_palettes=NULL;
				lookup_frame_sizes(conv, libvideo_palettes[fmt].v4l2_palette, p);
			}
			return -1;
		}
		// else
		//this raw format can be used for capture, fall through, and add it
	}
	XREALLOC(p->raw_palettes, int *, (*size+1)*sizeof(int));
	p->raw_palettes[*size] = fmt;
	*size += 1;
	return 0;
}

/*
 * This function searches the raw_palettes int array of current size 'size
 * for the format fmt. if it is found it returns 1, 0 otherwise
 */
static int has_raw_format(int *raw_palettes, int fmt, int size){
	while(--size>=0)
		if(raw_palettes[size]==fmt)
			return 1;

	return 0;
}

/*
 * This function calles v4lconvert_try_format to check if dst format
 * is supported. If it is, this function will fill in src with the format
 * that must be used to obtain dst
 * returns the result of v4lconvert_try_format
 */
static int try_format(int index, int w, int h, struct v4l2_format *dst,
		struct v4l2_format *src, struct v4lconvert_data *conv){
	CLEAR(*dst);
	CLEAR(*src);
	dst->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dst->fmt.pix.pixelformat = libvideo_palettes[index].v4l2_palette;
	dst->fmt.pix.width=w;
	dst->fmt.pix.height=h;
	return v4lconvert_try_format(conv,dst,src);
}

static int get_current_resolution(struct video_device *vdev, int *width, int *height) {
	struct v4l2_format fmt;

	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(vdev->fd, VIDIOC_G_FMT, &fmt) == 0) {
		*width = fmt.fmt.pix.width;
		*height = fmt.fmt.pix.height;
		return 0;
	}

	return -1;
}

//this function adds the given palette fmt to the list of
//supported palettes in struct device_info. It also
//checks with libv4l_convert if it is converted from another palette
//it returns 0 if everything went fine, LIBVIDEO_ERR_IOCTL otherwise
static int add_supported_palette(struct video_device *vdev, struct device_info *di, int fmt){
	struct v4l2_format dst, src;
	struct palette_info *curr;
	int i = 0, src_palette, w = 640, h = 480;
	struct v4l2_frmsizeenum s;

	di->nb_palettes++;
	XREALLOC(di->palettes, struct palette_info *,
			di->nb_palettes * sizeof(struct palette_info));

	curr = &di->palettes[(di->nb_palettes - 1)];
	CLEAR(*curr);
	curr->index = fmt;
	curr->size_type=FRAME_SIZE_UNSUPPORTED;

	//check if this format is the result of a conversion from another format
	//by libv4l_convert
	get_current_resolution(vdev, &w, &h);
	if(try_format(fmt,w,h,&dst,&src,di->convert)!=0){
		dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_ERR,
				"QRY: Error checking palette %s (libv4l convert says: %s)\n",
				libvideo_palettes[fmt].name,
				v4lconvert_get_error_message(di->convert));
		return LIBVIDEO_ERR_IOCTL;
	}

	if(v4lconvert_needs_conversion(di->convert,&src,&dst)==1){
		dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				"QRY: %s is a converted palette\n",
				libvideo_palettes[fmt].name
		);
		src_palette = find_v4l2_palette(src.fmt.pix.pixelformat);
		//it is converted from another format
		//adds the format returned by v4lconvert_needs_conversion
		dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				"QRY: from %d (%s)\n",
				src_palette,
				libvideo_palettes[src_palette].name
				);
		if(add_raw_format(di->convert, w, h, curr,src_palette, &i)==-1)
		//this raw format can not be used for capture. add_raw_format advertises
		//this converted format as a native one, and we MUST exit here.
			return 0;

		//check if there are other formats that can be converted to this one too
		//by trying other image sizes
		s.index = 0;
		s.pixel_format = dst.fmt.pix.pixelformat;
		while(v4lconvert_enum_framesizes(di->convert, &s)==0){
			if(s.type==V4L2_FRMSIZE_TYPE_DISCRETE){
				//try with this resolution
				//dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				//	"QRY: trying %dx%d\n",s.discrete.width, s.discrete.height);
				try_format(fmt,s.discrete.width,s.discrete.height,
						&dst,&src,di->convert);

				src_palette = find_v4l2_palette(src.fmt.pix.pixelformat);

				if(v4lconvert_needs_conversion(di->convert,&src,&dst)==1 &&
						!has_raw_format(curr->raw_palettes,	src_palette, i)){
					//it is converted from another format which is not
					//in the array yet. adds the format
					dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
												"QRY: from %d (%s)\n",
												src_palette,
												libvideo_palettes[src_palette].name
												);
					add_raw_format(di->convert, s.discrete.width, s.discrete.height, curr, src_palette, &i);
					if(i==-1){
						info("There is a bug in libvideo. Please report\n");
						info("this to the author through the v4l4j mailing\n");
						info("list at http://groups.google.com/group/v4l4j\n");
						return LIBVIDEO_ERR_IOCTL;
					}
				} //else no need for conversion or format already seen
				s.index++;
			} else {
				//TODO: frame size are stepwise, try other image sizes to see
				//TODO: if there are other raw image formats
				break;
			}
		}
		add_raw_format(di->convert, w, h, curr, -1, &i);
	} else {
		dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				"QRY: %s is a native palette\n",
				libvideo_palettes[fmt].name
		);
		//it is not converted from another image format
		curr->raw_palettes = NULL;

		//find supported resolutions
		lookup_frame_sizes(di->convert, libvideo_palettes[fmt].v4l2_palette, curr);
	}

	return 0;
}

//this function checks the supporte palettes
//it returns how many supported palettes there are, or LIBVIDEO_ERR_IOCTL
static int check_palettes_v4l2(struct video_device *vdev){
	vdev->info->convert = v4lconvert_create(vdev->fd);
	struct v4l2_fmtdesc fmtd;
	CLEAR(fmtd);
	struct device_info *di = vdev->info;
	di->palettes = NULL;
	int p;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Checking supported palettes.\n");

	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtd.index = 0;

	while(v4lconvert_enum_fmt(vdev->info->convert, &fmtd)>=0){
		dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG1,
				"QRY: looking for palette %#x\n", fmtd.pixelformat);

		if ((p=find_v4l2_palette(fmtd.pixelformat)) != -1) {
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
					"QRY: %s supported (%d)\n", libvideo_palettes[p].name, p);
			if(add_supported_palette(vdev, di, p)!=0){
				if(di->palettes)
					XFREE(di->palettes);

				return LIBVIDEO_ERR_IOCTL;
			}
		} else {
			info("libvideo has skipped an unsupported image format:\n");
			info("%s (%#x)\n", fmtd.description, fmtd.pixelformat);
			info("Please let the author know about this error.\n");
			info("See the ISSUES section in the libvideo README file.\n");
		}
		fmtd.index++;
	}
	
	return vdev->info->nb_palettes;
}
static int query_tuner(struct video_input_info *vi, int fd, int index){
	struct v4l2_tuner t;
	CLEAR(t);
	t.index = index;

	if (ioctl (fd, VIDIOC_G_TUNER, &t) != 0)
		return -1;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Tuner: %s - low: %u - high: %u - unit: %s\n",
			t.name, t.rangelow, t.rangehigh,
			t.capability & V4L2_TUNER_CAP_LOW ? "kHz": "MHz");

	XMALLOC(vi->tuner, struct tuner_info *, sizeof(struct tuner_info));
	strncpy(vi->tuner->name, (char *) t.name, NAME_FIELD_LENGTH);
	vi->tuner->index = index;
	vi->tuner->unit = t.capability & VIDEO_TUNER_LOW ? KHZ_UNIT : MHZ_UNIT;
	vi->tuner->rssi = t.signal;
	vi->tuner->type =  t.type == V4L2_TUNER_RADIO ? RADIO_TYPE : TV_TYPE;
	vi->tuner->rangehigh = t.rangehigh;
	vi->tuner->rangelow = t.rangelow;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				"QRY: Tuner: %s - low: %lu - high: %lu - unit: %d\n",
				vi->tuner->name, vi->tuner->rangelow, vi->tuner->rangehigh,
				vi->tuner->unit);

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
		if (vi[i].nb_stds) XFREE(vi[i].supported_stds);
	}
	XFREE(vi);
}

static void add_supported_std(struct video_input_info *vi, int std){
	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Adding standard %d\n", std);
	vi->nb_stds++;
	XREALLOC(vi->supported_stds, int *, vi->nb_stds * sizeof(int));
	vi->supported_stds[(vi->nb_stds - 1)] = std;
}

int check_inputs_v4l2(struct video_device *vdev){
	struct v4l2_input vi;
	int i, ret = 0;
	struct device_info *di = vdev->info;
	CLEAR(vi);
	di->inputs = NULL;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG, "QRY: querying inputs\n");

	//Check how many inputs there are
	while (-1 != ioctl(vdev->fd, VIDIOC_ENUMINPUT, &vi))
		vi.index++;

	di->nb_inputs = vi.index;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: found %d inputs\n", di->nb_inputs );

	XMALLOC(di->inputs, struct video_input_info *,
			di->nb_inputs * sizeof(struct video_input_info ));

	for (i=0; i<di->nb_inputs; i++) {
		CLEAR(vi);
		CLEAR(di->inputs[i]);
		vi.index = i;
		if (-1 == ioctl(vdev->fd, VIDIOC_ENUMINPUT, &vi)) {
			info("Failed to get details of input %d on device %s\n",
					i, vdev->file);
			ret = LIBVIDEO_ERR_IOCTL;
			free_video_inputs(di->inputs,i);
			goto end;
		}

		dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
				"QRY: input %d - %s - %s - tuner: %d\n", i, vi.name,
				(vi.type == V4L2_INPUT_TYPE_TUNER)?"Tuner" : "Camera",vi.tuner);

		strncpy(di->inputs[i].name, (char *) vi.name, NAME_FIELD_LENGTH);
		di->inputs[i].index = i;
		di->inputs[i].type =
			vi.type==V4L2_INPUT_TYPE_TUNER ? INPUT_TYPE_TUNER:INPUT_TYPE_CAMERA;

		if (vi.type & V4L2_INPUT_TYPE_TUNER) {
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
					"QRY: Querying tuner\n");
			if (-1 == query_tuner(&di->inputs[i], vdev->fd, vi.tuner)) {
				info("Failed to get details of tuner on input %d of device %s\n"
						, i, vdev->file);
				ret = LIBVIDEO_ERR_IOCTL;
				free_video_inputs(di->inputs,i);
				goto end;
			}
		} else {
			dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG, "QRY: No tuner\n");
			di->inputs[i].tuner = NULL;
		}

		if(vi.std & V4L2_STD_PAL) add_supported_std(&di->inputs[i], PAL);
		if(vi.std & V4L2_STD_NTSC) add_supported_std(&di->inputs[i], NTSC);
		if(vi.std & V4L2_STD_SECAM) add_supported_std(&di->inputs[i], SECAM);
		if(vi.std == V4L2_STD_UNKNOWN) add_supported_std(&di->inputs[i],WEBCAM);

	}
	end:
	return ret;
}

//this function enumerates the frame intervals for a given libvideo video format
//, width and height and modifies the pointer at p to point to either
//NULL, a struct frame_intv_discrete, struct frame_intv_continuous. It returns
//FRAME_INTV_UNSUPPORTED (p points  to NULL),
//FRAME_INTV_DISCRETE (p points to a an array of struct frame_intv_discrete, the
//last struct has its members set to 0) or
//FRAME_INTV_CONTINUOUS (p points to a struct frame_intv_continuous)
//The returned pointer must be freed by the caller (using free()).
static int list_frame_intv(struct device_info *dinfo, int fmt, int width,
		int height, void **p){
	return lookup_frame_intv(dinfo->convert,
				libvideo_palettes[fmt].v4l2_palette, width, height, p);
}

int query_device_v4l2(struct video_device *vdev){
	int ret = 0;
	struct v4l2_capability caps;

	dprint(LIBVIDEO_SOURCE_QRY, LIBVIDEO_LOG_DEBUG,
			"QRY: Querying V4L2 device.\n");

	if (check_v4l2(vdev->fd, &caps)==-1) {
		info("Error checking capabilities of V4L2 video device %s", vdev->file);
		ret = LIBVIDEO_ERR_NOCAPS;
		goto end;
	}
	//fill name field
	strncpy(vdev->info->name, (char *) caps.card, NAME_FIELD_LENGTH);

	//fill input field
	if(check_inputs_v4l2(vdev)==-1){
		info("Error checking available inputs on V4L2 video device %s",
				vdev->file);
		ret = LIBVIDEO_ERR_NOCAPS;
		goto end;
	}

	//fill palettes field
	if(check_palettes_v4l2(vdev)==LIBVIDEO_ERR_IOCTL){
		free_video_inputs(vdev->info->inputs, vdev->info->nb_inputs);
		info("Error checking supported palettes on V4L2 video device %s\n",
				vdev->file);
		ret = LIBVIDEO_ERR_NOCAPS;
		goto end;
	}

	vdev->info->list_frame_intv = list_frame_intv;

	end:
	return ret;
}

void free_video_device_v4l2(struct video_device *vd){
	int i;

	//for each palette
	for(i=0; i<vd->info->nb_palettes;i++){

		//check & free the frame size member
		if(vd->info->palettes[i].size_type==FRAME_SIZE_CONTINUOUS){

			//check & free the frame interval member
			//min res
			if(vd->info->palettes[i].continuous->interval_type_min_res==FRAME_INTV_DISCRETE)
				XFREE(vd->info->palettes[i].continuous->intv_min_res.discrete);
			else if(vd->info->palettes[i].continuous->interval_type_min_res==FRAME_INTV_CONTINUOUS)
				XFREE(vd->info->palettes[i].continuous->intv_min_res.continuous);

			//max res
			if(vd->info->palettes[i].continuous->interval_type_max_res==FRAME_INTV_DISCRETE)
				XFREE(vd->info->palettes[i].continuous->intv_max_res.discrete);
			else if(vd->info->palettes[i].continuous->interval_type_max_res==FRAME_INTV_CONTINUOUS)
				XFREE(vd->info->palettes[i].continuous->intv_max_res.continuous);

			//free image size
			XFREE(vd->info->palettes[i].continuous);

		} else if(vd->info->palettes[i].size_type==FRAME_SIZE_DISCRETE) {
			int j = -1;
			//loop over all frame sizes
			while(vd->info->palettes[i].discrete[++j].height!=0) {
				//free frame interval
				if(vd->info->palettes[i].discrete[j].interval_type==FRAME_INTV_DISCRETE)
					XFREE(vd->info->palettes[i].discrete[j].intv.discrete);
				else if(vd->info->palettes[i].discrete[j].interval_type==FRAME_INTV_CONTINUOUS)
					XFREE(vd->info->palettes[i].discrete[j].intv.continuous);
			}

			//free image size
			XFREE(vd->info->palettes[i].discrete);
		}

		//free the raw palettes array
		if(vd->info->palettes[i].raw_palettes)
			XFREE(vd->info->palettes[i].raw_palettes);
	}

	//free the palettes
	XFREE(vd->info->palettes);

	free_video_inputs(vd->info->inputs, vd->info->nb_inputs);

	v4lconvert_destroy(vd->info->convert);
}
