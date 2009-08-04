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

#include <jni.h>
#include <stdint.h>

#include "libvideo.h"
#include "common.h"
#include "debug.h"

/*
 * this function goes through the list of struct palette_info and returns a
 * pointer to the one with the given index, or NULL if not found
 */
static struct palette_info *find_palette_info(int index, struct v4l4j_device *d){
	int i;
	for(i=0;i<d->vdev->info->nb_palettes;i++)
		if(d->vdev->info->palettes[i].index==index)
			return &d->vdev->info->palettes[i];

	return NULL;
}

/*
 * this function returns the type of a frame size information
 * for a given image format (index) and pointer to v4l4j_device struct (o)
 * returns: int from enum frame_size_types
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_ResolutionInfo_doGetType(
		JNIEnv *e, jobject t, jint index, jlong o){
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	struct palette_info *i = find_palette_info(index, d);
	if(i==NULL){
		info("[V4L4J] Error looking up the palette with index %d\n", index);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up image format");
		return 0;
	}
	dprint(LOG_V4L4J, "[V4L4J] Returning resolution of type %d (%s)\n",
			i->size_type,
			(i->size_type==0)?"unsupported":
				((i->size_type==1)?"discrete":"continuous"));
	return i->size_type;
}

/*
 * this function sets the min,max,step width & height attributes
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_ResolutionInfo_doGetStepwise(
		JNIEnv *e, jobject t, jint index, jlong o){
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	struct palette_info *p;
	jclass this_class, stepwise_res_class, frame_intv_class;
	jmethodID ctor;
	jfieldID fid;
	jobject stepwise, intv_min, intv_max;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	/* Get handles on Java stuff */
	p = find_palette_info(index,d);
	if(p==NULL){
		info("[V4L4J] Error looking up the palette with index %d\n", index);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up image format");
		return;
	}

	this_class = (*e)->GetObjectClass(e, t);
	if(this_class == NULL){
		info("[V4L4J] Error looking up the ResolutionInfo class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up ResolutionInfo class");
		return;
	}

	frame_intv_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/FrameInterval");
	if(frame_intv_class == NULL){
		info("[V4L4J] Error looking up the FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up FrameInterval class");
		return;
	}

	ctor = (*e)->GetMethodID(e, frame_intv_class, "<init>",	"(IJ)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the ctor of FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of FrameInterval class");
		return;
	}

	dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval for the min res\n");
	//create the frame interval object for the min res
	intv_min = (*e)->NewObject(e, frame_intv_class, ctor,
			1, (uintptr_t) p->continuous);
	if(intv_min == NULL){
		info("[V4L4J] Error creating FrameInterval object\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error creating FrameInterval object");
		return;
	}

	dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval for the max res\n");
	//create the frame interval object for the max res
	intv_max = (*e)->NewObject(e, frame_intv_class, ctor,
			2, (uintptr_t) p->continuous);
	if(intv_max == NULL){
		info("[V4L4J] Error creating FrameInterval object\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error creating FrameInterval object");
		return;
	}

	stepwise_res_class = (*e)->FindClass(e,
			"au/edu/jcu/v4l4j/ResolutionInfo$StepwiseResolution");
	if(stepwise_res_class == NULL){
		info("[V4L4J] Error looking up the StepwiseResolution class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up StepwiseResolution class");
		return;
	}

	ctor = (*e)->GetMethodID(e, stepwise_res_class, "<init>",
		"(IIIIIILau/edu/jcu/v4l4j/FrameInterval;Lau/edu/jcu/v4l4j/FrameInterval;)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the ctor of StepwiseResolution class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of StepwiseResolution class");
		return;
	}

	dprint(LOG_V4L4J, "[V4L4L] Creating the stepwise res obj\n");
	//create StepwiseResolution object
	stepwise = (*e)->NewObject(e, stepwise_res_class, ctor,
			p->continuous->min_width, p->continuous->min_height,
			p->continuous->max_width, p->continuous->max_height,
			p->continuous->step_width, p->continuous->step_height,
			intv_min, intv_max);
	if(stepwise == NULL){
		info("[V4L4J] Error creating StepwiseResolution object\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error creating StepwiseResolution object");
		return;
	}

	//assign it to the stepwiseObject member
	fid = (*e)->GetFieldID(e, this_class, "stepwiseObject",
			"Lau/edu/jcu/v4l4j/ResolutionInfo$StepwiseResolution");
	if(fid == NULL){
		info("[V4L4J] Error looking up the StepwiseResolution fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the StepwiseResolution fieldID");
		return;
	}
	(*e)->SetObjectField(e, t, fid, stepwise);

	dprint(LOG_V4L4J, "[V4L4J] Created stepwise resolution: %d/%d/%d x %d/%d/%d\n",
			p->continuous->min_width,
			p->continuous->max_width,
			p->continuous->step_width,
			p->continuous->min_height,
			p->continuous->max_height,
			p->continuous->step_height
	);

}

