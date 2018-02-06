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
#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <system/window.h>
#include "libancillary/ancillary.h"
#include "utils.h"

#include <sys/errno.h>
#include <sys/types.h>

#include <sys/socket.h> //for recv

static hw_module_t const* module;
static alloc_device_t  *mAllocDev;
static gralloc_module_t const* mAllocMod = NULL;

static void gralloc_helper_init(){
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    if (module == NULL) {
		utils_log_error("FATAL: can't find the %s module\n", GRALLOC_HARDWARE_MODULE_ID);
		exit (0);
	}
    if (err == 0) {
        gralloc_open(module, &mAllocDev);
    }
    if (err == 0) {
        mAllocMod = (gralloc_module_t const *)module;
    }
}

void bufIncRef(struct android_native_base_t* base){
	base; //utils_log_print("Buffer %p incRef\n", base);
}

void bufDecRef(struct android_native_base_t* base){
	base; //utils_log_print("Buffer %p decRef\n", base);
}

ANativeWindowBuffer_t* buffer_alloc(uint32_t width, uint32_t height, uint32_t format, uint32_t usage){
	if (mAllocMod == NULL) gralloc_helper_init();
	ANativeWindowBuffer_t* buffer = (ANativeWindowBuffer_t*) malloc(sizeof(ANativeWindowBuffer_t));
	memset(buffer, 0, sizeof(ANativeWindowBuffer_t));
	
	buffer->common.magic = ANDROID_NATIVE_BUFFER_MAGIC;
	buffer->common.version = sizeof(ANativeWindowBuffer_t);
	memset(buffer->common.reserved, 0, sizeof(buffer->common.reserved));
	buffer->common.incRef = bufIncRef;
	buffer->common.decRef = bufDecRef;

	buffer->width = width;
	buffer->height = height;
	buffer->stride = 0;
	buffer->format = format;
	buffer->usage = usage;
	buffer->handle = NULL;
	
	int err;
	err = mAllocDev->alloc(mAllocDev, width, height, format, usage, &buffer->handle, &buffer->stride);
	if(err != 0) {
		utils_log_error("Failed to allocate gralloc buffer\n");
	}
	
	return buffer;
}

ANativeWindowBuffer_t* shared_buffer;
void buffer_set_shared(ANativeWindowBuffer_t* buffer){
	shared_buffer = buffer;
}

void buffer_write(int replysock, ANativeWindowBuffer_t* buf){
	ssize_t sent = send (replysock, buf, sizeof(ANativeWindowBuffer_t), 0);
	if (sent != sizeof(ANativeWindowBuffer_t)) {
		utils_log_error("%s: send() failed while sending serialized data on listening socket: %s", __PRETTY_FUNCTION__, strerror(errno));
		close (replysock);
		return ;
	}
	
	int handle_size = sizeof(native_handle_t) + sizeof(int)*(buf->handle->numFds + buf->handle->numInts);
	
	sent = send (replysock, buf->handle, handle_size, 0);
	if (sent < handle_size) {
		utils_log_error("%s: send() failed while sending serialized data on listening socket: %s", __PRETTY_FUNCTION__, strerror(errno));
		close (replysock);
		return ;
	}
	
	close (replysock);
	return ;
}

void buffer_fds_write(int replysock, ANativeWindowBuffer_t* buf){
	
	if (ancil_send_fds(replysock, buf->handle->data, buf->handle->numFds) != 0) {
		utils_log_error("%s: ERROR: ancil_send_fds() failed: %s (%s/%d)\n", __PRETTY_FUNCTION__, strerror(errno), __FILE__, __LINE__);
	}
			
	close (replysock);
	return ;
}

ANativeWindowBuffer_t* buffer_read(){
	//256 bytes. I don't know how much ints gralloc will use on different devices, so...
	size_t buf_size = sizeof(ANativeWindowBuffer_t) + sizeof(native_handle_t) + sizeof(int)*37;

	ANativeWindowBuffer_t* recv_buf = (ANativeWindowBuffer_t*) malloc(buf_size); 	
	
	int recvsock = -1;
	while (recvsock == -1) {
		utils_log_print("Trying to connect buffer producer...\n");
		recvsock = ipc_connect();
		usleep(1000*500);
	}
	
	ipc_command(recvsock, IPC_COMMAND_GET_GRAPHIC_BUFFER);
	recv (recvsock, recv_buf, buf_size, 0);
	close(recvsock);
	
	//buf->handle contains pointer actual for another process, need to fix that
	recv_buf->handle = (native_handle_t*)  ((void*)recv_buf + sizeof(ANativeWindowBuffer_t));
	
	recvsock = ipc_connect();
	ipc_command(recvsock, IPC_COMMAND_GET_GRAPHIC_BUFFER_FDS);
	
	if (ancil_recv_fds(recvsock, recv_buf->handle->data, recv_buf->handle->numFds) == 0) {
		utils_log_error("ERROR: ancil_recv_fd() failed: %s\n", strerror(errno));
		close (recvsock);
		return NULL;
	}
	//close (recvsock);
	
	//Finally fix incRef, decRef and register native handle
	
	recv_buf->common.incRef = bufIncRef;
	recv_buf->common.decRef = bufDecRef;
	
	//char *fdpath;
	//readlink("/proc/self/fd/11"
	
	//utils_log_error("asda (%d)\n", getpid());
	if (mAllocMod == NULL) gralloc_helper_init();
	mAllocMod->registerBuffer(mAllocMod, recv_buf->handle);
	//utils_log_error("qwewe\n");

	return recv_buf;
}

static void buffer_read_callback(int replysock){
	buffer_write(replysock, shared_buffer);
}

static void buffer_read_fds_callback(int replysock){
	buffer_fds_write(replysock, shared_buffer);
}

void buffer_register_read_callback(void){
	ipc_register_callback(IPC_COMMAND_GET_GRAPHIC_BUFFER, buffer_read_callback);
	ipc_register_callback(IPC_COMMAND_GET_GRAPHIC_BUFFER_FDS, buffer_read_fds_callback);
}

void buffer_lock(ANativeWindowBuffer_t* buf, void** vaddr){
	if (mAllocMod == NULL) gralloc_helper_init();
	mAllocMod->lock(mAllocMod, buf->handle, buf->usage, 0, 0, buf->width, buf->height, vaddr);
}

void buffer_unlock(ANativeWindowBuffer_t* buf){
	if (mAllocMod == NULL) gralloc_helper_init();
	mAllocMod->unlock(mAllocMod, buf->handle);
}
