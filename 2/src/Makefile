CC = gcc

CFLAGS = -Wall -Wextra

COMPILE = $(CC) $(CFLAGS)

default: build

build: server client

server: server.c
	$(COMPILE) -o server server.c

client: client.c
	$(COMPILE) -o client client.c
clean: 
	rm server client *.file
