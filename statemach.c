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

/**
 * @page completion neomutt completion API
 *
 * Neomutt completion API
 */
#include "statemach.h"

Completion *compl_new(MuttCompletionFlags flags)
{
  Completion *comp = mutt_mem_calloc(1, sizeof(Completion));

  // initialise the typed string as empty
  comp->typed_str = mutt_mem_calloc(MAX_TYPED, sizeof(char));
  logdeb(4, "Memory allocation for comp->typed_str done, bytes: %lu.",
         MAX_TYPED * sizeof(char));
  strncpy(comp->typed_str, "", 1);

  comp->cur_item = NULL;

  comp->state = MUTT_COMPL_NEW;
  comp->flags = flags;

  comp->items = mutt_mem_calloc(1, sizeof(struct CompletionList));
  logdeb(4, "Memory allocation for comp->items done.");
  ARRAY_INIT(comp->items);

  comp->regex = mutt_mem_calloc(1, sizeof(regex_t));
  comp->regex_compiled = false;
  return comp;
}

void compl_free(Completion *comp) {
  free(comp->typed_str);
  ARRAY_FREE(comp->items);
  regfree(comp->regex);
}

int compl_add(Completion *comp, const char *str, size_t buf_len)
{
  if (!compl_health_check(comp))
    return 0;

  if (!compl_str_check(str, buf_len))
    return 0;

  CompletionItem new_item = { 0 };

  // use a conservative memory allocation
  new_item.str = mutt_mem_calloc(mutt_str_len(str) + 1, sizeof(char));
  logdeb(4, "Mem alloc succeeded: new_item.str, bytes: %lu = %lu chars",
         (mutt_str_len(str) + 1) * sizeof(char), mutt_str_len(str));
  strncpy(new_item.str, str, buf_len);

  new_item.is_match = false;
  new_item.match_dist = -1;

  logdeb(4, "Added: '%s', (buf_len:%lu)", new_item.str, buf_len);

  // don't add duplicates
  if (!compl_check_duplicate(comp, new_item.str, buf_len))
  {
    ARRAY_ADD(comp->items, new_item);
    logdeb(4, "Added item '%s' successfully.", new_item.str);
  }
  else
  {
    logdeb(4, "Duplicate item '%s' skipped.", new_item.str);
    return 0;
  }

  return 1;
}

int compl_type(Completion *comp, const char *str, size_t buf_len)
{
  if (!compl_health_check(comp))
    return 0;

  if (!compl_str_check(str, buf_len))
    return 0;

  // copy typed string into completion
  strncpy(comp->typed_str, str, buf_len);

  logdeb(4, "Typing: '%s', (buf_len:%lu)", comp->typed_str, buf_len);

  comp->state = MUTT_COMPL_INIT;

  // flag regex compilation out of date after typing
  comp->regex_compiled = false;
  return 1;
}

bool compl_state_init(Completion *comp, char **result, size_t *match_len)
{
  logdeb(5, "Initialising completion...");
  int n_matches = 0;
  int dist = -1;
  CompletionItem *item = NULL;

  ARRAY_FOREACH(item, comp->items)
  {
    item->match_dist = match_dist(comp->typed_str, item->str, comp);
    if (dist >= 0)
    {
      logdeb(5, "'%s' matched: '%s'", comp->typed_str, item->str);
      item->is_match = true;

      // first found item gets assigned to match
      if (n_matches == 0)
        comp->cur_item = item;

      n_matches++;
    }
  }

  // TODO here we should sort the items by their match distance

  if (n_matches == 0)
  {
    comp->state = MUTT_COMPL_NOMATCH;
    comp->cur_item = NULL;
    logdeb(4, "No match for '%s'.", comp->typed_str);
    return false;
  }
  else if (n_matches >= 1)
  {
    if (n_matches > 1)
      comp->state = MUTT_COMPL_MULTI;
    else
      comp->state = MUTT_COMPL_MATCH;
  }
  else
  {
    // TODO this should never happen unless we overflow the int, what shall we do here?
    return false;
  }

  *match_len = mutt_str_len(comp->cur_item->str);
  *result = comp->cur_item->str;
  return true;
}

