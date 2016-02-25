#include <jni.h>
#include "common.h"
#include "debug.h"

struct x264_encoder {
	x264_picture_t pictureIn;
	x264_picture_t pictureOut;
	x264_nal_t* nals;
};

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_H264Encoder_doInit(JNIEnv *env, jobject self, jint csp, jint outWidth, jint outHeight) {
	struct x264_encoder* encoder;
	XMALLOC(encoder, struct x264_encoder*, sizeof(struct x264_encoder));
	
	x264_picture_alloc(&(encoder->pictureIn), csp, outWidth, outHeight);
	
	return (uintptr_t) encoder;
}
