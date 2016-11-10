/*
* Copyright (C) 2007-2009 Gilles Gigan (gilles.gigan@gmail.com)
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <jni.h>
#include <stdint.h>

#include "libvideo.h"
#include "common.h"
#include "debug.h"

/**
 * returns the type of frame interval for a given frame size struct
 * (0: struct frame_size_discrete, 1: struct frame_size_continuous (min res),
 * 2: struct frame_size_continuous(max res) )
 * 0: unsupported, 1:discrete, 2: stepwise
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameInterval_doGetFrameIntvType(JNIEnv *e, jclass me, jint type, jlong o) {
	int t = 0;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	switch(type){
	//frame_size_discrete
	case 0:
		t = ((struct frame_size_discrete*) (uintptr_t) o)->interval_type;
		break;

	//frame_size_stepwise (min res)
	case 1:
		t = ((struct frame_size_continuous*) (uintptr_t) o)->interval_type_min_res;
		break;

	//frame_size_stepwise (max res)
	case 2:
		t = ((struct frame_size_continuous*) (uintptr_t) o)->interval_type_max_res;
		break;

	//error
	default:
		info("[V4L4J] Error looking up the frame interval (wrong struct type %d)\n", type);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up frame intervals");
		break;
	}

	dprint(LOG_V4L4J, "[V4L4J] Returning frame interval of type %d (%s)\n", t, (t==0) ? "unsupported" : ((t==1) ? "discrete" : "stepwise"));
	return t;
}

/*
 * this function takes a pointer (o) to either a frame_size_discrete
 * or frame_size_continuous and a type (0: struct frame_size_discrete,
 * 1: struct frame_size_continuous (min res), 2: struct
 * frame_size_continuous(max res) and returns the struct frame_intv_discrete
 * associated with it, or NULL if not found
 */
static struct frame_intv_discrete *get_discrete(jlong o, jint type) {
	switch(type) {
		//frame_size_discrete
		case 0:
			return ((struct frame_size_discrete*) (uintptr_t) o)->intv.discrete;

		//frame_size_stepwise (min res)
		case 1:
			return ((struct frame_size_continuous*) (uintptr_t) o)->intv_min_res.discrete;

		//frame_size_stepwise (max res)
		case 2:
			return ((struct frame_size_continuous*) (uintptr_t) o)->intv_max_res.discrete;

		//frame_intv_discrete
		case 4:
			return (struct frame_intv_discrete*) (uintptr_t) o;

		//error
		default:
			info("[V4L4J] Error looking up the frame interval (unknown struct type %d)\n", type);
			return NULL;
	}
}

/*
 * this function takes a pointer (o) to either a frame_size_discrete
 * or frame_size_continuous and a type (0: struct frame_size_discrete,
 * 1: struct frame_size_continuous (min res), 2: struct
 * frame_size_continuous(max res) and returns the struct frame_intv_continuous
 * associated with it, or NULL if not found
 */
static struct frame_intv_continuous *get_continuous(jlong o, jint type) {
	switch(type) {
		//frame_size_discrete
		case 0:
			return ((struct frame_size_discrete*) (uintptr_t) o)->intv.continuous;
			
		//frame_size_stepwise (min res)
		case 1:
			return ((struct frame_size_continuous*) (uintptr_t) o)->intv_min_res.continuous;
			
		//frame_size_stepwise (max res)
		case 2:
			return ((struct frame_size_continuous*) (uintptr_t) o)->intv_max_res.continuous;

		//frame_intv_continuous
		case 5:
			return (struct frame_intv_continuous*) (uintptr_t) o;
			
		//error
		default:
			info("[V4L4J] Error looking up the frame interval (unknown struct type %d)\n", type);
			return NULL;
	}
}

/*
 * populate the discreteIntervals member with discrete frame sizes
 * for a given frame size struct
 * (0: struct frame_size_discrete, 1: struct frame_size_continuous (min res),
 * 2: struct frame_size_continuous(max res),  )
 */
JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_FrameInterval_doGetDiscrete(JNIEnv *e, jclass me, jobject result, jint type, jlong o) {

	int i = -1;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//get our pointer to struct frame_intv_discrete
	struct frame_intv_discrete *d = get_discrete(o, type);
	if(d == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up frame intervals");
		return NULL;
	}
	
	if (!result) {
		THROW_EXCEPTION(e, NULL_EXCP, "Result cannot be null");
		return NULL;
	}

	jmethodID add_method = lookupAddMethod(e, result);
	if(add_method == NULL)
		return NULL;

	jclass discrete_intv_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/FrameInterval$DiscreteInterval");
	if(discrete_intv_class == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up DiscreteInterval class");
		return NULL;
	}

	jmethodID ctor = (*e)->GetMethodID(e, discrete_intv_class, "<init>", "(II)V");
	if(ctor == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor for class DiscreteInterval");
		return NULL;
	}

	while(d[++i].numerator != 0) {
		//create DiscreteInterval object
		jobject discrete = (*e)->NewObject(e, discrete_intv_class, ctor, d[i].numerator, d[i].denominator);
		if(discrete == NULL){
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating DiscreteInterval object");
			return NULL;
		}

		dprint(LOG_V4L4J, "[V4L4J] Adding discrete interval %d/%d\n", d[i].numerator, d[i].denominator);
		//add to list
		(*e)->CallBooleanMethod(e, result, add_method, discrete);
		(*e)->DeleteLocalRef(e, discrete);
	}
	return result;
}

/*
 * populate the stepwiseInterval member with stepwise frame size
 * for a given frame size struct
 * (0: struct frame_size_discrete, 1: struct frame_size_continuous (min res),
 * 2: struct frame_size_continuous(max res) )
 */
JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_FrameInterval_doGetStepwise(JNIEnv *env, jclass me, jint type, jlong o) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);

	//get our pointer to struct frame_intv_continuous
	struct frame_intv_continuous* d = get_continuous(o, type);
	if(d == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up frame intervals");
		return NULL;
	}

	jclass stepwise_intv_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/FrameInterval$StepwiseInterval");
	if(stepwise_intv_class == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up StepwiseInterval class");
		return NULL;
	}

	jmethodID ctor = (*env)->GetMethodID(env, stepwise_intv_class, "<init>", "(IIIIII)V");
	if(ctor == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error looking up the ctor of StepwiseInterval class");
		return NULL;
	}

	//create StepwiseInterval object
	jobject stepwise = (*env)->NewObject(env, stepwise_intv_class, ctor,
			d->min.numerator, d->min.denominator,
			d->max.numerator, d->max.denominator,
			d->step.numerator, d->step.denominator);
	
	if(stepwise == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error creating StepwiseInterval object");
		return NULL;
	}
	
	dprint(LOG_V4L4J, "[V4L4J] Stepwise frame interval: min:%d/%d max:%d/%d step:%d/%d\n",
			d->min.numerator, d->min.denominator,
			d->max.numerator, d->max.denominator,
			d->step.numerator, d->step.denominator);
	
	return stepwise;
}
