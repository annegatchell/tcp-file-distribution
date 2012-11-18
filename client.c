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
#define MAX_RECEIVE_BUFFER_LENGTH 1024
#define STDIN 0
#define MAX_CMD_LINE 200
#define C_TO_C_PORTNUM "6050"


int sockfd; //connect to server on sockfd
int listenfd = 0; //listen on port listenfd
fd_set active_fd_set; 
char *listen_port_num = 0;
char files_name_string[20*80];
FILE *fileListFile;
char *file_list_name;



void build_select_list(){
    FD_ZERO(&active_fd_set);
    FD_SET(sockfd, &active_fd_set);
    FD_SET(STDIN, &active_fd_set);

    //NEED TO IMPLMENT LISTENER SOCKET
    if(listenfd != 0){
        // printf("new sock fd %d\n", listenfd);
        FD_SET(listenfd, &active_fd_set);
    }
}

void update_list_of_files(){
    char line[80];
    fileListFile = fopen(file_list_name, "r");
    int i = 0;
    int z = 0;
    memset(files_name_string,0,sizeof(files_name_string));
    while(fgets(line, 80, fileListFile) != NULL){
        //sscanf(line, "%s",files[i]);
        printf("%d\n", z);
        printf("%s\n", line);
        strcat(files_name_string, line);
        z += sizeof(line);
        //printf("%s\n", files[i]);
        i++;
    }
    fclose(fileListFile);
}

void send_file_list(){
    size_t bytes_sent;
   
    // char tmp[] = "files\n";
    // char *m = malloc(sizeof(tmp) + sizeof(files_name_string));
    // printf("hello?\n");
    // m = strcat(tmp, files_name_string);
    // printf("message %s\n", m);
    
    int len = strlen(files_name_string);
    if((bytes_sent = send(sockfd, files_name_string, len, 0)) == -1){
        perror("send error");
    }
    printf("message %s\n", files_name_string);
    printf("Bytes sent: %ld\n", bytes_sent);
}

void connect_to_other_client(char *server_ip){
    struct addrinfo hints, *servinfo;
    int status; //Error status
    char receive_buffer[MAX_RECEIVE_BUFFER_LENGTH];

    //Set up the address struct
    memset(&hints, 0, sizeof(hints));
    hints.ai_family= AF_UNSPEC; //AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets

    if((status = getaddrinfo(server_ip, C_TO_C_PORTNUM, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    }   

}

int interpret_commant(char command[]){
    char* temp;
    char intermediate[MAX_CMD_LINE];
    strcpy(intermediate, command);

    char cmd[16];
    int val;
    size_t bytes_sent;
    size_t bytes_received;

    char ip_recv[INET6_ADDRSTRLEN];

    printf("int %s\n", intermediate );
    if(strcmp(intermediate,"\n") == 0){
        strcpy(intermediate,"RAWR");
        printf("intermediate %s\n", intermediate);
        return 1;
    }

    printf("in here int %s\n", intermediate);
    temp = strtok(intermediate," ");
    strcpy(cmd, temp);
    printf("temp %s\n", temp);
    if((val = strcmp(intermediate, "Get")) == 0){
        printf("GET\n");
        printf("%s\n", command);
        if((bytes_sent = send(sockfd, command, MAX_CMD_LINE, 0)) == -1){
            perror("send error");
        }
        printf("Bytes sent: %ld\nWaiting for response.........\n", bytes_sent);
        FD_CLR(sockfd, &active_fd_set);
        if((bytes_received = recv(sockfd,ip_recv, INET6_ADDRSTRLEN,0)) < 0){
            perror("receive error");
            return 2;
        }
        else{
            printf("ip %s\n", ip_recv);
            connect_to_other_client(ip_recv);
        }   

    }
    else{
        temp = strtok(command, "\n");
        printf("temp2 %s\n", temp);
        if((val = strcmp(temp, "List"))==0){
            printf("LIST\n");
            if((bytes_sent = send(sockfd, cmd, sizeof(cmd), 0)) == -1){
                perror("send error");
            }
            printf("Bytes sent: %ld\n", bytes_sent);
        }
        else if((val = strcmp(temp, "SendMyFilesList")) == 0){
            printf("SENDMYFILELIST\n");
            send_file_list();
        }
        else{
            printf("Invalid command\n");
        }
    }
    return 0;

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
  
    struct addrinfo hints, *servinfo, *p, *listenerinfo;
    int status; //Error status
    char receive_buffer[MAX_RECEIVE_BUFFER_LENGTH];
    char * server_ip, *server_port_num, *client_name;
    struct sockaddr_storage client_addr; //Connector's address information
    socklen_t addr_size;

  
    
    memset(files_name_string, 0, sizeof(files_name_string));

     int BACKLOG = 1; //Number of clients allowed in queue
     int yes=1;
     char ip_s[INET6_ADDRSTRLEN];

    memset(receive_buffer,0,sizeof(receive_buffer));

    if(argc != 6){
    	printf("usage is ./client <client name> <server ip> <server port#> <list of files> <listen port>\n");
    	return 0;
    }
    server_ip = argv[2];
    server_port_num = argv[3];
    client_name = argv[1];
    file_list_name  = argv[4];
    listen_port_num = argv[5];

    //Get the list of files
    update_list_of_files();
    printf("%s\n", files_name_string);
    update_list_of_files();

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
		printf("Bytes received %d\n%s\n", 
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
    int i;
    char command[MAX_CMD_LINE];
    for(;;){
    	//now that you're listening, check to see if anyone is trying 
        // to connect
        // hint:  select()
        // Run select() with no timeout for now, waiting to see if there is
        // a pending connection on the socket waiitng to be accepted with accept
        // printf("beginning of loop\n");
        build_select_list();
        if ((readsocks = select(FD_SETSIZE, &active_fd_set, (fd_set*) 0, (fd_set*) 0, NULL)) == -1) {
            perror("select");
            exit(4);
        }
        else if(readsocks == 0){
            printf("Nothing to read\n");
        }
        for(i = 0; i < FD_SETSIZE; i++){
            // printf("for loop\n");
            if(FD_ISSET(i, &active_fd_set)){

            //If the connection is on the Server socket
                if(i == sockfd){
                    // printf("WHERE AM I\n");
                    if((bytes_received = recv(i,receive_buffer, MAX_RECEIVE_BUFFER_LENGTH,0)) < 0){
                        perror("receive error");
                        return 2;
                    }
                    else if(bytes_received == 0){
                        // printf("no bytes\n");
                    }
                    else if(bytes_received > 0){
                        printf("Got a packet from the server!\n");
                        printf("Bytes received %d\n%s\n", bytes_received, receive_buffer);
                    }
                    memset(receive_buffer, 0, sizeof(receive_buffer));
                }
            //If the connection is on the listening socket
                else if(i == listenfd){
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
                else if(i == STDIN){
                    fgets(command, sizeof(command), stdin);
                    if(!(interpret_commant(command))){
                        printf("Try again\n");
                    }
                    else{
                        printf("Success\n");
                    }
                }
            }
        }
        

	}
    freeaddrinfo(listenerinfo);

    return 0;
}