/*
 * this function sets the discreteValues attribute with a list of supported
 * resolutions
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_ResolutionInfo_doGetDiscrete(
		JNIEnv *e, jobject t, jint index, jlong o){
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	struct palette_info *p;
	int i = -1;
	jclass this_class, discrete_res_class, vector_class, frame_intv_class;
	jfieldID field;
	jmethodID add_method, discrete_res_ctor, frame_intv_ctor;
	jobject disc_attr, discrete, intv;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	/* Get handles on Java stuff */
	p = find_palette_info(index,d);
	if(p==NULL){
		info("[V4L4J] Error looking up the palette with index %d\n", index);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up image format");
		return;
	}

	this_class = (*e)->GetObjectClass(e, t);
	if(this_class == NULL){
		info("[V4L4J] Error looking up the ResolutionInfo class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up ResolutionInfo class");
		return;
	}

	field = (*e)->GetFieldID(e, this_class, "discreteValues",
			"Ljava/util/List;");
	if(field == NULL){
		info("[V4L4J] Error looking up the discreteValues fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the discreteValues fieldID");
		return;
	}

	disc_attr = (*e)->GetObjectField(e, t, field);
	if(disc_attr == NULL){
		info("[V4L4J] Error looking up the discreteValues member\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the discreteValues member");
		return;
	}

	vector_class = (*e)->FindClass(e, "java/util/Vector");
	if(vector_class == NULL){
		info("[V4L4J] Error looking up the Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Vector class");
		return;
	}

	add_method = (*e)->GetMethodID(e, vector_class, "addElement",
			"(Ljava/lang/Object;)V");
	if(add_method == NULL){
		info("[V4L4J] Error looking up the add method of Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the add method of Vector class");
		return;
	}

	frame_intv_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/FrameInterval");
	if(frame_intv_class == NULL){
		info("[V4L4J] Error looking up the FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up FrameInterval class");
		return;
	}

	frame_intv_ctor = (*e)->GetMethodID(e, frame_intv_class, "<init>",	"(IJ)V");
	if(frame_intv_ctor == NULL){
		info("[V4L4J] Error looking up the ctor of FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of FrameInterval class");
		return;
	}

	discrete_res_class = (*e)->FindClass(e,
			"au/edu/jcu/v4l4j/ResolutionInfo$DiscreteResolution");
	if(discrete_res_class == NULL){
		info("[V4L4J] Error looking up the DiscreteResolution class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up DiscreteResolution class");
		return;
	}

	discrete_res_ctor = (*e)->GetMethodID(e, discrete_res_class, "<init>",
			"(IILau/edu/jcu/v4l4j/FrameInterval;)V");
	if(discrete_res_ctor == NULL){
		info("[V4L4J] Error looking up the ctor of DiscreteResolution class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of DiscreteResolution class");
		return;
	}

	while(p->discrete[++i].width!=0){
		dprint(LOG_V4L4J, "[V4L4J] creating discrete resolution %dx%d\n",
							p->discrete[i].width,
							p->discrete[i].height);

		//create the frame interval object
		dprint(LOG_V4L4J, "[V4L4J] creating frame interval object first with "
				"ptr %p and type: 0\n",&p->discrete[i] );
		intv = (*e)->NewObject(e, frame_intv_class, frame_intv_ctor,
				0, (uintptr_t) &p->discrete[i]);
		if(intv == NULL){
			info("[V4L4J] Error creating FrameInterval object\n");
			THROW_EXCEPTION(e, JNI_EXCP, \
					"Error creating FrameInterval object");
			return;
		}

		//create DiscreteResolution object
		discrete = (*e)->NewObject(e, discrete_res_class, discrete_res_ctor,
				p->discrete[i].width,
				p->discrete[i].height,
				intv);
		if(discrete == NULL){
			info("[V4L4J] Error creating DiscreteResolution object\n");
			THROW_EXCEPTION(e, JNI_EXCP, \
					"Error creating DiscreteResolution object");
			return;
		}

		dprint(LOG_V4L4J, "[V4L4J] Done creating discrete resolution %dx%d\n",
								p->discrete[i].width,
								p->discrete[i].height);

		//add to vector
		(*e)->CallVoidMethod(e, disc_attr, add_method, discrete);
	}
}


