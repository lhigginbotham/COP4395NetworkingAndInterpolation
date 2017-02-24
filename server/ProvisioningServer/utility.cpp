#include "utility.hpp"

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

void ShutdownSockets() noexcept
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}