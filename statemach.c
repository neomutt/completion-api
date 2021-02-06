#include "config.h"
#include <stdint.h>
#include <stdlib.h>
#include "statemach.h"

struct Completion *comp_new(MuttCompletionFlags flags)
{
  struct Completion *comp = mutt_mem_calloc(1, sizeof(struct Completion));

  // initialise the typed string as empty
  comp->typed_str = mutt_mem_calloc(MAX_TYPED, sizeof(wchar_t));
  logdeb(4, "Memory allocation for comp->typed_str done, bytes: %lu.", MAX_TYPED * sizeof(wchar_t));
  wcsncpy(comp->typed_str, L"", 1);

  comp->cur_item = NULL;

  comp->state = MUTT_COMP_NEW;
  comp->flags = flags;

  // TODO call to memset is insecure?
  comp->items = mutt_mem_calloc(1, sizeof(struct CompletionList));
  logdeb(4, "Memory allocation for comp->items done.");
  ARRAY_INIT(comp->items);
  return comp;
}

// TODO use buf_len instead of str_len
int comp_add(struct Completion *comp, const char *str, size_t buf_len)
{
  if (!comp_health_check(comp)) return 0;

  if (!comp_str_check(str, buf_len)) return 0;

  struct CompItem new_item = { 0 };

  // keep the mb string buffer length for backconversion
  /* new_item.mb_buf_len = buf_len; */
  new_item.mb_buf_len = mutt_str_len(str);

  // use a conservative memory allocation: one wchar for each mbchar
  size_t str_buf_len = mutt_str_len(str) * sizeof(wchar_t) + 4;
  new_item.str = mutt_mem_calloc(mutt_str_len(str) + 1, sizeof(wchar_t));
  logdeb(4, "Mem alloc succeeded: new_item.str, bytes: %lu = %lu chars", (mutt_str_len(str) + 1) * sizeof(wchar_t), mutt_str_len(str));

  // this will not reallocate more memory
  mutt_mb_mbstowcs(&new_item.str, &str_buf_len, 0, str);
  new_item.is_match = false;

  logdeb(4, "Added: '%ls', (buf_len:%lu, wcslen:%lu)", new_item.str, str_buf_len,
         wcslen(new_item.str));

  // don't add duplicates
  if (!comp_check_duplicate(comp, new_item.str, str_buf_len)) {
    ARRAY_ADD(comp->items, new_item);
    logdeb(4, "Added item '%ls' successfully.", new_item.str);
  } else {
    logdeb(4, "Duplicate item '%ls' skipped.", new_item.str);
  }

  return 1;
}

int comp_type(struct Completion *comp, const char *str, size_t buf_len)
{
  if (!comp_health_check(comp)) return 0;

  if (!comp_str_check(str, buf_len)) return 0;

  // keep the mb string buffer length for backconversion
  comp->typed_mb_len = buf_len;

  // use a conservative memory allocation: one wchar for each mbchar
  size_t str_buf_len = mutt_str_len(str) * sizeof(wchar_t);
  // will reallocate if more memory is needed
  mutt_mb_mbstowcs(&comp->typed_str, &str_buf_len, 0, str);

  logdeb(4, "Typing: '%ls', (buf_len:%lu, wcslen:%lu)",
      comp->typed_str, str_buf_len, wcslen(comp->typed_str));

  comp->state = MUTT_COMP_INIT;
  return 1;
}

char *comp_complete(struct Completion *comp)
{
  if (!comp_health_check(comp)) return NULL;

  if (ARRAY_EMPTY(comp->items))
  {
    logdeb(4, "Completion on empty list: '%ls' -> ''", comp->typed_str);
    return NULL;
  }

  struct CompItem *item = NULL;
  int n_matches = 0;

  wchar_t *result = NULL;
  // TODO refactor match_len and ..._mb_len to be buffer sizes
  size_t match_len = 0;

  // TODO put different states into helper functions instead?
  switch (comp->state)
  {
    case MUTT_COMP_INIT:
      ARRAY_FOREACH(item, comp->items)
      {
        if (match(comp->typed_str, item->str, comp->flags))
        {
          logdeb(5, "'%ls' matched: '%ls'", comp->typed_str, item->str);
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
        logdeb(4, "No match for '%ls'.", comp->typed_str);
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
  char *match = mutt_mem_calloc(match_len + 1, sizeof(char));
  mutt_mb_wcstombs(match, match_len + 1, result, wcslen(result));
  logdeb(4, "Match is '%s' (buf:%lu, strl:%lu)\n", match, match_len + 1, mutt_str_len(match));
  return match;
}

int comp_health_check(const struct Completion *comp)
{
  if (!comp) {
    logerr("CompHealth: null pointer struct.");
    return 0;
  }
  if (!comp->typed_str)
  {
    logerr("CompHealth: null pointer typed string.");
    return 0;
  }

  if (!comp->items)
  {
    logerr("CompHealth: null pointer item list.");
    return 0;
  }

  return 1;
}

int comp_str_check(const char *str, size_t buf_len)
{
  if (!str)
  {
    logerr("StrHealth: nullpointer string.");
    return 0;
  }
  if (buf_len < 2 || mutt_str_len(str) == 0)
  {
    logwar("StrHealth: empty string.");
    return 0;
  }

  return 1;
}

int comp_wcs_check(const wchar_t *str, size_t buf_len)
{
  if (!str)
  {
    logerr("StrHealth: nullpointer string.");
    return 0;
  }
  if (buf_len < 2 || wcslen(str) == 0)
  {
    logwar("StrHealth: empty string.");
    return 0;
  }

  return 1;
}

int comp_get_size(struct Completion *comp)
{
  return ARRAY_SIZE(comp->items);
}

// TODO add check duplicate with char*
bool comp_check_duplicate(const struct Completion *comp, const wchar_t *str, size_t buf_len)
{
  if (!comp_health_check(comp)) return true;

  if (!comp_wcs_check(str, buf_len)) return true;

  struct CompItem *item;
  ARRAY_FOREACH(item, comp->items)
  {
    if (WSTR_EQ(item->str, str))
    {
      return true;
    }
  }

  return false;
}
