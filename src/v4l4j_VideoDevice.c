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

#include "libvideo-err.h"
#include "libvideo.h"
#include "common.h"
#include "debug.h"
#include "jpeg.h"

/*
 * Initialise a video device object
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doInit(JNIEnv *e, jclass t, jstring dev) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d;
	XMALLOC(d, struct v4l4j_device *, sizeof(struct v4l4j_device));
	
	const char * device_file = (*e)->GetStringUTFChars(e, dev, 0);
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling open_device()\n");
	d->vdev = open_device((char *)device_file);
	(*e)->ReleaseStringUTFChars(e, dev, device_file);
	
	if(d->vdev == NULL) {
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating new VideoDevice object");
		return 0;
	}
	
	return (jlong) (uintptr_t) d;
}

/*
 * Releases a video device object
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doRelease(JNIEnv *e, jclass t, jlong o) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	int ret;

	if ((ret = close_device(d->vdev)) != 0 ) {
		if (ret == LIBVIDEO_ERR_CAPTURE_IN_USE) {
			dprint(LOG_LIBVIDEO, "[V4L4J] Error closing video_device - capture in progress\n");
			THROW_EXCEPTION(e, RELEASE_EXCP, "Error closing video device, capture in progress");
		} else if (ret == LIBVIDEO_ERR_INFO_IN_USE){
			dprint(LOG_LIBVIDEO, "[V4L4J] Error closing video_device - DeviceInfo not released\n");
			THROW_EXCEPTION(e, RELEASE_EXCP, "Error closing video device, DeviceInfo in use");
		} else if (ret == LIBVIDEO_ERR_CONTROL_IN_USE){
			dprint(LOG_LIBVIDEO, "[V4L4J] Error closing video_device - ControlList not released\n");
			THROW_EXCEPTION(e, RELEASE_EXCP, "Error closing video device, ControlList in use");
		}
		return;
	}

	dprint(LOG_V4L4J, "[V4L4J] Freeing v4l4j device\n");
	XFREE(d);
}

static jintArray get_values(JNIEnv *e, struct control *l) {
	const int len = l->count_menu;
	jintArray values_array = (*e)->NewIntArray(e, len);

	if(!values_array) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the values array");
		return NULL;
	}
	
	int* values;
	XCALLOC(values, int*, len, sizeof(int));
	if (!values) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error allocating temporary values array");
		return NULL;
	}
	
	//FIXME can I change this from a 2-copy to a 1-copy method by
	//using GetIntArrayElements (or would that be even slower?)
	for(unsigned int i = 0; i < len; i++)
		values[i] = l->v4l2_menu[i].index;

	(*e)->SetIntArrayRegion(e, values_array, 0, l->count_menu, values);
	XFREE(values);
	
	return values_array;
}


static jobjectArray get_names(JNIEnv *e, struct control *l) {
	jclass string_class = (*e)->FindClass(e,"java/lang/String");
	if(!string_class){
		THROW_EXCEPTION(e, JNI_EXCP, "Error up class java.lang.String");
		return NULL;
	}
	
	jobjectArray names_array = (*e)->NewObjectArray(e, l->count_menu, string_class, NULL);
	(*e)->DeleteLocalRef(e, string_class);
	if(!names_array) {
		info("[V4L4J] Error creating the names array\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the name array");
		return NULL;
	}

	for(unsigned int i = 0; i < l->count_menu; i++) {
		const char* name;
		if(l->v4l2_ctrl->type == V4L2_CTRL_TYPE_MENU) {
			name = (const char*) l->v4l2_menu[i].name;
		} else if (l->v4l2_ctrl->type == V4L2_CTRL_TYPE_INTEGER_MENU) {
			//TODO fix w/ dynamic alloc?
			char control_name[64] = {0};	// hopefully long enough to hold the largest name
			snprintf(control_name, sizeof(control_name) - 1, "%lld", l->v4l2_menu[i].value);
			name = (const char*) control_name;
		} else {
			info("[V4L4J] Unknown menu type\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Unknown menu control type\n");
			return NULL;
		}
		jstring nameStr = (*e)->NewStringUTF(e, name);
		(*e)->SetObjectArrayElement(e, names_array, i, nameStr);
		// Release JNI reference to element (we keep the reference to the array, though)
		(*e)->DeleteLocalRef(e, nameStr);
	}
	return names_array;
}

static int translate_type(JNIEnv *e, int t) {
	//Lookup the name of the constant
	const char* fieldName;
	switch (t) {
		case V4L2_CTRL_TYPE_BUTTON:
			fieldName = "CTRL_TYPE_BUTTON";
			break;
		case V4L2_CTRL_TYPE_BOOLEAN:
			fieldName = "CTRL_TYPE_SWITCH";
			break;
		case V4L2_CTRL_TYPE_INTEGER:
			fieldName = "CTRL_TYPE_SLIDER";
			break;
		case V4L2_CTRL_TYPE_MENU:
		case V4L2_CTRL_TYPE_INTEGER_MENU:
			fieldName = "CTRL_TYPE_DISCRETE";
			break;
		case V4L2_CTRL_TYPE_STRING:
			fieldName = "CTRL_TYPE_STRING";
			break;
		case V4L2_CTRL_TYPE_INTEGER64:
			fieldName = "CTRL_TYPE_LONG";
			break;
		case V4L2_CTRL_TYPE_BITMASK:
			fieldName = "CTRL_TYPE_BITMASK";
			break;
		default:
			THROW_EXCEPTION(e, JNI_EXCP, "Unknown control type %d", t);
			return -1;
	}
	
	jclass constants = (*e)->FindClass(e, CONSTANTS_CLASS);
	if(constants == NULL) {
		info("[V4L4J] Error looking up the V4L4JConstants class (%s)\n", CONSTANTS_CLASS);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the V4L4JConstants class");
		return -1;
	}
	
	jfieldID fid = (*e)->GetStaticFieldID(e, constants, fieldName, "I");
	if (!fid) {
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up field (int) V4L4JConstants#%s", fieldName);
		return -1;
	}
	
	int result = (int) (*e)->GetStaticIntField(e, constants, fid);
	(*e)->DeleteLocalRef(e, constants);
	return result;
}



/*
 * Init the control interface - Creates a list of controls
 */
