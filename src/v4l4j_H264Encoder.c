#include <stdint.h>
#include <jni.h>
#include "common.h"
#include "debug.h"
#include <x264.h>

static jclass H264Encoder_class = NULL;
static jfieldID H264Encoder_object_fid = NULL;

#define H264_PACKAGE V4L4J_PACKAGE "encoder/h264/"
#define H264_ENCODE_FAIL_EXCEPTION H264_PACKAGE "H264EncodeFailedException"
#define H264_NO_NALS_EXCEPTION H264_PACKAGE "H264NoNalsException"

static inline jclass getH264EncoderClass(JNIEnv* env, jobject self) {
	if (H264Encoder_class == NULL && !(H264Encoder_class = (*env)->GetObjectClass(env, self)))
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up class H264Encoder");
	return H264Encoder_class;
}

static inline x264_t* getPointer(JNIEnv* env, jobject self) {
	if (H264Encoder_object_fid == NULL) {
		if ((H264Encoder_object_fid = (*env)->GetFieldID(env, getH264EncoderClass(env, self), "object", "J")) == NULL) {
			info("[V4L4J] Error looking up the object fieldID of H264Encoder\n");
			THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the object fieldID of H264Encoder");
		}
	}
	
	long ptr = (*env)->GetLongField(env, self, H264Encoder_object_fid);
	return (x264_t*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doInit(JNIEnv * env, jclass me, jlong parameters) {
	LOG_FN_ENTER();
	
	x264_param_t* params = (struct x264_param_t*)(uintptr_t) parameters;
	
	x264_t* encoder = x264_encoder_open(params);
	
	if (!encoder)
		THROW_EXCEPTION(env, INIT_EXCP, "Error opening x264 encoder");
	
	return (jlong) (uintptr_t) encoder;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doGetParams(JNIEnv * env, jclass me, jlong object) {
	LOG_FN_ENTER();
	x264_t* encoder = (x264_t*) (uintptr_t) object;
	
	x264_param_t* result;
	XMALLOC(result, struct x264_param_t*, sizeof(struct x264_param_t));
	
	x264_encoder_parameters(encoder, result);
	
	return (uintptr_t) result;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doSetParams(JNIEnv * env, jclass me, jlong object, jlong parameters) {
	LOG_FN_ENTER();
	x264_t* encoder = (x264_t*) (uintptr_t) object;
	
	x264_param_t* params = (x264_param_t*) (uintptr_t) parameters;
	
	x264_encoder_reconfig(encoder, params);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_close(JNIEnv * env, jobject self) {
	LOG_FN_ENTER();
	x264_t* encoder = getPointer(env, self);
	
	x264_encoder_close(encoder);
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Encoder_doEncode(JNIEnv * env, jclass me, jlong object, jlong in_ptr, jobject output) {
	LOG_FN_ENTER();
	x264_t* encoder = (x264_t*) (uintptr_t) object;
	
	x264_picture_t* pic_in = (struct x264_picture_t*) (uintptr_t) in_ptr;
	
	int num_nals;
	x264_nal_t* nals;
	x264_picture_t pic_out;
	int frame_size = x264_encoder_encode(encoder, &nals, &num_nals, pic_in, &pic_out);
	
	if (frame_size < 0) {
		THROW_EXCEPTION(env, H264_ENCODE_FAIL_EXCEPTION, "Error code %d", frame_size);
		return frame_size;
	} else if (!nals) {
		THROW_EXCEPTION(env, H264_NO_NALS_EXCEPTION, "x264_encoder_encode returned no valid nals. (error %d)", frame_size);
		return frame_size;
	}
	
	unsigned int written = (unsigned) frame_size;
	
	// Get pointer to bytebuffer data
	jbyteArray arrayRef;
	unsigned int output_len;
	void (*output_release)(JNIEnv* env, jbyteArray arrayRef, unsigned char* ptr);
	u8* output_buffer = getBufferPointer(env, output, &arrayRef, &output_len, &output_release);
	if (!output_buffer) {
		THROW_EXCEPTION(env, ARG_EXCP, "Could not get pointer to buffer");
		return -1;
	} else if (written > output_len) {
		THROW_EXCEPTION(env, OVERFLOW_EXCP, "Frame size was %d bytes, buffer was %d", frame_size, output_len);
		(*output_release)(env, arrayRef, output_buffer);
		return -1;
	}
	//TODO change to memcpy_s?
	memcpy(output_buffer, nals->p_payload, frame_size);
	
	while (x264_encoder_delayed_frames(encoder) && written < output_len) {
		frame_size = x264_encoder_encode(encoder, &nals, &num_nals, NULL, &pic_out);
		if (nals && frame_size > 0 && ((written + frame_size) < output_len)) {
			memcpy(&(output_buffer[written]), nals->p_payload, frame_size);
			written += (unsigned) frame_size;
		} else {
			if (frame_size < 0)
				THROW_EXCEPTION(env, H264_ENCODE_FAIL_EXCEPTION, "Error code %d", frame_size);
			if (!nals)
				THROW_EXCEPTION(env, H264_NO_NALS_EXCEPTION, "x264_encoder_encode returned no valid nals. (error %d)", frame_size);
			if (written + frame_size > output_len)
				THROW_EXCEPTION(env, OVERFLOW_EXCP, "Frame size was %d bytes, buffer was %d", frame_size, output_len);
			(*output_release)(env, arrayRef, output_buffer);
			return written;
		}
	}
	(*output_release)(env, arrayRef, output_buffer);
	return written;
}