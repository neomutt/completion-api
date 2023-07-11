#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <regex.h>
#include "mutt/array.h"
#include "mutt/string2.h"
#include "mutt/mbyte.h"
#include "mutt/logging.h"
#include "config.h"
#include "completion.h"
#include "matching.h"

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

typedef uint8_t MuttCompletionState;

#ifndef MUTT_COMPL_NEW
#define MUTT_COMPL_NEW        0        /// < Initial state
#define MUTT_COMPL_INIT      (1 << 0)  /// < Initial state
#define MUTT_COMPL_MULTI     (1 << 1)  /// < Multiple matches with common stem
#define MUTT_COMPL_MATCH     (1 << 2)  /// < Match found
#define MUTT_COMPL_NOMATCH   (1 << 3)  /// < No Match found
#endif

// TODO how can we best handle this...?
// could use wcscoll as well (locale aware)
#define WSTR_EQ(s1, s2) wcscmp(s1, s2) == 0

struct CompItem {
  wchar_t *str;
  size_t mb_buf_len;
  bool is_match;
};

ARRAY_HEAD(CompletionList, struct CompItem);

struct Completion {
  wchar_t *typed_str;
  size_t typed_mb_len;
  struct CompItem *cur_item;
  size_t stem_len;
  MuttCompletionState state;
  MuttCompletionFlags flags;
  struct CompletionList *items;
  // store the compiled regcomp regex for faster list matching
  bool regex_compiled;
  regex_t *regex;
};

struct Completion *compl_new(MuttCompletionFlags flags);
// TODO add an initialiser which takes a const ARRAY of strings with arbitrary size
// TODO handle strings with dynamic size (keep track of longest string)
int compl_add(struct Completion *comp, const char *str, size_t buf_len);
int compl_type(struct Completion *comp, const char *str, size_t buf_len);

// TODO take a default string (multibyte instead of wchar_t
bool compl_state_init(struct Completion *comp, wchar_t **result, size_t *match_len);
bool compl_state_match(struct Completion *comp, wchar_t **result, size_t *match_len);
void compl_state_multi(struct Completion *comp, wchar_t **result, size_t *match_len);

char* compl_complete(struct Completion *comp);

// these are helper functions to check string health etc.
// TODO there must be equivalent mutt functions to handle user input safely
int compl_health_check(const struct Completion *comp);
int compl_str_check(const char *str, size_t buf_len);
int compl_wcs_check(const wchar_t *str, size_t buf_len);
int compl_get_size(struct Completion *comp);
bool compl_check_duplicate(const struct Completion *comp, const wchar_t *str, size_t buf_len);
