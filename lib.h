#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <regex.h>
#include "mutt/array.h"

#ifndef COMPLETION
#define COMPLETION
typedef uint8_t MuttCompletionState;

#define COMPL_STATE_NEW        0        /// < Initial state
#define COMPL_STATE_INIT      (1 << 0)  /// < Initial state
#define COMPL_STATE_SINGLE    (1 << 1)  /// < Match found
#define COMPL_STATE_MULTI     (1 << 2)  /// < Multiple matches with common stem
#define COMPL_STATE_NOMATCH   (1 << 3)  /// < No Match found

enum MuttMatchMode
{
  COMPL_MODE_EXACT = 1,
  COMPL_MODE_FUZZY,
  COMPL_MODE_REGEX
};

typedef uint8_t MuttMatchFlags;

#define COMPL_MATCH_NOFLAGS           0  /// this means cycle results, case-sensitive
#define COMPL_MATCH_IGNORECASE  (1 << 1) /// ignore case when matching
#define COMPL_MATCH_FIRSTMATCH  (1 << 2) /// < Return only the first match
#define COMPL_MATCH_SHOWALL     (1 << 3) /// < Return non-matches after all matches

// needed for regcomp error reporting
#define COMPL_REGERRORSIZE 30

typedef struct CompletionItem {
  char *str;
  int match_dist;
  bool is_match;
} CompletionItem;

ARRAY_HEAD(CompletionList, CompletionItem);
ARRAY_HEAD(CompletionStringList, char *);

typedef struct Completion {
  CompletionItem *typed_item;
  CompletionItem *cur_item;
  size_t stem_len;
  MuttCompletionState state;
  enum MuttMatchMode mode;
  MuttMatchFlags flags;
  struct CompletionList *items;
  // store the compiled regcomp regex for faster list matching
  bool regex_compiled;
  regex_t regex;
} Completion;
#endif
