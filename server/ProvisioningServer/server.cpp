#include "socket.hpp"
#include "utility.hpp"
#include <iostream>
#include <string.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>

#define MYPORT "4951"    // the port users will be connecting to

#define MAXBUFLEN 100

static const char MESSAGE[] = "Hello, World!\n";

int main(void)
{
	evutil_socket_t sockfd;
	int result = 5;
	InitializeSockets();

	Socket socket(AF_INET, SOCK_DGRAM, AI_PASSIVE);

	sockfd = socket.Open("4951");
	evutil_make_socket_nonblocking(sockfd);

	struct event_base* base = event_base_new();

	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	printf("listener: waiting to recvfrom...\n");

	
	while (result > 0)
	{
		socket.Receive(sockfd);

	}

	socket.CloseSocket(sockfd);
	ShutdownSockets();
	return 0;
}
