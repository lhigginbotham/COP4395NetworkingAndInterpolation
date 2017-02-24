#include "socket.hpp"

Socket::Socket()
{

}

Socket::~Socket()
{

}

int Socket::CloseSocket(int socketfd)
{
#if PLATFORM == PLATFORM_WINDOWS
	return closesocket(socketfd);
#else
	return close(socketfd);
#endif
}

bool Send(int sockfd, char message[], int length, int flag, sockaddr*ai_addr, size_t ai_addrlen)
{
	int numbytes;
	if ((numbytes = sendto(sockfd, message, length, flag,
		ai_addr, ai_addrlen)) == -1) {
		perror("talker: sendto");
		return false;
	}
	return true;
}