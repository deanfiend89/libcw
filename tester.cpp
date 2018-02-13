/*
 *  This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * Project home page: http://github.com/twaik/libcw
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <math.h>
#include <sys/resource.h>

#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <WindowSurface.h>
#include <EGLUtils.h>

#include <sys/file.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "utils.h"
#include "cutils/ashmem.h"
#include "libancillary/ancillary.h"
#include "ipc.h"
#include "windowInternals.h"

using namespace android;

static void checkEglError(const char* op, EGLBoolean returnVal = EGL_TRUE) {
	if (returnVal != EGL_TRUE) { utils_log_error("%s() returned %d\n", op, returnVal); }

	for (EGLint error = eglGetError(); error != EGL_SUCCESS; error = eglGetError()) {
		utils_log_error("after %s() eglError %s (0x%x)\n", op, egl_strerror(error), error);
	}
}

EGLDisplay dpy;
EGLContext context;
EGLSurface surface;
EGLint w, h;

EGLBoolean returnValue;
EGLConfig myConfig = {0};

EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
EGLint s_configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE };
EGLint majorVersion;
EGLint minorVersion;

static int initGLES(){
	checkEglError("<init>");
	dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY); checkEglError("eglGetDisplay");
	if (dpy == EGL_NO_DISPLAY) {
		utils_log_error("eglGetDisplay returned EGL_NO_DISPLAY.\n");
		exit(0);
	}

	returnValue = eglInitialize(dpy, &majorVersion, &minorVersion); checkEglError("eglInitialize", returnValue);
	utils_log_error("EGL version %d.%d\n", majorVersion, minorVersion);
	if (returnValue != EGL_TRUE) {
		utils_log_error("eglInitialize failed\n");
		exit(0);
	}

	static WindowSurface windowSurface; //static is needed to prevent windowSurface destructing. 

	EGLNativeWindowType window = windowSurface.getSurface();
	returnValue = selectConfigForNativeWindow(dpy, s_configAttribs, window, &myConfig);
	if (returnValue) { utils_log_error("selectConfigForNativeWindow() returned %d", returnValue); exit(0); }; checkEglError("selectConfigForNativeWindow");

	surface = eglCreateWindowSurface(dpy, myConfig, window, NULL); checkEglError("eglCreateWindowSurface");
	if (surface == EGL_NO_SURFACE) { utils_log_error("gelCreateWindowSurface failed.\n"); exit(0); }

	context = eglCreateContext(dpy, myConfig, EGL_NO_CONTEXT, context_attribs); checkEglError("eglCreateContext");
	if (context == EGL_NO_CONTEXT) { utils_log_error("eglCreateContext failed\n"); exit(0); }
	returnValue = eglMakeCurrent(dpy, surface, surface, context); checkEglError("eglMakeCurrent", returnValue);
	if (returnValue != EGL_TRUE) { exit(0); }
	
	eglQuerySurface(dpy, surface, EGL_WIDTH, &w); checkEglError("eglQuerySurface");
	eglQuerySurface(dpy, surface, EGL_HEIGHT, &h); checkEglError("eglQuerySurface");
	
	GLint dim = w < h ? w : h; utils_log_error("Window dimensions: %d x %d\n", w, h);

	return 0;
}

ANativeWindowBuffer_t* buf;
GLuint bufferTexture;

int main(int argc, char** argv){
	initGLES();
	
	/*ANativeWindowBuffer* rrbuf;
	rrbuf = buffer_read();
	
	int controlFD;
	ACustomNativeWindow_read_control(&controlFD);
	printf("ControlFD is %d\n", controlFD);
	windowControl* control = (windowControl*) mmap(NULL, sizeof(windowControl), PROT_READ | PROT_WRITE, MAP_SHARED, controlFD, 0);
	if (control == MAP_FAILED) {
		utils_log_error("Failed to map shared window control\n");
	}

	printf("ParentPID is %d\n", control->parentPID);
	
	//glGenTextures(1, &rootTexture);
	glBindTexture(GL_TEXTURE_2D, rootTexture);
	glBindNativeBufferToTex(rrbuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
	
	/*for (;;){
			control->frameAvailable = 0;
			glDrawTex();
			eglSwapBuffers (dpy, surface);  // get the rendered buffer to the screen
		if (getpgid(control->parentPID) < 0) { printf("Server exited, closing...\n"); exit(0); }

	}*/
	GLuint rootTexture;
	void *listener;
	
	glActiveTexture(GL_TEXTURE0);DBG;
    glBindTexture(GL_TEXTURE_2D, rootTexture);DBG;
    listener = ACustomNativeWindow_getListener("/dev/shm/sock-shared");DBG;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);DBG;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);DBG;
    int err;
	while (1)
	{
		err = ACustomNativeWindow_frame_available(listener);
		switch(err){
			case 1: glDrawTex(); break;
			case CW_ERROR_PROCESS_DIED: utils_log_error("Parent process died: exiting...\n"); exit(0); break;
			default: utils_log_error("Unknown response\n");
		}
		eglSwapBuffers (dpy, surface); 
		//utils_log_print("err = %d\n", err);
		//if (getpgid(control->parentPID) < 0) { printf("Server exited, closing...\n"); exit(0);
	}
	return 1;
}
