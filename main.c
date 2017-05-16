#define _POSIX_C_SOURCE 201112L
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <weld_socket.h>

struct addrinfo * tcp_addr(char * addr, char * port)
{
	struct addrinfo hints =
		{ .ai_socktype = SOCK_STREAM
		, .ai_family = AF_UNSPEC
		};

	struct addrinfo * addrinfo;
	int e = getaddrinfo(addrinfo, port, &hints, &res);

	if (e) {
		printf("%s (port %s): %s\n", addr, port, gai_strerror(e));
		exit(EXIT_FAILURE);
	}

	return addrinfo;
}

int addr_socket(struct addrinfo * addr)
{
	return socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
}

int addr_bind(int fd, struct addrinfo * addr)
{
	return bind(fd, addr->ai_addr, addr->ai_addrlen);
}

int addr_connect(int fd, struct addrinfo * addr)
{
	return connect(fd, addr->ai_addr, addr->ai_addrlen);
}

enum setup_action
	{ BIND
	, CONNECT
	};

int tcp_setup(enum setup_action action, char * addr, char * port)
{
	struct addrinfo * addrinfo = tcp_addr(addr, port);

	int fd;

	struct addrinfo * item;
	for(item = addrinfo; item != NULL; item = item->ai_next) {
		fd = addr_socket(item);

		if (fd < 0) {
			switch (errno) {
				case EAFNOSUPPORT:
					// Address family not supported.
					// Most likely disabled IPv6.
					// Try next.
					continue;
				case EPROTONOSUPPORT:
					// (address family, protocol) combination not supported.
					// Maybe `getaddrinfo` picked some funky protocol instead
					// of TCP. Maybe we should enforce TCP?
					// Try next.
					continue;
				case EPROTOTYPE:
					// (socket type, protocol) combination not supported.
					// `getaddrinfo` picked a non-stream protocol.
					// Should never happen. I hope.
					// Bail out.
				case EMFILE:
					// Out of FDs for process.
					// Bail out.
				case ENFILE:
					// Out of FDs for system.
					// Bail out.
				case ENOBUF:
					// Out of system resources.
					// Bail out.
				case ENOMEM:
					// Out of memory.
					// Bail out.
					perror("socket");
					exit(1);
				case EACCES:
					// Access denied.
					// This process is not allowed to use create this type of
					// socket.
					// Try next.
					perror("socket");
					continue;
			}
		}

		if (action == CONNECT) {
			int e = addr_connect(fd, item);

			if (e) {
				switch (errno) {
					case ECONNREFUSED:
						// Try next address.
					case EINTR:
						// Interrupted. Async establishment.
						// Must now do async poll.
					case ENETUNREACH:
						// Try next
					case EPROTOTYPE:
						// The socket on the other end is not TCP.
						// Bad namelookup result?
						// Try next
					case ETIMEOUT:
						// Try next
					case EADDRINUSE:
						// Shit, it's i bruk, try next
					case ECONNRESET:
						// How is this different from ECONNREFUSED
						// Try next
					case EHOSTUNREACH:
						// try next
					case EACCES:
						// Try next.
					case ENETDOWN:
						// try next
					case EADDRNOTAVAIL:
						// OS ran out of available tcp ports.
						// Try next. Maybe we'll get a different socket type.
						// Flag error.
					case ENOBUFS:
						// Ran out of buffer space.
						// Abort.
					default:
						// This list of errors in not exaustive. But should be
						// enough for blocking tcp using getaddrinfo with a fresh
						// socket.
						perror("Unexpected error in connect");
				}
				close(fd);
				continue;
			}
		}

		if (action == BIND) {
			setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
			{
				int e = addr_bind(fd, item);

				if (e) {
					perror("bind");
					close(fd);
					continue;
				}
			}

			{
				int e = listen(fd, SOMAXCONN);

				if (e) {
					perror("listen");
					close(fd);
					continue;
				}
			}
		}

		break;
	}

	if (item == NULL)
		fd = -1;
	
	freeaddrinfo(addrinfo);
	return fd;
}

int tcp_connect(char * addr, char * port)
{
	return tcp_setup(CONNECT, addr, port);
}

int tcp_bind(char * addr, char * port)
{
	return tcp_setup(BIND, addr, port);
}

void tcp_rst(int fd)
	// Immeadietly RST the connection, discarding any data in send buffers.
{
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (int[]){1, 0}, sizeof(int));
	close(fd);
}

int main(int argc, char ** argv)
{
	if (argc != 5) {
		printf("Usage: %s bind-address bind-port target-address target-port\n", argv[0]);
		exit(1);
	}

	char * bind_addr = argv[1];
	char * bind_port = argv[2];
	char * target_addr = argv[3];
	char * target_port = argv[4];

	int bindfd = tcp_bind(bind_addr, bind_port);

	for (;;) {
		int clientfd = accept(bindfd, NULL, NULL);
		int targetfd = tcp_connect(target_addr, target_port);

		weld_sockets(clientfd, targetfd);
	}

	return 0;
}
