CFLAGS = -Wall -g

CC = gcc
SRCS = client.c server.c
LOGS = client-log.txt server-log.txt
HELPER = helper.c
OBJS = client.o server.o

all: client server
client: client.o 
	${CC} ${CFLAGS} -o $@ client.o 

server: server.o
	${CC} ${CFLAGS} -lpthread -o $@ server.o

server.o: server.c
	${CC} ${CFLAGS} -c server.c

client.o: client.c
	${CC} ${CFLAGS} -c client.c

${OBJS}: ${HELPER}

clean:
	rm *.o client server *${LOGS}
