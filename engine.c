/**
 * @file
 * Autocompletion API statemachine
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
#include "private.h"

/**
 * Function to allocate and initialise a new Completion struct
 *
 * @param mode which matching mode to use (see COMPL_MODE_*)
 * @retval ptr pointer to an initialised Completion struct
 */
Completion *compl_new(enum MuttMatchMode mode)
{
  Completion *comp = mutt_mem_calloc(1, sizeof(Completion));

  // initialise the typed item
  comp->typed_item = mutt_mem_calloc(1, sizeof(CompletionItem));
  comp->typed_item->buf = buf_new("");
  comp->typed_item->is_match = true;
  comp->typed_item->match_dist = -(MAX_TYPED + 1);

  comp->cur_item = comp->typed_item;

  comp->state = COMPL_STATE_NEW;
  comp->mode = mode;
  comp->flags = COMPL_MATCH_NOFLAGS;

  comp->items = mutt_mem_calloc(1, sizeof(struct CompletionList));
  logdeb(4, "Memory allocation for comp->items done.");
  ARRAY_INIT(comp->items);
  ARRAY_ADD(comp->items, *comp->cur_item);

  comp->regex_compiled = false;
  return comp;
}

/**
 * Function to allocate and initialise a new Completion struct, along with
 * adding items
 *
 * @param list a CompletionStringList of items to add right after initialisation
 * @retval ptr pointer to an initialised Completion struct
 */
Completion * compl_from_array(const struct CompletionStringList *list, enum MuttMatchMode mode)
{
  Completion *comp = compl_new(mode);

  char **item;
  ARRAY_FOREACH(item, list)
  {
    struct Buffer *buf = buf_new(*item);
    compl_add(comp, buf);
    buf_free(&buf);
  };

  return comp;
}

/**
 * Function to free the Completion struct
 *
 * @param comp Completion struct to free
 */
void compl_free(Completion *comp) {
  CompletionItem *item;
  ARRAY_FOREACH(item, comp->items)
  {
    buf_free(&item->buf);
  };

  /* the typed item is the only one which is allocated */
  free(comp->typed_item);
  ARRAY_FREE(comp->items);
}

/**
 * adds a new string to the list of possible completions
 *
 * @param comp Completion struct
 * @param str string to add
 */
int compl_add(Completion *comp, const struct Buffer *buf)
{
  if (!compl_health_check(comp))
    return 0;

  if (buf_is_empty(buf))
    return 0;

  CompletionItem new_item = { 0 };

  // use buffer copying for memory allocation
  new_item.buf = buf_dup(buf);

  new_item.is_match = false;
  new_item.match_dist = -1;

  // don't add duplicates
  if (!compl_check_duplicate(comp, new_item.buf))
  {
    ARRAY_ADD(comp->items, new_item);
    logdeb(4, "Added item '%s' successfully.", buf_strdup(new_item.buf));
  }
  else
  {
    logdeb(4, "Duplicate item '%s' skipped.", buf_strdup(new_item.buf));
    return 0;
  }

  return 1;
}

/**
 * compile the regular expression from the typed string
 *
 * @param comp Completion struct
 * @retval success 1 if successful, 0 otherwise
 *
 * @note this is automatically called when using the API functions
 */
int compl_compile_regex(Completion *comp) {
  int comp_flags = REG_EXTENDED | REG_NEWLINE;

  if (comp->flags & COMPL_MATCH_IGNORECASE)
    comp_flags |= REG_ICASE;

  int errcode = regcomp(&comp->regex, buf_strdup(comp->typed_item->buf), comp_flags);

  // successful compilation
  if (errcode == 0)
  {
    comp->regex_compiled = true;
    return 1;
  }

  // try a error message size of 20 first
  char *errmsg = calloc(20, sizeof(char));
  int errsize = regerror(errcode, &comp->regex, errmsg, 20);

  // potential reallocation to fit the whole error message
  if (errsize >= 20)
  {
    free(errmsg);
    char *errmsg = calloc(errsize, sizeof(char));
    regerror(errcode, &comp->regex, errmsg, errsize);
  }

  logerr("RegexCompilation: Error when compiling string. %s", errmsg);

  free(errmsg);
  comp->regex_compiled = false;
  return 0;
}

/**
 * type a string to be completed (user input)
 *
 * @param comp Completion struct
 * @param buf user input
 * @retval success 1 if successful, 0 otherwise
 */
int compl_type(Completion *comp, const struct Buffer *buf)
{
  if (!compl_health_check(comp))
    return 0;

  if (buf_is_empty(buf))
    return 0;

  // no actual change in the typed string
  if (buf_str_equal(buf, comp->typed_item->buf)) {
    return 1;
  }

  // copy typed string into completion
  buf_copy(comp->typed_item->buf, buf);

  logdeb(4, "Typing: '%s'", buf_strdup(comp->typed_item->buf));

  comp->state = COMPL_STATE_INIT;

  // flag regex compilation out of date after typing
  comp->regex_compiled = false;
  return 1;
}

