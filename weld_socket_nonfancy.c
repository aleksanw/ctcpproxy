#include <pthread.h>
#include <unistd.h> // NULL

#include <splice_all.h>

#define len(arr) (sizeof(arr)/sizeof(arr[0]))

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
