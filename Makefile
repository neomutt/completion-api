CFLAGS=-Wall -g -std=c99 -lbsd

all: completion tests

completion.o: completion.c

completion_item.o: completion_item.c

tests: completion.o completion.h completion_item.h completion_item.o

clean:
	rm -f completion tests
