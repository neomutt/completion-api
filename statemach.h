#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "mutt/array.h"
#include "mutt/string2.h"
#include "completion.h"

#ifndef MAX_TYPED
#define MAX_TYPED 100
#endif

typedef uint8_t MuttCompletionState;

#define MUTT_COMP_NEW        0        /// < Initial state
#define MUTT_COMP_INIT      (1 << 0)  /// < Initial state
#define MUTT_COMP_MULTI     (1 << 1)  /// < Multiple matches with common stem
#define MUTT_COMP_MATCH     (1 << 2)  /// < Match found
#define MUTT_COMP_NOMATCH   (1 << 3)  /// < No Match found

struct CompItem {
  char *str;
  size_t str_len;
  bool is_match;
};

ARRAY_HEAD(CompletionList, struct CompItem);

struct Completion {
  char *typed_str;
  char *cur_str;
  size_t typed_len;
  size_t stem_len;
  size_t cur_len;
  MuttCompletionState state;
  MuttCompletionFlags flags;
  struct CompletionList *items;    // for now, stick with the linked list: TODO switch to Arraylist later
};

struct Completion *comp_new(MuttCompletionFlags flags);
void comp_add(struct Completion *comp, char *str, size_t str_len);
void comp_type(struct Completion *comp, char *str, size_t str_len);
char* comp_complete(struct Completion *comp);

/* comp_calc(comp) */
/*     User presses <tab> */
/*     state is INITIAL */
/*         str = "a", len = 1 */
/*         foreach item: does item match str? */
/*             set item.is_match */
/*             find common stem */
/*         common stem is "ap" */
/*         str = "ap", len = 2, stem_len = 2 */
/*     state = COMMON */

/* comp_calc(comp) */
/*     User presses <tab> for second time */
/*     state is COMMON, so starting at cur_match, find first match */
/*         str = "apfel", len = 5, cur_match = "apfel" */
/*     state = MATCH */

/* comp_calc(comp) */
/*     User presses <tab> for third time */
/*     state is MATCH, so starting at cur_match, find first match */
/*         str = "apples", len = 6, cur_match = "apples" */
/*     state = MATCH */

/* comp_calc(comp) */
/*     User presses <tab> for fourth time */
/*     state is MATCH, so starting at cur_match, find first match */
/*         str = "apply", len = 5, cur_match = "apply" */
/*     state = MATCH */

/* comp_calc(comp) */
/*     User presses <tab> for fifth time */
/*     state is MATCH, so starting at cur_match, find first match */
/*         no more matches */
/*     str = "a", len = stem_len, cur_match = NULL */
/*     state = COMMON */

/* comp_free(comp) */
/*     Clean up */
