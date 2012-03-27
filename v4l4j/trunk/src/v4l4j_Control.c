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
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_Control_doGetValue(JNIEnv *e, jobject t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int val = 0, ret;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_value(dev: %s, ctrl name:%s)\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name);
	ret = get_control_value(d->vdev,d->vdev->control->controls[id].v4l2_ctrl, &val, 0);

	if(ret != 0) {
		THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		return -1;
	}

	dprint(LOG_V4L4J, "[V4L4J] get_control_value(dev: %s, ctrl name:%s) = %d\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name, val);
	return val;
}


/*
 * Set a new value on a v4l2 control
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_Control_doSetValue(JNIEnv *e, jobject t, jlong object, jint id, jint value){
	int ret, v = value;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling set_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name,value);
	ret = set_control_value(d->vdev, d->vdev->control->controls[id].v4l2_ctrl, &v, 0);
	if(ret != 0) {
		if(ret == LIBVIDEO_ERR_OUT_OF_RANGE){
			THROW_EXCEPTION(e, INVALID_VAL_EXCP, "Invalid value %d for control '%s': value out of range", value, d->vdev->control->controls[id].v4l2_ctrl->name);
		} else if(ret == LIBVIDEO_ERR_STREAMING){
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
 * get the current value of a v4l2 string control
 */
JNIEXPORT jstring JNICALL Java_au_edu_jcu_v4l4j_Control_doGetStringValue(JNIEnv *e, jobject t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int ret;
	char *val;
	jstring result;

	XMALLOC(val, char*, d->vdev->control->controls[id].v4l2_ctrl->maximum + 1); // "+ 1" as per spec

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_value(dev: %s, ctrl name:%s)\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name);
	ret = get_control_value(d->vdev,d->vdev->control->controls[id].v4l2_ctrl, val, d->vdev->control->controls[id].v4l2_ctrl->maximum + 1);
	if(ret != 0) {
		XFREE(val);
		THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for string control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		return NULL;
	}

	dprint(LOG_V4L4J, "[V4L4J] get_control_value(dev: %s, string ctrl name:%s) = %s\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name, val);

	result = (*e)->NewStringUTF(e, val);

	XFREE(val);

	return result;
}


/*
 * Set a new value on a v4l2 string control
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_Control_doSetStringValue(JNIEnv *e, jobject t, jlong object, jint id, jstring jvalue, jint size){
	int ret;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	const char * value = (*e)->GetStringUTFChars(e, jvalue, 0);
	size++;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling set_control_value(dev: %s, ctrl name:%s, val: %s - byte size: %d)\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name, value, size);
	ret = set_control_value(d->vdev, d->vdev->control->controls[id].v4l2_ctrl, value, size);
	(*e)->ReleaseStringUTFChars(e, jvalue, value);

	if(ret != 0) {
		if(ret == LIBVIDEO_ERR_OUT_OF_RANGE){
			THROW_EXCEPTION(e, INVALID_VAL_EXCP, "Invalid value for string control %s : value out of range", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else if(ret == LIBVIDEO_ERR_STREAMING){
			THROW_EXCEPTION(e, CTRL_EXCP, "Cannot set value for string control '%s' while streaming", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else {
			THROW_EXCEPTION(e, CTRL_EXCP, "Error setting current value for string control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		}
	}
}

/*
 * get the current value of a v4l2 integer64 control
 */
JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_Control_doGetLongValue(JNIEnv *e, jobject t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int ret;
	jlong result;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling get_control_value(dev: %s, ctrl name:%s)\n", d->vdev->file, d->vdev->control->controls[id].v4l2_ctrl->name);
	ret = get_control_value(d->vdev,d->vdev->control->controls[id].v4l2_ctrl, &result, 0);
	if(ret != 0) {
		THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for long control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		return 0;
	}

	dprint(LOG_V4L4J, "[V4L4J] get_control_value(dev: %s, integer64 ctrl name:%s) = %lld\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name, (unsigned long long)result);

	return result;
}


/*
 * Set a new value on a v4l2 integer64 control
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_Control_doSetLongValue(JNIEnv *e, jobject t, jlong object, jint id, jlong jvalue){
	int ret;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling set_control_value(dev: %s, ctrl name:%s, val: %lld)\n", d->vdev->file,d->vdev->control->controls[id].v4l2_ctrl->name, (unsigned long long)jvalue);
	ret = set_control_value(d->vdev, d->vdev->control->controls[id].v4l2_ctrl, &jvalue, 0);

	if(ret != 0) {
		if(ret == LIBVIDEO_ERR_OUT_OF_RANGE){
			THROW_EXCEPTION(e, INVALID_VAL_EXCP, "Invalid value for long control %s : value out of range", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else if(ret == LIBVIDEO_ERR_STREAMING){
			THROW_EXCEPTION(e, CTRL_EXCP, "Cannot set value for long control '%s' while streaming", d->vdev->control->controls[id].v4l2_ctrl->name);
		} else {
			THROW_EXCEPTION(e, CTRL_EXCP, "Error setting current value for long control '%s'", d->vdev->control->controls[id].v4l2_ctrl->name);
		}
	}
}
