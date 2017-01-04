#include <stdint.h>
#include <jni.h>
#include "common.h"
#include "debug.h"
#include <x264.h>

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_init(JNIEnv* env, jclass me, jint csp, jint width, jint height) {
	LOG_FN_ENTER();
	x264_picture_t* result;
	XMALLOC(result, x264_picture_t*, sizeof(x264_picture_t));
	
	x264_picture_alloc(result, csp, width, height);
	
	return (uintptr_t) result;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_putInPlane(JNIEnv* env, jclass me, jlong ptr, jint plane, jobject output) {
	LOG_FN_ENTER();
	x264_picture_t* picture = (x264_picture_t*) (uintptr_t) ptr;
	
	if (plane < 0 || plane > picture->img.i_plane) {
		THROW_EXCEPTION(env, ARG_EXCP, "Invalid plane %d", plane);
		return;
	}
	jbyteArray arrayRef;
	unsigned int buffer_len;
	void (*output_release)(JNIEnv* env, jbyteArray arrayRef, unsigned char* ptr);
	u8* output_buffer = getBufferPointer(env, output, &arrayRef, &buffer_len, &output_release);
	if (!output_buffer) {
		THROW_EXCEPTION(env, ARG_EXCP, "Could not get pointer to buffer");
		return;
	}
	
	int buffer_offset = getBufferPosition(env, output);
	if (buffer_offset < 0)
		return;
	if ((unsigned) buffer_offset > buffer_len) {
		THROW_EXCEPTION(env, ARG_EXCP, "Buffer offset was greater than length");
		return;
	}
	
	//TODO check plane sizes
	unsigned int len = buffer_len - (unsigned) buffer_offset;
	dprint(LOG_V4L4J, "[V4L4J] Copying %u bytes to plane %d of image\n", len, plane);
	memcpy(picture->img.plane[plane], &(output_buffer[buffer_offset]), len);
	
	output_release(env, arrayRef, output_buffer);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_doSetAndGetPts(JNIEnv* env, jclass me, jobject ptr, jint pts) {
	LOG_FN_ENTER();
	x264_picture_t* picture = (x264_picture_t*) (uintptr_t) ptr;
	if (pts > 0)
		picture->i_pts = pts;
	return picture->i_pts;
}

JNIEXPORT jobject JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_doGetBuffer(JNIEnv* env, jclass me, jlong ptr, jint plane) {
	LOG_FN_ENTER();
	x264_picture_t* picture = (x264_picture_t*) (uintptr_t) ptr;
	if (plane < 0 || plane > picture->img.i_plane) {
		THROW_EXCEPTION(env, ARG_EXCP, "Invalid plane %d", plane);
		return NULL;
	}
	//TODO fix capacity
	return (*env)->NewDirectByteBuffer(env, picture->img.plane[plane], 1);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_putImage(JNIEnv* env, jclass me, jlong ptr, jint imf, jobject buffer) {
	LOG_FN_ENTER();
	x264_picture_t* picture = (x264_picture_t*) (uintptr_t) ptr;
	
	//TODO finish
}



JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Picture_close(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	jclass H264Picture_class = (*env)->GetObjectClass(env, self);
	if (H264Picture_class == NULL) {
		THROW_EXCEPTION(env, JNI_EXCP, "Unable to look up class H264Picture");
		return;
	}
	jfieldID H264Picture_object_fid = (*env)->GetFieldID(env, H264Picture_class, "object", "J");
	
	x264_picture_t* picture = (x264_picture_t*) (uintptr_t) (*env)->GetLongField(env, self, H264Picture_object_fid);
	x264_picture_clean(picture);
	XFREE(picture);
}