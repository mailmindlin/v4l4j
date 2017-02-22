/*
* Copyright (C) 2017 mailmindlin
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
*/
#include <jni.h>
#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "debug.h"
#include "jniutils.h"

#ifndef _Included_au_edu_jcu_v4l4j_impl_v4l_FrameGrabber
#define _Included_au_edu_jcu_v4l4j_impl_v4l_FrameGrabber
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_AbstractGrabber_doInit(JNIEnv *e, jobject self, jlong object, jint width, jint height, jint channel, jint std, jint numBuffers) {
	LOG_FN_ENTER();
	
	struct video_device* vdev = (struct video_device*) (uintptr_t) object;
	
	/*
	 * i n i t _ c a p t u r e _ d e v i c e ( )
	 */
	dprint(LOG_LIBVIDEO, "[LIBVIDEO] Calling init_capture_device()\n");
	struct capture_device *capture = init_capture_device(vdev, width, height, channel, std, numBuffers);
	
	if(c == NULL) {
		dprint(LOG_V4L4J, "[V4L4J] init_capture_device failed\n");
		THROW_EXCEPTION(e, INIT_EXCP, "Error initializing device '%s'. Make sure it is a valid V4L device file and check the file permissions.", d->vdev->file);
		return -1;
	}
	
	return (jlong) (uintptr_t) capture;
}


#ifdef __cplusplus
}
#endif
#endif//Close include guard