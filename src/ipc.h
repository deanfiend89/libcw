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

#ifndef __XDROID_IPC_H__
#define __XDROID_IPC_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define IPC_DEFAULT_CONNECTION_PATH "/dev/shm/sock-shared"

typedef enum {
	IPC_MODE_REQUEST,
	IPC_MODE_REPLY_OK,
	IPC_MODE_REPLY_ERROR,
} ipc_mode_t;

typedef struct {
	ipc_mode_t mode;
	int32_t command;
	int32_t value;
} ipc_command_t;

typedef void (*ipc_callback)(int);

void ipc_set_connection_path(char* path);
int ipc_connect();
void ipc_command(int com_socket, int32_t command);

//Server side only stuff
int ipc_register_callback(uint32_t callback_id, ipc_callback func);
int ipc_start_server();

int droix_ipc_register_fb_fd(size_t);
void droix_ipc_register_serialized_buf(void *data, size_t size, int* fds, int n_fds);

#ifdef __cplusplus
}
#endif
#endif // __XDROID_IPC_H__
