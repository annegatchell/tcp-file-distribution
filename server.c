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

/*
 * You should use a globally declared linked list or an array to 
 * keep track of your connections.  Be sure to manage either properly
 */
struct clientListEntry{
	int sock_num;
	char client_name[24];
	struct clientListEntry *next;
};

struct file_entry
{
	char* file_name;
};

struct clientList
{
	struct clientListEntry *first;
	//after_last points to the same place as the next pointer of the last entry
	struct clientListEntry *after_last;  
};

struct clientList clients = {0, 0};
fd_set active_fd_set;  // temp file descriptor list for select()
int sockfd, newsockfd = 0; //Listen on sockfd, new connection on newsockfd

//thread function declaration
//void *connection(void *);

//global variables
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// char logFileName[64];


int getClientFromSocket(int s, struct clientListEntry *client){
	struct clientListEntry *current;
	if(clients.first != 0){
		current = clients.first;
		while(current != 0){
			if (s == current->sock_num){
				client = current;
				return 1;
			}
			else{
				printf("clients %s\n",clients.first->client_name);
				current = current->next;
			}
		}
	}
	printf("clients %s\n",clients.first->client_name);
	return -1;
}


void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void build_select_list(){
	FD_ZERO(&active_fd_set);
	FD_SET(sockfd, &active_fd_set);

	if(newsockfd != 0){
		printf("new sock fd %d\n", newsockfd);
		FD_SET(newsockfd, &active_fd_set);
	}

	struct clientListEntry *current;
	if(clients.first != 0){
		current = clients.first;
		while(current != 0){
			if(current->sock_num != 0){
				FD_SET(current->sock_num, &active_fd_set);
				current = current->next;
			}
		}
	}
}

void handle_data(struct clientListEntry *client){
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
    	int a = 3;
    }
    else
    {
        printf("Bytes received %d\nmessage %s\n", bytes_received, buffer);
        char* msg = "got cha msg, foo\n";
        int len, bytes_sent;
        len = strlen(msg);
        if((bytes_sent = send(client->sock_num, msg, len, 0)) == -1){
        	perror("send error");
        	return 2;
        }
        printf("Sent welcome: Bytes sent: %d\n", bytes_sent);
    }
			            
}

void check_existing_connections(){
	struct clientListEntry *current;
	if(clients.first != 0){
		current = clients.first;
		while(current != 0){
			if(current->sock_num != 0){
				if(FD_ISSET(current->sock_num, &active_fd_set)){
					handle_data(current);
				}
				current = current->next;
			}
		}
	}
}

void send_message_to_all_clients(char* msg, size_t size){
	struct clientListEntry *current;
	size_t bytes_sent;
	if(clients.first != 0){
		current = clients.first;
		while(current != 0){
			if(current->sock_num != 0){
				if((bytes_sent = send(current->sock_num, msg, size, 0)) == -1){
        			perror("send error");
        			break;
    			}
				current = current->next;
			}
		}
	}
	
}

void send_updated_files_list(){
	int x = 1;
}

void add_file_list_to_table(char file_list[]){
	int x = 2;
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
   	struct clientListEntry *current_client;
   	struct clientListEntry *new_client;
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

	    //if someone is trying to connect, you'll have to accept() 
		//the connection
        //newsockfd = accept(...)
        
        // for(i = 0; i < FD_SETSIZE; i++){
        	// if(FD_ISSET(i, &active_fd_set)){
        		
        		//If the connection is on the listening socket
        		if(FD_ISSET(sockfd, &active_fd_set)){
        		// if(i == sockfd){
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
			    	printf("server: got connection from %s\n", s);
	    		}
	    		//This is the branch for brand new clients
	    		else if(FD_ISSET(newsockfd, &active_fd_set))
	    		// else if(i == newsockfd)
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
		                add_file_list_to_table(file_list);
			            //########################
			            // This is the point where you should save the files in the hashtable
			            // and add the client name to the list of clients
			            //########################
		                //new_client = {i, receive_buf, NULL};
		                // free(file_list);
		              
		                clients.after_last = malloc(sizeof(struct clientListEntry));
		                //clients.last->next = new_client;//malloc(sizeof(struct clientListEntry));
		                clients.after_last->sock_num = newsockfd;
		                strcpy(clients.after_last->client_name, client_name);
		                // clients.after_last->client_name = client_name;
		                clients.after_last->next = 0;
		                if(clients.first == 0){
		                	clients.first = clients.after_last;
		                }
		                clients.after_last = clients.after_last->next;

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

			            //if there are others connected to the server, probably good to notify them
						//that someone else has joined.
						// char ugh[] = "New client joined: ";
						// char * new_client_msg;
						// new_client_msg = malloc(sizeof(client_name) + sizeof(ugh));
						// new_client_msg = strcat(ugh, client_name);
							// printf("HERE\n");
			            send_message_to_all_clients(client_name, sizeof(client_name));
			            send_updated_files_list();

			            //reset the newsockfd to 0, so that we don't keep coming to this branch for
			            //this client
			            newsockfd = 0;



						//pthread_mutex_lock(&mutex);
						//now add your new user to your global list of users
						//pthread_mutex_unlock(&mutex);

						//now you need to start a thread to take care of the 
						//rest of the messages for that client
						//r = pthread_create(&th, NULL, &connection, (void *)i);
						//if (r != 0) { fprintf(stderr, "thread create failed\n"); }

						//Get this guy off the select list, since the thread is watching now
						//FD_CLR (i, &active_fd_set);
						
						//A requirement for 5273 students:
						//at this point...
						//whether or not someone connected, you should probably
						//look for clients that should be timed out
						//and kick them out
						//oh, and notify everyone that they're gone.
		            }
                }
                check_existing_connections();
                // else if(getClientFromSocket(i, current_client) != -1){
                // 	//printf("WHAT DO I DO HERE?\n");
                // }
                // else{

                // 	continue;
                // }
        	// }
        }
    }

	
	
	
	

    //}
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
