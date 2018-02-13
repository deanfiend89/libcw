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
#include <stdarg.h>
#include <EGL/egl.h>
#include <cutils/ashmem.h>
#include <libancillary/ancillary.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "windowInternals.h"
#include "utils.h"
#define CW_API __attribute__((visibility("default")))


static ACustomNativeWindow* current_shared_window;

static void winIncRef(struct android_native_base_t* base){
	base; //utils_log_print("Buffer %p incRef\n", base);
}

static void winDecRef(struct android_native_base_t* base){
	base; //utils_log_print("Buffer %p decRef\n", base);
}

static int winSetSwapInterval(struct ANativeWindow* window, int interval){
	window; interval;
	return 0;
}

static int winCancelBuffer(struct ANativeWindow* window, struct ANativeWindowBuffer* buffer, int fenceFd){
	window; buffer; fenceFd;
	utils_log_print("Window requested cancel()\n");
	return 0;
}

static int winCancelBuffer_DEPRECATED(struct ANativeWindow* window, struct ANativeWindowBuffer* buffer){
	int r, fenceFd = -1;
	r = winCancelBuffer(window, buffer, fenceFd);
	if (fenceFd != -1) close (fenceFd);
	return r;
}

int winDequeueBuffer(struct ANativeWindow* window, struct ANativeWindowBuffer** buffer, int* fenceFd) {
	ACustomNativeWindow *win = (ACustomNativeWindow*)window; fenceFd;
	//For case bufferCount = 1
	*buffer = win->buffers[0];
	//utils_log_print("Window requested dequeue()\n");
	return 0;
}

int winDequeueBuffer_DEPRECATED(struct ANativeWindow* window, struct ANativeWindowBuffer** buffer) {
	int r, fenceFd = -1;
	r = winDequeueBuffer(window, buffer, &fenceFd);
	if (fenceFd != -1) close (fenceFd);
	return r;
}

int winLockBuffer_DEPRECATED(struct ANativeWindow* window, struct ANativeWindowBuffer* buffer){
	window; buffer;
	utils_log_print("Window requested lock()\n");
	return 0;
}

int winQueueBuffer(struct ANativeWindow* window, struct ANativeWindowBuffer* buffer, int fenceFd){
	ACustomNativeWindow *win = (ACustomNativeWindow*)window; buffer; fenceFd;
	//utils_log_print("Window requested queue()\n");
	win->control->frameAvailable = 1;
	win->control->frameAvailableNum = 1;

	return 0;
}

int winQueueBuffer_DEPRECATED(struct ANativeWindow* window, struct ANativeWindowBuffer* buffer){
	int r, fenceFd = -1;
	r = winQueueBuffer(window, buffer, fenceFd);
	if (fenceFd != -1) close (fenceFd);
	return r;
}

int winPerform(struct ANativeWindow* window, int op, ... ){
	ACustomNativeWindow *win = (ACustomNativeWindow*)window;
	va_list ap;
	va_start(ap, op);

	switch (op) {
	case NATIVE_WINDOW_SET_USAGE:
		utils_log_error("set usage %d\n", va_arg(ap,int));
		return 0;
	case NATIVE_WINDOW_CONNECT:
	case NATIVE_WINDOW_DISCONNECT:
	case NATIVE_WINDOW_API_CONNECT:
	case NATIVE_WINDOW_API_DISCONNECT:
		return 0;
	case NATIVE_WINDOW_SET_BUFFERS_FORMAT:
		utils_log_error("set buffers format %d\n", va_arg(ap,int));
		return 0;
	case NATIVE_WINDOW_SET_BUFFERS_TRANSFORM:
		//utils_log_error("set buffers transform %d\n", va_arg(ap,int));
		return 0;
	case NATIVE_WINDOW_SET_BUFFERS_TIMESTAMP:
		utils_log_error("set buffers timestamp %lld\n", va_arg(ap,long long));
		return 0;
	case NATIVE_WINDOW_SET_SCALING_MODE:
		utils_log_error("set scaling mode %d\n", va_arg(ap,int));
		return 0;
	case NATIVE_WINDOW_SET_BUFFERS_DIMENSIONS: {
		unsigned int w = va_arg(ap,unsigned int);
		unsigned int h = va_arg(ap,unsigned int);
		if ((w == win->width) && (h == win->height)) {
			//utils_log_error("set buffers dimensions %d x %d\n", w, h);
			return 0;
		}
		utils_log_error("cannot resize buffers to %d x %d\n", w, h);
		return -1;
	}
	case NATIVE_WINDOW_SET_BUFFER_COUNT: {
		unsigned int count = va_arg(ap,unsigned int);
		utils_log_error("set buffer count %d\n", count);
		return 0;
	}
	default:
		utils_log_error("perform %d unknown!\n", op);
		return -ENODEV;
	}
	return -ENODEV;
}

