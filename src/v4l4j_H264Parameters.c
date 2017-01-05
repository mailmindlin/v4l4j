#include <stdint.h>
#include <jni.h>
#include "common.h"
#include "debug.h"
#include <x264.h>

//Weak globals
static jclass H264Parameters_class = NULL;
static jfieldID H264Parameters_object_fid = NULL;

static x264_param_t* getPointer(JNIEnv* env, jobject self) {
	jclass localRef = (*env)->NewLocalRef(env, H264Parameters_class);
	if (localRef == NULL) {
		//We don't have a reference to class H264Parameters
		localRef = (*env)->GetObjectClass(env, self);
		H264Parameters_object_fid = (*env)->GetFieldID(env, localRef, "object", "J");
		//Create a new weak global reference
		H264Parameters_class = (*env)->NewWeakGlobalRef(env, localRef);
	}
	
	long ptr = (*env)->GetLongField(env, self, H264Parameters_object_fid);
	dprint(LOG_V4L4J, "[PTR] Got pointer address to x264_param_t: %#08x\n", ptr);
	(*env)->DeleteLocalRef(env, localRef);
	return (struct x264_param_t*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_allocate(JNIEnv * env, jclass clazz) {
	LOG_FN_ENTER();
	
	struct x264_param_t* params;
	XMALLOC(params, x264_param_t*, sizeof(x264_param_t));
	
	return (uintptr_t) params;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_close(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	XFREE(params);
	return;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_initDefault(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	x264_param_default(params);
}
/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    initWithPreset
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_initWithPreset(JNIEnv* env, jobject self, jint preset, jint tune) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	if (preset < 0 || preset >= ARRAY_SIZE(x264_preset_names) - 1) {
		THROW_EXCEPTION(e, INVALID_VALUE, "Invalid value (%d) for preset.", preset);
		return -1;
	}
	
	if (tune < 0 || tune >= ARRAY_SIZE(x264_tune_names) - 1) {
		THROW_EXCEPTION(e, INVALID_VALUE, "Invalid value (%d) for tune.", tune)
		return -1;
	}
	char* preset_name = x264_preset_names[preset];
	char* tune_name = x264_tune_names[tune];
	
	dprint(LOG_V4L4J, "[PARAM] Initializing with preset '%s', tune '%s'\n", preset_name, tune_name);
	int result = x264_param_default_preset(params, preset_name, tune_name);
	if (result != 0) {
		THROW_EXCEPTION(env, JNI_EXCP, "Error initializing with preset #%d (%s) (error code %d)", preset, preset_name, result);
		return;
	}
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    applyFastFirstPass
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_applyFastFirstPass(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	x264_param_apply_fastfirstpass(params);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_applyProfile(JNIEnv* env, jobject self, jint profile) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	//x264_profile_names is 0-terminated, so subtract 1 from its size
	if (profile < 0 || profile >= ARRAY_SIZE(x264_profile_names) - 1)
		return;
	
	const char* profile_name = x264_profile_names[profile];
	
	dprint(LOG_V4L4J, "[PARAM] Applying profile '%s'\n", profile_name);
	int result = x264_param_apply_profile(params, profile_name);
	if (result != 0) {
		THROW_EXCEPTION(env, JNI_EXCP, "Cannot set profile #%d (%s) on x264 (error code %d)", profile, profile_name, result);
		return;
	}
}

static inline jint setParamByName(x264_param_t* params, JNIEnv* env, jstring key, const char* value) {
	const jchar* c_key = (*env)->GetStringChars(env, key, NULL);
	
	int result = x264_param_parse(params, (char*) c_key, value);
	
	(*env)->ReleaseStringChars(env, key, c_key);
	
	return result;
}
/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setParamByName
 * Signature: (JLjava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setParamByName__JLjava_lang_String_2Z(JNIEnv* env, jobject self, jstring name, jboolean value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	return setParamByName(params, env, name, (const char*) &value);
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setParamByName
 * Signature: (JLjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setParamByName__JLjava_lang_String_2I(JNIEnv* env, jobject self, jstring name, jint value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	return setParamByName(params, env, name, (const char*) &value);
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setParamByName
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setParamByName__JLjava_lang_String_2Ljava_lang_String_2(JNIEnv* env, jobject self, jstring name, jstring value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	const jchar* c_value = (*env)->GetStringChars(env, value, NULL);
	int result = setParamByName(params, env, name, (const char*) &value);
	(*env)->ReleaseStringChars(env, value, c_value);
	
	return result;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setInputDimension
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setInputDimension(JNIEnv* env, jobject self, jint width, jint height) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->i_width = width;
	params->i_height = height;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setCsp(JNIEnv* env, jobject self, jint csp) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->i_csp = csp;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setVfrInput(JNIEnv* env, jobject self, jboolean value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->b_vfr_input = value;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setRepeatHeaders(JNIEnv* env, jobject self, jboolean value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->b_repeat_headers = value;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setAnnexb(JNIEnv* env, jobject self, jboolean value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->b_annexb = value;
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_getCsp(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	return params->i_csp;
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_getWidth(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	return params->i_width;
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_getHeight(JNIEnv* env, jobject self) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	return params->i_height;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setCrop
 * Signature: (JIIII)V
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setCrop(JNIEnv* env, jobject self, jint left, jint top, jint right, jint bottom) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->crop_rect.i_left = left;
	params->crop_rect.i_top = top;
	params->crop_rect.i_right = right;
	params->crop_rect.i_bottom = bottom;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setThreads(JNIEnv* env, jobject self, jint numThreads) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->i_threads = numThreads;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setFps(JNIEnv* env, jobject self, jint numerator, jint denominator) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->i_fps_num = numerator;
	params->i_fps_den = denominator;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setKeyintMax(JNIEnv* env, jobject self, jint value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->i_keyint_max = value;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setIntraRefresh(JNIEnv* env, jobject self, jboolean value) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->b_intra_refresh = value;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setRfConstant(JNIEnv* env, jobject self, jfloat value, jfloat max) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->rc.f_rf_constant = value;
	params->rc.f_rf_constant_max = max;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setRcMethod(JNIEnv* env, jobject self, jint method) {
	LOG_FN_ENTER();
	x264_param_t* params = getPointer(env, self);
	
	params->rc.i_rc_method = method;
}