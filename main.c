#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>

#include <stdio.h>

#define LISTEN_ADDR "127.0.0.1"
#define LISTEN_PORT 8000

#define TARGET_ADDR "127.0.0.1"
#define TARGET_PORT 8001

struct sockaddr_in tcp_addr(char * addr, u_short port)
{
	return (struct sockaddr_in)
		{ .sin_family = AF_INET
		, .sin_addr.s_addr = inet_addr(addr)
		, .sin_port = htons(port)
		};
}

void * splice_all(void * fds)
{
	int from = ((int *) fds)[0];
	int to   = ((int *) fds)[1];

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

	// Performance todos (after benchmark):
	//  - adjust pipe size using fcntl if needed (possibly up to
	//    /proc/sys/fs/pipe-max-size, or down if buffer bloat becomes a
	//    problem)

	// Undocumented behaviour of splice: If len larger than SSIZE_MAX,
	// EINVAL is returned. This could have something to do with the
	// write call, which is implementation-defined when nbytes is over
	// SSIZE_MAX.
	
	for (int i = 0; i < 4; i++) {
		pthread_t thread;
		pthread_create(&thread, NULL, splice_all, splice_pairs[i]);
	}
}

int main()
{
	struct sockaddr_in bindaddr = tcp_addr(LISTEN_ADDR, LISTEN_PORT);
	struct sockaddr_in targetaddr = tcp_addr(TARGET_ADDR, TARGET_PORT);

	int bindfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(bindfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	bind(bindfd, (struct sockaddr *) &bindaddr, sizeof(bindaddr));
	listen(bindfd, SOMAXCONN);

	for (;;) {
		int clientfd = accept(bindfd, NULL, NULL);

		int targetfd = socket(PF_INET, SOCK_STREAM, 0);
		connect(targetfd, (struct sockaddr *) &targetaddr, sizeof(targetaddr));

		weld_sockets(clientfd, targetfd);
	}

	return 0;
}
