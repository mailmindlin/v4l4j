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

#include "libv4l.h"
#include "common.h"
#include "debug.h"
#include "libv4l-err.h"


/*
 * get the current value of a v4l2 control
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_Control_doGetValue(JNIEnv *e, jobject t, jlong object, jint id){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;
	int val = 0, ret;

	dprint(LOG_LIBV4L, "[LIBV4L] Calling get_control_value(dev: %s, ctrl name:%s)\n", d->vdev->file,d->vdev->controls->ctrl[id].name);
	ret = get_control_value(d->vdev,&(d->vdev->controls->ctrl[id]), &val);

	if(ret != 0) {
		if(ret == LIBV4L_ERR_STREAMING)
			THROW_EXCEPTION(e, CTRL_EXCP, "Can not get value for control '%s' while streaming", d->vdev->controls->ctrl[id].name);
		else
			THROW_EXCEPTION(e, CTRL_EXCP, "Error getting current value for control '%s'", d->vdev->controls->ctrl[id].name);
		return -1;
	}

	dprint(LOG_LIBV4L, "[LIBV4L] get_control_value(dev: %s, ctrl name:%s) = %d\n", d->vdev->file,d->vdev->controls->ctrl[id].name, val);
	return val;
}


/*
 * Set a new value on a v4l2 control
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_Control_doSetValue(JNIEnv *e, jobject t, jlong object, jint id, jint value){
	int ret;
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) object;

	dprint(LOG_LIBV4L, "[LIBV4L] Calling set_control_value(dev: %s, ctrl name:%s, val: %d)\n", d->vdev->file,d->vdev->controls->ctrl[id].name,value);
	ret = set_control_value(d->vdev, &(d->vdev->controls->ctrl[id]), value);
	if(ret != 0) {
		if(ret == LIBV4L_ERR_OUT_OF_RANGE)
			THROW_EXCEPTION(e, CTRL_EXCP, "Error: value %d for control '%s' out of range", value, d->vdev->controls->ctrl[id].name);
		else
			THROW_EXCEPTION(e, CTRL_EXCP, "Error setting current value for control '%s'", d->vdev->controls->ctrl[id].name);
	}
}


