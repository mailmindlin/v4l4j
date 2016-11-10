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
#include "libvideo-palettes.h"
#include "common.h"
#include "debug.h"

static jobject create_tuner_object(JNIEnv *e, struct tuner_info *tuner) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	jclass tuner_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/TunerInfo");
	if(!tuner_class) {
		info("[V4L4J] Error looking up the tuner class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up tuner class");
		return 0;
	}

	jmethodID ctor = (*e)->GetMethodID(e, tuner_class, "<init>", "(Ljava/lang/String;IIIJJ)V");
	if(!ctor) {
		info("[V4L4J] Error looking up the constructor of tuner class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up constructor of tuner class");
		return 0;
	}

	dprint(LOG_V4L4J, "[V4L4J] Creating tunerInfo object: index: %d - name '%s' - low: %lu - high: %lu - unit: %d - type: %d\n",
			tuner->index, tuner->name, tuner->rangelow, tuner->rangehigh, tuner->unit, tuner->type);

	jstring name = (*e)->NewStringUTF(e, (const char*) tuner->name);
	jobject result = (*e)->NewObject(e, tuner_class, ctor,
			name, tuner->index, tuner->unit, tuner->type, (jlong) (tuner->rangelow & 0xFFFFFFFF), (jlong) (tuner->rangehigh & 0xFFFFFFFF));
	(*e)->DeleteLocalRef(e, tuner_class);
	(*e)->DeleteLocalRef(e, name);
	return result;
}

static void create_inputs_object(JNIEnv *e, jobject t, jclass this_class, struct video_device *vd){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	jclass input_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/InputInfo");
	if(input_class == NULL){
		info( "[V4L4J] Error looking up the InputInfo class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up InputInfo class");
		return;
	}

	jfieldID inputs_field = (*e)->GetFieldID(e, this_class, "inputs", "Ljava/util/List;");
	if(inputs_field == NULL){
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the inputs attribute ID");
		return;
	}

	jobject input_list_object = (*e)->GetObjectField(e, t, inputs_field);
	if(input_list_object == NULL){
		THROW_EXCEPTION(e, JNI_EXCP, "Error retrieving up the inputs attribute");
		return;
	}

	jmethodID add_method = lookupAddMethod(e, input_list_object);
	if(add_method == NULL)
		return;

	jmethodID ctor_wotuner = (*e)->GetMethodID(e, input_class, "<init>", "(Ljava/lang/String;[II)V");
	if(ctor_wotuner == NULL){
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of Input  class");
		return;
	}

	jmethodID ctor_wtuner = (*e)->GetMethodID(e, input_class, "<init>", "(Ljava/lang/String;[ILau/edu/jcu/v4l4j/TunerInfo;I)V");
	if(ctor_wtuner == NULL){
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of InputInfo class");
		return;
	}

	for(int i = 0; i< vd->info->nb_inputs; i++){
		struct video_input_info* vi = &vd->info->inputs[i];
		//build the input object

		//create the short[] with the supported standards
		jintArray stds = (*e)->NewIntArray(e, vi->nb_stds);
		if(!stds) {
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating array");
			return;
		}
		dprint(LOG_V4L4J, "[V4L4J] Setting new stds array with array[%d] @ %p\n",vi->nb_stds, vi->supported_stds);
		(*e)->SetIntArrayRegion(e, stds, 0, vi->nb_stds,  vi->supported_stds);

		//create the input object
		jstring name = (*e)->NewStringUTF(e, (const char*) vi->name);
		jobject obj;
		if(vd->info->inputs[i].tuner == NULL) {
			dprint(LOG_V4L4J, "[V4L4J] Creating input object (w/o tuner): name '%s' - supported standards (%d): %p - index: %d\n", vi->name, vi->nb_stds, vi->supported_stds, vi->index);
			obj = (*e)->NewObject(e, input_class, ctor_wotuner, name, stds, vi->index);
		} else {
			dprint(LOG_V4L4J, "[V4L4J] Creating input object (with tuner): name '%s' - supported standards(%d): %p - index: %d\n", vi->name, vi->nb_stds, vi->supported_stds, vi->index);
			jobject tuner = create_tuner_object(e, vi->tuner);
			obj = (*e)->NewObject(e, input_class, ctor_wtuner, name, stds, tuner, vi->index);
			(*e)->DeleteLocalRef(e, tuner);
		}
		(*e)->DeleteLocalRef(e, stds);

		//store it in the list
		if(obj == NULL) {
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating input object");
			return;
		}
		(*e)->CallVoidMethod(e, input_list_object, add_method, obj);
		(*e)->DeleteLocalRef(e, obj);
	}
	(*e)->DeleteLocalRef(e, input_class);
	(*e)->DeleteLocalRef(e, input_list_object);
}

static void create_formats_object(JNIEnv *e, jobject t, jclass this_class, struct v4l4j_device *d) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	jclass format_list_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/ImageFormatList");
	if(format_list_class == NULL){
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up ImageFormatList class");
		return;
	}
	
	jmethodID format_list_ctor = (*e)->GetMethodID(e, format_list_class, "<init>", "(J)V");
	if(format_list_ctor == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of ImageFormatList class");
		return;
	}

	jfieldID formats_field = (*e)->GetFieldID(e, this_class, "formats", "Lau/edu/jcu/v4l4j/ImageFormatList;");
	if(formats_field == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the formats attribute ID");
		return;
	}
	
	//Creates an ImageFormatList
	jobject obj = (*e)->NewObject(e, format_list_class, format_list_ctor, (jlong) (uintptr_t)d);
	if(obj == NULL) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the format list");
		return;
	}
	(*e)->SetObjectField(e, t, formats_field, obj);
}

/*
 * Get info about a v4l device given its device file
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_DeviceInfo_getInfo(JNIEnv *e, jobject t, jlong v4l4j_device){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) v4l4j_device;
	struct video_device *vd = d->vdev;
	
	// Get handles on needed Java objects
	jclass this_class = (*e)->GetObjectClass(e,t);
	if(this_class == NULL){
		info("[V4L4J] Error looking up the DeviceInfo class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the DeviceInfo class");
		return;
	}

	jfieldID name_field = (*e)->GetFieldID(e, this_class, "name", "Ljava/lang/String;");
	if(name_field == NULL){
		info("[V4L4J] Error looking up the name attribute\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the name attribute");
		return;
	}


	dprint(LOG_LIBVIDEO, "[LIBVIDEO] call to get_device_info\n");
	//get data from libvideo
	if(get_device_info(vd) != NULL){
		//fill in values in DeviceInfo object
		/* set the name field */
		jstring name = (*e)->NewStringUTF(e, vd->info->name);
		(*e)->SetObjectField(e, t, name_field, name);
		//We don't *have* to release this, but it makes me feel better
		(*e)->DeleteLocalRef(e, name);

		/* set the inputs field */
		dprint(LOG_V4L4J, "[V4L4J] Creating inputInfo objects\n");
		create_inputs_object(e, t, this_class, vd);

		/* set the formats field */
		dprint(LOG_V4L4J, "[V4L4J] Creating Format objects\n");
		create_formats_object(e, t, this_class, d);
	} else
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error getting information from video device");

}

JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_DeviceInfo_doListIntervals(JNIEnv *e, jobject t, jlong o, jint imf, jint w, jint h) {
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	jclass frame_intv_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/FrameInterval");
	if(frame_intv_class == NULL){
		info("[V4L4J] Error looking up the FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up FrameInterval class");
		return NULL;
	}

	jmethodID ctor = (*e)->GetMethodID(e, frame_intv_class, "<init>",	"(IJ)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the ctor of FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of FrameInterval class");
		return NULL;
	}

	void* p;
	int type = d->vdev->info->list_frame_intv(d->vdev->info, imf, w, h, &p);

	jobject frame_intv;
	switch(type){
		case FRAME_INTV_UNSUPPORTED:
			dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval (unsupported)\n");
			//create the frame interval object
			frame_intv = (*e)->NewObject(e, frame_intv_class, ctor, 3, (jlong) (uintptr_t) p);
			if(frame_intv == NULL) {
				THROW_EXCEPTION(e, JNI_EXCP, "Error creating FrameInterval object");
				return NULL;
			}
			break;
		case FRAME_INTV_DISCRETE:
			dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval (discrete)\n");
			//create the frame interval object
			frame_intv = (*e)->NewObject(e, frame_intv_class, ctor, 4, (jlong) (uintptr_t) p);
			XFREE(p);
			if(frame_intv == NULL) {
				THROW_EXCEPTION(e, JNI_EXCP, "Error creating FrameInterval object");
				return NULL;
			}
			break;
		case FRAME_INTV_CONTINUOUS:
			dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval (stepwise)\n");
			//create the frame interval object
			frame_intv = (*e)->NewObject(e, frame_intv_class, ctor, 5, (jlong) (uintptr_t) p);
			XFREE(p);
			if(frame_intv == NULL) {
				THROW_EXCEPTION(e, JNI_EXCP, "Error creating FrameInterval object");
				return NULL;
			}
			break;
		default:
			info("[V4L4J] There is a bug in v4l4j. Please report this on the\n");
			info("[V4L4J] V4L4J mailing list.\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating the FrameInterval object");
			return NULL;
	}
	return frame_intv;
}

/*
 * Get info about a v4l device given its device file
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_DeviceInfo_doRelease(JNIEnv *e, jobject t, jlong v4l4j_device) {
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) v4l4j_device;
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] call to release_device_info\n");
	release_device_info(d->vdev);
}


