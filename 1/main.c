#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>

#include "auxiliary.h"


//DEFINES
#define ENTRYPORT 8080
#define SOCKET_BACKLOG 3



int main()
{
	//start by opening a socket of course.

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	int acceptedfd;
	int count;

	struct sockaddr_in addr;
	
	char buffer[1000];
		
	socklen_t addrLen = sizeof(addr); //probably unnecessary but eh
	
	if(socketfd <= 0) //fatal error, print and exit
		fatalError("Socket creation error");

	//possible use of setsockopt here, but not needed for now


	//setting up for bind
	addr.sin_family = AF_INET;
	addr.sin_port = htons( ENTRYPORT );
	addr.sin_addr.s_addr = INADDR_ANY;

	//and might as well set the 0 bytes
	
	for(int i = 0; i < 8; i++)
		addr.sin_zero[i] = 0;
	
	//now we bind
	if(bind(socketfd, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) //nonzero return is fatal error
		fatalError("Socket Bind Error");

	//listen
	if(listen(socketfd, SOCKET_BACKLOG) < 0)
		fatalError("Socket Listen Error");

	//accept

	if((acceptedfd = accept(socketfd, (struct sockaddr *) &addr, &addrLen)) < 0)
		fatalError("Socket Accept Error");

		
	count = read(acceptedfd, buffer, 999);
	if(count < 0)
		fatalError("Read error");
	buffer[999] = '\0';
	printf("%s", buffer);
	
	if(close(acceptedfd))
		fatalError("Socket Close Error");
	
	

	
	if(close(socketfd))
		fatalError("Socket Close Error");
	
	
}
