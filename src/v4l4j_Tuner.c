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

#include "common.h"
#include "debug.h"

static jfieldID Tuner_object_fid = NULL;

static struct v4l4j_device* getPointer(JNIEnv* env, jobject self) {
	if (Tuner_object_fid == NULL) {
		jclass Tuner_class = (*env)->FindClass(env, "au/edu/jcu/v4l4j/Tuner");
		Tuner_object_fid = (*env)->GetFieldID(env, Tuner_class, "object", "J");
	}
	
	long ptr = (*env)->GetLongField(env, self, Tuner_object_fid);
	return (struct v4l4j_device*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_Tuner_getFreq(JNIEnv* env, jclass me, jlong object, jint index){
	LOG_FN_ENTER();

	struct v4l4j_device *d = (struct v4l4j_device*) (uintptr_t) object;
	unsigned int f;
	if((*d->vdev->tuner_action->get_tuner_freq)(d->vdev,index, &f)!=0){
		dprint(LOG_V4L4J, "[V4L4J] failed getting tuner frequency\n");
		THROW_EXCEPTION(env, GENERIC_EXCP, "Error getting tuner frequency");
		return 0;
	}
	dprint(LOG_V4L4J, "[V4L4J] got freq: raw: %u long long: %llu %lld\n", f, (unsigned long long)(f & 0xffffffff), (long long)(f & 0xffffffff));
	return (jlong) (f & 0xffffffff);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_Tuner_setFreq(JNIEnv* env, jobject self, jint index, jlong f){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = getPointer(env, self);

	dprint(LOG_CALLS, "[V4L4J] setting freq to %lld - %u\n", (long long) f, (unsigned int) (f&0xffffffff));
	if((*d->vdev->tuner_action->set_tuner_freq)(d->vdev, index, (unsigned int) (f & 0xffffffff))!= 0){
		dprint(LOG_V4L4J, "[V4L4J] failed setting tuner frequency\n");
		THROW_EXCEPTION(env, GENERIC_EXCP, "Error setting tuner frequency");
	}
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_Tuner_getAfc(JNIEnv* env, jobject self, jint index){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = getPointer(env, self);
	int r,a;

	if((*d->vdev->tuner_action->get_rssi_afc)(d->vdev, index, &r, &a) != 0){
		dprint(LOG_V4L4J, "[V4L4J] failed getting AFC\n");
		THROW_EXCEPTION(env, GENERIC_EXCP, "Error getting AFC");
		return 0;
	}
	return a;
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_Tuner_getRssi(JNIEnv* env, jobject self, jint index){
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	struct v4l4j_device *d = getPointer(env, self);
	int r,a;

	if((*d->vdev->tuner_action->get_rssi_afc)(d->vdev, index, &r, &a)!= 0){
		dprint(LOG_V4L4J, "[V4L4J] failed getting RSSI\n");
		THROW_EXCEPTION(env, GENERIC_EXCP, "Error getting RSSI");
		return 0;
	}
	return r;
}


