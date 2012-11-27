CFLAGS = -Wall -g

CC = gcc
SRCS = client.c server.c
LOGS = client-log.txt server-log.txt
HELPER = helper.c
OBJS = client.o server.o

all: client1/client client2/client client3/client client4/client server

client1/client: client.o 
	${CC} ${CFLAGS} -o $@ client.o 
client2/client: client.o
	${CC} ${CFLAGS} -o $@ client.o
client3/client: client.o
	${CC} ${CFLAGS} -o $@ client.o
client4/client: client.o
	${CC} ${CFLAGS} -o $@ client.o

server: server.o
	${CC} ${CFLAGS} -lpthread -o $@ server.o

server.o: server.c
	${CC} ${CFLAGS} -c server.c

client.o: client.c
	${CC} ${CFLAGS} -c client.c

${OBJS}: ${HELPER}

clean:
	rm *.o server client4/client client3/client client1/client client2/client
