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

OUT	= test_completion test_statemach

SRC_SHARED	= completion.c
SRC_STATE	= test_statemach.c statemach.c
SRC_FUZZY 	= test_fuzzy.c fuzzy.c
SRC_COMP	= test_completion.c

OBJ_SHARED	= $(SRC_SHARED:%.c=%.o)
OBJ_STATE	= $(SRC_STATE:%.c=%.o) $(OBJ_SHARED)
OBJ_FUZZY	= $(SRC_FUZZY:%.c=%.o)
OBJ_COMP	= $(SRC_COMP:%.c=%.o) $(OBJ_SHARED)

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test_statemach: $(OBJ_STATE)
	$(CC) -o $@ $(OBJ_STATE) $(LDFLAGS)

test_completion: $(OBJ_COMP)
	$(CC) -o $@ $(OBJ_COMP) $(LDFLAGS)

test_fuzzy: $(OBJ_FUZZY)
	$(CC) -o $@ $(OBJ_FUZZY) $(LDFLAGS)

test:	test_statemach test_completion
	./test_statemach
	./test_completion

clean:
	$(RM) $(OBJ_SHARED) $(OBJ_STATE) $(OBJ_COMP) $(OUT)

distclean: clean
	$(RM) tags
	$(RM) *.gc??
	$(RM) lcov.info lcov

tags:	$(SRC) $(HDR) force
	ctags -R .

lcov: all test force
	$(RM) lcov test_statemach.gc?? test_completion.gc??
	lcov -t "result" -o lcov.info -c -d .
	genhtml -o lcov lcov.info

format: *.c
	clang-format -i $?

compile_commands:
	bear make all

force:

