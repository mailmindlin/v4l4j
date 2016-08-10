#include <stdint.h>
#include <jni.h>
#include "common.h"
#include "debug.h"
#include <x264.h>

static jclass H264Encoder_class = NULL;
static jfieldID H264Encoder_object_fid = NULL;
static jfieldID H264Encoder_frameNum_fid = NULL;

#define H264_PACKAGE V4L4J_PACKAGE "encoder/h264/"
#define H264_ENCODE_FAIL_EXCEPTION H264_PACKAGE "H264EncodeFailedException"
#define H264_NO_NALS_EXCEPTION H264_PACKAGE "H264NoNalsException"

static x264_t* getPointer(JNIEnv* env, jobject self) {
	if (H264Encoder_class == NULL)
		H264Encoder_class = (*env)->GetObjectClass(env, self);
	if (H264Encoder_object_fid == NULL)
		H264Encoder_object_fid = (*env)->GetFieldID(env, H264Encoder_class, "object", "J");
	
	long ptr = (*env)->GetLongField(env, self, H264Encoder_object_fid);
	return (x264_t*) (uintptr_t) ptr;
}
static int getAndIncrementFrameNum(JNIEnv* env, jobject self) {
	if (H264Encoder_class == NULL)
		H264Encoder_class = (*env)->GetObjectClass(env, self);
	if (H264Encoder_frameNum_fid == NULL)
		H264Encoder_frameNum_fid = (*env)->GetFieldID(env, H264Encoder_class, "frameNum", "I");
	
	int value = (*env)->GetIntField(env, self, H264Encoder_frameNum_fid);
	(*env)->SetIntField(env, self, H264Encoder_frameNum_fid, value + 1);
	return value;
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
	
	return (uintptr_t) result;
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
	
	x264_picture_t* pic_in = (struct x264_picture_t*) (uintptr_t) in_ptr;
	x264_picture_t* pic_out = (struct x264_picture_t*) (uintptr_t) in_ptr;
	
	pic_in.i_pts = getAndIncrementFrameNum(env, self);
	
	int i_nal;
	x264_nal_t* nals;
	int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, pic_in, pic_out);
	
	if (frame_size < 0)
		THROW_EXCEPTION(env, H264_ENCODE_FAIL_EXCEPTION, "Error code %d", frame_size);
	else if (!nals)
		THROW_EXCEPTION(env, H264_NO_NALS_EXCEPTION, "x264_encoder_encode returned no valid nals.");
	
	return frame_size;
}