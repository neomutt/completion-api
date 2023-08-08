/**
 * @file
 * Autocompletion API
 *
 * @authors
 * Copyright (C) 2023 Simon V. Reichel <simonreichel@giese-optik.de>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

// user functions
Completion *compl_new(enum MuttMatchMode mode);
Completion *compl_from_array(const struct CompletionStringList *list, enum MuttMatchMode mode);
void        compl_free(Completion *comp);

// TODO handle strings with dynamic size (keep track of longest string)
int         compl_add(Completion *comp, const char *str, const size_t buf_len);
int         compl_type(Completion *comp, const char *str, const size_t buf_len);

// this is the main interface function for users to collect/cycle the next matched string
char *      compl_complete(Completion *comp);

#endif
