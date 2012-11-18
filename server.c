/*
 * Skeleton code of a server using TCP protocol.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include "helper.c"

#define MAX_CONNECTS 50
#define LOG_FILE "server-log.txt"
#define MAX_BUFFER_SIZE 500
#define MAX_FILENAME_SIZE 80
typedef struct file_entry FILE_ENTRY;
typedef struct clientListEntry CLIENT_LIST_ENTRY;
typedef struct clientList CLIENT_LIST;
typedef struct fileList FILE_LIST;

void send_updated_files_list();
/*
 * You should use a globally declared linked list or an array to 
 * keep track of your connections.  Be sure to manage either properly
 */
struct clientListEntry{
	int sock_num;
	char client_name[24];
	char ip[INET6_ADDRSTRLEN];
	CLIENT_LIST_ENTRY *next;
};

struct file_entry
{
	char file_name[MAX_FILENAME_SIZE];
	int size;
	struct clientListEntry *client;
	FILE_ENTRY *next;	
};

struct clientList
{
	CLIENT_LIST_ENTRY *first;
	CLIENT_LIST_ENTRY *tail;  
};


struct fileList
{
	FILE_ENTRY *first;
	int number_of_files;
	FILE_ENTRY *tail;

};

CLIENT_LIST clients = {NULL, NULL};
fd_set active_fd_set;  // temp file descriptor list for select()
int sockfd, newsockfd = 0; //Listen on sockfd, new connection on newsockfd
FILE_LIST files = {NULL, 0, NULL};


//thread function declaration
//void *connection(void *);

//global variables
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// char logFileName[64];
void traverseClients(){
	CLIENT_LIST_ENTRY *current;
	current = clients.first;
	while(current){
		printf("Client: %s, Socket: %d IP: %s\n",current->client_name, current->sock_num, current->ip);
		current = current->next;
	}
}

void traverseFiles(){
	FILE_ENTRY *current;
	current = files.first;
	while(current){
		printf("Filename: %s Size: %d Owner: %s IP: %s\n", current->file_name, current->size, current->client->client_name, current->client->ip);
		current = current->next;
	}
}

int getClientFromSocket(int s, struct clientListEntry **client){
	CLIENT_LIST_ENTRY *current;
	if(clients.first != 0){
		current = clients.first;
		while(current != 0){
			if (s == current->sock_num){
				// printf("client %p\n", client);
				// printf("*client %p\n", *client);
				// printf("**client->client_name %p\n", **client);
				*client = current;
				// printf("current ptr %p\n", current);
				// printf("client ptr %p\n", *client);
				// printf("Test access %s\n", *client->client_name);
				return 1;
			}
			
			current = current->next;
		}
	}
	return -1;
}


void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void build_select_list(){
	FD_ZERO(&active_fd_set);
	// printf("adding the listening socket %d to the activelist\n", newsockfd);
	FD_SET(sockfd, &active_fd_set);


	if(newsockfd != 0){
		printf("adding the new socket %d to the activelist\n", newsockfd);
		FD_SET(newsockfd, &active_fd_set);
	}

	CLIENT_LIST_ENTRY *current;
	current = clients.first;
	while(current){
		if(current->sock_num != 0){
			// printf("adding the socket %d to the activelist\n", current->sock_num);
			FD_SET(current->sock_num, &active_fd_set);
			current = current->next;
		}
	}
}

int find_IP_for_file(char filename[], char addr[]){
	FILE_ENTRY *current;
	current = files.first;
	while(current){
		if (strcmp(current->file_name, filename) == 0){
			printf("client ip %s\n", current->client->ip);
			strcpy(addr,current->client->ip);
			return 1;
		}
		current = current->next;
	}
	return 0;
}

