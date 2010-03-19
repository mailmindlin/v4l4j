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

static jobject create_tuner_object(JNIEnv *e, jobject t, struct tuner_info *tuner) {
	jclass tuner_class;
	jmethodID ctor;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	tuner_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/TunerInfo");
	if(tuner_class == NULL){
		info("[V4L4J] Error looking up the tuner class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up tuner class");
		return 0;
	}

	ctor = (*e)->GetMethodID(e, tuner_class, "<init>", "(Ljava/lang/String;IIIJJ)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the constructor of tuner class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up constructor of tuner class");
		return 0;
	}

	dprint(LOG_V4L4J, "[V4L4J] Creating tunerInfo object: index: %d - name '%s' - low: %lu - high: %lu - unit: %d - type: %d\n",
			tuner->index, tuner->name, tuner->rangelow, tuner->rangehigh, tuner->unit, tuner->type);

	return (*e)->NewObject(e, tuner_class, ctor,
			(*e)->NewStringUTF(e,(const char *)tuner->name), tuner->index, tuner->unit , tuner->type,
			(jlong) (tuner->rangelow & 0xffffffff), (jlong) (tuner->rangehigh & 0xffffffff));
}

static void create_inputs_object(JNIEnv *e, jobject t, jclass this_class, struct video_device *vd){
	jclass input_class, vector_class;
	jfieldID inputs_field;
	jmethodID ctor_wotuner, ctor_wtuner, add_method;
	jintArray stds;
	jobject obj, input_list_object;

	int i;
	struct video_input_info *vi;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	input_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/InputInfo");
	if(input_class == NULL){
		info( "[V4L4J] Error looking up the InputInfo class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up InputInfo class");
		return;
	}

	vector_class = (*e)->FindClass(e, "java/util/Vector");
	if(vector_class == NULL){
		info("[V4L4J] Error looking up the Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Vector java objects");
		return;
	}

	add_method = (*e)->GetMethodID(e, vector_class, "addElement", "(Ljava/lang/Object;)V");
	if(add_method == NULL){
		info("[V4L4J] Error looking up the add method of Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the add method of Vector objects");
		return;
	}

	inputs_field = (*e)->GetFieldID(e, this_class, "inputs", "Ljava/util/List;");
	if(inputs_field == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the inputs attribute ID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the inputs attribute ID");
		return;
	}

	input_list_object = (*e)->GetObjectField(e, t, inputs_field);
	if(input_list_object == NULL){
		info("[V4L4J] Error looking up the inputs attribute\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the inputs attribute");
		return;
	}

	ctor_wotuner = (*e)->GetMethodID(e, input_class, "<init>", "(Ljava/lang/String;[II)V");
	if(ctor_wotuner == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the constructor of Input class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of Input  class");
		return;
	}

	ctor_wtuner = (*e)->GetMethodID(e, input_class, "<init>", "(Ljava/lang/String;[ILau/edu/jcu/v4l4j/TunerInfo;I)V");
	if(ctor_wtuner == NULL){
		info("[V4L4J] Error looking up the constructor of InputInfo class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of InputInfo class");
		return;
	}

	for(i = 0; i< vd->info->nb_inputs; i++){
		vi = &vd->info->inputs[i];
		//build the input object

		//create the short[] with the supported standards
		stds= (*e)->NewIntArray(e, vi->nb_stds);
		if(stds == NULL){
			info( "[V4L4J] Error creating array\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating array");
			return;
		}
		dprint(LOG_V4L4J, "[V4L4J] Setting new stds array with array[%d] @ %p\n",vi->nb_stds, vi->supported_stds);
		(*e)->SetIntArrayRegion(e, stds, 0, vi->nb_stds,  vi->supported_stds);

		//create the input object
		if(vd->info->inputs[i].tuner==NULL) {

			dprint(LOG_V4L4J, "[V4L4J] Creating input object (wo tuner): name '%s' - supported standards (%d): %p - index: %d\n",
					vi->name,vi->nb_stds, vi->supported_stds, vi->index);
			obj = (*e)->NewObject(e, input_class, ctor_wotuner, (*e)->NewStringUTF(e, (const char *)vi->name), stds, vi->index);

		} else {

			dprint(LOG_V4L4J, "[V4L4J] Creating input object (with tuner): name '%s' - supported standards(%d): %p - index: %d\n",
					vi->name, vi->nb_stds, vi->supported_stds, vi->index);
			obj = (*e)->NewObject(e, input_class, ctor_wtuner,
					(*e)->NewStringUTF(e, (const char *)vi->name), stds,
					create_tuner_object(e, t, vi->tuner), vi->index);
		}

		//store it in the list
		if(obj == NULL){
			info("[V4L4J] Error creating input object\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating input object");
			return;
		}
		(*e)->CallVoidMethod(e, input_list_object, add_method, obj);
	}


}

