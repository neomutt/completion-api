#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t MuttCompletionFlags;
#define MUTT_COMP_NO_FLAGS          0  /// < No flags are set
#define MUTT_COMP_IGNORECASE  (1 << 0) /// < Ignore the case of letters
#define MUTT_COMP_FIRSTMATCH  (1 << 1) /// < Return only the first match
/* #define MUTT_COMP_OPTIONAL    (1 << 2) /// < Allow optional field processing */
/* #define MUTT_COMP_STAT_FILE   (1 << 3) /// < Used by attach_format_str */
/* #define MUTT_COMP_ARROWCURSOR (1 << 4) /// < Reserve space for arrow_cursor */
/* #define MUTT_COMP_INDEX       (1 << 5) /// < This is a main index entry */
/* #define MUTT_COMP_NOFILTER    (1 << 6) /// < Do not allow filtering on this pass */
/* #define MUTT_COMP_PLAIN       (1 << 7) /// < Do not prepend DISP_TO, DISP_CC ... */

#ifndef MUTT_COMPLETION_H
#define MUTT_COMPLETION_H

bool capital_diff(char ch1, char ch2);

bool match(char *str1, char *str2, MuttCompletionFlags flags);

struct CompletionItem *complete(struct CompletionItem *items, char *typed_string, size_t typed_len, MuttCompletionFlags flags);

struct CompletionItem *cycle_completion(struct CompletionItem *current);

#endif /* ifndef MUTT_COMPLETION_H */
