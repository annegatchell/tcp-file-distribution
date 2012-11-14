/*
 * Example of client using TCP protocol.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>

// You may/may not use pthread for the client code. The client is communicating with
// the server most of the time until he recieves a "GET <file>" request from another client.
// You can be creative here and design a code similar to the server to handle multiple connections.
// #define PORT "6000"
#define LOG_FILE "client-log.txt"
#define MAX_RECEIVE_BUFFER_LENGTH 500


int main(int argc, char *argv[])

{
	struct timeval currTime;
	time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    FILE *logFile;
    int sockfd; //connect to server on sockfd
    int numBytes;
    struct addrinfo hints, *servinfo, *p;
    int status; //Error status
    char receive_buffer[MAX_RECEIVE_BUFFER_LENGTH];
    char * server_ip, *server_port_num, *client_name;


    if(argc != 4){
    	printf("usage is ./client <client name> <server ip> <server port #>\n");
    	return 0;
    }
    server_ip = argv[2];
    server_port_num = argv[3];
    client_name = argv[1];


//Log the start up time
    gettimeofday(&currTime,NULL);
	nowtime = currTime.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof (tmbuf), "%Y-%m-%d %H:%M:%S\n", nowtm);
	char* logFileName = LOG_FILE;
    logFile = fopen(logFileName,"a");
    fprintf(logFile,"Client started at %s\n", tmbuf);
    fclose(logFile);

//Set up the address struct
    memset(&hints, 0, sizeof(hints));
    hints.ai_family= AF_UNSPEC; //AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets

    if((status = getaddrinfo(server_ip, server_port_num, &hints, &servinfo)) != 0){
    	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    	return 2;
    }	
//Loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next){
    	if((sockfd = socket(p->ai_family, p->ai_socktype, 
    		p->ai_protocol)) == -1){
    		perror("Client could not set up socket\n");
    		continue;
    	}

    	if ((status = connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        printf("Client connected!\n");
        break;
    }
//If no connection was made, then exit
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

//
    char *msg = client_name;
	int len, bytes_sent;
	
	//Send a message to server with name and files list
	len = strlen(msg);
	if((bytes_sent = send(sockfd, msg, len, 0)) == -1){
		perror("send error");
		return 2;
	}
	printf("Bytes sent: %d\n", bytes_sent);

	int bytes_received;

	//Wait for the welcome response and hashtable
	if((bytes_received = recv(sockfd,receive_buffer,
			MAX_RECEIVE_BUFFER_LENGTH,0)) == -1){
		perror("receive error");
		return 2;
	}
	else{
		printf("Bytes received %d\n%s", 
					bytes_received, receive_buffer);
	}

	//########## SEND MORE THINGS TO TEST THREADS
	
	int i;
	//Send a message to server with name and files list
	char msg2[100];
	for(i = 0; i < 20; i++){
		sprintf(msg2,"test %d from %s", i, client_name);
		len = strlen(msg2);
		printf("%s\n", msg2);
		if((bytes_sent = send(sockfd, msg2, len, 0)) == -1){
			perror("send error");
			return 2;
		}
		printf("Bytes sent: %d\n", bytes_sent);
	}
	


    return 0;
}
