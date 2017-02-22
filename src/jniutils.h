/**
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __JNIUTILS_H
#define __JNIUTILS_H

jmethodID lookupAddMethod(JNIEnv *env, jobject list) __attribute__ ((pure, visibility ("hidden")));
jclass lookupClassSafe(JNIEnv *env, jobject obj) __attribute__ ((pure, visibility ("hidden")));
int getBufferPosition(JNIEnv *env, jobject buffer) __attribute__ ((pure, visibility ("hidden")));
void setBufferPosition(JNIEnv *env, jobject buffer, int position) __attribute__((visibility ("hidden")));
void setBufferLimit(JNIEnv *env, jobject buffer, int limit) __attribute__((visibility ("hidden")));
int getBufferLimit(JNIEnv *env, jobject limit) __attribute__ ((pure, visibility ("hidden")));
unsigned char* getBufferPointer(JNIEnv *env, jobject buffer, jbyteArray* arrayRef, unsigned int* off, unsigned int* len, void (**release)(JNIEnv* env, jbyteArray arrayRef, unsigned char* ptr)) __attribute__((visibility ("hidden")));
#endif