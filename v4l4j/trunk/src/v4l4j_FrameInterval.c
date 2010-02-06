/*
* Copyright (C) 2007-2009 Gilles Gigan (gilles.gigan@gmail.com)
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
 * returns the type of frame interval for a given frame size struct
 * (0: struct frame_size_discrete, 1: struct frame_size_continuous (min res),
 * 2: struct frame_size_continuous(max res) )
 * 0: unsupported, 1:discrete, 2: stepwise
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_FrameInterval_doGetFrameIntvType(
		JNIEnv *e, jobject obj, jint type, jlong o) {
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
		info("[V4L4J] Error looking up the frame interval "
				"(wrong struct type %d)\n", type);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up frame intervals");
		break;
	}

	dprint(LOG_V4L4J, "[V4L4J] Returning frame interval of type %d (%s)\n",t,
			(t==0)?"unsupported":((t==1)?"discrete":"stepwise"));
	return t;
}

/*
 * this function takes a pointer (o) to either a frame_size_discrete
 * or frame_size_continuous and a type (0: struct frame_size_discrete,
 * 1: struct frame_size_continuous (min res), 2: struct
 * frame_size_continuous(max res) and returns the struct frame_intv_discrete
 * associated with it, or NULL if not found
 */
static struct frame_intv_discrete *get_discrete(jlong o, jint type){
	struct frame_intv_discrete *d = NULL;

	switch(type){
	//frame_size_discrete
	case 0:
		d =	((struct frame_size_discrete*) (uintptr_t) o)->intv.discrete;
		break;

	//frame_size_stepwise (min res)
	case 1:
		d =	((struct frame_size_continuous*) (uintptr_t) o)->intv_min_res.discrete;
		break;

	//frame_size_stepwise (max res)
	case 2:
		d = ((struct frame_size_continuous*) (uintptr_t) o)->intv_max_res.discrete;
		break;

	//frame_intv_discrete
	case 4:
		d = (struct frame_intv_discrete*) (uintptr_t) o;
		break;
	//error
	default:
		info("[V4L4J] Error looking up the frame interval "
				"(wrong struct type %d)\n", type);
		break;
	}

	return d;
}

/*
 * this function takes a pointer (o) to either a frame_size_discrete
 * or frame_size_continuous and a type (0: struct frame_size_discrete,
 * 1: struct frame_size_continuous (min res), 2: struct
 * frame_size_continuous(max res) and returns the struct frame_intv_continuous
 * associated with it, or NULL if not found
 */
static struct frame_intv_continuous *get_continuous(jlong o, jint type){
	struct frame_intv_continuous *d = NULL;

	switch(type){
	//frame_size_discrete
	case 0:
		d =	((struct frame_size_discrete*) (uintptr_t) o)->intv.continuous;
		break;

	//frame_size_stepwise (min res)
	case 1:
		d =	((struct frame_size_continuous*) (uintptr_t) o)->intv_min_res.continuous;
		break;

	//frame_size_stepwise (max res)
	case 2:
		d = ((struct frame_size_continuous*) (uintptr_t) o)->intv_max_res.continuous;
		break;

	//frame_intv_continuous
	case 5:
		d = (struct frame_intv_continuous*) (uintptr_t) o;
		break;

	//error
	default:
		info("[V4L4J] Error looking up the frame interval "
				"(wrong struct type %d)\n", type);
		break;
	}

	return d;
}

