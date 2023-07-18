#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <regex.h>
#include "mutt/array.h"

#ifndef MUTT_COMPL_NEW
typedef uint8_t MuttCompletionState;

#define MUTT_COMPL_NEW        0        /// < Initial state
#define MUTT_COMPL_INIT      (1 << 0)  /// < Initial state
#define MUTT_COMPL_SINGLE    (1 << 1)  /// < Match found
#define MUTT_COMPL_MULTI     (1 << 2)  /// < Multiple matches with common stem
#define MUTT_COMPL_NOMATCH   (1 << 3)  /// < No Match found
#endif

#ifndef MUTT_MATCH_NOFLAGS
enum MuttMatchMode
{
  COMPL_MODE_EXACT = 1,
  COMPL_MODE_FUZZY,
  COMPL_MODE_REGEX
};

typedef uint8_t MuttMatchFlags;

#define MUTT_MATCH_NOFLAGS           0
#define MUTT_MATCH_IGNORECASE  (1 << 1) /// ignore case when matching
#define MUTT_MATCH_FIRSTMATCH  (1 << 2) /// < Return only the first match
#define MUTT_MATCH_SHOWALL     (1 << 3) /// < Return non-matches after all matches
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
ARRAY_HEAD(CompletionStringList, char *);

typedef struct Completion {
  char *typed_str;
  CompletionItem *cur_item;
  size_t stem_len;
  MuttCompletionState state;
  enum MuttMatchMode mode;
  MuttMatchFlags flags;
  struct CompletionList *items;
  // store the compiled regcomp regex for faster list matching
  bool regex_compiled;
  regex_t *regex;
} Completion;
#endif
