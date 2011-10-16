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


/*
 * Initialise a video device object
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doInit(JNIEnv *e, jobject t, jstring dev){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d;
	XMALLOC(d, struct v4l4j_device *, sizeof(struct v4l4j_device));

	const char * device_file = (*e)->GetStringUTFChars(e, dev, 0);
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling open_device()\n");
	d->vdev = open_device((char *)device_file);
	(*e)->ReleaseStringUTFChars(e, dev,device_file);

	if(d->vdev ==NULL) {
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating new VideoDevice object");
		return 0;
	}

	return (uintptr_t) d;
}

/*
 * Releases a video device object
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doRelease(JNIEnv *e, jobject t, jlong o){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;
	int ret;

	if ((ret = close_device(d->vdev)) !=0 ) {
		if (ret== LIBVIDEO_ERR_CAPTURE_IN_USE){
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

static jintArray get_values(JNIEnv *e, struct control *l){
	int i,values[l->count_menu];
	jintArray values_array = (*e)->NewIntArray(e, l->count_menu);

	if(values_array == NULL){
		info("[V4L4J] Error creating the values array\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the values array");
		return NULL;
	}

	for(i=0;i<l->count_menu;i++)
		values[i] = l->v4l2_menu[i].index;

	(*e)->SetIntArrayRegion(e, values_array, 0, l->count_menu, values);

	return values_array;
}


static jobjectArray get_names(JNIEnv *e, struct control *l){
	int i;
	jclass string_class = (*e)->FindClass(e,"java/lang/String");
	if(string_class == NULL){
		info("[V4L4J] Error looking up string class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error up string class");
		return NULL;
	}

	jobjectArray names_array = (*e)->NewObjectArray(e, l->count_menu, string_class ,NULL);

	if(names_array == NULL){
		info("[V4L4J] Error creating the names array\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the name array");
		return NULL;
	}

	for(i=0;i<l->count_menu;i++)
		(*e)->SetObjectArrayElement(e, names_array, i, (*e)->NewStringUTF(e, (const char *) l->v4l2_menu[i].name));

	return names_array;
}

static int translate_type(JNIEnv *e, int t){
	jfieldID fid;
	jclass constants = (*e)->FindClass(e,CONSTANTS_CLASS);

	if(constants == NULL){
		info("[V4L4J] Error looking up the V4L4JConstants class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the V4L4JConstants class");
		return -1;
	}

	if(t == V4L2_CTRL_TYPE_BUTTON){
		fid = (*e)->GetStaticFieldID(e, constants, "CTRL_TYPE_BUTTON", "I");
		if(fid == NULL){
			info( "[V4L4J] Error looking up the BUTTON field in V4L4JConstants class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the BUTTON field in V4L4JConstants class");
			return -1;
		}
	} else if(t == V4L2_CTRL_TYPE_BOOLEAN){
		fid = (*e)->GetStaticFieldID(e, constants, "CTRL_TYPE_SWITCH", "I");
		if(fid == NULL){
			info("[V4L4J] Error looking up the SWITCH field in V4L4JConstants class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the SWITCH field in V4L4JConstants class");
			return -1;
		}
	} else if(t == V4L2_CTRL_TYPE_INTEGER){
		fid = (*e)->GetStaticFieldID(e, constants, "CTRL_TYPE_SLIDER", "I");
		if(fid == NULL){
			info("[V4L4J] Error looking up the SLIDER field in V4L4JConstants class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the SLIDER field in V4L4JConstants class");
			return -1;
		}
	} else if(t == V4L2_CTRL_TYPE_MENU){
		fid = (*e)->GetStaticFieldID(e, constants, "CTRL_TYPE_DISCRETE", "I");
		if(fid == NULL){
			info( "[V4L4J] Error looking up the DISCRETE field in V4L4JConstants class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the DISCRETE field in V4L4JConstants class");
			return -1;
		}
	} else if(t == V4L2_CTRL_TYPE_STRING){
		fid = (*e)->GetStaticFieldID(e, constants, "CTRL_TYPE_STRING", "I");
		if(fid == NULL){
			info( "[V4L4J] Error looking up the STRING field in V4L4JConstants class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the STRING field in V4L4JConstants class");
			return -1;
		}
	} else if(t == V4L2_CTRL_TYPE_INTEGER64){
		fid = (*e)->GetStaticFieldID(e, constants, "CTRL_TYPE_LONG", "I");
		if(fid == NULL){
			info("[V4L4J] Error looking up the CTRL_TYPE_LONG field in V4L4JConstants class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the CTRL_TYPE_LONG field in V4L4JConstants class");
			return -1;
		}
	} else if(t == V4L2_CTRL_TYPE_BITMASK){
		fid = (*e)->GetStaticFieldID(e, constants, "CTRL_TYPE_BITMASK", "I");
		if(fid == NULL){
			info("[V4L4J] Error looking up the CTRL_TYPE_BITMASK field in V4L4JConstants class\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the CTRL_TYPE_BITMASK field in V4L4JConstants class");
			return -1;
		}
	} else {
		info("[V4L4J] Unknown control type: %d\n", t);
		THROW_EXCEPTION(e, JNI_EXCP, "Unknown control type: %d\n", t);
		return -1;
	}

	return (*e)->GetStaticIntField(e, constants, fid);
}



/*
 * Init the control interface - Creates a list of controls
 */
JNIEXPORT jobjectArray JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doGetControlList(JNIEnv *e, jobject t, jlong object){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	struct control_list *l;
	jclass v4l2ControlClass;
	jmethodID ctor;
	jobjectArray ctrls, names_array;
	jintArray values_array;
	jobject element;
	int i, type;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_list()\n");
	l = get_control_list(d->vdev);

	//Creates the java objects matching v4l2 controls
	dprint(LOG_V4L4J, "[V4L4J] Creating the Control array[%d]\n", l->count);
	v4l2ControlClass = (*e)->FindClass(e,CONTROL_CLASS);
	if(v4l2ControlClass == NULL){
		info("[V4L4J] Error looking up the Control class\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Control class");
		return 0;
	}

	ctor = (*e)->GetMethodID(e, v4l2ControlClass, "<init>", "(ILjava/lang/String;IIII[Ljava/lang/String;[IJ)V");
	if(ctor == NULL){
		info("[V4L4J] Error looking up the Control class constructor\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Control class constructor");
		return 0;
	}

	//initialise the ctrls field of FrameGrabber object (this)
	ctrls = (*e)->NewObjectArray(e, l->count, v4l2ControlClass, NULL);
	if(ctrls == NULL){
		info("[V4L4J] Error creating the control array\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the control array");
		return 0;
	}

	//Construct a V4L2Control Java object for each V4L2control in l
	for(i = 0; i< l->count; i++) {
		if(l->controls[i].count_menu==0) {
			values_array = NULL;
			names_array = NULL;
		} else {
			//create the values and names arrays
			if((values_array = get_values(e,&l->controls[i])) == NULL) {
				release_control_list(d->vdev);
				return 0;
			}

			if((names_array = get_names(e,&l->controls[i])) ==NULL){
				release_control_list(d->vdev);
				return 0;
			}
		}
		if((type = translate_type(e, l->controls[i].v4l2_ctrl->type))==-1)
			return 0;

		dprint(LOG_V4L4J, "[V4L4J] Creating Control %d - name: %s - type: %d\n", i, l->controls[i].v4l2_ctrl->name, type);
		element = (*e)->NewObject(e, v4l2ControlClass, ctor, i,\
			(*e)->NewStringUTF(e, (const char *)l->controls[i].v4l2_ctrl->name),\
			l->controls[i].v4l2_ctrl->minimum, l->controls[i].v4l2_ctrl->maximum, \
			l->controls[i].v4l2_ctrl->step, type, names_array, values_array, object);
		if(element == NULL){
			info("[V4L4J] Error creating the control '%s'\n", l->controls[i].v4l2_ctrl->name);
			release_control_list(d->vdev);
			THROW_EXCEPTION(e, JNI_EXCP, "Error creating the control '%s'", l->controls[i].v4l2_ctrl->name);
			return 0;
		}
		(*e)->SetObjectArrayElement(e, ctrls, i, element);
	}
	return ctrls;
}

/*
 * Releases the control interface
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doReleaseControlList(JNIEnv *e, jobject t, jlong o){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling release_control_list()\n");
	release_control_list(d->vdev);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doGetTunerActions(JNIEnv *e, jobject t, jlong o){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_tuner_actions()\n");
	get_tuner_actions(d->vdev);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doReleaseTunerActions(JNIEnv *e, jobject t, jlong o){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) o;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling release_tuner_actions()\n");
	release_tuner_actions(d->vdev);
}