/*
 * populate the discreteIntervals member with discrete frame sizes
 * for a given frame size struct
 * (0: struct frame_size_discrete, 1: struct frame_size_continuous (min res),
 * 2: struct frame_size_continuous(max res),  )
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameInterval_doGetDiscrete(
		JNIEnv *e, jobject t, jint type, jlong o) {

	int i = -1;
	jclass this_class, discrete_intv_class, vector_class;
	jfieldID field;
	jmethodID add_method, ctor;
	jobject disc_attr, discrete;
	struct frame_intv_discrete *d;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//get our pointer to struct frame_intv_discrete
	if((d=get_discrete(o, type)) == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up frame intervals");
		return;
	}


	/* Get handles on Java stuff */
	this_class = (*e)->GetObjectClass(e, t);
	if(this_class == NULL){
		info("[V4L4J] Error looking up the FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up FrameInterval class");
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


	discrete_intv_class = (*e)->FindClass(e,
			"au/edu/jcu/v4l4j/FrameInterval$DiscreteInterval");
	if(discrete_intv_class == NULL){
		info("[V4L4J] Error looking up the DiscreteInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up DiscreteInterval class");
		return;
	}

	ctor = (*e)->GetMethodID(e, discrete_intv_class, "<init>",
			"(II)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the ctor of DiscreteInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of DiscreteInterval class");
		return;
	}

	while(d[++i].numerator!=0){
		//create DiscreteInterval object
		discrete = (*e)->NewObject(e, discrete_intv_class, ctor,
				d[i].numerator,d[i].denominator);
		if(discrete == NULL){
			info("[V4L4J] Error creating DiscreteInterval object\n");
			THROW_EXCEPTION(e, JNI_EXCP, \
					"Error creating DiscreteInterval object");
			return;
		}

		dprint(LOG_V4L4J, "[V4L4J] Adding discrete interval %d/%d\n",
				d[i].numerator,d[i].denominator);
		//add to vector
		(*e)->CallVoidMethod(e, disc_attr, add_method, discrete);
	}
}

/*
 * populate the stepwiseInterval member with stepwise frame size
 * for a given frame size struct
 * (0: struct frame_size_discrete, 1: struct frame_size_continuous (min res),
 * 2: struct frame_size_continuous(max res) )
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_FrameInterval_doGetStepwise(
		JNIEnv *e, jobject t, jint type, jlong o) {
	struct frame_intv_continuous *d;
	jclass this_class, stepwise_intv_class;
	jmethodID ctor;
	jfieldID fid;
	jobject stepwise;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	//get our pointer to struct frame_intv_continuous
	if((d=get_continuous(o, type)) == NULL){
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up frame intervals");
		return;
	}
	/* Get handles on Java stuff */
	this_class = (*e)->GetObjectClass(e, t);
	if(this_class == NULL){
		info("[V4L4J] Error looking up the FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up FrameInterval class");
		return;
	}

	stepwise_intv_class = (*e)->FindClass(e,
			"au/edu/jcu/v4l4j/FrameInterval$StepwiseInterval");
	if(stepwise_intv_class == NULL){
		info("[V4L4J] Error looking up the StepwiseInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up StepwiseInterval class");
		return;
	}

	ctor = (*e)->GetMethodID(e, stepwise_intv_class, "<init>", "(IIIIII)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the ctor of StepwiseInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of StepwiseInterval class");
		return;
	}

	//create StepwiseInterval object
	stepwise = (*e)->NewObject(e, stepwise_intv_class, ctor,
			d->min.numerator, d->min.denominator,
			d->max.numerator, d->max.denominator,
			d->step.numerator, d->step.denominator);
	if(stepwise == NULL){
		info("[V4L4J] Error creating StepwiseInterval object\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error creating StepwiseInterval object");
		return;
	}

	//assign it to the stepwiseInterval member
	fid = (*e)->GetFieldID(e, this_class, "stepwiseInterval",
			"Lau/edu/jcu/v4l4j/FrameInterval$StepwiseInterval");
	if(fid == NULL){
		info("[V4L4J] Error looking up the StepwiseInterval fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the StepwiseInterval fieldID");
		return;
	}
	(*e)->SetObjectField(e, t, fid, stepwise);

	dprint(LOG_V4L4J, "[V4L4J] Stepwise frame interval: min:%d/%d max:%d/%d step:%d/%d\n",
			d->min.numerator, d->min.denominator,
			d->max.numerator, d->max.denominator,
			d->step.numerator, d->step.denominator);

}
