#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <strings.h>
#include <wchar.h>
#include <locale.h>
#include "mutt/array.h"
#include "mutt/string2.h"
#include "mutt/mbyte.h"
#include "mutt_logging.h"
#include "config.h"
#include "completion.h"
#include "fuzzy.h"

#ifndef MAX_TYPED
#define MAX_TYPED 100
#endif

// TODO replace with mutt_error(...), mutt_warning(...), mutt_message(...), mutt_debug(LEVEL, ...)
#ifndef LOGGING
#define logerr(M, ...) printf("ERR: %s%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define logwar(M, ...) printf("WAR: %s%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define loginf(M, ...) printf("INF: %s%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define logdeb(L, M, ...) printf("DBG%d: %s%d: " M "\n",\
    L, __FILE__, __LINE__, ##__VA_ARGS__)
#endif

// TODO how can we best handle this...?
// could use wcscoll as well (locale aware)
#define WSTR_EQ(s1, s2) wcscmp(s1, s2) == 0

struct Completion *compl_new(MuttCompletionFlags flags);
// TODO add an initialiser which takes a const ARRAY of strings with arbitrary size
// TODO handle strings with dynamic size (keep track of longest string)
int compl_add(struct Completion *comp, const char *str, const size_t buf_len);
int compl_type(struct Completion *comp, const char *str, const size_t buf_len);

// TODO take a default string (multibyte instead of wchar_t
bool compl_state_init(struct Completion *comp, char **result, size_t *match_len);
bool compl_state_match(struct Completion *comp, char **result, size_t *match_len);
void compl_state_multi(struct Completion *comp, char **result, size_t *match_len);

char* compl_complete(struct Completion *comp);

// these are helper functions to check string health etc.
// TODO there must be equivalent mutt functions to handle user input safely
int compl_health_check(const struct Completion *comp);
int compl_str_check(const char *str, const size_t buf_len);
int compl_str_check(const char *str, const size_t buf_len);
int compl_get_size(struct Completion *comp);
bool compl_check_duplicate(const struct Completion *comp, const char *str, const size_t buf_len);

// the main matching function
int match_dist(const char *src, const char *tar, const struct Completion *comp);
