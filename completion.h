#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <regex.h>
#include "mutt/array.h"

#ifndef MUTT_COMPL_NEW
typedef uint8_t MuttCompletionState;

#define MUTT_COMPL_NEW        0        /// < Initial state
#define MUTT_COMPL_INIT      (1 << 0)  /// < Initial state
#define MUTT_COMPL_MULTI     (1 << 1)  /// < Multiple matches with common stem
#define MUTT_COMPL_MATCH     (1 << 2)  /// < Match found
#define MUTT_COMPL_NOMATCH   (1 << 3)  /// < No Match found
#endif

#ifndef MUTT_COMPL_NO_FLAGS
typedef uint8_t MuttCompletionFlags;

#define MUTT_COMPL_NO_FLAGS          0  /// < No flags are set
#define MUTT_COMPL_FIRSTMATCH  (1 << 1) /// < Return only the first match
#endif

#ifndef MUTT_MATCH_EXACT
typedef uint8_t MuttMatchFlags;

#define MUTT_MATCH_EXACT             0  /// normal exact string matching
#define MUTT_MATCH_FUZZY       (1 << 0) /// use fuzzy string matching
#define MUTT_MATCH_REGEX       (2 << 0) /// use regular expression matching
#define MUTT_MATCH_IGNORECASE  (3 << 0) /// ignore the case of letters
#endif

#ifndef MUTT_MATCHING
#define MUTT_MATCHING
#define REGERRORSIZE 30
#endif

#ifndef MUTT_COMPLETION
#define MUTT_COMPLETION
typedef struct CompletionItem {
  char *str;
  int match_dist;
  bool is_match;
} CompletionItem;

ARRAY_HEAD(CompletionList, CompletionItem);

typedef struct Completion {
  char *typed_str;
  CompletionItem *cur_item;
  size_t stem_len;
  MuttCompletionState state;
  MuttCompletionFlags flags;
  struct CompletionList *items;
  // store the compiled regcomp regex for faster list matching
  bool regex_compiled;
  regex_t *regex;
} Completion;
#endif