bool compl_state_match(Completion *comp, char **result, size_t *match_len)
{
  comp->state = MUTT_COMPL_INIT;
  comp->cur_item = NULL;

  *result = comp->typed_str;
  *match_len = mutt_str_len(comp->typed_str);
  return true;
}

void compl_state_multi(Completion *comp, char **result, size_t *match_len)
{
  CompletionItem *item = NULL;

  // TODO is ARRAY_FOREACH_FROM overflow safe? It seems to work for now, but maybe check twice!
  ARRAY_FOREACH_FROM(item, comp->items, ARRAY_IDX(comp->items, comp->cur_item) + 1)
  {
    if (item->is_match)
    {
      comp->cur_item = item;
      *result = comp->cur_item->str;
      *match_len = mutt_str_len(comp->cur_item->str);
      break;
    }
  }

  // when we reach the end, step back to the typed string
  if (*result == NULL)
  {
    comp->state = MUTT_COMPL_INIT;
    comp->cur_item = NULL;
    *result = comp->typed_str;
    *match_len = mutt_str_len(comp->typed_str);
  }
}

char *compl_complete(Completion *comp)
{
  if (!compl_health_check(comp))
    return NULL;

  if (ARRAY_EMPTY(comp->items))
  {
    logdeb(4, "Completion on empty list: '%s' -> ''", comp->typed_str);
    return NULL;
  }

  // recompile out-of-date regex
  if ((comp->flags & MUTT_MATCH_REGEX) && !comp->regex_compiled)
  {
    // TODO move error handling to own function?
    int errcode = regcomp(comp->regex, comp->typed_str, REG_EXTENDED);
    if (errcode != 0)
    {
      // try a error message size of 20 first
      char *errmsg = calloc(20, sizeof(char));
      int errsize = regerror(errcode, comp->regex, errmsg, 20);

      // potential reallocation to fit the whole error message
      if (errsize >= 20)
      {
        free(errmsg);
        char *errmsg = calloc(errsize, sizeof(char));
        regerror(errcode, comp->regex, errmsg, errsize);
      }

      logerr("RegexCompilation: Error when compiling string. %s", errmsg);

      free(errmsg);

      // TODO how do we handle a failed regex compilation
      // fall back to fuzzy matching instead
      comp->flags = comp->flags & MUTT_MATCH_FUZZY;
      comp->regex_compiled = false;
    }
    else
    {
      comp->regex_compiled = true;
    }
  }

  char *result = NULL;
  // TODO refactor match_len and ..._mb_len to be buffer sizes
  size_t match_len = 0;

  switch (comp->state)
  {
    case MUTT_COMPL_INIT:
      if (!compl_state_init(comp, &result, &match_len))
        return NULL;
      break;

    case MUTT_COMPL_MATCH: // return to typed string after matching single item
      if (!compl_state_match(comp, &result, &match_len))
        return NULL;
      break;

    case MUTT_COMPL_MULTI: // use next match
      compl_state_multi(comp, &result, &match_len);
      break;
    case MUTT_COMPL_NEW: // TODO no items added yet, do nothing?
    default:
      return NULL;
  }

  // convert result back to multibyte
  char *match = mutt_mem_calloc(match_len + 1, sizeof(char));
  strncpy(match, result, match_len + 1);
  logdeb(4, "Match is '%s' (buf:%lu)\n", match, mutt_str_len(match));
  return match;
}

