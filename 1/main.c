#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>

#include "auxiliary.h"
#include "handlerequest.h"

//DEFINES
#define ENTRYPORT 8080
#define SOCKET_BACKLOG 5





int main()
{
	

	int socketfd, acceptedfd;
	

	struct sockaddr_in addr;	
	socklen_t addrLen = sizeof(addr); //probably unnecessary but eh

	//open socket
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(socketfd <= 0) //fatal error, print and exit
		fatalError("Socket creation error");



	
	//possibly use of setsockopt here, but not needed for now


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

	//how begins the accept loop.
	fprintf(stderr, "Now listening on port %d.", ENTRYPORT);
	while(1)
	{
		if((acceptedfd = accept(socketfd, (struct sockaddr *) &addr, &addrLen)) < 0)
			fatalError("Socket Accept Error");

		fprintf(stderr, "Recieved connection, sending to handler.\n");

/*		
#ifdef FORK_SOCKET
		int temp = fork();
		if(temp < 0)
			fatalError("Fork Error");
		if(temp) //fork the process, one to handle the http request, other to continue looping
		{
			//child process here
			if(close(socketfd)) //no need for the parent socket here, so we close it in this child
				perror("Main socket close error"); //decided this shouldn't be fatal
			//handle request
			return handlerequest(acceptedfd); //exit process here
		}
		else
		{
			//parent process
			if(close(acceptedfd)) //acceptedfd now exists in the other process, no need for it here
				perror("Accepted socket close error"); 
		}
		#else
*/
		handlerequest(acceptedfd);
//#endif

	}

	if(close(socketfd))
		perror("Main socket close error at exit");
	fprintf(stderr, "Exiting main process now.\n");
	return 0;
	
}
