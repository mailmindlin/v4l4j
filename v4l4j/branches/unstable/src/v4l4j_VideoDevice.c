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

#include "libv4l-err.h"
#include "libv4l.h"
#include "common.h"
#include "debug.h"


/*
 * Initialise a video device object
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doInit(JNIEnv *e, jobject t, jstring dev){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d;
	XMALLOC(d, struct v4l4j_device *, sizeof(struct v4l4j_device));
	if(d==NULL) {
		THROW_EXCEPTION(e, INIT_EXCP, "Error creating new v4l4j object - out of memory");
		return 0;
	}

	const char * device_file = (*e)->GetStringUTFChars(e, dev, 0);
	d->vdev = open_device((char *)device_file);
	(*e)->ReleaseStringUTFChars(e, dev,device_file);

	if(d->vdev ==NULL) {
		THROW_EXCEPTION(e, INIT_EXCP, "Error creating new VideoDevice object");
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
		if (ret== LIBV4L_ERR_CAPTURE_IN_USE){
			dprint(LOG_LIBV4L, "[V4L4J] Error closing video_device - capture in progress\n");
			THROW_EXCEPTION(e, RELEASE_EXCP, "Error closing video device, capture in progress");
		} else if (ret == LIBV4L_ERR_INFO_IN_USE){
			dprint(LOG_LIBV4L, "[V4L4J] Error closing video_device - DeviceInfo not released\n");
			THROW_EXCEPTION(e, RELEASE_EXCP, "Error closing video device, DeviceInfo in use");
		} else if (ret == LIBV4L_ERR_CONTROL_IN_USE){
			dprint(LOG_LIBV4L, "[V4L4J] Error closing video_device - ControlList not released\n");
			THROW_EXCEPTION(e, RELEASE_EXCP, "Error closing video device, ControlList in use");
		}
		return;
	}

	dprint(LOG_V4L4J, "[V4L4J] Freeing v4l4j device\n");
	XFREE(d);
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_VideoDevice_doCheckJPEGSupport(JNIEnv *e, jobject t, jlong o, jintArray fmtsArray){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	jint *fmts = (*e)->GetIntArrayElements(e, fmtsArray, NULL);
	int jpeg_fmts[NB_JPEG_SUPPORTED_FORMATS]= JPEG_SUPPORTED_FORMATS;
	int i,j, l = (*e)->GetArrayLength(e, fmtsArray);
	if(fmts == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error getting the image format array\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error getting the image format array");
		return 0;
	}

	for(i=0; i<l; i++)
		for(j=0;j<NB_JPEG_SUPPORTED_FORMATS;j++)
			if(fmts[i]==jpeg_fmts[j]){
				(*e)->ReleaseIntArrayElements(e, fmtsArray, fmts, 0);
				return 1;
			}

	(*e)->ReleaseIntArrayElements(e, fmtsArray, fmts, 0);
	return 0;
}

static jintArray get_values(JNIEnv *e, struct control *l){
	int i,values[l->count_menu];
	jintArray values_array = (*e)->NewIntArray(e, l->count_menu);
	
	if(values_array == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error creating the values array\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating the values array");
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
		dprint(LOG_V4L4J, "[V4L4J] Error looking up string class\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error up string class");
		return NULL;
	}
	
	jobjectArray names_array = (*e)->NewObjectArray(e, l->count_menu, string_class ,NULL);
	
	if(names_array == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error creating the names array\n");
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating the name array");
		return NULL;
	}
	
	for(i=0;i<l->count_menu;i++)
		(*e)->SetObjectArrayElement(e, names_array, i, (*e)->NewStringUTF(e, (const char *) l->v4l2_menu[i].name));

	return names_array;
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
	int i;

	l = get_control_list(d->vdev);

	//Creates the java objects matching v4l2 controls
	dprint(LOG_V4L4J, "[V4L4J] Creating the Control array[%d]\n", l->count);
	v4l2ControlClass = (*e)->FindClass(e,CONTROL_CLASS);
	if(v4l2ControlClass == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the Control class\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up Control class");
		return 0;
	}

	ctor = (*e)->GetMethodID(e, v4l2ControlClass, "<init>", "(ILjava/lang/String;IIII[Ljava/lang/String;[IJ)V");
	if(ctor == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error looking up the Control class constructor\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error looking up Control class constructor");
		return 0;
	}

	//initialise the ctrls field of FrameGrabber object (this)
	ctrls = (*e)->NewObjectArray(e, l->count, v4l2ControlClass, NULL);
	if(ctrls == NULL){
		dprint(LOG_V4L4J, "[V4L4J] Error creating the control array\n");
		release_control_list(d->vdev);
		THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating the control array");
		return 0;
	}

	//Construct a V4L2Control Java object for each V4L2control in l
	for(i = 0; i< l->count; i++) {
		dprint(LOG_V4L4J, "[V4L4J] Creating Control %d - name: %s\n", i, l->controls[i].v4l2_ctrl->name);
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
		element = (*e)->NewObject(e, v4l2ControlClass, ctor, i,\
			(*e)->NewStringUTF(e, (const char *)l->controls[i].v4l2_ctrl->name),\
			l->controls[i].v4l2_ctrl->minimum, l->controls[i].v4l2_ctrl->maximum, \
			l->controls[i].v4l2_ctrl->step, l->controls[i].v4l2_ctrl->type, names_array, values_array, object);
		if(element == NULL){
			dprint(LOG_V4L4J, "[V4L4J] Error creating the control '%s'\n", l->controls[i].v4l2_ctrl->name);
			release_control_list(d->vdev);
			THROW_EXCEPTION(e, GENERIC_EXCP, "Error creating the control '%s'", l->controls[i].v4l2_ctrl->name);
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

	release_control_list(d->vdev);
}

