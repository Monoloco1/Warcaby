#include <sys/types.h>   /* basstringic system data types */
#include <sys/socket.h>  /* basic socket definitions */
#include <sys/time.h>	/* timeval{} for select() */
#include <time.h>				/* timespec{} for pselect() */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>   /* inet(3) functions */
#include <errno.h>
#include <fcntl.h>			   /* for nonblocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/tcp.h>		/* for TCP_MAXSEG */
#include <unistd.h>
#include 	<net/if.h>
#include	<sys/utsname.h>
#include <linux/un.h>
#include 	<poll.h>

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <map>
using std::string, std::vector, std::array, std::cout;


#define SA	  struct sockaddr
#define MAXLINE 1024
#define DEF_MPORT 13333	//multicast
#define DEF_MADDR "224.0.0.1"
#define LISTENQ 2
#define MULTICAST_DELAY 5

#define DEF_UPORT 13333 //unicast

#define PEER_DISCONNECT_MESSAGE 		"pdis"//"peer disconnect"
#define CONNECTION_ACKNOWLEDGE_MESSAGE 	"ackm"//"acknowledge message"
#define READ_ERROR_MESSAGE 				"errr"//"error reading your message, please resend"
#define ASSIGNED_WHITE_MESSAGE			"assw"//"you have been assigned the white pieces"
#define ASSIGNED_BLACK_MESSAGE			"assb"//"you have been assigned the black pieces"
#define SEND_MOVE						"move"// send move data
#define MOVE_OK							"mvok"// your move is accepted

const array<string, 7> messNumber = {
	PEER_DISCONNECT_MESSAGE,
	CONNECTION_ACKNOWLEDGE_MESSAGE,
	READ_ERROR_MESSAGE,
	ASSIGNED_WHITE_MESSAGE,
	ASSIGNED_BLACK_MESSAGE,
	SEND_MOVE,
	MOVE_OK
};

// const std::map<string, int> messNumber = {
// 	{PEER_DISCONNECT_MESSAGE,			1},
// 	{CONNECTION_ACKNOWLEDGE_MESSAGE, 	2},
// 	{READ_ERROR_MESSAGE, 				3},
// 	{ASSIGNED_WHITE_MESSAGE, 			4},
// 	{ASSIGNED_BLACK_MESSAGE, 			5},
// 	{"todo", 			6}
// };



int readData(int sockFD, char * recvline, int n);
int sendData(int sockFD, char * buffer, int n);
int family_to_level(int family);
int mcast_join(int sockfd, const SA *grp, socklen_t grplen, const char *ifname, u_int ifindex);
int snd_udp_socket(const char *serv, int port, SA **saptr, socklen_t *lenp);