#include "statemach.h"

struct Completion *compl_new(MuttCompletionFlags flags)
{
  struct Completion *comp = mutt_mem_calloc(1, sizeof(struct Completion));

  // initialise the typed string as empty
  comp->typed_str = mutt_mem_calloc(MAX_TYPED, sizeof(char));
  logdeb(4, "Memory allocation for comp->typed_str done, bytes: %lu.", MAX_TYPED * sizeof(char));
  strncpy(comp->typed_str, "", 1);

  comp->cur_item = NULL;

  comp->state = MUTT_COMPL_NEW;
  comp->flags = flags;

  comp->items = mutt_mem_calloc(1, sizeof(struct CompletionList));
  logdeb(4, "Memory allocation for comp->items done.");
  ARRAY_INIT(comp->items);

  comp->regex = NULL;
  comp->regex_compiled = false;
  return comp;
}

int compl_add(struct Completion *comp, const char *str, size_t buf_len)
{
  if (!compl_health_check(comp)) return 0;

  if (!compl_str_check(str, buf_len)) return 0;

  struct CompItem new_item = { 0 };

  // use a conservative memory allocation
  new_item.str = mutt_mem_calloc(mutt_str_len(str) + 1, sizeof(char));
  logdeb(4, "Mem alloc succeeded: new_item.str, bytes: %lu = %lu chars", (mutt_str_len(str) + 1) * sizeof(char), mutt_str_len(str));
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

int compl_type(struct Completion *comp, const char *str, size_t buf_len)
{
  if (!compl_health_check(comp)) return 0;

  if (!compl_str_check(str, buf_len)) return 0;

  // copy typed string into completion
  strncpy(comp->typed_str, str, buf_len);

  logdeb(4, "Typing: '%s', (buf_len:%lu)", comp->typed_str, buf_len);

  comp->state = MUTT_COMPL_INIT;

  // flag regex compilation out of date after typing
  comp->regex_compiled = false;
  return 1;
}

bool compl_state_init(struct Completion *comp, char **result, size_t *match_len)
{
  int n_matches = 0;
  int dist = -1;
  struct CompItem *item = NULL;

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

bool compl_state_match(struct Completion *comp, char **result, size_t *match_len)
{
  comp->state = MUTT_COMPL_INIT;
  comp->cur_item = NULL;

  *result = comp->typed_str;
  *match_len = mutt_str_len(comp->typed_str);
  return true;
}

void compl_state_multi(struct Completion *comp, char **result, size_t *match_len)
{
  struct CompItem *item = NULL;

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

char *compl_complete(struct Completion *comp)
{
  if (!compl_health_check(comp)) return NULL;

  if (ARRAY_EMPTY(comp->items))
  {
    logdeb(4, "Completion on empty list: '%s' -> ''", comp->typed_str);
    return NULL;
  }

  // recompile out-of-date regex
  if ((comp->flags & MUTT_MATCH_REGEX) && !comp->regex_compiled) {
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
      regfree(comp->regex);

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
      if (!compl_state_init(comp, &result, &match_len)) return NULL;
      break;

    case MUTT_COMPL_MATCH: // return to typed string after matching single item
      if (!compl_state_match(comp, &result, &match_len)) return NULL;
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

int compl_health_check(const struct Completion *comp)
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

int compl_get_size(struct Completion *comp)
{
  return ARRAY_SIZE(comp->items);
}

bool compl_check_duplicate(const struct Completion *comp, const char *str, size_t buf_len)
{
  if (!compl_health_check(comp)) return true;

  if (!compl_char_check(str, buf_len)) return true;

  struct CompItem *item;
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
static int dist_regex(const char *src, const char *tar, const struct Completion *comp)
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
 * If MUTT_COMPL_IGNORECASE is set, it will ignore case.
 *
 * @param src source string
 * @param tar target string
 * @param flags completion flags
 */
static int dist_exact(const char *src, const char *tar, const struct Completion *comp)
{
  int len_src = MBCHARLEN(src);
  int len_tar = MBCHARLEN(tar);

  // if only one of them is a multibyte, they can't be the same
  if (len_src == -1 || len_tar == -1)
    return -1;

  // string lengths need to match exactly
  if (len_src != len_tar)
    return -1;

  // not a multibyte string: use ascii comparison functions
  if (len_src == -1)
  {
    // ignore-case match
    if ((comp->flags & MUTT_MATCH_IGNORECASE) && (strcasecmp(src, tar) == 0))
    {
      return 0;
    }
    // case-sensitive match
    else if (strcmp(src, tar) == 0)
    {
      return 0;
    }

    // no match
    return -1;
  }

  // a multibyte string: convert to widechar and compare
  wchar_t w_src[len_src + 1];
  wchar_t w_tar[len_src + 1];
  mbstowcs(w_src, src, len_src);
  mbstowcs(w_tar, tar, len_tar);

  // ignore-case match
  if ((comp->flags & MUTT_MATCH_IGNORECASE) && (wcscasecmp(w_src, w_tar) == 0))
    return 0;

  // case-sensitive match
  if (wcscmp(w_src, w_tar))
    return 0;

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
int match_dist(const char *src, const char *tar, const struct Completion *comp)
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
