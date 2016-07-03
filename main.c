#include <arpa/inet.h>
#include <unistd.h> // NULL

#include <weld_socket.h>

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
