#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>

typedef uint8_t MuttCompletionFlags;
#define MUTT_COMPL_NO_FLAGS          0  /// < No flags are set
#define MUTT_COMPL_IGNORECASE  (1 << 0) /// < Ignore the case of letters
#define MUTT_COMPL_FIRSTMATCH  (1 << 1) /// < Return only the first match

#ifndef MUTT_COMPLETION_H
#define MUTT_COMPLETION_H

bool capital_diff(wchar_t ch1, wchar_t ch2);

bool match(const wchar_t *str1, const wchar_t *str2, MuttCompletionFlags flags);

#endif /* ifndef MUTT_COMPLETION_H */
