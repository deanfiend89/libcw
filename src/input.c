#include "utils.h"
#include "ipc.h"
#include <window_input.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#define INPUT_API __attribute__((visibility("default")))

static int server_pipe_fds[2];
static int client_pipe_fd;

static void input_read_callback(int replysock){
	if (ancil_send_fd(replysock, server_pipe_fds[0])<0)
		utils_log_error("Failed to send shared window control input fd\n");
}

INPUT_API void input_create_queue(){
	//pipe(server_pipe_fds);
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, server_pipe_fds) < 0) {
		perror("opening stream socket pair");
		exit(1);
	}
	ipc_register_callback(IPC_COMMAND_GET_INPUT_QUEUE, input_read_callback);
}

INPUT_API int input_event_available(){
	if(server_pipe_fds[1] == 0) return;
	int bytesAvailable = 0;
	ioctl(server_pipe_fds[1], FIONREAD, &bytesAvailable);
	return bytesAvailable;
}

INPUT_API inline void input_event_write(uint8_t x, uint8_t y, uint8_t action){
	if(client_pipe_fd == 0) {
		int recvsock = ipc_connect();
		ipc_command(recvsock, IPC_COMMAND_GET_INPUT_QUEUE);
		
		if (ancil_recv_fd(recvsock, &client_pipe_fd) == 0) {
			utils_log_error("ERROR: ancil_recv_fd() failed: %s(%d)\n", strerror(errno), errno);
			close (recvsock);
			return;
		}
		close (recvsock);
	}
	if(client_pipe_fd == 0) return;
	inputEvent event = {x, y, action};
	write(client_pipe_fd, &event, sizeof(inputEvent));
}

INPUT_API void input_event_read(uint8_t* x, uint8_t* y, uint8_t* action){
	inputEvent event;
	read(server_pipe_fds[1], &event, sizeof(inputEvent));
	*x = event.x;
	*y = event.y;
	*action = event.action;
}
