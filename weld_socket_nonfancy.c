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

	for (;;) {
		uint8_t buf[8*1024];
		ssize_t received = recv(from, buf, sizeof(buf), 0);

		if (received == 0) {
			close(to);
			return NULL;
		}

		ssize_t sent = 0;
		while (sent < received) {
			sent += send(to, (buf + sent), (received - sent), 0);
		}
	}
}

void weld_sockets(int sock_a, int sock_b)
{
	int splice_pairs[2][2] = 
		{ {sock_a, sock_b}
		, {sock_b, sock_a}
		};
	
	for (int i = 0; i < len(splice_pairs); i++) {
		pthread_t thread;
		pthread_create(&thread, NULL, splice_all, splice_pairs[i]);
	}
}
