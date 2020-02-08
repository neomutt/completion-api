/**
 * @file
 * Autocompletion API
 *
 * @authors
 * Copyright (C) 2019 Simon V. Reichel <simonreichel@giese-optik.de>
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

/**
 * @page completion neomutt completion API
 *
 * Neomutt completion API
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "completion.h"

bool capital_diff(char ch1, char ch2) {
  // only alphabetic chars can differ in caps
  if (!(isalpha(ch1) && isalpha(ch2))) {
    return false;
  }

  // both chars capitalized should be the same
  if (toupper(ch1) == toupper(ch2)) {
    return true;
  }

  return false;
}

bool match(char *str1, char *str2, MuttCompletionFlags flags) {
  // longer string can not be matched anyway
  if (strlen(str1) > strlen(str2)) {
    return false;
  }

  // TODO maybe use int strncasecmp(const char *s1, const char *s2, size_t n) from <strings.h>?
  // character-wise comparison
  for (int i = 0; i < strlen(str1); ++i) {
    if ((flags & MUTT_COMP_IGNORECASE) && capital_diff(str1[i], str2[i])) {
      continue;
    }

    if (str1[i] != str2[i]) {
      return false;
    }
  }

  return true;
}

struct CompletionItem *complete(struct CompletionItem *items, char *typed_string, size_t typed_len, MuttCompletionFlags flags) {
  size_t list_len = items->list_length;
  struct CompletionItem *matchlist = calloc(list_len, sizeof(struct CompletionItem));
  matchlist[0].full_string = "";

  int n = 0;

  // iterate through possible completions
  for (int i = 0; i < list_len; i++) {
    if (match(typed_string, items[i].full_string, flags)) {
      // return first match only
      if (flags & MUTT_COMP_FIRSTMATCH) {
        free(matchlist);
        items[i].list_length = 1;
        return &items[i];
      } else {
        matchlist[n] = items[i];
        n++;
        // TODO add match to matchlist
      }
    }
  }

  // found no match
  if (matchlist[0].full_string[0] == '\0') {
    free(matchlist);
    return NULL;
  } else {
    matchlist[0].list_length = n;
    return &matchlist[0];
  }
}
