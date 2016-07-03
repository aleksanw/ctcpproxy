#include <pthread.h>
#include <unistd.h> // pipe, NULL

#include <splice_all.h>

#define len(arr) (sizeof(arr)/sizeof(arr[0]))

void weld_sockets(int sock_a, int sock_b)
{
	int send_pipe[2]; pipe(send_pipe);
	int recv_pipe[2]; pipe(recv_pipe);

	int splice_pairs[4][2] = 
		{ {sock_a, send_pipe[1]}
		, {send_pipe[0], sock_b}
		, {sock_b, recv_pipe[1]}
		, {recv_pipe[0], sock_a}
		};
	
	for (int i = 0; i < len(splice_pairs); i++) {
		pthread_t thread;
		pthread_create(&thread, NULL, splice_all, splice_pairs[i]);
	}
}
