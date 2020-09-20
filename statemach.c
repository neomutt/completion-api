#include "config.h"
#include <stdint.h>
#include <stdlib.h>
#include "statemach.h"

struct Completion *comp_new(MuttCompletionFlags flags)
{
  // TODO check calloc success!
  struct Completion *comp = calloc(1, sizeof(struct Completion));

  comp->typed_str = calloc(MAX_TYPED, 1);
  comp->typed_len = 0;
  mutt_str_copy(comp->typed_str, "", 1);

  comp->cur_item = NULL;

  comp->state = MUTT_COMP_NEW;
  comp->flags = flags;

  comp->items = calloc(1, sizeof(struct CompletionList));
  // TODO call to memset is insecure?
  ARRAY_INIT(comp->items);
  return comp;
}

void comp_add(struct Completion *comp, char *str, size_t str_len)
{
  struct CompItem new_item = { 0 };

  // TODO should we copy the string into new memory or just use the callers allocation
  new_item.str = str;
  new_item.str_len = str_len;
  new_item.is_match = false;

  // TODO what about duplicates? better handle them here, I guess
  ARRAY_ADD(comp->items, new_item);
}

void comp_type(struct Completion *comp, char *str, size_t str_len)
{
  // TODO check other str copy options: overflow safety etc
  mutt_strn_copy(comp->typed_str, str, str_len, MAX_TYPED);
  comp->typed_len = str_len;

  comp->state = MUTT_COMP_INIT;
}

char *comp_complete(struct Completion *comp)
{
  struct CompItem *item = NULL;
  int n_matches = 0;

  // TODO put different states into helper functions instead?
  switch (comp->state)
  {
    case MUTT_COMP_INIT:
      ARRAY_FOREACH(item, comp->items)
      {
        if (match(comp->typed_str, item->str, comp->flags))
        {
          item->is_match = true;

          if (n_matches == 0)
            comp->cur_item = item;
          n_matches++;
        }
      }

      if (n_matches == 0)
      {
        comp->state = MUTT_COMP_NOMATCH;
        comp->cur_item = NULL;
        return NULL;
      }
      else if (n_matches >= 1)
      {
        if (n_matches > 1)
        {
          comp->state = MUTT_COMP_MULTI;
        }
        else
        {
          comp->state = MUTT_COMP_MATCH;
        }
      }
      else
      {
        // TODO this should never happen unless we overflow the int, what shall we do here?
      }

      return comp->cur_item->str;

    case MUTT_COMP_MATCH: // return to typed string after matching single item
      comp->state = MUTT_COMP_INIT;
      comp->cur_item = NULL;
      return comp->typed_str;

    case MUTT_COMP_MULTI: // use next match
      // TODO is ARRAY_FOREACH_FROM overflow safe? It seems to work for now, but maybe check twice!
      ARRAY_FOREACH_FROM(item, comp->items, ARRAY_IDX(comp->items, comp->cur_item) + 1)
      {
        if (item->is_match)
        {
          comp->cur_item = item;
          return comp->cur_item->str;
        }
      }

      // when we reach the end, step back to the typed string
      comp->state = MUTT_COMP_INIT;
      comp->cur_item = NULL;
      return comp->typed_str;
    case MUTT_COMP_NEW: // TODO no items added yet, do nothing?
    default:
      return NULL;
  }
}
