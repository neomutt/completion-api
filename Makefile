CFLAGS=-Wall -g -std=c99 -lbsd

all: completion tests

completion.o: completion.c completion_item.h completion_item.o

completion_item.o: completion_item.c

tests: completion.o completion.h completion_item.h completion_item.o

state_tests: completion.o completion.h statemach.h statemach.o mutt/array.h mutt/string2.h mutt/string.o

clean:
	rm -f *.o completion tests tags state_tests
