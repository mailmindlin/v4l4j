#include <stdint.h>
#include <jni.h>
#include "common.h"
#include "debug.h"
#include <x264.h>

static jclass H264Picture_class = NULL;
static jfieldID H264Picture_object_fid = NULL;

static x264_picture_t* getPointer(JNIEnv* env, jobject self) {
	if (H264Picture_class == NULL) {
		H264Picture_class = (*env)->GetObjectClass(env, self);
		H264Picture_object_fid = (*env)->GetFieldID(env, H264Picture_class, "object", "J");
	}
	
	long ptr = (*env)->GetLongField(env, self, H264Picture_object_fid);
	return (struct x264_picture_t*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_init(JNIEnv* env, jclass me, jint csp, jint width, jint height) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_picture_t* result;
	XMALLOC(result, x264_picture_t*, sizeof(x264_picture_t));
	
	x264_picture_alloc(result, csp, width, height);
	
	return (uintptr_t) result;
}
JNIEXPORT void JNICALL JAva_au_edu_jcu_v4l4j_encoder_h264_H264Picture_putInPlane(JNIEnv* env, jclass me, jlong ptr, jint plane, jobject buf) {
	LOG_FN_ENTER();
	x264_picture_t* picture = (x264_picture_t*) (uintptr_t) ptr;
	
	if (!buf) {
		THROW_EXCEPTION(env, NULL_EXCP, "ByteBuffer must not be null");
		return;
	}
	void* data = (*env)->GetDirectBufferAddress(env, buf);
	if (!data) {
		THROW_EXCEPTION(env, ARG_EXCP, "ByteBuffer must be direct");
		return;
	}
	
	picture.img.plane[plane] = data;
}

JNIEXPORT void JNICALL JAva_au_edu_jcu_v4l4j_encoder_h264_H264Picture_doSetAndGetPts(JNIEnv* env, jclass me, jobject ptr, jint pts) {
	LOG_FN_ENTER();
	x264_picture_t* picture = (x264_picture_t*) (uintptr_t) ptr;
	if (pts > 0)
		picture->i_pts = pts;
	return picture->i_pts;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_close(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	x264_picture_t* picture = getPointer(env, self);
	
	x264_picture_clean(picture);
	XFREE(picture);
}