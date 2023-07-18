NEOMUTTDIR	?= ../neo
RM	= rm -fr

CFLAGS	+= -Wall
CFLAGS	+= -g
CFLAGS	+= -O0
CFLAGS	+= -I$(NEOMUTTDIR)
CFLAGS	+= -I$(NEOMUTTDIR)/test
CFLAGS	+= -std=c99

LDFLAGS	+= -L$(NEOMUTTDIR)
LDFLAGS	+= -lmutt
LDFLAGS	+= -lpcre2-8

# Enable code coverage
CFLAGS	+= -fprofile-arcs -ftest-coverage
LDFLAGS	+= -fprofile-arcs -ftest-coverage

# Enable Address Sanitizer
# CFLAGS	+= -fsanitize=address -fsanitize-recover=address
# LDFLAGS	+= -fsanitize=address -fsanitize-recover=address

OUT	= test_exact test_statemach test_matching test_regex test_fuzzy

SRC_STATE	= test_statemach.c statemach.c fuzzy.c
SRC_MATCH 	= test_matching.c statemach.c fuzzy.c
SRC_FUZZY 	= test_fuzzy.c fuzzy.c statemach.c
SRC_REGEX 	= test_regex.c fuzzy.c statemach.c
SRC_EXACT	= test_exact.c statemach.c fuzzy.c

OBJ_MATCH	= $(SRC_MATCH:%.c=%.o)
OBJ_FUZZY	= $(SRC_FUZZY:%.c=%.o)
OBJ_REGEX	= $(SRC_REGEX:%.c=%.o)
OBJ_EXACT	= $(SRC_EXACT:%.c=%.o)
OBJ_STATE	= $(SRC_STATE:%.c=%.o)

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test_statemach: $(OBJ_STATE)
	$(CC) -o $@ $(OBJ_STATE) $(LDFLAGS)

test_exact: $(OBJ_EXACT)
	$(CC) -o $@ $(OBJ_EXACT) $(LDFLAGS)

test_matching: $(OBJ_MATCH)
	$(CC) -o $@ $(OBJ_MATCH) $(LDFLAGS)

test_fuzzy: $(OBJ_FUZZY)
	$(CC) -o $@ $(OBJ_FUZZY) $(LDFLAGS)

test_regex: $(OBJ_REGEX)
	$(CC) -o $@ $(OBJ_REGEX) $(LDFLAGS)

test:	test_statemach test_exact test_matching test_fuzzy test_regex
	./test_statemach
	./test_exact
	./test_matching
	./test_fuzzy
	./test_regex

clean:
	$(RM) $(OBJ_SHARED) $(OBJ_STATE) $(OBJ_EXACT) $(OBJ_MATCH) $(OBJ_FUZZY) $(OBJ_REGEX) $(OUT) *.gcda *.gcno

distclean: clean
	$(RM) tags
	$(RM) *.gc??
	$(RM) lcov.info lcov

tags:	$(SRC) $(HDR) force
	ctags -R .

lcov: all test force
	$(RM) lcov test_statemach.gc?? test_exact.gc??
	lcov -t "result" -o lcov.info -c -d .
	genhtml -o lcov lcov.info

format: *.c
	clang-format -i $?

compile_commands:
	bear -- make all

force:

