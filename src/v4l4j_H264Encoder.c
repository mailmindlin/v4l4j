#include <stdint.h>
#include <jni.h>
#include "common.h"
#include "debug.h"
#include <x264.h>

static jclass H264Encoder_class = NULL;
static jfieldID H264Encoder_object_fid = NULL;

static x264_t* getPointer(JNIEnv* env, jobject self) {
	if (H264Encoder_class == NULL) {
		H264Encoder_class = (*env)->GetObjectClass(env, self);
		H264Encoder_object_fid = (*env)->GetFieldID(env, H264Encoder_class, "object", "L");
	}
	
	long ptr = (*env)->GetLongField(env, self, H264Encoder_object_fid);
	return (x264_t*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doInit(JNIEnv * env, jobject self, jlong parameters) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	x264_param_t* params = (struct x264_param_t*)(uintptr_t) parameters;
	
	x264_t* encoder = x264_encoder_open(params);
	
	return (uintptr_t) encoder;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doGetParams(JNIEnv * env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_t* encoder = getPointer(env, self);
	
	x264_param_t* result;
	XMALLOC(result, struct x264_param_t*, sizeof(struct x264_param_t));
	
	x264_encoder_parameters(encoder, result);
	
	return result;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doSetParams(JNIEnv * env, jobject self, jlong parameters) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_t* encoder = getPointer(env, self);
	
	x264_param_t* params = (x264_param_t*) (uintptr_t) parameters;
	
	x264_encoder_reconfig(encoder, params);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_close(JNIEnv * env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_t* encoder = getPointer(env, self);
	
	x264_encoder_close(encoder);
}
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doEncode(JNIEnv * env, jobject self, jlong in_ptr, jlong out_ptr) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_t* encoder = getPointer(env, self);
	
	int i_nal;
	x264_nal_t** nals;
	