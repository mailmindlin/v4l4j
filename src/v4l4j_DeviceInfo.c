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

#include "libv4l.h"
#include "palettes.h"
#include "common.h"
#include "debug.h"

static jobject create_tuner_object(JNIEnv *e, jobject t, jclass this_class, struct tuner *tuner) {
	jclass tuner_class;
	jmethodID ctor;

	tuner_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/Tuner");
	if(tuner_class == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the tuner class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up tuner class");
		return 0;
	}

	ctor = (*e)->GetMethodID(e, tuner_class, "<init>", "(Ljava/lang/String;IIJJ)V");
	//ctor = (*e)->GetMethodID(e, tuner_class, "<init>", "(Ljava/lang/String;II)V");
	if(ctor == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the constructor of tuner class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up constructor of tuner class");
		return 0;
	}

	dprint(LOG_V4L4J, "[V4L4J] Creating tuner object: name '%s' - low: %ld - high: %ld - unit: %d - type: %d\n",
			tuner->name, tuner->rangelow, tuner->rangehigh, tuner->unit, tuner->type);

	return (*e)->NewObject(e, tuner_class, ctor,
			(*e)->NewStringUTF(e,(const char *)tuner->name), tuner->unit , tuner->type,
			(jlong) (tuner->rangelow & 0xffffffff), (jlong) (tuner->rangehigh & 0xffffffff));
}

static void create_inputs_object(JNIEnv *e, jobject t, jclass this_class, struct video_device *vd){
	jclass input_class, vector_class;
	jfieldID inputs_field;
	jmethodID ctor, add_method;
	jintArray stds;
	jobject obj, input_list_object;

	int i;
	struct video_input *vi;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	input_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/Input");
	if(input_class == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the input class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up input class");
		return;
	}

	vector_class = (*e)->FindClass(e, "java/util/Vector");
	if(vector_class == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the Vector class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up Vector java objects");
		return;
	}

	add_method = (*e)->GetMethodID(e, vector_class, "addElement", "(Ljava/lang/Object;)V");
	if(add_method == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the add method of Vector class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the add method of Vector objects");
		return;
	}

	inputs_field = (*e)->GetFieldID(e, this_class, "inputs", "Ljava/util/List;");
	if(inputs_field == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the inputs attribute ID\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the inputs attribute ID");
		return;
	}

	input_list_object = (*e)->GetObjectField(e, t, inputs_field);
	if(input_list_object == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the inputs attribute\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the inputs attribute");
		return;
	}

	for(i = 0; i< vd->nb_inputs; i++){
		vi = &vd->inputs[i];
		//build the input object

		//create the short[] with the supported standards
		stds= (*e)->NewIntArray(e, vi->nb_stds);
		if(stds == NULL){
			dprint(LOG_V4L4J, "[V4L4J] Error creating array\n");
			THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating array");
			return;
		}
		dprint(LOG_V4L4J, "[V4L4J] Setting new stds array with array[%d] @ %p\n",vi->nb_stds, vi->supported_stds);
		(*e)->SetIntArrayRegion(e, stds, 0, vi->nb_stds,  vi->supported_stds);

		//create the input object
		if(vd->inputs[i].tuner==NULL) {
			ctor = (*e)->GetMethodID(e, input_class, "<init>", "(Ljava/lang/String;[I)V");
			if(ctor == NULL){
				dprint(LOG_V4L4J, "[V4L4J] Error looking up the constructor of Input class\n");
				THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the constructor of Input  class");
				return;
			}
			dprint(LOG_V4L4J, "[V4L4J] Creating input object (wo tuner): name '%s' - supported standards (%d): %p\n", vi->name,vi->nb_stds, vi->supported_stds);
			obj = (*e)->NewObject(e, input_class, ctor, (*e)->NewStringUTF(e, (const char *)vi->name), stds, t);

		} else {
			ctor = (*e)->GetMethodID(e, input_class, "<init>", "(Ljava/lang/String;[ILau/edu/jcu/v4l4j/Tuner;)V");
			if(ctor == NULL){
				dprint(LOG_V4L4J, "[V4L4J] Error looking up the constructor of Input class\n");
				THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the constructor of Input  class");
				return;
			}
			dprint(LOG_V4L4J, "[V4L4J] Creating input object (with tuner): name '%s' - supported standards(%d): %p\n", vi->name, vi->nb_stds, vi->supported_stds);
			obj = (*e)->NewObject(e, input_class, ctor,
					(*e)->NewStringUTF(e, (const char *)vi->name), stds,
					create_tuner_object(e, t, this_class, vi->tuner), t);
		}

		//store it in the list
		if(obj == NULL){
			dprint(LOG_V4L4J, "[V4L4J] Error creating input object\n");
			THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating input object");
			return;
		}
		(*e)->CallVoidMethod(e, input_list_object, add_method, obj);
	}


}

