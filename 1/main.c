#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>


#include "auxiliary.h"


//DEFINES
#define MAIN_PORT 8001




int main()
{
	//start by opening a socket of course.

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	
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

	if(close(socketfd))
		fatalError("Socket Close Error");
	
	
}
