all: ctcpproxy-splice ctcpproxy-nonfancy

ctcpproxy-splice: main.c weld_socket.h weld_socket_splice.c
	gcc -I. -flto -pthread -o $@ $^

ctcpproxy-nonfancy: main.c weld_socket.h weld_socket_nonfancy.c
	gcc -I. -flto -pthread -o $@ $^
