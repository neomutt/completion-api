#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>

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

bool capital_diff(wchar_t ch1, wchar_t ch2);

bool match(const wchar_t *str1, const wchar_t *str2, MuttCompletionFlags flags);

#endif /* ifndef MUTT_COMPLETION_H */
