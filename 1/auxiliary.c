#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void fatalError(const char* prefix)
{
	perror(prefix);
	exit(EXIT_FAILURE);
}

void generalError(const char* message)
{
	fprintf(stderr, "%s", message);
	exit(EXIT_FAILURE);
}