/**
 * qsort sorting function for CompletionItems.
 *
 * Items will be sorted following these criteria (in sequence)
 *  - match
 *  - match distance
 *  - alphabetical
 *
 * @param a pointer to CompletionItem a
 * @param b pointer to CompletionItem b
 * @retval cmp -1 if a precedes b, 0 if a equals b, 1 if b preceds a
 */
static int compl_sort_fn(const void *a, const void *b) {
  const CompletionItem *itema = (const CompletionItem *)a;
  const CompletionItem *itemb = (const CompletionItem *)b;

  // non-matches go to the back of the list (and are sorted alphabetically)
  if (itema->is_match && !itemb->is_match)
    return -1;
  else if (!itema->is_match && itemb->is_match)
    return 1;
  else if (!itema->is_match && !itemb->is_match)
    return buf_coll(itema->buf, itemb->buf);

  // the typed string stays at the front
  if (itema->match_dist == -(MAX_TYPED + 1))
    return -1;
  else if (itemb->match_dist == -(MAX_TYPED + 1))
    return 1;

  // matches are sorted by match distance
  int dist_diff = itema->match_dist - itemb->match_dist;
  if (dist_diff != 0)
    return dist_diff;

  // matches with equal match distance are sorted alphabetically
  return buf_coll(itema->buf, itemb->buf);
}

static void compl_state_init(Completion *comp)
{
  logdeb(5, "Initialising completion...");
  int n_matches = 0;
  CompletionItem *item = NULL;

  ARRAY_FOREACH_FROM(item, comp->items, 1)
  {
    item->match_dist = match_dist(item->buf, comp);
    if (item->match_dist >= 0)
    {
      logdeb(5, "'%s' matched: '%s'", buf_strdup(comp->typed_item->buf), buf_strdup(item->buf));
      item->is_match = true;

      n_matches++;
    }
  }

  ARRAY_SORT(comp->items, compl_sort_fn);

  if (n_matches == 0)
  {
    comp->state = COMPL_STATE_NOMATCH;
    comp->cur_item = comp->typed_item;
    logdeb(4, "No match for '%s'.", buf_strdup(comp->typed_item->buf));
  }
  else if (n_matches >= 1)
  {
    if (n_matches > 1)
      comp->state = COMPL_STATE_MULTI;
    else
      comp->state = COMPL_STATE_SINGLE;

    // first found item gets assigned to match
    comp->cur_item = ARRAY_GET(comp->items, 1);
  }
}

static void compl_state_single(Completion *comp)
{
  int next_i = ARRAY_IDX(comp->items, comp->cur_item) + 1;

  // cycle back to beginning if reaching end of array
  if (next_i == ARRAY_SIZE(comp->items))
    next_i = 0;

  // cycle back if next item is not a match
  if (!(ARRAY_GET(comp->items, next_i)->is_match) && !(comp->flags & COMPL_MATCH_SHOWALL))
    next_i = 0;

  // switch to next match
  comp->cur_item = ARRAY_GET(comp->items, next_i);
}

static void compl_state_multi(Completion *comp)
{
  CompletionItem *item = NULL;

  int next_i = ARRAY_IDX(comp->items, comp->cur_item) + 1;

  // cycle back to beginning
  if (next_i == ARRAY_SIZE(comp->items))
    next_i = 0;

  // TODO is ARRAY_FOREACH_FROM overflow safe? It seems to work for now, but maybe check twice!
  ARRAY_FOREACH_FROM(item, comp->items, next_i)
  {
    // assign next match
    if (item->is_match || (comp->flags & COMPL_MATCH_SHOWALL))
    {
      comp->cur_item = item;
      return;
    }
  }

  // when we reach the end without finding anything, step back to the typed item
  comp->cur_item = comp->typed_item;
}

struct Buffer *compl_complete(Completion *comp)
{
  if (!compl_health_check(comp))
    return NULL;

  if (ARRAY_EMPTY(comp->items))
  {
    logdeb(4, "Completion on empty list: '%s' -> ''", buf_strdup(comp->typed_item->buf));
    return NULL;
  }

  // recompile out-of-date regex
  if ((comp->mode == COMPL_MODE_REGEX) && !comp->regex_compiled)
  {
    if (compl_compile_regex(comp) != 0)
    {
      return NULL;
      // TODO how do we handle a failed regex compilation
      // fall back to fuzzy matching instead?
      // comp->mode = comp->flags & COMPL_MODE_FUZZY;
    }
  }

