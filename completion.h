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

#ifndef MUTT_MATCH_EXACT
typedef uint8_t MuttMatchFlags;

#define MUTT_MATCH_EXACT             0  /// normal exact string matching
#define MUTT_MATCH_FUZZY       (1 << 1) /// use fuzzy string matching
#define MUTT_MATCH_REGEX       (1 << 2) /// use regular expression matching
#define MUTT_MATCH_IGNORECASE  (1 << 3) /// ignore the case of letters
#define MUTT_MATCH_FIRSTMATCH  (1 << 4) /// < Return only the first match
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
  MuttMatchFlags flags;
  struct CompletionList *items;
  // store the compiled regcomp regex for faster list matching
  bool regex_compiled;
  regex_t *regex;
} Completion;
#endif