static void create_formats_object(JNIEnv *e, jobject t, jclass this_class, struct video_device *vd){
	jclass format_class, vector_class;
	jfieldID formats_field;
	jmethodID ctor, add_method;
	jobject obj, format_list_object;
	int i;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	format_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/ImageFormat");
	if(format_class == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the format class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up format class");
		return;
	}

	vector_class = (*e)->FindClass(e, "java/util/Vector");
	if(vector_class == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the Vector class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up Vector java objects");
		return;
	}

	add_method = (*e)->GetMethodID(e, vector_class, "addElement", "(Ljava/lang/Object;)V");
	if(add_method == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the add method of Vector class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the add method of Vector objects");
		return;
	}

	formats_field = (*e)->GetFieldID(e, this_class, "formats", "Ljava/util/List;");
	if(formats_field == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the formats attribute ID\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the formats attribute ID");
		return;
	}

	format_list_object = (*e)->GetObjectField(e, t, formats_field);
	if(format_list_object == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the formats attribute\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the formats attribute");
		return;
	}

	ctor = (*e)->GetMethodID(e, format_class, "<init>", "(Ljava/lang/String;I)V");
	if(ctor == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the constructor of ImageFormat class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the constructor of ImageFormat class");
		return;
	}

	for(i=0; i<vd->nb_palettes; i++){
		dprint(LOG_V4L4J, "[V4L4J] Creating ImageFormat object: name %s - id: %d\n",
				libv4l_palettes[vd->palettes[i]].name, vd->palettes[i]);
		obj = (*e)->NewObject(e, format_class, ctor,
				(*e)->NewStringUTF(e, (const char *) libv4l_palettes[vd->palettes[i]].name ),
				vd->palettes[i]);
		(*e)->CallVoidMethod(e, format_list_object, add_method, obj);
	}
}

/*
 * Get info about a v4l device given its device file
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_DeviceInfo_getInfo(JNIEnv *e, jobject t, jstring f){
	jfieldID name_field, formats_field;
	jclass this_class;
	struct video_device *vd;
	const char * device_file = (*e)->GetStringUTFChars(e, f, 0);
	vd = query_device(device_file);
	(*e)->ReleaseStringUTFChars(e, f,device_file);

	this_class = (*e)->GetObjectClass(e,t);
	if(this_class == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the DeviceInfo class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the DeviceInfo class");
		return;
	}

	name_field = (*e)->GetFieldID(e, this_class, "name", "Ljava/lang/String;");
	if(name_field == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the name attribute\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the name attribute");
		return;
	}


	formats_field = (*e)->GetFieldID(e, this_class, "formats", "Ljava/util/List;");
	if(name_field == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the formats attribute\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up the formats attribute");
		return;
	}

	if(vd!=NULL){
		/* set the name field */
		(*e)->SetObjectField(e, t, name_field, (*e)->NewStringUTF(e, vd->name));

		/* set the inputs field */
		create_inputs_object(e, t, this_class, vd);

		/* set the formats field */
		create_formats_object(e, t, this_class, vd);

		free_video_device(vd);
	}
}
