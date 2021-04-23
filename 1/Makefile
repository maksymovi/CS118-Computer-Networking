CC=gcc
CFLAGS= -Wall -Wextra  -g -O2 #-Werror
DEBUGFLAGS = -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fsanitize=bounds


CFILES=main.c auxiliary.c
OFILES=main.o auxiliary.o
HEADERS=auxiliary.h


CLEANFILES=server *~ *.o


default: server




%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OFILES)
	$(CC) -o server $(OFILES)


debug: CFLAGS += $(DEBUFGLAGS)
debug: server



clean:
	rm -f $(CLEANFILES)

.PHONY: clean default debug
