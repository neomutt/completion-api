#include "config.h"
#include <stdint.h>
#include <stdlib.h>
#include "statemach.h"

struct Completion *comp_new(MuttCompletionFlags flags)
{
  struct Completion *comp = mutt_mem_calloc(1, sizeof(struct Completion));

  // initialise the typed string as empty
  comp->typed_str = mutt_mem_calloc(MAX_TYPED, sizeof(wchar_t));
  wcsncpy(comp->typed_str, L"", 1);

  comp->cur_item = NULL;

  comp->state = MUTT_COMP_NEW;
  comp->flags = flags;

  comp->items = mutt_mem_calloc(1, sizeof(struct CompletionList));
  // TODO call to memset is insecure?
  ARRAY_INIT(comp->items);
  return comp;
}

void comp_add(struct Completion *comp, const char *str, size_t str_len)
{
  struct CompItem new_item = { 0 };

  // keep the mb string buffer length for backconversion
  new_item.mb_buf_len = str_len;

  // use a conservative memory allocation: one wchar for each mbchar
  size_t str_buf_len = strlen(str) * sizeof(wchar_t);
  new_item.str = mutt_mem_calloc(strlen(str), sizeof(wchar_t));

  // this will reallocate the memory if more is needed
  mutt_mb_mbstowcs(&new_item.str, &str_buf_len, 0, str);
  new_item.is_match = false;

  printf("Added: '%ls', (buf:%lu, strl:%lu)\n", new_item.str, str_buf_len,
         wcslen(new_item.str));

  // TODO what about duplicates? better handle them here, I guess
  ARRAY_ADD(comp->items, new_item);
}

void comp_type(struct Completion *comp, const char *str, size_t buf_len)
{
  // TODO free existing memory before typing again

  // keep the mb string buffer length for backconversion
  comp->typed_mb_len = buf_len;

  // use a conservative memory allocation: one wchar for each mbchar
  size_t str_buf_len = strlen(str) * sizeof(wchar_t);
  // will reallocate if more memory is needed
  mutt_mb_mbstowcs(&comp->typed_str, &str_buf_len, 0, str);

  printf("Typed: '%ls', (buf:%lu, strl:%lu)\n", comp->typed_str, str_buf_len,
         wcslen(comp->typed_str));

  comp->state = MUTT_COMP_INIT;
}

char *comp_complete(struct Completion *comp)
{
  struct CompItem *item = NULL;
  int n_matches = 0;

  wchar_t *result = NULL;
  size_t match_len = 0;

  // TODO put different states into helper functions instead?
  switch (comp->state)
  {
    case MUTT_COMP_INIT:
      ARRAY_FOREACH(item, comp->items)
      {
        if (match(comp->typed_str, item->str, comp->flags))
        {
          printf("This matched: %ls\n", item->str);
          item->is_match = true;

          if (n_matches == 0)
          {
            comp->cur_item = item;
          }
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

      match_len = comp->cur_item->mb_buf_len;
      result = comp->cur_item->str;
      break;

    case MUTT_COMP_MATCH: // return to typed string after matching single item
      comp->state = MUTT_COMP_INIT;
      comp->cur_item = NULL;

      result = comp->typed_str;
      match_len = comp->typed_mb_len;
      break;

    case MUTT_COMP_MULTI: // use next match
      // TODO is ARRAY_FOREACH_FROM overflow safe? It seems to work for now, but maybe check twice!
      ARRAY_FOREACH_FROM(item, comp->items, ARRAY_IDX(comp->items, comp->cur_item) + 1)
      {
        if (item->is_match)
        {
          comp->cur_item = item;
          result = comp->cur_item->str;
          match_len = comp->cur_item->mb_buf_len;
          break;
        }
      }

      // when we reach the end, step back to the typed string
      if (result == NULL)
      {
        comp->state = MUTT_COMP_INIT;
        comp->cur_item = NULL;
        result = comp->typed_str;
        match_len = comp->typed_mb_len;
      }
      break;
    case MUTT_COMP_NEW: // TODO no items added yet, do nothing?
    default:
      return NULL;
  }

  // convert result back to multibyte
  /* size_t match_len = wcslen(result) * 4 * sizeof(char); */
  char *match = mutt_mem_calloc(match_len, sizeof(char));
  mutt_mb_wcstombs(match, match_len, result, wcslen(result));
  printf("match is '%s' (buf:%lu, strl:%lu)\n", match, match_len, wcslen(result));
  return match;
}
