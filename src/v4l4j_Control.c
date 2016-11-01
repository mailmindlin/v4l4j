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
#include "libvideo-err.h"


/*
 * get the current value of a v4l2 control
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_Control_doGetValue(JNIEnv *e, jclass t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);
	struct v4l4j_device *dev = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_value(dev: %s, ctrl name:%s)\n", dev->vdev->file, dev->vdev->control->controls[id].v4l2_ctrl->name);
	int ret = get_control_value(dev->vdev, dev->vdev->control->controls[id].v4l2_ctrl, &val, 0);
	if(ret != 0) {
		THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for control '%s'", dev->vdev->control->controls[id].v4l2_ctrl->name);
		return -1;
	}
	
	int val = 0;
	dprint(LOG_V4L4J, "[V4L4J] get_control_value(dev: %s, ctrl name:%s) = %d\n", dev->vdev->file, dev->vdev->control->controls[id].v4l2_ctrl->name, val);
	return val;
}


/*
 * Set a new value on a v4l2 control
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_Control_doSetValue(JNIEnv *e, jclass t, jlong object, jint id, jint value) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling set_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->vdev->file, d->vdev->control->controls[id].v4l2_ctrl->name,value);
	int v = value;
	int ret = set_control_value(d->vdev, d->vdev->control->controls[id].v4l2_ctrl, &v, 0);
	if(ret != 0) {
		if(ret == LIBVIDEO_ERR_OUT_OF_RANGE) {
			THROW_EXCEPTION(e, INVALID_VAL_EXCP, "Invalid value %d for control '%s': value out of range", value, d->vdev->control->controls[id].v4l2_ctrl->name);
		} else if(ret == LIBVIDEO_ERR_STREAMING) {
			THROW_EXCEPTION(e, CTRL_EXCP, "Cannot set value for control '%s' while streaming", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else {
			THROW_EXCEPTION(e, CTRL_EXCP, "Error setting current value for control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		}
		return 0;
	}
	dprint(LOG_V4L4J, "[V4L4J] set_control_value(dev: %s, ctrl name:%s) desired = %d - set to: %d\n", d->vdev->file, d->vdev->control->controls[id].v4l2_ctrl->name, value, v);
	return v;
}

/*
 * Get the current value of a v4l2 string control
 */
JNIEXPORT jstring JNICALL Java_au_edu_jcu_v4l4j_Control_doGetStringValue(JNIEnv *e, jclass t, jlong object, jint id) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	
	struct v4l2_queryctrl* control = d->vdev->control->controls[id].v4l2_ctrl;
	
	char *val;
	XMALLOC(val, char*, control->maximum + 1); // "+ 1" as per spec

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_value(dev: %s, ctrl name:%s)\n", d->vdev->file, control->name);
	if(get_control_value(d->vdev, control, val, control->maximum + 1) != 0) {
		//There was an error reading the control's value
		XFREE(val);
		THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for string control '%s'", control->name);
		return NULL;
	}

	dprint(LOG_V4L4J, "[V4L4J] get_control_value(dev: %s, string ctrl name:%s) = %s\n", d->vdev->file, control->name, val);
	
	//Convert the char* to a Java String
	jstring result = (*e)->NewStringUTF(e, val);
	
	XFREE(val);
	
	return result;
}


/*
 * Set a new value on a v4l2 string control
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_Control_doSetStringValue(JNIEnv *e, jclass t, jlong object, jint id, jstring jvalue) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	//TODO replace with GetStringRegion
	const char* value = (*e)->GetStringUTFChars(e, jvalue, 0);
	if (value == NULL) {
		// OOM exception already thrown
		dprint(LOG_V4L4J, " Error getting string\n");
		return;
	}

	// copy the java string value in case it gets modified by the driver
	char* copy = strdup(value);
	if (copy == NULL) {
		THROW_EXCEPTION(e, CTRL_EXCP, "Error copying new string value");
		return;
	}

	(*e)->ReleaseStringUTFChars(e, jvalue, value);
	
	int size = strlen(copy) + 1;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling set_control_value(dev: %s, ctrl name:%s, val: %s - byte size: %d)\n", d->vdev->file, d->vdev->control->controls[id].v4l2_ctrl->name, copy, size);
	int err = set_control_value(d->vdev, d->vdev->control->controls[id].v4l2_ctrl, copy, size);
	
	free(copy);
	
	if(err) {
		if(err == LIBVIDEO_ERR_OUT_OF_RANGE) {
			THROW_EXCEPTION(e, INVALID_VAL_EXCP, "Invalid value for string control %s : value out of range", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else if(err == LIBVIDEO_ERR_STREAMING) {
			THROW_EXCEPTION(e, CTRL_EXCP, "Cannot set value for string control '%s' while streaming", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else {
			THROW_EXCEPTION(e, CTRL_EXCP, "Error (#%d) setting current value for string control '%s'", err, d->vdev->control->controls[id].v4l2_ctrl->name);
		}
	}
}

/*
 * get the current value of a v4l2 integer64 control
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_Control_doGetLongValue(JNIEnv *e, jobject t, jlong object, jint id) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_value(dev: %s, ctrl name:%s)\n", d->vdev->file, d->vdev->control->controls[id].v4l2_ctrl->name);
	jlong result;
	if(get_control_value(d->vdev, d->vdev->control->controls[id].v4l2_ctrl, &result, 0) != 0) {
		THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for long control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		return 0;
	}

	dprint(LOG_V4L4J, "[V4L4J] get_control_value(dev: %s, integer64 ctrl name:%s) = %lld\n", d->vdev->file, d->vdev->control->controls[id].v4l2_ctrl->name, (unsigned long long)result);

	return result;
}


/*
 * Set a new value on a v4l2 integer64 control
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_Control_doSetLongValue(JNIEnv *e, jclass t, jlong object, jint id, jlong jvalue) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n", __PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling set_control_value(dev: %s, ctrl name:%s, val: %lld)\n", d->vdev->file, d->vdev->control->controls[id].v4l2_ctrl->name, (unsigned long long)jvalue);
	int ret = set_control_value(d->vdev, d->vdev->control->controls[id].v4l2_ctrl, &jvalue, 0);

	if(ret != 0) {
		if(ret == LIBVIDEO_ERR_OUT_OF_RANGE) {
			THROW_EXCEPTION(e, INVALID_VAL_EXCP, "Invalid value for long control %s : value out of range", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else if(ret == LIBVIDEO_ERR_STREAMING) {
			THROW_EXCEPTION(e, CTRL_EXCP, "Cannot set value for long control '%s' while streaming", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else {
			THROW_EXCEPTION(e, CTRL_EXCP, "Error setting current value for long control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		}
	}
}
