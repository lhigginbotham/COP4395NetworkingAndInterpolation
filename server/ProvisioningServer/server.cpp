#include "socket.hpp"
#include "utility.hpp"
#include <iostream>
#include <string.h>

#define MYPORT "4951"    // the port users will be connecting to

#define MAXBUFLEN 100

int closeSocket(int sockfd)
{
#if PLATFORM == PLATFORM_WINDOWS
	closesocket(sockfd);
	return 1;
#else
	close(sockfd);
	return 1;
#endif
}

int main(void)
{
	int sockfd;

	InitializeSockets();

	Socket socket(AF_INET, SOCK_DGRAM, AI_PASSIVE);

	sockfd = socket.Open("4951");
	
	printf("listener: waiting to recvfrom...\n");
	socket.Receive(sockfd);

	//char buf[100] = "World";
	//&(((struct sockaddr_in*)sa)->sin_addr)
	//if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
	//	p->ai_addr, p->ai_addrlen)) == -1) {

	//}

	socket.CloseSocket(sockfd);
	ShutdownSockets();
	return 0;
}