int winQuery(const struct ANativeWindow* window, int what, int* value){
	ACustomNativeWindow *win = (ACustomNativeWindow*)window;
	switch (what) {
	case NATIVE_WINDOW_WIDTH:
	case NATIVE_WINDOW_DEFAULT_WIDTH:
		*value = win->width;
		//utils_log_error("query window width: %d\n", *value);
		return 0;
	case NATIVE_WINDOW_HEIGHT:
	case NATIVE_WINDOW_DEFAULT_HEIGHT:
		*value = win->height;
		//utils_log_error("query window height: %d\n", *value);
		return 0;
	case NATIVE_WINDOW_FORMAT:
		*value = win->format;
		//utils_log_error("query window format: %d\n", *value);
		return 0;
	case NATIVE_WINDOW_TRANSFORM_HINT:
		//utils_log_error("query transform hint: 0\n");
		*value = 0;
		return 0;
	case NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS:
		//utils_log_error("query min undequeued buffers: 1\n");
		*value = 1;
		return 0;
	case NATIVE_WINDOW_CONCRETE_TYPE:
		//utils_log_error("query concrete type: 0\n");
		*value = NATIVE_WINDOW_FRAMEBUFFER;
		return 0;
	case NATIVE_WINDOW_BUFFER_AGE:
		//utils_log_error("query buffer age: 0\n");
		*value = 2;
		return 0;
	default:
		*value = 0;
		//utils_log_error("query %d unknown!\n", what);
		return 0; //XXX Ignored error
	}
	return 0;
}

static void setIntValue(int *ptr, int value){
	memcpy (ptr, &value, sizeof(int));
}

static void setUint32Value(uint32_t *ptr, uint32_t value){
	memcpy (ptr, &value, sizeof(uint32_t));
}

static void setFloatValue(float *ptr, float value){
	memcpy (ptr, &value, sizeof(float));
}

CW_API EGLNativeWindowType ACustomNativeWindow_create(int w, int h){
	ACustomNativeWindow* win = malloc(sizeof(ACustomNativeWindow));
	win->base.common.magic = ANDROID_NATIVE_WINDOW_MAGIC;
	win->base.common.version = sizeof(ANativeWindow);
	memset(win->base.common.reserved, 0, sizeof(win->base.common.reserved));
	win->base.common.incRef = winIncRef;
	win->base.common.decRef = winDecRef;
	
	//These are read-only. 
	setUint32Value(&win->base.flags, 0);
	setIntValue(&win->base.minSwapInterval, 0);
	setIntValue(&win->base.maxSwapInterval, 0);
	setFloatValue(&win->base.xdpi, 0);
	setFloatValue(&win->base.ydpi, 0);
	/*win->base.flags = 0;
	win->base.minSwapInterval = 0;
	win->base.maxSwapInterval = 0;
	win->base.xdpi = 0;
	win->base.ydpi = 0;*/

	win->base.setSwapInterval = winSetSwapInterval;
	win->base.cancelBuffer = winCancelBuffer;
	win->base.cancelBuffer_DEPRECATED = winCancelBuffer_DEPRECATED;
	win->base.dequeueBuffer = winDequeueBuffer;
	win->base.dequeueBuffer_DEPRECATED = winDequeueBuffer_DEPRECATED;
	win->base.lockBuffer_DEPRECATED = winLockBuffer_DEPRECATED;
	win->base.queueBuffer = winQueueBuffer;
	win->base.queueBuffer_DEPRECATED = winQueueBuffer_DEPRECATED;
	
	win->base.perform = winPerform;
	win->base.query = winQuery;
	
	win->width = w;
	win->height = h;
	win->format = HAL_PIXEL_FORMAT_RGBA_8888;
	
	win->controlFD = ashmem_create_region("shared window control", sizeof(windowControl));
	if (win->controlFD <= 0) {
		utils_log_error("Failed to create shared window control\n");
	}
	
	win->control = mmap(NULL, sizeof(windowControl), PROT_READ | PROT_WRITE, MAP_SHARED, win->controlFD, 0);
	if (win->control == MAP_FAILED) {
		utils_log_error("Failed to map shared window control\n");
	}
	
	win->control->parentPID = getpid();
	
	win->bufferCount = 1;
	win->buffers[0] = (ANativeWindowBuffer_t*) buffer_alloc(win->width, win->height, win->format, GRALLOC_USAGE_HW_TEXTURE);
	utils_log_print("Window created\n");
	
	return (EGLNativeWindowType) win;
}

