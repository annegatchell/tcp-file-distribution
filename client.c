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
#include "helper.c"

// You may/may not use pthread for the client code. The client is communicating with
// the server most of the time until he recieves a "GET <file>" request from another client.
// You can be creative here and design a code similar to the server to handle multiple connections.
// #define PORT "6000"
#define LOG_FILE "client-log.txt"
#define MAX_RECEIVE_BUFFER_LENGTH 500


int sockfd; //connect to server on sockfd
int listenfd = 0; //listen on port listenfd
fd_set active_fd_set; 
char *listen_port_num = 0;

void build_select_list(){
    FD_ZERO(&active_fd_set);
    FD_SET(sockfd, &active_fd_set);

    //NEED TO IMPLMENT LISTENER SOCKET
    if(listenfd != 0){
        // printf("new sock fd %d\n", listenfd);
        FD_SET(listenfd, &active_fd_set);
    }
}

void update_list_of_files(FILE *fileListFile, char files[][80], char* log_file_name, char files_name_string[]){
    char line[80];
    fileListFile = fopen(log_file_name, "rt");
    int i = 0;
    int z = 0;
    while(fgets(line, 80, fileListFile) != NULL){
        sscanf(line, "%s",files[i]);
        printf("%d\n", z);
        printf("%s\n", line);
        strcat(files_name_string, line);
        z += sizeof(line);
        //printf("%s\n", files[i]);
        i++;
    }

}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        printf("INET\n");
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    printf("INET6\n");
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[])

{
	struct timeval currTime;
	time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    FILE *logFile;
    
    int numBytes;
    struct addrinfo hints, *servinfo, *p, *listenerinfo;
    int status; //Error status
    char receive_buffer[MAX_RECEIVE_BUFFER_LENGTH];
    char * server_ip, *server_port_num, *client_name, *log_file_name;
    struct sockaddr_storage client_addr; //Connector's address information
    socklen_t addr_size;

    FILE *fileListFile;
    char line[80];
    char files[20][80];
    char files_name_string[20*80];
    memset(files_name_string, 0, sizeof(files_name_string));

     int BACKLOG = 1; //Number of clients allowed in queue
     int yes=1;
     char ip_s[INET6_ADDRSTRLEN];


    if(argc != 6){
    	printf("usage is ./client <client name> <server ip> <server port#> <list of files> <listen port>\n");
    	return 0;
    }
    server_ip = argv[2];
    server_port_num = argv[3];
    client_name = argv[1];
    log_file_name  = argv[4];
    listen_port_num = argv[5];

    //Get the list of files
    update_list_of_files(fileListFile, files, argv[4], files_name_string);
    printf("%s\n", files_name_string);

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
    char *msg = malloc(sizeof(client_name) + sizeof("\n") + sizeof(files_name_string));
    msg = strcat(client_name, "\n");
    msg = strcat(msg, files_name_string);
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
    //We GOT A CONNECTION FROM THE SERVER. SET UP LISTENING PORT
    memset(&hints, 0, sizeof(hints));
    hints.ai_family= AF_UNSPEC; //AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     //fill in my IP for me

    if ((status = getaddrinfo(NULL, listen_port_num, &hints, &listenerinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

     // loop through all the results and bind to the first we can
    printf("HERE\n");
    for(p = listenerinfo; p != NULL; p = p->ai_next) {
    /*
     * Open a TCP socket (an Internet stream socket).
     */
        if ((listenfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Server could not set up socket\n");
            continue;
        }
    /*
    *   Get rid of any pesky leftover sockets
    */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
    }
     /*
     * Bind our local address so that the client can send to us.
     */
        if((status = bind(listenfd, p->ai_addr, p->ai_addrlen)) == -1){
            perror("Server could not bind to socket\n");
        }
    /*
     * here you should listen() on your TCP socket
     */
        if((listen(listenfd, BACKLOG)) == -1){
            perror("Server can't listen!!!\n");
        }
    }

    int readsocks;
    for(;;){
    	//now that you're listening, check to see if anyone is trying 
        // to connect
        // hint:  select()
        // Run select() with no timeout for now, waiting to see if there is
        // a pending connection on the socket waiitng to be accepted with accept
        build_select_list();
        if ((readsocks = select(FD_SETSIZE, &active_fd_set, (fd_set*) 0, (fd_set*) 0, NULL)) == -1) {
            perror("select");
            exit(4);
        }
        else if(readsocks == 0){
            printf("Nothing to read\n");
        }
        else
        {
        //If the connection is on the Server socket
            if(FD_ISSET(sockfd, &active_fd_set)){
            // if(i == sockfd){
                
                if((bytes_received = recv(sockfd,receive_buffer,
                            MAX_RECEIVE_BUFFER_LENGTH,0)) == -1){
                    perror("receive error");
                    return 2;
                }
                else if(bytes_received == 0){
                    4;
                }
                else if(bytes_received > 0){
                    printf("Got a packet from the server!\n");
                    printf("Bytes received %d\n%s\n", 
                    bytes_received, receive_buffer);
                }
                else{
                    4;
                }
            }
        //If the connection is on the listening socket
            else if(FD_ISSET(listenfd, &active_fd_set)){
            // if(i == sockfd){
                printf("Got a connection from another client!\n");
                //Accept the connection
                listenfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_size);
                if(listenfd < 0){
                    perror ("accept");
                    exit (EXIT_FAILURE);
                }
                //add new socket to the listening list
                //FD_SET(newsockfd, &active_fd_set);
                inet_ntop(client_addr.ss_family,
                            get_in_addr((struct sockaddr *)&client_addr), ip_s, sizeof(ip_s)); 
                printf("server: got connection from %s\n", ip_s);
            }



        }

    	//Send a message to server with name and files list
        int i;
    	char msg2[100];
    	for(i = 0; i < 0; i++){
    		sprintf(msg2,"test %d from %s", i, client_name);
    		len = strlen(msg2);
    		printf("%s\n", msg2);
    		if((bytes_sent = send(sockfd, msg2, len, 0)) == -1){
    			perror("send error");
    			return 2;
    		}
    		// printf("Bytes sent: %d\n", bytes_sent);
    		if((bytes_received = recv(sockfd,receive_buffer,
    			MAX_RECEIVE_BUFFER_LENGTH,0)) == -1){
    		perror("receive error");
    		return 2;
    		}
    		else{
    			// printf("Bytes received %d\n%s", 
    			// 			bytes_received, receive_buffer);
    		}
    	}
	}
    freeaddrinfo(listenerinfo);

    return 0;
}
