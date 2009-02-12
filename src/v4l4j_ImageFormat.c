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

#include "palettes.h"
#include "common.h"
#include "debug.h"

/*
 * populate the formats static attribute of the ImageFormat class with all image formats
 * supported by libv4l (in the libv4l_palettes array in palettes.h)
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_ImageFormat_listFormats(JNIEnv *e, jclass this_class){
	jclass vector_class;
	jmethodID ctor, add_method;
	jfieldID field;
	jobject obj, vector;
	int i;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	/* Get handles on Java stuff */
	vector_class = (*e)->FindClass(e, "java/util/Vector");
	if(vector_class == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Vector java objects");
		return;
	}

	ctor = (*e)->GetMethodID(e, vector_class, "<init>", "()V");
	if(ctor == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the constructor of Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of Vector objects");
		return;
	}
	add_method = (*e)->GetMethodID(e, vector_class, "addElement", "(Ljava/lang/Object;)V");
	if(add_method == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the add method of Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the add method of Vector objects");
		return;
	}

	field = (*e)->GetStaticFieldID(e, this_class, "formats", "Ljava/util/List;");
	if(field == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the formats attribute\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the formats attribute");
		return;
	}

	/* creates the list object and assign it to the formats attribute*/
	dprint(LOG_V4L4J, "[V4L4J] Creating new vector object\n");
	vector = (*e)->NewObject(e, vector_class, ctor);
	dprint(LOG_V4L4J, "[V4L4J] Setting new vector object\n");
	(*e)->SetStaticObjectField(e, this_class, field, vector);

	/* Populates the list object*/
	ctor = (*e)->GetMethodID(e, this_class, "<init>", "(Ljava/lang/String;I)V");
	if(ctor == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the constructor of ImageFormat class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the constructor of ImageFormat objects");
		return;
	}

	for(i=0; i<NB_SUPPORTED_PALETTE; i++){
		obj = (*e)->NewObject(e, this_class, ctor, (*e)->NewStringUTF(e, (const char *)libv4l_palettes[i].name), i);
		(*e)->CallVoidMethod(e, vector, add_method, obj);
	}
}

