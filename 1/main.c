#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

#include "auxiliary.h"


//DEFINES
#define MAIN_PORT 8001
#define SOCKET_BACKLOG 5



int main()
{
	//start by opening a socket of course.

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	int acceptedfd;

	FILE * acceptStream;
	struct sockaddr_in addr;

	char buffer[1000];
	char* bufptr;
	//things for accepted sockets
	struct sockaddr_in acceptedAddr;
	socklen_t acceptedAddrLen; //probably unnecessary but eh
	
	if(socketfd < 0) //fatal error, print and exit
		fatalError("Socket creation error");

	//possible use of setsockopt here, but not needed for now

	//setting up for bind
	addr.sin_family = AF_INET;
	addr.sin_port = htonl(MAIN_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//and might as well set the 0 bytes
	for(int i = 0; i < 8; i++)
		addr.sin_zero[i] = 0;

	//now we bind
	if(bind(socketfd, (struct sockaddr *) &addr, sizeof(addr))) //nonzero return is fatal error
		fatalError("Socket Bind Error");

	//listen
	if(listen(socketfd, SOCKET_BACKLOG))
		fatalError("Socket Listen Error");

	//accept
	acceptedfd = accept(socketfd, (struct sockaddr *) &acceptedAddr, &acceptedAddrLen);

	if(acceptedfd < 0)
		fatalError("Socket Accept Error");

	acceptStream = fdopen(acceptedfd, "r");
	
	while((bufptr = fgets(buffer, 1000, acceptStream)) != NULL)
	{
		printf("%s", buffer);
	}
	printf("\n");
	
	if(close(acceptedfd))
		fatalError("Socket Close Error");
	
	

	
	if(close(socketfd))
		fatalError("Socket Close Error");
	
	
}