static void ACustomNativeWindow_read_callback(int replysock){
	if (ancil_send_fd(replysock, current_shared_window->controlFD)<0)
		utils_log_error("Failed to send shared window control FD\n");
}

CW_API void ACustomNativeWindow_start_sharing(EGLNativeWindowType window){
	ACustomNativeWindow* win = (ACustomNativeWindow*) window;
	buffer_set_shared(win->buffers[0]);
	buffer_register_read_callback();
	
	current_shared_window = win;
	ipc_register_callback(IPC_COMMAND_GET_SHARED_WINDOW_CONTROL, ACustomNativeWindow_read_callback);
		
	if(ipc_start_server()<0){
		utils_log_error("IPC server did not start. Existing. \n");
		exit (1);
	}
	utils_log_print("IPC registered\n");
}

void ACustomNativeWindow_read_control(int *fd){
	int recvsock = ipc_connect();
	ipc_command(recvsock, IPC_COMMAND_GET_SHARED_WINDOW_CONTROL);
	
	if (ancil_recv_fd(recvsock, fd) == 0) {
		utils_log_error("ERROR: ancil_recv_fd() failed: %s(%d)\n", strerror(errno), errno);
		close (recvsock);
		return;
	}	
}


//____________________________________________________//
//  There are definitions of window listener methods  //
//____________________________________________________//

/*CW_API int ACustomNativeWindow_frame_available(EGLNativeWindowType window) {
	ACustomNativeWindow* win = (ACustomNativeWindow*) window;
	if (!win->control) return -1;
	return win->control->frameAvailable;
}*/

CW_API void* ACustomNativeWindow_getListener(char* path){
	ipc_set_connection_path(path);
	
	void* rrbuf;
	rrbuf = buffer_read();
	
	ACustomNativeWindow_listener *listener = (ACustomNativeWindow_listener*) malloc(sizeof(ACustomNativeWindow_listener));

	int controlFD;
	ACustomNativeWindow_read_control(&controlFD);
	listener->control = (windowControl*) mmap(NULL, sizeof(windowControl), PROT_READ | PROT_WRITE, MAP_SHARED, controlFD, 0);
	if (listener->control == MAP_FAILED) {
		utils_log_error("Failed to map shared window control\n");
		return 0;
	}
	
	memset(listener->pid, 0, 128);
	sprintf(listener->pid, "/proc/%d", listener->control->parentPID);
	glBindNativeBufferToTex(rrbuf);
	return listener;
}

CW_API void ACustomNativeWindow_releaseListener(void *listener_ptr){
	ACustomNativeWindow_listener *listener = (ACustomNativeWindow_listener*) listener_ptr;
	munmap(listener->control, sizeof(windowControl));
	free(listener);
}

CW_API int ACustomNativeWindow_frame_available(void* listener_ptr) {
	ACustomNativeWindow_listener *listener = (ACustomNativeWindow_listener*) listener_ptr;
	static struct stat sts;
	//utils_log_error("listener is %p\n", listener);
	//utils_log_error("pid is %s\n", listener->pid);
	//utils_log_error("control is %p\n", listener->control);
	if (stat(listener->pid, &sts) == -1 && errno == ENOENT) {
		return CW_ERROR_PROCESS_DIED;
	}
	
	return listener->control->frameAvailable;
}
