all: ctcpproxy-splice ctcpproxy-nonfancy ctcpproxy-pipes

ctcpproxy-splice: main.c weld_socket_pipes.c splice_all_splice.c
	gcc -I. -flto -pthread -o $@ $^

ctcpproxy-nonfancy: main.c weld_socket_nonfancy.c splice_all_nonfancy.c
	gcc -I. -flto -pthread -o $@ $^

ctcpproxy-pipes: main.c weld_socket_pipes.c splice_all_nonfancy.c
	gcc -I. -flto -pthread -o $@ $^
