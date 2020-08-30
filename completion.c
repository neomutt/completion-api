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

#include "config.h"
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

  // initialise first item of linked list
  struct CompletionItem *matches = init_list();

  // start completion search from first item
  struct CompletionItem *current = find_first(items);

  // iterate through possible completions
  while(current != NULL) {
    if (match(typed_string, current->full_string, flags)) {
      matches = add_item(matches, current);

      // return first match only
      if (flags & MUTT_COMP_FIRSTMATCH) {
        return matches;
      }
    }

    current = current->next;
  }

  // found no match: destroy
  if (is_empty(matches)) {
    clear_list(matches, MUTT_COMP_LIST_BOTH);
    return NULL;
  }

  return find_first(matches);
}

struct CompletionItem *cycle_completion(struct CompletionItem *current) {
  // TODO make this a completion_item function?
  if (current->next != NULL)
  {
    return current->next;
  }
  else
  {
    return find_first(current);
  }
}