static void create_formats_object(JNIEnv *e, jobject t, jclass this_class,
		struct v4l4j_device *d){
	jclass format_list_class;
	jfieldID formats_field;
	jmethodID format_list_ctor;
	jobject obj;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	format_list_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/ImageFormatList");
	if(format_list_class == NULL){
		info("[V4L4J] Error looking up the ImageFormatList class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up ImageFormatList class");
		return;
	}

	format_list_ctor = (*e)->GetMethodID(e, format_list_class, "<init>", "(J)V");
	if(format_list_ctor == NULL){
		info("[V4L4J] Error looking up the constructor of ImageFormatList class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of ImageFormatList class");
		return;
	}

	formats_field = (*e)->GetFieldID(e, this_class, "formats", "Lau/edu/jcu/v4l4j/ImageFormatList;");
	if(formats_field == NULL){
		info("[V4L4J] Error looking up the formats attribute ID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the formats attribute ID");
		return;
	}


	//Creates an ImageFormatList
	obj =  (*e)->NewObject(e, format_list_class, format_list_ctor, (jlong) (uintptr_t)d);
	if(obj == NULL){
		info("[V4L4J] Error creating the format list\n");
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
	jfieldID name_field;
	jclass this_class;
	struct video_device *vd = d->vdev;

	//get handles on needed java objects
	this_class = (*e)->GetObjectClass(e,t);
	if(this_class == NULL){
		info("[V4L4J] Error looking up the DeviceInfo class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the DeviceInfo class");
		return;
	}

	name_field = (*e)->GetFieldID(e, this_class, "deviceFile", "Ljava/lang/String;");
	if(name_field == NULL){
		info("[V4L4J] Error looking up the deviceFile attribute\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the deviceFile attribute");
		return;
	}


	dprint(LOG_LIBVIDEO, "[LIBVIDEO] call to get_device_info\n");
	//get data from libvideo
	if(get_device_info(vd)!=NULL){
		//fill in values in DeviceInfo object

		if (getPlatform() == kLinuxPlatform) {
			dprint(LOG_LIBVIDEO, "[V4L4J] setting device file name to '%s'\n"
					, vd->id.device_handle);
			/* set the name field */
			(*e)->SetObjectField(e, t, name_field,
					(*e)->NewStringUTF(e, vd->id.device_handle));
		}

		/* set the inputs field */
		dprint(LOG_V4L4J, "[V4L4J] Creating inputInfo objects\n");
		create_inputs_object(e, t, this_class, vd);

		/* set the formats field */
		dprint(LOG_V4L4J, "[V4L4J] Creating Format objects\n");
		create_formats_object(e, t, this_class, d);
	} else
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error getting information from video device");

}

JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_DeviceInfo_doListIntervals(
		JNIEnv *e, jobject t, jlong o, jint imf, jint w, jint h){
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	void *p;
	int type;
	jobject frame_intv;
	jclass frame_intv_class;
	jmethodID ctor;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	frame_intv_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/FrameInterval");
	if(frame_intv_class == NULL){
		info("[V4L4J] Error looking up the FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up FrameInterval class");
		return NULL;
	}

	ctor = (*e)->GetMethodID(e, frame_intv_class, "<init>",	"(IJ)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the ctor of FrameInterval class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the ctor of FrameInterval class");
		return NULL;
	}

	type = d->vdev->info->list_frame_intv(d->vdev->info, imf, w, h, &p);

	switch(type){
	case FRAME_INTV_UNSUPPORTED:
		dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval (unsupported)\n");
		//create the frame interval object
		frame_intv = (*e)->NewObject(e, frame_intv_class, ctor,
				3, (jlong) (uintptr_t) p);
		if(frame_intv == NULL){
			info("[V4L4J] Error creating FrameInterval object\n");
			THROW_EXCEPTION(e, JNI_EXCP, \
					"Error creating FrameInterval object");
			return NULL;
		}
		break;
	case FRAME_INTV_DISCRETE:
		dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval (discrete)\n");
		//create the frame interval object
		frame_intv = (*e)->NewObject(e, frame_intv_class, ctor,
				4, (jlong) (uintptr_t) p);
		XFREE(p);
		if(frame_intv == NULL){
			info("[V4L4J] Error creating FrameInterval object\n");
			THROW_EXCEPTION(e, JNI_EXCP, \
					"Error creating FrameInterval object");
			return NULL;
		}
		break;
	case FRAME_INTV_CONTINUOUS:
		dprint(LOG_V4L4J, "[V4L4L] Creating the frame interval (stepwise)\n");
		//create the frame interval object
		frame_intv = (*e)->NewObject(e, frame_intv_class, ctor,
				5, (jlong) (uintptr_t) p);
		XFREE(p);
		if(frame_intv == NULL){
			info("[V4L4J] Error creating FrameInterval object\n");
			THROW_EXCEPTION(e, JNI_EXCP, \
					"Error creating FrameInterval object");
			return NULL;
		}
		break;
	default:
		info("[V4L4J] There is a bug in v4l4j. Please report this on the\n");
		info("[V4L4J] V4L4J mailing list.\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error creating the FrameInterval object");
		return NULL;
	}
	return frame_intv;
}

/*
 * Get info about a v4l device given its device file
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_DeviceInfo_doRelease(
		JNIEnv *e, jobject t, jlong v4l4j_device){
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) v4l4j_device;
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] call to release_device_info\n");
	release_device_info(d->vdev);
}


