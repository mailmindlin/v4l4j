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



#include "libvideo.h"
#include "libvideo-err.h"
#include "log.h"
#include "libvideo-palettes.h"
#include "version.h"


/*
 * Copies the version info in the given char *
 * It must be allocated by caller. char [40] is enough
 */
char *get_libvideo_version(char * c) {
	snprintf(c, 39,"%d.%d-%s", VER_MAJ, VER_MIN, VER_REV);
	return c;
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
	printf("Device name: %s\n",v->id.name);
	printf("Device file: %s\n",v->id.device_handle);
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

