CC=gcc

DEBUGFLAGS = -fsanitize=address  -fno-omit-frame-pointer -fsanitize=bounds
CFLAGS= -Wall -Wextra  -g -O2 $(DEBUGFLAGS)

CFILES=main.c auxiliary.c handlerequest.c
OFILES=main.o auxiliary.o handlerequest.o
HEADERS=auxiliary.h handlerequest.h


CLEANFILES=server *~ $(OFILES)


default: server




%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS) -lasan

server: $(OFILES)
	$(CC) -o server $(OFILES) -lasan


debug: CFLAGS += $(DEBUFGLAGS)
debug: server



clean:
	rm -f $(CLEANFILES)

.PHONY: clean default debug
