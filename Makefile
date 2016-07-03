CFLAGS += -I. -flto -pthread

all: ctcpproxy-nonfancy ctcpproxy-pipes ctcpproxy-splice

ctcpproxy-nonfancy: main.c weld_socket.h splice_all.h weld_socket_nonfancy.c splice_all_nonfancy.c
	$(CC) $(CFLAGS) -o $@ $^
ctcpproxy-pipes:    main.c weld_socket.h splice_all.h weld_socket_pipes.c splice_all_nonfancy.c
	$(CC) $(CFLAGS) -o $@ $^
ctcpproxy-splice:   main.c weld_socket.h splice_all.h weld_socket_pipes.c splice_all_splice.c
	$(CC) $(CFLAGS) -o $@ $^
