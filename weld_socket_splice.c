#define _GNU_SOURCE

#include <pthread.h>
#include <arpa/inet.h>
#include <limits.h> // SSIZE_MAX
#include <unistd.h> // close, pipe, NULL
#include <fcntl.h> // splice

#define len(arr) (sizeof(arr)/sizeof(arr[0]))

void * splice_all(void * fds)
{
	int from = ((int *) fds)[0];
	int to   = ((int *) fds)[1];

	// Performance todos (after benchmark):
	//  - adjust pipe size using fcntl if needed (possibly up to
	//    /proc/sys/fs/pipe-max-size, or down if buffer bloat becomes a
	//    problem)

	// Undocumented behaviour of splice: If len larger than SSIZE_MAX,
	// EINVAL is returned. This could have something to do with the
	// write call, which is implementation-defined when nbytes is over
	// SSIZE_MAX.

	for (;;) {
		ssize_t transferred = splice(from, NULL, to, NULL, SSIZE_MAX, 0);
		if (transferred == 0) {
			close(to);
			return NULL;
		}
	}
}

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
