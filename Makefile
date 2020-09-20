NEO	?= ../neo
RM	= rm -fr

CFLAGS	+= -Wall
CFLAGS	+= -g
CFLAGS	+= -O0
CFLAGS	+= -I$(NEO)
CFLAGS	+= -I$(NEO)/test
CFLAGS	+= -std=c99

LDFLAGS	+= -L$(NEO)
LDFLAGS	+= -lmutt
LDFLAGS	+= -lpcre2-8

# Enable code coverage
CFLAGS	+= -fprofile-arcs -ftest-coverage
LDFLAGS	+= -fprofile-arcs -ftest-coverage

# Enable Address Sanitizer
# CFLAGS	+= -fsanitize=address -fsanitize-recover=address
# LDFLAGS	+= -fsanitize=address -fsanitize-recover=address

OUT	= state_tests tests

SRC_SHARED	= completion.c
SRC_STATE	= state_tests.c statemach.c
SRC_TESTS	= tests.c

OBJ_SHARED	= $(SRC_SHARED:%.c=%.o)
OBJ_STATE	= $(SRC_STATE:%.c=%.o) $(OBJ_SHARED)
OBJ_TESTS	= $(SRC_TESTS:%.c=%.o) $(OBJ_SHARED)

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

state_tests: $(OBJ_STATE)
	$(CC) -o $@ $(OBJ_STATE) $(LDFLAGS)

tests: $(OBJ_TESTS)
	$(CC) -o $@ $(OBJ_TESTS) $(LDFLAGS)

test:	state_tests tests
	./state_tests
	./tests

clean:
	$(RM) $(OBJ_SHARED) $(OBJ_STATE) $(OBJ_TESTS) $(OUT)

distclean: clean
	$(RM) tags
	$(RM) *.gc??
	$(RM) lcov.info lcov

tags:	$(SRC) $(HDR) force
	ctags -R .

lcov: all test force
	$(RM) lcov state_tests.gc?? tests.gc??
	lcov -t "result" -o lcov.info -c -d .
	genhtml -o lcov lcov.info

format: *.c
	clang-format -i $?

compile_commands:
	bear make all

force:

