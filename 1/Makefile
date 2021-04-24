CC=gcc

DEBUGFLAGS = -fsanitize=address  -fno-omit-frame-pointer -fsanitize=bounds
CFLAGS= -Wall -Wextra  -g -O2

LINKER=

DEBUGLINKER= -lasan

CFILES=main.c auxiliary.c handlerequest.c
OFILES=main.o auxiliary.o handlerequest.o
HEADERS=auxiliary.h handlerequest.h


CLEANFILES=server *~ $(OFILES)


default: server


%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OFILES)
	$(CC) -o server $(OFILES) $(LINKER)


debug: CFLAGS += $(DEBUGFLAGS)
debug: LINKER += $(DEBUGLINKER)
debug: server $(OFILES)



clean:
	rm -f $(CLEANFILES)

.PHONY: clean default debug
