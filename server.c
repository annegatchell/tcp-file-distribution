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

#define MAX_CONNECTS 50
#define LOG_FILE "server-log.txt"
#define MAX_BUFFER_SIZE 500

/*
 * You should use a globally declared linked list or an array to 
 * keep track of your connections.  Be sure to manage either properly
 */

//thread function declaration
void *connection(void *);

//global variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// char logFileName[64];

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc,char *argv[])
{
    
    struct timeval currTime;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    pthread_t th;
    int r;
    FILE *logFile;
    int sockfd, newsockfd = 0; //Listen on sockfd, new connection on newsockfd
    struct addrinfo hints, *servinfo, *p; //servinfo points to results
    struct sockaddr_storage client_addr; //Connector's address information
    socklen_t addr_size;
    struct sigaction sa;
    int status; //The error status
    int BACKLOG = 1; //Number of clients allowed in queue

    struct timeval new_timeout; //The default timeout we are using for the select()
    new_timeout.tv_sec = 2; //2 secs
    new_timeout.tv_usec = 500000; //0.5 secs

    fd_set active_fd_set;  // temp file descriptor list for select()

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

   
   
   	FD_ZERO(&active_fd_set); //clear the select set
   	FD_SET(sockfd, &active_fd_set); //add socket to the listening list
	//printf("%d\n",FD_SETSIZE);
    for ( ; ; ) //endless loop
    {
    //now that you're listening, check to see if anyone is trying 
	//to connect
	//hint:  select()
    //Run select() with no timeout for now, waiting to see if there is
    //a pending connection on the socket waiitng to be accepted with accept
	    if (select(FD_SETSIZE, &active_fd_set, NULL, NULL, NULL) == -1) {
	        perror("select");
	        exit(4);
	    }

	    //if someone is trying to connect, you'll have to accept() 
		//the connection
        //newsockfd = accept(...)
        int i;
        for(i = 0; i < FD_SETSIZE; i++){
        	if(FD_ISSET(i, &active_fd_set)){
        		//If the connection is on the listening socket
        		if(i == sockfd){
			    	printf("Got a connection!\n");
			    	//Accept the connection
			    	newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
			    	if(newsockfd < 0){
			    		perror ("accept");
		                exit (EXIT_FAILURE);
			    	}
			    	//add new socket to the listening list
			    	FD_SET(newsockfd, &active_fd_set);
			    	inet_ntop(client_addr.ss_family,
		          				get_in_addr((struct sockaddr *)&client_addr), s, sizeof(s)); 
			    	printf("server: got connection from %s\n", s);
	    		}
	    		//#########-> Need to differentiate between the new members and the existing ones
	    		else if(i == newsockfd)
	    		//if you've accepted the connection, you'll probably want to
				//check "select()" to see if they're trying to send data, 
			    //like their name, and if so
				//recv() whatever they're trying to send
                {
	                /* Data arriving on an already-connected socket. */
	                if((bytes_received = recv(i,receive_buf,MAX_BUFFER_SIZE,0)) < 0)
	                {
	                	close(i);
	                	FD_CLR (i, &active_fd_set);
	                }
	                else{
		                printf("Bytes received %d\n message %s\n", bytes_received, receive_buf);
		            //########################
		            // This is the point where you should save the files in the hashtable
		            //########################

		            //since you're talking nicely now.. probably a good idea send them
					//a message to welcome them to the service.
		            char* welcome_msg = "Welcome to the File Sharing System\n";
		            int len, bytes_sent;
		            len = strlen(welcome_msg);
		            if((bytes_sent = send(i, welcome_msg, len, 0)) == -1){
		            	perror("send error");
		            	return 2;
		            }
		            printf("Sent welcome: Bytes sent: %d\n", bytes_sent);

		            //if there are others connected to the server, probably good to notify them
					//that someone else has joined.


					pthread_mutex_lock(&mutex);
					//now add your new user to your global list of users
					pthread_mutex_unlock(&mutex);

					//now you need to start a thread to take care of the 
					//rest of the messages for that client
					r = pthread_create(&th, 0, connection, (void *)i);
					if (r != 0) { fprintf(stderr, "thread create failed\n"); }

					//A requirement for 5273 students:
					//at this point...
					//whether or not someone connected, you should probably
					//look for clients that should be timed out
					//and kick them out
					//oh, and notify everyone that they're gone.
		            }
                }
                else{
                	printf("LAST ELSE\n");
                }
        	}
        } 
        return 1;
    }

	
	return 1;
	
	

    //}
    //freeaddrinfo(servinfo);
}





//-----------------------------------------------------------------------------
void *connection(void *sockid) {
    int s = (int)sockid;

    char buffer[1000];
    struct timeval curTime;
    int e, rc = 0;

    pthread_detach(pthread_self());  //automatically clears the threads memory on exit

    printf("A THREAD OMG %d\n", s);
    /*
     * Here we handle all of the incoming text from a particular client.
     */

    //rc = recv()
    while (rc > 0)
    {
		//if I received an 'exit' message from this client
		pthread_mutex_lock(&mutex);
		//remove myself from the vector of active clients
		pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
		printf("Shouldn't see this!\n");
		
		
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

    //should probably never get here
    return 0;
}
