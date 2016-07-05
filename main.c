#define _POSIX_C_SOURCE 201112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <weld_socket.h>

#define LISTEN_ADDR "127.0.0.1"
#define LISTEN_PORT 8000

#define TARGET_ADDR "127.0.0.1"
#define TARGET_PORT 8001

struct addrinfo * tcp_addr(char * addr, char * port)
{
	struct addrinfo hints =
		{ .ai_socktype = SOCK_STREAM
		, .ai_family = AF_UNSPEC
	    };

	struct addrinfo * res;
	int e = getaddrinfo(addr, port, &hints, &res);

	if (e) {
		puts(gai_strerror(e));
		exit(EXIT_FAILURE);
	}

	return res;
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
		item = item->ai_next;
		fd = addr_socket(item);

		if (fd < 0) {
			perror("socket");
			continue;
		}

		if (action == CONNECT) {
			int e = addr_connect(fd, item);

			if (e) {
				perror("connect");
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

	if (item == NULL) {
		exit(2);
	}
	
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


int main()
{
	int bindfd = tcp_bind("localhost", "8000");

	for (;;) {
		int clientfd = accept(bindfd, NULL, NULL);
		int targetfd = tcp_connect("localhost", "8001");

		weld_sockets(clientfd, targetfd);
	}

	return 0;
}
