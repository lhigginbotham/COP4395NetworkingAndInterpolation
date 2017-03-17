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
	
	int i;
	const char **methods = event_get_supported_methods();
	printf("Starting Libevent %s.  Available methods are:\n",
		event_get_version());
	for (i = 0; methods[i] != NULL; ++i) {
		printf("    %s\n", methods[i]);
	}

	struct event_base* base = event_base_new();

	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	printf("Using Libevent with backend method %s.",
		event_base_get_method(base));
	int f = event_base_get_features(base);
	if ((f & EV_FEATURE_ET))
		printf("  Edge-triggered events are supported.");
	if ((f & EV_FEATURE_O1))
		printf("  O(1) event notification is supported.");
	if ((f & EV_FEATURE_FDS))
		printf("  All FD types are supported.");
	puts("");

	

	printf("listener: waiting to recvfrom...\n");
	while (result > 0)
	{
		socket.Receive(sockfd);

	}

	socket.CloseSocket(sockfd);
	ShutdownSockets();
	return 0;
}
