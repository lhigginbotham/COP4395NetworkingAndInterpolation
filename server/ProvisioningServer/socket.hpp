#ifndef SOCKET_H_
#define SOCKET_H_

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

class Socket {
public:
	Socket();
	~Socket();
	int CloseSocket(int sockfd);
	bool Send(int sockfd, char* message[], int length, int flag, sockaddr*ai_addr, size_t ai_addrlen);
};

#endif