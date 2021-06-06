CC = gcc

CFLAGS = -Wall -Wextra

COMPILE = $(CC) $(CFLAGS)

UID = 605208884

default: build

build: server client

server: server.c
	$(COMPILE) -o server server.c

client: client.c
	$(COMPILE) -o client client.c

dist: server.c client.c Makefile README
	tar -czvf $(UID).tar.gz server.c client.c Makefile README
clean: 
	rm -f server client *.file
