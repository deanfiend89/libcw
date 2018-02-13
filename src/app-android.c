/* San Angeles Observation OpenGL ES version example
 * Copyright 2009 The Android Open Source Project
 * All rights reserved.
 *
 * This source is free software; you can redistribute it and/or
 * modify it under the terms of EITHER:
 *   (1) The GNU Lesser General Public License as published by the Free
 *       Software Foundation; either version 2.1 of the License, or (at
 *       your option) any later version. The text of the GNU Lesser
 *       General Public License is included with this source in the
 *       file LICENSE-LGPL.txt.
 *   (2) The BSD-style license that is included with this source in
 *       the file LICENSE-BSD.txt.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 * LICENSE-LGPL.txt and LICENSE-BSD.txt for more details.
 */
#include <stdlib.h>
#include <jni.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <android/log.h>
#include <android/input.h>
#include <stdint.h>
#include <dlfcn.h>
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <pthread.h>


#include <window.h>
#include <window_input.h>
#define APP_API __attribute__((visibility("default")))

#define LOG_TAG "ANGELES"
#define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
//#define DBG LOG_INFO("Here! (%s/%d)\n", __FILE__, __LINE__)

//static windowControl* control;
static GLuint rootTexture;

static int sWindowWidth  = 320;
static int sWindowHeight = 480;
static int sNativeRunning = 0;

static JavaVM* jvm = NULL;
static thread_attached = 0;
static jobject DemoGLSurfaceView_instance;
static jmethodID pleaseRender_ID;

static void *listener;
static pthread_t updater_thread_id;
/*
APP_API void
Java_com_example_SanAngeles_DemoGLSurfaceView_nativeRegisterClassInstance(JNIEnv* env, jobject obj){
	//(*env)->GetJavaVM(env, &jvm);
	DemoGLSurfaceView_instance = (*env)->NewGlobalRef(env, obj);	
	if (pleaseRender_ID == NULL){
		jclass cls = (*env)->GetObjectClass(env, DemoGLSurfaceView_instance);
		pleaseRender_ID = (*env)->GetMethodID(env, cls, "pleaseRender", "()V");
	}
	if (pleaseRender_ID == NULL) {
		utils_log_error("Can not find pleaseRender() method\n");
		return;
	}
}

static void requestRender(void){
	static JNIEnv* env;
	if (env == NULL && pleaseRender_ID == NULL){
		jint res = (*jvm)->AttachCurrentThread(jvm, &env, NULL);
		thread_attached = 1;
		if (res < 0) {	
			utils_log_error("Can not attach JavaVM thread\n");
			thread_attached = 0;
			(*jvm)->DetachCurrentThread(jvm);
		}
	}
	if (thread_attached) (*env)->CallVoidMethod(env, DemoGLSurfaceView_instance, pleaseRender_ID);
}


static void *updater_thread(void *cookie)
{
	for (;;){
		if (!sNativeRunning) { 
			usleep(1000*100); 
			continue;
		}
		int err = ACustomNativeWindow_frame_available(listener);
		switch(err){
			case 1: requestRender(); break;
			case CW_ERROR_PROCESS_DIED: utils_log_error("Parent process died: exiting...\n"); exit(0); break;
			default: utils_log_error("Unknown response\n"); goto finish; break;
		}
	}
	finish: thread_attached = 0; (*jvm)->DetachCurrentThread(jvm);
}*/

/* Call to initialize the graphics state */
APP_API void
//Java_com_example_SanAngeles_DemoRenderer_nativeInit(JNIEnv* env, jobject obj)
Java_com_example_SanAngeles_DemoRenderer_onSurfaceCreated(JNIEnv* env, jobject obj, jlong GL, jlong config)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rootTexture);
    listener = ACustomNativeWindow_getListener("/dev/shm/sock-shared");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //pthread_create(&updater_thread_id, NULL, updater_thread, NULL);
}

APP_API void
//Java_com_example_SanAngeles_DemoRenderer_nativeResize( JNIEnv*  env, jobject  thiz, jint w, jint h )
Java_com_example_SanAngeles_DemoRenderer_onSurfaceChanged( JNIEnv*  env, jobject  thiz, jlong GL, jint w, jint h )
{
    sWindowWidth  = w;
    sWindowHeight = h;
    __android_log_print(ANDROID_LOG_INFO, "SanAngeles", "resize w=%d h=%d", w, h);
}

/* Call to finalize the graphics state */
APP_API void
Java_com_example_SanAngeles_DemoRenderer_nativeDone( JNIEnv*  env )
{
	
}

APP_API void
Java_com_example_SanAngeles_DemoGLSurfaceView_nativeInput(JNIEnv* env, jobject thiz, jint action, jfloat x, jfloat y)
{
	char *act;
	utils_log_print("Action = %d\n", action);
	switch(action){
		case AMOTION_EVENT_ACTION_UP: act = "UP"; break;
		case AMOTION_EVENT_ACTION_DOWN: act = "DOWN"; break;
		case AMOTION_EVENT_ACTION_MOVE: act = "MOVE"; break;
		default: utils_log_error("Unknown event %d\n", action);
	}
	utils_log_print("X: %.0f Y: %.0f A: %s\n", x, y, act);
	input_event_write(x, y, action);
}

APP_API void
Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env )
{
	sNativeRunning = 0;
}

APP_API void
Java_com_example_SanAngeles_DemoGLSurfaceView_nativeResume( JNIEnv*  env )
{
	sNativeRunning = 1;
}

APP_API void
//Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env, jobject thiz)
Java_com_example_SanAngeles_DemoRenderer_onDrawFrame(JNIEnv* env, jobject thiz, jlong GL /*unused*/)
{
	if (!sNativeRunning) return;
	int err = ACustomNativeWindow_frame_available(listener);
	switch(err){
		case 1: glDrawTex(); break;
		case CW_ERROR_PROCESS_DIED: utils_log_error("Parent process died: exiting...\n"); exit(0); break;
		default: utils_log_error("Unknown response\n");
	}
}
