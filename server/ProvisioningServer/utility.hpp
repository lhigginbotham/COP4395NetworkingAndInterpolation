#ifndef UTILITY_H_
#define UTILITY_H_

#if PLATFORM == PLATFORM_WINDOWS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#endif

bool InitializeSockets();
void ShutdownSockets() noexcept;
void *get_in_addr(struct sockaddr *sa);
#endif
