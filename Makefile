CC= /usr/bin/gcc
all:	tcpclient tcpserver udpclient udpserver

udpclient: udpclient.c;
	${CC} udpclient.c -o udpclient

udpserver: udpserver.c;
	${CC} udpserver.c -o udpserver

clean:
	rm tcpclient tcpserver udpclient udpserver
