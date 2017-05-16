#include <unistd.h> // close, NULL
#include <stdint.h> // uint8_t

void * splice_all(void * fds)
{
	int from = ((int *) fds)[0];
	int to   = ((int *) fds)[1];

	for (;;) {
		uint8_t buf[8*1024];
		do {
			ssize_t received = read(from, buf, sizeof(buf));
		while (received == EINTR);

		if (received <= 0) { // If closed or error
			do {
				int e = close(to);
			} while (e && errno == EINTR);
			return NULL;
		}

		ssize_t sent_total = 0;
		while (sent < received) {
			do {
				int sent = write(to, (buf + sent), (received - sent));
			} while (sent == EINTR);
			sent_total += sent;
		}
	}
}
