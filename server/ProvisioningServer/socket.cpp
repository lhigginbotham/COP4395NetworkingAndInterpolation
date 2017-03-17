#include "socket.hpp"

Socket::Socket(int ai_family, int ai_socktype, int ai_flags)
{
	memset(&hints, 0, sizeof hints);
	hints.ai_family = ai_family;
	hints.ai_socktype = ai_socktype;
	hints.ai_flags = ai_flags;

}

Socket::~Socket()
{

}

int Socket::CloseSocket(evutil_socket_t socketfd)
{
#if PLATFORM == PLATFORM_WINDOWS
	return closesocket(socketfd);
#else
	return close(socketfd);
#endif
}

evutil_socket_t Socket::Open(char port[6])
{
	int rv;
	evutil_socket_t sockfd;
	struct addrinfo *servinfo;
	

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		std::cout << "getaddrinfo failed\n" << gai_strerror(rv) << "\n";
		ShutdownSockets();
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			CloseSocket(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return -1;
	}

	freeaddrinfo(servinfo);

	return sockfd;
}

int Socket::Receive(evutil_socket_t sockfd)
{
	char s[INET6_ADDRSTRLEN];
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		ShutdownSockets();
		return -1;
	}

	printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	return 0;
}

bool Socket::Send(int sockfd, char message[], int length, int flag, sockaddr*ai_addr, size_t ai_addrlen)
{
	int numbytes;
	if ((numbytes = sendto(sockfd, message, length, flag,
		ai_addr, ai_addrlen)) == -1) {
		perror("talker: sendto");
		return false;
	}
	return true;
}