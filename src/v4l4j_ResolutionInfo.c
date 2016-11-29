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
#include "jniutils.h"

/*
 * this function goes through the list of struct palette_info and returns a
 * pointer to the one with the given index, or NULL if not found
 */
static struct palette_info *find_palette_info(int index, struct v4l4j_device *device) {
	for(unsigned int i = 0; i < device->vdev->info->nb_palettes; i++)
		if(device->vdev->info->palettes[i].index == index)
			return &device->vdev->info->palettes[i];
	return NULL;
}

/*
 * this function returns the type of a frame size information
 * for a given image format (index) and pointer to v4l4j_device struct (o)
 * returns: int from enum frame_size_types
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_ResolutionInfo_doGetType(JNIEnv *env, jclass me, jint index, jlong o) {
	LOG_FN_ENTER();
	struct v4l4j_device *device = (struct v4l4j_device *) (uintptr_t) o;

	struct palette_info *i = find_palette_info(index, device);
	if(i == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up image format (could not look up palette with index %d)", index);
		return 0;
	}
	dprint(LOG_V4L4J, "[V4L4J] Returning resolution of type %d (%s)\n",
			i->size_type, (i->size_type == 0) ? "unsupported": ((i->size_type == 1) ? "discrete" : "continuous"));
	return i->size_type;
}

/*
 * this function sets the min, max, step width & height attributes
 */
JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_ResolutionInfo_doGetStepwise(JNIEnv *env, jclass me, jint index, jlong o) {
	LOG_FN_ENTER();
	struct v4l4j_device *device = (struct v4l4j_device *) (uintptr_t) o;
	
	/* Get handles on Java stuff */
	struct palette_info* palette = find_palette_info(index, device);
	if(palette == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up image format (could not look up palette with index %d)", index);
		return NULL;
	}

	jclass frame_intv_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/FrameInterval");
	if(frame_intv_class == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up class au.edu.jcu.v4l4j.FrameInterval");
		return NULL;
	}

	jmethodID frameIntervalConstructor = (*env)->GetMethodID(env, frame_intv_class, "<init>", "(IJ)V");
	if(frameIntervalConstructor == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up the constructor au.edu.jcu.v4l4j.FrameInterval(int ptr_type, long o)");
		return NULL;
	}

	dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval for the min res\n");
	//create the frame interval object for the min res
	jobject intv_min = (*env)->NewObject(env, frame_intv_class, frameIntervalConstructor, 1, (jlong) (uintptr_t) palette->continuous);
	if(intv_min == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error creating FrameInterval object");
		return NULL;
	}
	
	dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval for the max res\n");
	//create the frame interval object for the max res
	jobject intv_max = (*env)->NewObject(env, frame_intv_class, frameIntervalConstructor, 2, (jlong) (uintptr_t) palette->continuous);
	(*env)->DeleteLocalRef(env, frame_intv_class);
	if(intv_max == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error creating FrameInterval object");
		return NULL;
	}
	
	jclass stepwise_res_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/ResolutionInfo$StepwiseResolution");
	if(stepwise_res_class == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up class StepwiseResolution");
		return NULL;
	}
	
	jmethodID stepwiseResolutionConstructor = (*env)->GetMethodID(env, stepwise_res_class, "<init>", "(IIIIIILau/edu/jcu/v4l4j/FrameInterval;Lau/edu/jcu/v4l4j/FrameInterval;)V");
	if(stepwiseResolutionConstructor == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up the constructor of class StepwiseResolution");
		return NULL;
	}

	dprint(LOG_V4L4J, "[V4L4L] Creating the stepwise resolution object\n");
	//create StepwiseResolution object
	jobject stepwise = (*env)->NewObject(env, stepwise_res_class, stepwiseResolutionConstructor,
			palette->continuous->min_width, palette->continuous->min_height,
			palette->continuous->max_width, palette->continuous->max_height,
			palette->continuous->step_width, palette->continuous->step_height,
			intv_min, intv_max);
	
	if(stepwise == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error creating StepwiseResolution object");
		return NULL;
	}
	
	dprint(LOG_V4L4J, "[V4L4J] Created stepwise resolution: %d/%d/%d x %d/%d/%d\n",
			palette->continuous->min_width,
			palette->continuous->max_width,
			palette->continuous->step_width,
			palette->continuous->min_height,
			palette->continuous->max_height,
			palette->continuous->step_height
	);
	
	return stepwise;
}

/*
 * this function sets the discreteValues attribute with a list of supported
 * resolutions
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_ResolutionInfo_doGetDiscrete(JNIEnv *env, jclass me, jobject discreteValues, jint index, jlong object){
	LOG_FN_ENTER();
	struct v4l4j_device *device = (struct v4l4j_device *) (uintptr_t) object;

	/* Get handles on Java stuff */
	struct palette_info* palette = find_palette_info(index, device);
	if(palette == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up image format (could not look up palette with index %d)", index);
		return;
	}

	jmethodID add_method = lookupAddMethod(env, discreteValues);
	if(add_method == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up the add method of discreteValues List class");
		return;
	}

	jclass frame_intv_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/FrameInterval");
	if(frame_intv_class == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up class au.edu.jcu.v4l4j.FrameInterval");
		return;
	}

	jmethodID frame_intv_ctor = (*env)->GetMethodID(env, frame_intv_class, "<init>", "(IJ)V");
	if(frame_intv_ctor == NULL){
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up the constructor of class FrameInterval");
		return;
	}

	jclass discrete_res_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/ResolutionInfo$DiscreteResolution");
	if(discrete_res_class == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up class au.edu.jcu.v4l4j.ResolutionInfo.DiscreteResolution");
		return;
	}

	jmethodID discrete_res_ctor = (*env)->GetMethodID(env, discrete_res_class, "<init>", "(IILau/edu/jcu/v4l4j/FrameInterval;)V");
	if(discrete_res_ctor == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up the constructor of class DiscreteResolution");
		return;
	}

	unsigned int i = 0;
	struct frame_size_discrete* resolution;
	while((resolution = &palette->discrete[i++])->width != 0) {
		dprint(LOG_V4L4J, "[V4L4J] creating discrete resolution %dx%d\n", resolution->width, resolution->height);

		//create the frame interval object
		dprint(LOG_V4L4J, "[V4L4J] creating frame interval object first with ptr %p and type: 0\n", resolution);
		jobject intv = (*env)->NewObject(env, frame_intv_class, frame_intv_ctor, 0, (jlong) (uintptr_t) resolution);
		if(intv == NULL) {
			THROW_EXCEPTION(env, JNI_EXCP, "Error creating FrameInterval object");
			return;
		}

		//create DiscreteResolution object
		jobject discrete = (*env)->NewObject(env, discrete_res_class, discrete_res_ctor, resolution->width, resolution->height, intv);
		(*env)->DeleteLocalRef(env, intv);
		if(discrete == NULL) {
			THROW_EXCEPTION(env, JNI_EXCP, "Error creating DiscreteResolution object");
			return;
		}

		dprint(LOG_V4L4J, "[V4L4J] Done creating discrete resolution %dx%d\n", resolution->width, resolution->height);

		//add to list
		(*env)->CallVoidMethod(env, discreteValues, add_method, discrete);
		(*env)->DeleteLocalRef(env, discrete);
	}
}