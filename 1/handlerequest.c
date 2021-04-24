#include <stdio.h>
#include <unistd.h>
#include <string.h>


#include "auxiliary.h"

#define REQUEST_BUFFER_SIZE 2048

//might be using cpp features here, decided to put this into a seperate file as a result.

static const char html[] = "text/html; charset=UTF-8";
static const char jpeg[] = "image/jpeg";
static const char png[] = "image/png";
static const char plaintext[] = "text/plain";
static const char gif[] = "image/gif";
static const char binary[] = "application/octet-stream";


void fixspaces(char* str)
{
	int i = 0;
	int j = 0;
	//looking for %20s replacing them with spaces. will iterate through this in two ways.
	//in a real webserver im supposed to do this with all %##s but since that isn't the spec requirements im leaning on the side of caution.

	//The more I look at this function the more I'm impressed that I was able to come up with it. 
	for(; str[j] != '\0'; j++, i++)
	{
		if(str[j] == '%' && str[j+1] == '2' && str[j+2] == '0')
		{
			str[i] = ' ';
			j+= 2;
		}
		else
			str[i] = str[j];
	}
	str[i] = str[j];
	return;
}

int handlerequest(int socketfd)
{
	unsigned int i;
	FILE* sockf = fdopen(socketfd, "r");
	FILE* requested;
	//apparently converting file descriptors to istreams can't be done, guess I'll be using C.
	//for now I think this buffer will be enough for whatever is thrown at us
	char buffer[REQUEST_BUFFER_SIZE];
	const char* typestring;
	char* filename;
	char* httpversion;
//for now assuming buffer does not surpass 500 characters, since filenames typically can only be 255 characters long.

	
	if(fgets(buffer, REQUEST_BUFFER_SIZE, sockf) == NULL)
	{
		//error or end of file, either way handle it
		generalError("Message fgets failed:");
		return -1; //cant read buffer
	}
	
	if(memcmp("GET /", buffer, 5))
	{
		//message does not start with GET, abort
		generalError("Message does not start with GET");
		return -1; //returning for now
	}
	//strtok is a lifesaver

	strtok(buffer, " \r\\");
	filename = strtok(NULL, " \r\\") + 1; // +1 gets rid of front slash

	httpversion = strtok(NULL, " \r\\");

	fixspaces(filename);
	//all strings null terminated, at this point we can start crafting a response.
	//Doesn't seem its necessary to scan the rest of this header.

	//going to craft a response in here since it doesn't seem too difficult

	//first check if file exists and is readable
	fprintf(stderr, "Attempting to access '%s'\n", filename);
	if(access(filename, R_OK) || (requested = fopen(filename, "r")) == NULL)
	{
		//file does not exists, or access cannot be granted.
		fprintf(stderr, "404 Error accessing '%s': ", filename);
		dprintf(socketfd, "%s 404 Not Found/r/n", httpversion); //This isn't quite always a 404, or rather shouldn't be since more errors can happen here than a mere file not found. But apparently we are not being graded on error handling so yea. 
		perror(NULL);
		fflush(sockf);
		fclose(sockf);
		close(socketfd);
		return -1;
	}

	//want to find the type of the file, do to this we have to find the end of the filename, strlen is easiest for this.

	i = strlen(filename);
	//5 away fron null terminator for .html, 4 for the 3 letter one
	if(i >= 4)
	{
		if(!strcmp(filename + i - 4, ".gif"))
			typestring = gif;
		else if(!strcmp(filename + i - 4, ".txt"))
			typestring = plaintext;
		else if(!strcmp(filename + i - 4, ".jpg"))
			typestring = jpeg;
		else if(!strcmp(filename + i - 4, ".png"))
			typestring = png;
		else if(i >= 5 && !strcmp(filename + i - 5, ".html"))
			typestring = html;
		else
			typestring = binary;
	}
	else
		typestring = binary;
	
	//assuming everything is good we begin here, TODO, allow other types of content like text

	//from https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
	//calculate length of file to be sent
	fseek(requested, 0, SEEK_END);
	i = ftell(requested);
	rewind(requested);
	//now we can begin, we can directly just write the response
	
	if(dprintf(socketfd, "%s 200 OK\r\nContent-Type: %s\r\nContent-Length: %u\r\n\r\n", httpversion, typestring, i) < 0)
	{
		perror("Error writing header to socket");
	}
	//now we can reuse buffer since we no longer need the content
	while((i = fread(buffer, sizeof(char), REQUEST_BUFFER_SIZE, requested)))
		if(write(socketfd, buffer, i) == 0)
			perror("Error writing to socket");
	fflush(sockf);
	fclose(requested);
	fclose(sockf);
	close(socketfd);
	fprintf(stderr, "HTTP response complete\n");
	return 0;

	
}

int printrequest(int socketfd, int outputfd)
{

	FILE* sockf = fdopen(socketfd, "r"); //dprintf is apparently not POSIX, not that I care too much, only costs me an additional line
	
	char buf[1000];
	while(fgets(buf, 1000, sockf) != NULL)
		if(dprintf(outputfd, "%s", buf) < 0)
			return -1;
	printf("\n");
	return 0;
}