void interpret_commant(char command[], CLIENT_LIST_ENTRY *client){
    char* temp;
    char* intermediate = malloc(sizeof(command));
    strcpy(intermediate, command);
    char cmd[16];
    int val;
    size_t bytes_sent;
    printf("%s\n", command);
    char* msg;
    char ip_temp[INET6_ADDRSTRLEN];

    printf("in here\n");
    if((temp = strtok(intermediate," ")) != NULL){
    	printf("%s\n", temp);
	    // strcpy(cmd, temp);
	    
	    if((val = strcmp(temp, "Get")) == 0){
	        printf("GET\n");
	      	
	        if((temp = strtok(NULL,"\n")) != NULL){
	        	printf("1 %s\n", temp);
	        }
	        if(find_IP_for_file(temp,ip_temp) == 0){
	        	printf("No file here\n");
	        	msg = "Sorry, that file is not available\n";
	        	if((bytes_sent = send(client->sock_num, msg, strlen(msg), 0)) == -1){
	    			perror("send error");
				}
				else{
	    			printf("no file sent msg to client %s\nmsg: %s bytes: %ld\n", client->client_name, msg,bytes_sent);
				}
	        }
	        else{
	        	printf("ip %s\n", ip_temp);
		        if((bytes_sent = send(client->sock_num, ip_temp, sizeof(ip_temp), 0)) == -1){
	    			perror("send error");
				}
				else{
	    			printf("yes files sent msg to client %s\nmsg: %s bytes: %ld\n", client->client_name, ip_temp,bytes_sent);
				}
			}
	    }
	    else if((val = strcmp(temp, "List\n"))==0){
	        printf("LIST\n");
	        send_updated_files_list();
	    }
	    else{
	    	send_updated_files_list();
	    }
	}
    else{
    	send_updated_files_list();
    }

}

void handle_data(CLIENT_LIST_ENTRY *client){
	// printf("HANDLING THE DATA\n");
	char buffer[1024];
	int bytes_received;
	memset(&buffer, 0, sizeof(buffer));
	if((bytes_received = recv(client->sock_num,buffer,sizeof(buffer),0)) < 0)
    {
    	close(client->sock_num);
    	FD_CLR (client->sock_num, &active_fd_set);
    	//#####REMOVE FROM LIST
    }
    else if(bytes_received == 0){
    	3;
    }
    else
    {
        printf("Bytes received %d\nmessage %s\n", bytes_received, buffer);
        interpret_commant(buffer, client);
        char* msg = "got cha msg, foo\n";
        int len, bytes_sent;
        len = strlen(msg);
        if((bytes_sent = send(client->sock_num, msg, len, 0)) == -1){
        	perror("send error");
        }
        printf("Sent welcome: Bytes sent: %d\n", bytes_sent);
    }
			            
}

void check_existing_connections(int i){
	// printf("Getting in here\n");
	CLIENT_LIST_ENTRY *current;
	if(clients.first != 0){
		current = clients.first;
		while(current != 0){
			// printf("check_existing_connections client name %s\n",current->client_name);
			if(current->sock_num != 0){
				if(current->sock_num == i){
					handle_data(current);
				}
			}
			current = current->next;
		}
	}
}

void send_message_to_all_clients(char* msg, size_t size){
	int itr = 0; 
	CLIENT_LIST_ENTRY *current;
	size_t bytes_sent;
	if(clients.first != 0){
		current = clients.first;
		while(current != 0){
			printf("send msg to all clients client name %s, %d\n", current->client_name, itr);
			if(current->sock_num != 0){
				if((bytes_sent = send(current->sock_num, msg, size, 0)) == -1){
        			perror("send error");
        			break;
    			}
    			else{
	    			printf("sent msg to client %s\nmsg: %s bytes: %ld\n", current->client_name, msg,bytes_sent);
    			}
			}
			current = current->next;
			itr++;
		}
	}
	
}

