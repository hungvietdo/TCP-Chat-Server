/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/
 
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	MAXLINE		4096	/* max text line length */ 

#include "functions.h"
void addTCPClient(struct sockaddr_in cliaddr, int sockfd);
void Initialize();
void remove_tclient(int sockt);

socket_info UClient_List[FD_SETSIZE];
socket_info TClient_List[FD_SETSIZE];
int number_of_uclient, number_of_tclient;
 
//the thread function
void *connection_handler(void *);
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client, servaddrTCP, servaddrUDP;
	int 				socket_TCP,socket_UDP;
	
	
	//To serve TCP server
	socket_TCP = socket(AF_INET, SOCK_STREAM, 0); //tcp server
	reuseport(socket_TCP);
	bzero(&servaddrTCP, sizeof(servaddrTCP));
	servaddrTCP.sin_family      = AF_INET;
	servaddrTCP.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddrTCP.sin_port        = htons(SERV_PORT);
	
	//To serve UDP server
	socket_UDP = socket(AF_INET, SOCK_DGRAM, 0); //udp server
	reuseport(socket_UDP);
	bzero(&servaddrUDP, sizeof(servaddrUDP));
	servaddrUDP.sin_family      = AF_INET;
	servaddrUDP.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddrUDP.sin_port        = htons(SERV_PORT);
	
	bind(socket_TCP, (SA *) &servaddrTCP, sizeof(servaddrTCP));
	bind(socket_UDP, (SA *) &servaddrUDP, sizeof(servaddrUDP));

	listen(socket_TCP, LISTENQ);
	listen(socket_UDP, LISTENQ);
    puts("bind done");
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	for (;;) 
    {
		//Handling TCP client
		if (client_sock = accept(socket_TCP, (struct sockaddr *)&client, (socklen_t*)&c))
		{
	        puts("Connection accepted");
		
			//add new TCP client to the list
			addTCPClient(client,client_sock);
			
			if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
	        {
	            perror("could not create thread");
	            return 1;
	        }
	        //Now join the thread , so that we dont terminate before the thread
	        //pthread_join( thread_id , NULL);
	        puts("Handler assigned");
		}
	 
		
		// //Handling UDP client
	// 	if (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))
	// 	{
	//         puts("Connection accepted");
	//
	// 		printf("Handling client %d\n", ntohs(client.sin_port) );
	//
	//         if( pthread_create( &thread_id , NULL ,  connection_handler (client) , (void*) &client_sock) < 0)
	//         {
	//             perror("could not create thread");
	//             return 1;
	//         }
	//         //Now join the thread , so that we dont terminate before the thread
	//         //pthread_join( thread_id , NULL);
	//         puts("Handler assigned");
	// 	}
	//
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }     
    return 0;
}

void sendMesgto_T_CLient_List(char mesg[MAXLINE])
{
	int i,sd;
	struct sockaddr_in cliaddr;
	
	cliaddr.sin_family = AF_INET;
	
	for (i=1;i<=number_of_tclient;i++)
	{	
		cliaddr = TClient_List[i].from;
		
		printf("TClient: %d , socket %d\n",ntohs(TClient_List[i].from.sin_port),TClient_List[i].sockfd);
		printf("mesg to send %s\n", mesg );
		
		send(TClient_List[i].sockfd,mesg , strlen(mesg), 0 ); 
		
		//sendto(TClient_List[i].sockfd, mesg, len, 0, (SA *) &cliaddr, sizeof(cliaddr));
	}
}

void addTCPClient(struct sockaddr_in cliaddr, int sockfd)
{
	int i;
	int inthelist = 0;
	
	for (i=1;i<=number_of_tclient;i++)
		if (TClient_List[i].check < 0)
			{
				break;
			}
			else
			{
				if (TClient_List[i].from.sin_port == cliaddr.sin_port)
				{
					printf("already in the list\n");
					inthelist = 1;
					break;
				}
			}
		
	//Client is not in the list	
	if (inthelist==0)
	{
		TClient_List[i].check =1;
		TClient_List[i].from = cliaddr;
		TClient_List[i].sockfd = sockfd;

		if (i>number_of_tclient) number_of_tclient++;

		if (i == FD_SETSIZE)
		{
			printf("too many TCP clients\n");
			exit(0);
		}
	}
	
	printf("number_of_tclient: %d\n", number_of_tclient);
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
     
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //end of string marker
		client_message[read_size] = '\0';
		
		//printf("sending to the soocket%d\n", sock );
		
		
		sendMesgto_T_CLient_List(client_message);
		//Send the message back to client
        //write(sock , client_message , strlen(client_message));
		
		//clear the message buffer
		memset(client_message, 0, 2000);
    }
     
    if(read_size == 0)
    {	
		//remove client out of the list
		remove_tclient(sock);
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    return 0;
} 
void remove_tclient(int sock)
{
	int i;
 
	for (i=1;i<=number_of_tclient;i++)
	{
		if (TClient_List[i].sockfd == sock)
		{
				TClient_List[i].check=-1; //any client equal to sock will be deleted
				//break;
		}
	}
	
}
void Initialize() {

	int i;
	for (i=0;i<=FD_SETSIZE;i++)
	{
		UClient_List[i].check=-1;
		TClient_List[i].check=-1;	
	}
	number_of_uclient = 0;
	number_of_tclient = 0;
    
}