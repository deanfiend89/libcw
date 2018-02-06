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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <sys/resource.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>

#include "libancillary/ancillary.h"
#include "utils.h"
#include "ipc.h"

struct sockaddr_un addr;

#define MAX_CALLBACKS_COUNT 32

typedef struct {
	int32_t id;
	ipc_callback func;
} ipc_callback_map;

static pthread_t ipc_listening_thread_id = 0; //Server side part
static int server_socket = -1;

static ipc_callback_map callbacks[MAX_CALLBACKS_COUNT];
static char* ipc_connection_path = IPC_DEFAULT_CONNECTION_PATH;

void ipc_set_connection_path(char* path){
	ipc_connection_path = path;
}

int ipc_connect(){
	int addrlen;
	int recvsock;
	int descriptor;
	int size;

	memset (&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	sprintf (&addr.sun_path[1], "%s", ipc_connection_path);
	addrlen = sizeof(addr.sun_family) + strlen(&addr.sun_path[1]) + 1;
	
	recvsock = socket (AF_UNIX, SOCK_STREAM, 0);
	if (!recvsock) { utils_log_error ("cannot create UNIX socket: %s\n", strerror(errno)); }
	if (connect (recvsock, (struct sockaddr *)&addr, addrlen) != 0)
	{
		utils_log_error("%s: cannot connect to UNIX socket %s: %s, len %d\n", __PRETTY_FUNCTION__, addr.sun_path + 1, strerror(errno), addrlen);
		close (recvsock);
		return -1;
	}

	//utils_log_print("\n\n%s: connected to socket %s\n", __PRETTY_FUNCTION__, &addr.sun_path[1]);
	return recvsock;
}

void ipc_command(int com_socket, int32_t comand){
	ipc_command_t command;
	command.mode = IPC_MODE_REQUEST;
	command.command = comand;
	ssize_t sent = send (com_socket, &command, sizeof(command), 0);
	//utils_log_print("%s: Sent %d bytes of command\n", __PRETTY_FUNCTION__, sent);
	if (sent != sizeof(command))
	{
		utils_log_error("%s: send() failed on socket %s: %s\n", __PRETTY_FUNCTION__, addr.sun_path + 1, strerror(errno));
		close (com_socket);
		return ;
	}
}

static void *ipc_listening_thread(){
	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);
	int replysock;
	utils_log_print ("%s: thread started\n", __PRETTY_FUNCTION__);
	while ((replysock = accept (server_socket, (struct sockaddr *)&addr, &len)) != -1)
	{
		ipc_command_t command;
		if (recv (replysock, &command, sizeof(command), 0) != sizeof(command))
		{
			utils_log_error("%s: ERROR: recv() returned not %d bytes\n", __PRETTY_FUNCTION__, sizeof(command));
			close (replysock);
			continue;
		}
		//sock_callback(replysock, &command);
		
		ipc_command_t reply;
		if (command.mode != IPC_MODE_REQUEST) {
			utils_log_error("Callback mode mismatch!\n");
			reply.mode = IPC_MODE_REPLY_ERROR;
			if (send (replysock, &reply, sizeof(reply), 0) != sizeof(reply)) {
				utils_log_error("%s: send() failed while replying on listening socket: %s\n", __PRETTY_FUNCTION__, strerror(errno));
				close (replysock);
				continue ;
			}
		}
		
		//looking for command_id match
		int i;
		for (i=0; i<MAX_CALLBACKS_COUNT; i++){
			if (callbacks[i].id == command.command) {
				callbacks[i].func(replysock);
				//close(replysock);
				break;
			}
		}
		if (i != MAX_CALLBACKS_COUNT-1) continue;
		
		utils_log_error("Callback command %d unknown!\n", command.command);
		reply.mode = IPC_COMMAND_UNKNOWN;
		if (send (replysock, &reply, sizeof(reply), 0) != sizeof(reply)) {
			utils_log_error("%s: send() failed while replying on listening socket: %s\n", __PRETTY_FUNCTION__, strerror(errno));
			close (replysock);
			continue ;
		}

		close (replysock);	
	}
	return NULL;
}

int ipc_start_server() {
	//check for existing of callbacks first
	int i, callback_exists;
	for (i=0, callback_exists=0; i<MAX_CALLBACKS_COUNT; i++){
		if (callbacks[i].id != 0 && callbacks[i].func != NULL) callback_exists = 1;
	}
	
	if (!callback_exists) {
		utils_log_error("No registered callbacks detected.\n");
		return -1;
	}
	
	server_socket = socket (AF_UNIX, SOCK_STREAM, 0);
	if (!server_socket){
		utils_log_error("%s: cannot create UNIX socket: %s\n", __PRETTY_FUNCTION__, strerror(errno));
		errno = EINVAL;
		return -1;
	}
	struct sockaddr_un addr;
	int len;
	memset (&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	sprintf (&addr.sun_path[1], "%s", ipc_connection_path);
	len = sizeof(addr.sun_family) + strlen(&addr.sun_path[1]) + 1;
	if (bind (server_socket, (struct sockaddr *)&addr, len) != 0){ utils_log_error("Can not bind socket on path %s\n", ipc_connection_path);}
	utils_log_error("%s: bound UNIX socket %s\n", __PRETTY_FUNCTION__, addr.sun_path + 1);
	if (listen (server_socket, 4) != 0)
	{
		utils_log_error("%s: listen failed\n", __PRETTY_FUNCTION__);
		errno = ENOMEM;
		return -1;
	}
	
	pthread_create (&ipc_listening_thread_id, NULL, &ipc_listening_thread, NULL);
	return 0;
}

int ipc_register_callback(uint32_t callback_id, ipc_callback func){
	int i;
	
	for (i=0; i<MAX_CALLBACKS_COUNT; i++){
		if (callbacks[i].id == callback_id) {
			utils_log_error("Callback ID %d is already registered. You are trying to register this ID twice or ID definition is duplicate in different symbols. \n", callback_id);
			return 1;
		}
	}
	
	for (i=0; i<MAX_CALLBACKS_COUNT; i++){
		if (callbacks[i].id == IPC_COMMAND_UNKNOWN) {
			callbacks[i].id = callback_id;
			callbacks[i].func = func;
			return 0;
		}
	}
	
	//Should never happen
	return 1;
}
