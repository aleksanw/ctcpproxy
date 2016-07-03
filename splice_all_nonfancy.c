#include <unistd.h> // close, NULL
#include <stdint.h> // uint8_t

void * splice_all(void * fds)
{
	int from = ((int *) fds)[0];
	int to   = ((int *) fds)[1];

	for (;;) {
		uint8_t buf[8*1024];
		ssize_t received = read(from, buf, sizeof(buf));

		if (received == 0) {
			close(to);
			return NULL;
		}

		ssize_t sent = 0;
		while (sent < received) {
			sent += write(to, (buf + sent), (received - sent));
		}
	}
}
