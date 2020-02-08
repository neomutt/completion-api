CFLAGS=-Wall -g -std=c99

all: completion tests

completion.o: completion.c

tests: completion.o completion.h

clean:
	rm -f completion tests
