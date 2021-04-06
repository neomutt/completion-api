#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "fuzzy.h"

typedef uint8_t MuttMatchFlags;

#ifndef MUTT_MATCH_FLAGS
#define MUTT_MATCH_NORMAL          0  /// normal exact string matching
#define MUTT_MATCH_FUZZY     (1 << 0) /// use fuzzy string matching
#define MUTT_MATCH_REGEX     (2 << 0) /// use regular expression matching
#endif

#ifndef MUTT_MATCHING
#define MUTT_MATCHING
#define REGERRORSIZE 30
int match_dist(char *a, char *tar, MuttMatchFlags flags);
int dist_regex(char *src, char *tar);
int dist_normal(char *src, char *tar);
#endif /* ifndef MUTT_MATCHING */