void send_updated_files_list(){
	traverseFiles();
	char *file_list_buffer;
	size_t file_list_buf_size= files.number_of_files*MAX_FILENAME_SIZE;
	file_list_buffer = malloc(file_list_buf_size);
	FILE_ENTRY *current;
	current = files.first;
	if(files.first){
		while(current){
			strcat(file_list_buffer, current->file_name);
			current = current->next;
		}
		send_message_to_all_clients(file_list_buffer, file_list_buf_size);
	}	
	else{
		char msg[] = "No files available for sharing";
		send_message_to_all_clients(msg, sizeof(msg));
	}
	memset(file_list_buffer,0,files.number_of_files*MAX_FILENAME_SIZE);
	// free(file_list_buffer);

}

void add_file_list_to_table(char file_list[], int recv_port){
	traverseClients();
	//Use strtok to find our happy jolly file names
	//char* strtok( char* str, const char* delim );
	char* temp;
	temp = strtok(file_list, "\n");
	// printf("TEMP %s\n", temp);
	FILE_ENTRY *new_entry;
	if((new_entry = (FILE_ENTRY *)malloc(sizeof(FILE_ENTRY))) == NULL) 
		              		{fprintf(stderr, "Can't allocate memory for new file\n");}
	
	strcpy(new_entry->file_name, temp);
	printf("string after copy: %s\n", new_entry->file_name);
	new_entry->size = 0;
	//What client is it? get from recv_port
	CLIENT_LIST_ENTRY *tmp_client;
	printf("rec_v port %d\n", recv_port);
	if(getClientFromSocket(recv_port, &tmp_client) == -1){

		fprintf(stderr, "add_file_list_to_table: Error in getting the client from socket\n");
	}
	else{
		// printf("tmp_client ptr%p\n", tmp_client);
		// printf("tmp after get client: %s\n",tmp_client->client_name);
		new_entry->client = tmp_client;
		printf("client after get client: %s\n",new_entry->client->client_name);
	}
	
	files.number_of_files++;
	new_entry->next = 0;
	if(!files.first){
    	files.first = new_entry;
    	files.tail = new_entry;
    }
    else{
    	files.tail->next = new_entry;
    	files.tail= files.tail->next;
    }		           
    new_entry = files.tail->next;
    // if((new_entry = (CLIENT_LIST_ENTRY *)malloc(sizeof(CLIENT_LIST_ENTRY))) == NULL) 
  		// {fprintf(stderr, "Can't allocate memory for new client\n");}

//#####Continue the traversal of the string
	while((temp = strtok(NULL, "\n")) != NULL){
		//ADD THE FILE TO THE FILE LIST
		//Copy filename to file in file_entry
		if((new_entry = (FILE_ENTRY *)malloc(sizeof(FILE_ENTRY))) == NULL) 
		              		{fprintf(stderr, "Can't allocate memory for new file\n");}

		strcpy(new_entry->file_name, temp);
		printf("string after copy: %s\n", new_entry->file_name);
		new_entry->size = 0;
		new_entry->client = tmp_client;
		files.number_of_files++;
		new_entry->next = 0;
		if(!files.first){
	    	files.first = new_entry;
	    	files.tail = new_entry;
	    }
	    else{
	    	files.tail->next = new_entry;
	    	files.tail= files.tail->next;
	    }		           
	    new_entry = files.tail->next;
	}
	
	//print files:


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



int main(int argc,char *argv[])
{
    
    struct timeval currTime;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    FILE *logFile;
    
    struct addrinfo hints, *servinfo, *p; //servinfo points to results
    struct sockaddr_storage client_addr; //Connector's address information
    socklen_t addr_size;
    
    int status; //The error status
    int BACKLOG = 1; //Number of clients allowed in queue

    struct timeval new_timeout; //The default timeout we are using for the select()
    new_timeout.tv_sec = 2; //2 secs
    new_timeout.tv_usec = 500000; //0.5 secs

    

    char receive_buf[MAX_BUFFER_SIZE] = "TEST";
    int bytes_received;

    char s[INET6_ADDRSTRLEN];
    int yes=1;


    //check arguments here
    if (argc != 2)  {
		printf("usage is: ./pserver <port#>\n");
		return 0;
    }

//Log the startup time
    gettimeofday(&currTime,NULL);
	nowtime = currTime.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof (tmbuf), "%Y-%m-%d %H:%M:%S\n", nowtm);
  	char* logFileName = LOG_FILE;
    logFile = fopen(logFileName,"a");
    fprintf(logFile,"Server started at %s", tmbuf);
    fclose(logFile);

    //Set up the 
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     //fill in my IP for me
 
    if ((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

     // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
    /*
     * Open a TCP socket (an Internet stream socket).
     */
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Server could not set up socket\n");
            continue;
        }
    /*
    *	Get rid of any pesky leftover sockets
    */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
    }
     /*
     * Bind our local address so that the client can send to us.
     */
        if((status = bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
        	perror("Server could not bind to socket\n");
        }
    /*
     * here you should listen() on your TCP socket
     */
     	if((listen(sockfd, BACKLOG)) == -1){
     		perror("Server can't listen!!!\n");
     	}
    }

    
   	// FD_ZERO(&active_fd_set); //clear the select set
   	// FD_SET(sockfd, &active_fd_set); //add socket to the listening list

   	CLIENT_LIST_ENTRY *new_entry;
   	if((new_entry = (CLIENT_LIST_ENTRY *)malloc(sizeof(CLIENT_LIST_ENTRY))) == NULL) 
		              		{fprintf(stderr, "Can't allocate memory for new client\n");}
   	int readsocks; //number of sockets ready for reading
	//printf("%d\n",FD_SETSIZE);
    for ( ; ; ) //endless loop
    {
    //now that you're listening, check to see if anyone is trying 
	//to connect
	//hint:  select()
    //Run select() with no timeout for now, waiting to see if there is
    //a pending connection on the socket waiitng to be accepted with accept
    	build_select_list();
	    if ((readsocks = select(FD_SETSIZE, &active_fd_set, (fd_set*) 0, (fd_set*) 0, NULL)) == -1) {
	        perror("select");
	        exit(4);
	    }
	    else if(readsocks == 0){
	    	printf("Nothing to read\n");
	    	
	    }
	    else{
	    	// printf("HERE!!!\n");
	    	// printf("readsocks %d\n", readsocks);
	    }
	    //if someone is trying to connect, you'll have to accept() 
		//the connection
        //newsockfd = accept(...)
        int i;
        for(i = 0; i < FD_SETSIZE; i++){
        	if(FD_ISSET(i, &active_fd_set)){
        		
        		//If the connection is on the listening socket
        		// if(FD_ISSET(sockfd, &active_fd_set)){
        		if(i == sockfd){
			    	printf("Got a connection!\n");
			    	//Accept the connection
			    	newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
			    	if(newsockfd < 0){
			    		perror ("accept");
		                exit (EXIT_FAILURE);
			    	}
			    	//add new socket to the listening list
			    	//FD_SET(newsockfd, &active_fd_set);
			    	inet_ntop(client_addr.ss_family,
		          				get_in_addr((struct sockaddr *)&client_addr), s, sizeof(s)); 
			    	printf("server: got connection from %s, socket %d\n", s, newsockfd);
			    	
	    		}
	    		//This is the branch for brand new clients
	    		// else if(FD_ISSET(newsockfd, &active_fd_set))
	    		else if(i == newsockfd)
	    		//if you've accepted the connection, you'll probably want to
				//check "select()" to see if they're trying to send data, 
			    //like their name, and if so
				//recv() whatever they're trying to send
                {
	                /* Data arriving on an already-connected socket. */
	                if((bytes_received = recv(newsockfd,receive_buf,MAX_BUFFER_SIZE,0)) < 0)
	                {
	                	close(newsockfd);
	                	FD_CLR (newsockfd, &active_fd_set);
	                }
	                else{
	                	char client_name[80];
	                	char file_list[MAX_BUFFER_SIZE];
	                	int c = 0;
	                	while(receive_buf[c] != '\n'){
	                		client_name[c] = receive_buf[c];
	                		c++;
	                	}
	                	client_name[c] = 0;
	                	strcpy(file_list,&receive_buf[c]);
		                printf("Bytes received %d\nname:%s\nfilelist:%s\n", bytes_received, client_name,file_list);
		                
			            //########################
			            // This is the point where you should save the files in the hashtable
			            // and add the client name to the list of clients
			            //#######################

		                new_entry->sock_num = i;
		                //#####FOR NOW WE ARE JUST USING LOCALHOST IP ADDRS FOR ALL CLIENTS//
		                strcpy(new_entry->ip, "localhost");
		                strcpy(new_entry->client_name, client_name);
		                new_entry->next = NULL;
		                if(!clients.first){
		                	clients.first = new_entry;
		                	clients.tail = new_entry;
		                }
		                else{
		                	clients.tail->next = new_entry;
		                	clients.tail= clients.tail->next;
		                }		           
		                new_entry = clients.tail->next;
		                if((new_entry = (CLIENT_LIST_ENTRY *)malloc(sizeof(CLIENT_LIST_ENTRY))) == NULL) 
		              		{fprintf(stderr, "Can't allocate memory for new client\n");}
		             

		                //### Add file to file list, now that it has a client to point to
		                add_file_list_to_table(file_list, i);

			            //since you're talking nicely now.. probably a good idea send them
						//a message to welcome them to the service.
			            char* welcome_msg = "Welcome to the File Sharing System\n";
			            int len, bytes_sent;
			            len = strlen(welcome_msg);
			            if((bytes_sent = send(newsockfd, welcome_msg, len, 0)) == -1){
			            	perror("send error");
			            	return 2;
			            }
			            printf("Sent welcome: Bytes sent: %d\n", bytes_sent);
			            send_message_to_all_clients(client_name, sizeof(client_name));
			            send_updated_files_list();

			            //reset the newsockfd to 0, so that we don't keep coming to this branch for
			            //this client
			            newsockfd = 0;
		            }
                }
                else{
                	// printf("exiting connections check\n");
                	check_existing_connections(i);
                }
                
                // else if(getClientFromSocket(i, current_client) != -1){
                // 	//printf("WHAT DO I DO HERE?\n");
                // }
                // else{

                // 	continue;
                // }
        	}
        	//
        }
    }

	freeaddrinfo(servinfo);
    return 0;
}





