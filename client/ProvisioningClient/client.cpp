#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3
#define _WIN32_WINNT _WIN32_WINNT_VISTA

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#endif

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib")
#endif

#include <json.hpp>

bool InitializeSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2, 2),
		&WsaData)
		== NO_ERROR;
#else
	return true;
#endif
}

int CloseSocket(int socketfd)
{
#if PLATFORM == PLATFORM_WINDOWS
	return closesocket(socketfd);
#else
	return close(socketfd);
#endif
}

void ShutdownSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

#include <iostream>
#include <string.h>
#include "frequency.hpp"

#define SERVERPORT "4951"    // the port users will be connecting to

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	InitializeSockets();

	if (argc != 3) {
		fprintf(stderr, "usage: talker hostname message\n");
		ShutdownSockets();
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		ShutdownSockets();
		return 1;
	}

	// loop through all the results and make a socket
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		ShutdownSockets();
		return 2;
	}

	std::default_random_engine generator;
	unsigned long int i = 0;
	int num = 0;
	std::time_t previousTime = 0;
	while (true) {
		if (i < 100000000)
		{
			i++;
			continue;
		}
		if (num > 10)
			num = 0;
		std::chrono::time_point<std::chrono::system_clock> times = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(times);
		if (previousTime >= time)
		{
			continue;
		}
		for (int i = 0; i < 15; i++)
		{
			std::string t = Message(argv[2], generator, num, time, i);
			nlohmann::json freq = nlohmann::json::parse(t.c_str());
			std::string s = freq.dump();
			std::cout << "Out: " << freq.dump() << "\n";
			const char* spoint = s.c_str();
			if ((numbytes = sendto(sockfd, spoint, strlen(spoint), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				ShutdownSockets();
				exit(1);
			}
		}
		i = 0;
		num++;
		previousTime = time;
	}
	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	CloseSocket(sockfd);

	ShutdownSockets();

	return 0;
}