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
#include <bsd/string.h>
#include "completion_item.h"

void free_completion(struct CompletionItem *items) {
  // TODO
}

struct CompletionItem *find_first(struct CompletionItem *from) {
  struct CompletionItem *cur = from;

  // propagate backwards until the first item is found
  while (cur->prev != NULL) {
    cur = cur->prev;
  }

  return cur;
}

struct CompletionItem *init_list() {
  struct CompletionItem *list = calloc(1, sizeof(struct CompletionItem));
  list->full_string = calloc(1, 1);
  strlcpy(list->full_string, "", 1);
  list->itemlength = 0;
  list->next = NULL;
  list->prev = NULL;
  return list;
}

struct CompletionItem *copy_item(struct CompletionItem *from) {
  struct CompletionItem *copy = calloc(1, sizeof(struct CompletionItem));
  copy->full_string = calloc(1, from->itemlength+1);
  strlcpy(copy->full_string, from->full_string, from->itemlength+1);
  copy->itemlength = from->itemlength;
  copy->next = NULL;
  copy->prev = NULL;
  return copy;
}

void replace_item(struct CompletionItem *base, struct CompletionItem *with) {
  if (base->full_string) free(base->full_string);
  base->full_string = calloc(1, with->itemlength+1);
  strlcpy(base->full_string, with->full_string, with->itemlength+1);
  base->itemlength = with->itemlength;
  base->next = NULL;
  base->prev = NULL;
}

struct CompletionItem *add_item(struct CompletionItem *base, struct CompletionItem *item)
{
  struct CompletionItem *end = base;

  // find end of linked list
  if (base->next != NULL) {
    while (end->next != NULL) {
      end = end->next;
    }
  }

  // if empty, add as first
  if (is_empty(base)) {
    replace_item(base, item);

    return base;
  }

  // add to the end
  end->next = copy_item(item);
  end->next->prev = end;

  return end->next;
}

bool is_empty(struct CompletionItem *base)
{
  struct CompletionItem *first = find_first(base);

  if (first->itemlength == 0) {
    return true;
  }

  return false;
}

void clear_list(struct CompletionItem *from, MuttCompletionWipeFlags flags)
{
  struct CompletionItem *next = from;
  struct CompletionItem *current = from;
  // find end of list in positive direction
  while (next != NULL)
  {
    current = next;
    next = current->next;
  }

  // here we found the last item
  next = current;

  // delete last item first, work backwards
  while (next != NULL)
  {
    current = next;
    next = current->prev;
    free(current);
  }
}
