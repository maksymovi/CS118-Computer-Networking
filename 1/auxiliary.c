#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void fatalError(char* prefix)
{
	perror(prefix);
	exit(EXIT_FAILURE);
}