int compl_health_check(const Completion *comp)
{
  if (!comp)
  {
    logerr("CompHealth: null pointer Completion struct.");
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

int compl_str_check(const char *str, size_t buf_len)
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

int compl_char_check(const char *str, size_t buf_len)
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

int compl_get_size(Completion *comp)
{
  return ARRAY_SIZE(comp->items);
}

bool compl_check_duplicate(const Completion *comp, const char *str, size_t buf_len)
{
  if (!compl_health_check(comp))
    return true;

  if (!compl_char_check(str, buf_len))
    return true;

  CompletionItem *item;
  ARRAY_FOREACH(item, comp->items)
  {
    if (mutt_str_cmp(item->str, str) == 0)
    {
      return true;
    }
  }

  return false;
}

/**
 * dist_regex calculates the string distance between the source- and target-string,
 * by utilising the compiled regular expression
 *
 * @param src source string
 * @param tar target string
 * @param regex compiled regular expression
 * @retval int distance between the strings (or -1 if no match at all)
 */
static int dist_regex(const char *src, const char *tar, const Completion *comp)
{
  int dist = -1;
  regmatch_t pmatch[1];

  // check for a match
  if (regexec(comp->regex, tar, 1, pmatch, REG_NOTEOL & REG_NOTBOL) == REG_NOMATCH)
  {
    return -1;
  }

  size_t src_len = mutt_str_len(src);
  size_t tar_len = mutt_str_len(tar);

  // match distance is the number of additions needed to match the string
  // TODO this is a naive implementation, what about complex regexes like "[abcdefghijk]+" = "a"
  if (src_len > tar_len)
    dist = tar_len - src_len;

  if (src_len <= tar_len)
    dist = 0;

  return dist;
}

/**
 * matches the source against the target string, using exact comparison.
 * Returns -1 if there is no match, or 0 if the strings match.
 * If MUTT_MATCH_IGNORECASE is set, it will ignore case.
 *
 * @param src source string
 * @param tar target string
 * @param flags completion flags
 */
static int dist_exact(const char *src, const char *tar, const Completion *comp)
{
  bool src_mbs = is_mbs(src);
  bool tar_mbs = is_mbs(tar);

  int len_src = mutt_str_len(src);
  int len_tar = mutt_str_len(tar);

  // source string length needs to be shorter for substring matching
  if (len_src > len_tar)
    return -1;

  // compare all bytes of src with tar
  if (comp->flags & MUTT_MATCH_IGNORECASE)
  {
    if (src_mbs || tar_mbs)
    {
      len_src = mbs_char_count(src);
      len_tar = mbs_char_count(tar);
      wchar_t w_src[len_src + 1];
      wchar_t w_tar[len_tar + 1];
      mbstowcs(w_src, src, len_src + 1);
      mbstowcs(w_tar, tar, len_src + 1);

      for (int i = 0; i < mbs_char_count(src); i++)
      {
        if (iswalpha(w_src[i]) && iswalpha(w_tar[i])) {
          if (towlower(w_src[i]) != towlower(w_tar[i]))
            return -1;
        }
        else if (w_src[i] != w_tar[i]) {
          return -1;
        }
      }

      return (len_tar - len_src);
    }

    if (strncasecmp(src, tar, len_src) == 0)
      return len_tar - len_src;
  }
  else {
    if (strncmp(src, tar, len_src) != 0)
      return -1;

    // insertions are calculated differently for mbs
    if (src_mbs || tar_mbs)
      return (mbs_char_count(tar) - mbs_char_count(src));
    else
      return len_tar - len_src;
  }

  // no match
  return -1;
}

/**
 * match_dist calculates the string distance between source- and target-string,
 * based on the match method (MuttMatchFlags)
 *
 * @param stra source string
 * @param strb target string
 * @retval int distance between the strings (or -1 if no match at all)
 */
int match_dist(const char *src, const char *tar, const Completion *comp)
{
  int dist = 0;

  switch (comp->flags)
  {
    case MUTT_MATCH_FUZZY:
      dist = dist_dam_lev(src, tar, comp);
      break;
    case MUTT_MATCH_REGEX:
      dist = dist_regex(src, tar, comp);
      break;
    case MUTT_MATCH_EXACT:
    default:
      dist = dist_exact(src, tar, comp);
      break;
  }

  return dist;
}