JNIEXPORT jobjectArray JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doGetControlList(JNIEnv *e, jclass t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_list()\n");
	struct control_list *l = get_control_list(d->vdev);

	//Creates the java objects matching v4l2 controls
	dprint(LOG_V4L4J, "[V4L4J] Creating the Control array[%d]\n", l->count);
	jclass v4l2ControlClass = (*e)->FindClass(e, CONTROL_CLASS);
	if(v4l2ControlClass == NULL){
		info("[V4L4J] Error looking up the Control class\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Control class");
		return 0;
	}

	jmethodID ctor = (*e)->GetMethodID(e, v4l2ControlClass, "<init>", "(ILjava/lang/String;IIII[Ljava/lang/String;[IJ)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the Control class constructor\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Control class constructor");
		return 0;
	}

	//initialise the ctrls field of FrameGrabber object (this)
	jobjectArray ctrls = (*e)->NewObjectArray(e, l->count, v4l2ControlClass, NULL);
	if(ctrls == NULL){
		info("[V4L4J] Error creating the control array\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the control array");
		return 0;
	}

	//Construct a V4L2Control Java object for each V4L2control in l
	for(unsigned int i = 0; i< l->count; i++) {
		jobjectArray names_array;
		jintArray values_array;
		if(l->controls[i].count_menu == 0) {
			names_array = NULL;
			values_array = NULL;
		} else {
			// Create the name and value arrays
			if(!(names_array = get_names(e,&l->controls[i])) || !(values_array = get_values(e, &l->controls[i]))) {
				//Exception already thrown
				release_control_list(d->vdev);
				return 0;
			}
		}
		int type = translate_type(e, l->controls[i].v4l2_ctrl->type);
		if(type == -1)
			//Exception already thrown
			return 0;

		dprint(LOG_V4L4J, "[V4L4J] Creating Control %d - name: %s - type: %d\n", i, l->controls[i].v4l2_ctrl->name, type);
		jstring ctrlName = (*e)->NewStringUTF(e, (const char*) l->controls[i].v4l2_ctrl->name);
		jobject element = (*e)->NewObject(e, v4l2ControlClass, ctor, i,\
			ctrlName, l->controls[i].v4l2_ctrl->minimum, l->controls[i].v4l2_ctrl->maximum, \
			l->controls[i].v4l2_ctrl->step, type, names_array, values_array, object);
		//Release JNI references
		(*e)->DeleteLocalRef(e, ctrlName);
		if (names_array)
			(*e)->DeleteLocalRef(e, names_array);
		if (values_array)
			(*e)->DeleteLocalRef(e, values_array);
		if(!element) {
			info("[V4L4J] Error creating the control '%s'\n", l->controls[i].v4l2_ctrl->name);
			release_control_list(d->vdev);
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating the control '%s'", l->controls[i].v4l2_ctrl->name);
			return 0;
		}
		(*e)->SetObjectArrayElement(e, ctrls, i, element);
		(*e)->DeleteLocalRef(e, element);
	}
	
	return ctrls;
}

/*
 * Releases the control interface
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doReleaseControlList(JNIEnv *e, jclass t, jlong o){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling release_control_list()\n");
	release_control_list(d->vdev);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doGetTunerActions(JNIEnv *e, jclass t, jlong o){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_tuner_actions()\n");
	get_tuner_actions(d->vdev);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doReleaseTunerActions(JNIEnv *e, jclass t, jlong o){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling release_tuner_actions()\n");
	release_tuner_actions(d->vdev);
}