  switch (comp->state)
  {
    case COMPL_STATE_INIT:
      compl_state_init(comp);
      break;

    // no match -> keep the typed item
    case COMPL_STATE_NOMATCH:
      comp->cur_item = comp->typed_item;
      break;

    // return to typed string after matching single item
    case COMPL_STATE_SINGLE:
      compl_state_single(comp);
      break;

    case COMPL_STATE_MULTI: // use next match
      // matching only first hit -> return to the initial match
      if (comp->flags & COMPL_MATCH_FIRSTMATCH)
        compl_state_single(comp);
      else
        compl_state_multi(comp);
      break;
    case COMPL_STATE_NEW:
    default:
      comp->cur_item = comp->typed_item;
  }

  struct Buffer *result = comp->cur_item->buf;

  // TODO shall we use an existing pointer argument instead?

  // allocate new pointer for result
  struct Buffer *match = buf_dup(result);
  logdeb(4, "Match is '%s'\n", buf_strdup(match));
  return match;
}

int compl_health_check(const Completion *comp)
{
  if (!comp)
  {
    logerr("CompHealth: null pointer Completion struct.");
    return 0;
  }
  if (comp->typed_item->buf == NULL)
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

int compl_get_size(Completion *comp)
{
  return ARRAY_SIZE(comp->items);
}

bool compl_check_duplicate(const Completion *comp, const struct Buffer *buf)
{
  if (!compl_health_check(comp))
    return true;

  if (buf_is_empty(buf))
    return true;

  CompletionItem *item;
  ARRAY_FOREACH(item, comp->items)
  {
    if (buf_str_equal(item->buf, buf))
      return true;
  };

  return false;
}

/**
 * dist_regex calculates the string distance between the source- and target-string,
 * by utilising the compiled regular expression
 *
 * @param tar target string
 * @param regex compiled regular expression
 * @retval int distance between the strings (or -1 if no match at all)
 */
static int dist_regex(const char *tar, const Completion *comp)
{
  int dist = -1;
  regmatch_t pmatch[1];

  if (!comp->regex_compiled)
  {
    logerr("DistRegex: Regex needs to be compiled first!");
    return -1;
  }

  /* int regex_flags = REG_EXTENDED | REG_NOTEOL | REG_NOTBOL; */
  int regex_flags = REG_EXTENDED;

  // check for a match
  if (regexec(&comp->regex, tar, 1, pmatch, regex_flags) == REG_NOMATCH)
  {
    logdeb(4, "DistRegex: No match found.");
    return -1;
  }

  size_t src_len = mutt_str_len(comp->typed_item->buf->data);
  size_t tar_len = mutt_str_len(tar);

  // match distance is the number of additions needed to match the string
  // TODO this is a naive implementation, what about complex regexes like "[abcdefghijk]+" = "a"
  if (src_len <= tar_len)
    dist = tar_len - src_len;

  if (src_len > tar_len)
    dist = src_len - tar_len;

  return dist;
}

/**
 * matches the source against the target string, using exact comparison.
 * Returns -1 if there is no match, or 0 if the strings match.
 * If COMPL_MATCH_IGNORECASE is set, it will ignore case.
 *
 * @param src source string
 * @param tar target string
 * @param flags completion flags
 */
static int dist_exact(const char *tar, const Completion *comp)
{
  char *src = buf_strdup(comp->typed_item->buf);
  bool src_mbs = is_mbs(src);
  bool tar_mbs = is_mbs(tar);

  int len_src = mutt_str_len(src);
  int len_tar = mutt_str_len(tar);

  // source string length needs to be shorter for substring matching
  if (len_src > len_tar)
    return -1;

  // compare all bytes of src with tar
  if (comp->flags & COMPL_MATCH_IGNORECASE)
  {
    if (src_mbs || tar_mbs)
    {
      len_src = mbs_char_count(src);
      len_tar = mbs_char_count(tar);
      wchar_t w_src[len_src + 1];
      wchar_t w_tar[len_tar + 1];
      // TODO add an equivalent function to mutt/mbyte
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

    if (mutt_istrn_cmp(src, tar, len_src) == 0)
      return len_tar - len_src;
  }
  else {
    if (!mutt_strn_equal(src, tar, len_src))
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
 * match_dist calculates the string distance between the typed and target-string,
 * based on the match method (MuttMatchFlags)
 *
 * When using COMPL_MODE_REGEX, the regular expression needs to be compiled
 * first (compl_compile_regex).
 *
 * @param strb target string
 * @retval int distance between the strings (or -1 if no match at all)
 */
int match_dist(const struct Buffer *tar, const Completion *comp)
{
  int dist = -1;
  const char *target = buf_strdup(tar);

  switch (comp->mode)
  {
    case COMPL_MODE_FUZZY:
      return dist_dam_lev(target, comp);
    case COMPL_MODE_REGEX:
      return dist_regex(target, comp);
    case COMPL_MODE_EXACT:
    default:
      return dist_exact(target, comp);
  }

  return dist;
}
