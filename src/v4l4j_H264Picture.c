#include <jni.h>
#include "common.h"
#include "debug.h"
#include "x264.h"

static jclass H264Picture_class = NULL;
static jfieldID H264Picture_object_fid = NULL;

static x264_picture_t* getPointer(JNIEnv* env, jobject self) {
	if (H264Picture_class == NULL) {
		H264Picture_class = (*env)->GetObjectClass(env, self);
		H264Picture_object_fid = (*env)->GetFieldID(env, H264Picture_class, "object", "L");
	}
	
	long ptr = (*env)->GetLongField(env, self, H264Picture_object_fid);
	return (struct x264_picture_t*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_init(JNIEnv* env, jobject self, jint csp, jint width, jint height) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_picture_t* result;
	
	x264_picture_alloc(result, csp, width, height);
	x264_picture_init(result);
	
	return (uintptr_t) result;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_close(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_picture_t* picture = getPointer(env, self);
	
	x264_picture_clean(picture);
}