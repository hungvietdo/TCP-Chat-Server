
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h> /*inet_ntoa function*/
#include <netdb.h>
#include <string.h>
#include<signal.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

#define	SERV_PORT		 10111			/* TCP and UDP */

#define SA      struct sockaddr
#define MAXHOSTNAME 80
#define BUFSIZE 1024
#define MAX_LEN 1024
#define MAX_UCLIENT 100

u_char          TimeToLive;
u_char          loop;

typedef struct {
	struct sockaddr_in from;
    char* ipaddress;
	char* hostname;
    int port;
	char* char_port;
	int check;
	int sockfd;
} socket_info;


socket_info 		randomUDPinfo();
void 			reuseport(int s);
void 			joingroup(int s, char *group);
void 			setttlvalue(int s, u_char * ttl_value);
void 			setloopback(int s, u_char loop);
void            UDPsendMessage(int inSock, int outSock,  char *MulticastIPAddress, int UDPport);
void 			UDPgetMessage(int inSock, int outSock,struct   sockaddr_in HostSend, int UDPport);
void			SendaMessagetoUDPGroup(char *recvBuf, int outSock,  char *MulticastIPAddress, int UDPport);

