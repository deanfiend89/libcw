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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "ipc.h"
#include <system/window.h>
#include <window.h>

#ifdef __cplusplus
extern "C" {
#endif
void utils_log_print( const char* format, ... );
void utils_log_error( const char* format, ... );
void utils_log_set_print(int (*new_print)(const char*, ...));
void utils_log_set_error_print(int (*new_print)(const char*, ...));

ANativeWindowBuffer_t* buffer_alloc(uint32_t width, uint32_t height, uint32_t format, uint32_t usage);
void buffer_set_shared(ANativeWindowBuffer_t* buffer);
ANativeWindowBuffer_t* buffer_read();
void buffer_register_read_callback(void);
void buffer_lock(ANativeWindowBuffer_t* buf, void** vaddr);
void buffer_unlock(ANativeWindowBuffer_t* buf);

void ACustomNativeWindow_read_control(int *fd);

static inline void checkGlError(const char* op) {
	GLint error;
    for (error = glGetError(); error; error = glGetError()) {
        utils_log_error("after %s() glError (0x%x)\n", op, error);
    }
}

enum {
	IPC_COMMAND_UNKNOWN,
	IPC_COMMAND_GET_FD,
	IPC_COMMAND_GET_GRAPHIC_BUFFER,
	IPC_COMMAND_GET_GRAPHIC_BUFFER_FDS,
	IPC_COMMAND_GET_SHARED_WINDOW_CONTROL,
	IPC_COMMAND_GET_INPUT_QUEUE,
};

#ifdef __cplusplus
}
#endif
#endif //__UTILS_H__
