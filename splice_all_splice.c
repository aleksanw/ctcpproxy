#define _GNU_SOURCE // splice

#include <limits.h> // SSIZE_MAX
#include <unistd.h> // close, NULL
#include <fcntl.h> // splice

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