/*//-----------------------------------------------------------------------------
void *connection(void *sockid) {
    int s = (int)sockid;

    char buffer[1000];
    struct timeval curTime;
    int e, rc = 0;
    int bytes_received;

    pthread_detach(pthread_self());  //automatically clears the threads memory on exit

    printf("A THREAD OMG %d\n", s);
   
    // Here we handle all of the incoming text from a particular client.
    
    for(;;){
    	    	//printf("for\n");

	if((bytes_received = recv(s,buffer,sizeof(buffer),0)) < 0)
	{
		close(s);
	}
    else
    {
        printf("In thread, Bytes received %d\ninthread message: %s\n", bytes_received, buffer);
    }
    //rc = recv()
    if (bytes_received > 0)
    {
    	//printf("here\n");
		if(0){// I received an 'exit' message from this client
		pthread_mutex_lock(&mutex);
		//remove myself from the vector of active clients
		pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
		printf("Shouldn't see this!\n");
		}
		
		//A requirement for 5273 students:
	    //if I received a new files list from this client, the
		//server must “Push”/send the new updated hash table to all clients it is connected to.
		
		
		//loop through global client list and
		//e =write(..);  
		if (e == -1) //broken pipe.. someone has left the room
		{
		    pthread_mutex_lock(&mutex);
		    //so remove them from our list
		    pthread_mutex_unlock(&mutex);
		}
	
    }
	}

    //should probably never get here
    return 0;
} */
