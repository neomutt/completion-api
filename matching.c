#include <regex.h>
#include "matching.h"

# define STR_FIND_IDX strchr

/**
 * match_dist calculates the string distance between test- and target-string,
 * based on the match method (MuttMatchFlags)
 *
 * @param stra source string
 * @param strb target string
 * @retval int distance between the strings (or -1 if no match at all)
 */
int match_dist(char *src, char *tar, MuttMatchFlags flags)
{

  // TODO algorithm:
  // find each substring separated by *s ('*_help_*1' -> '_help_', '1')
  // do fuzzy matching for each substring, add up the dam_lev distances
  int dist = 0;

  switch (flags)
  {
    case MUTT_MATCH_FUZZY:
      dist = dist_dam_lev(src, tar);
      break;
    case MUTT_MATCH_REGEX:
      dist = dist_regex(src, tar);
      break;
    case MUTT_MATCH_NORMAL:
    default:
      dist = dist_normal(src, tar);
      break;
  }

  return dist;
}


int dist_normal(char *src, char *tar)
{
  if (strcmp(src, tar) == 0)
  {
    return 0;
  }
  // TODO include flag for case sensitive/insensitive

  return -1;
}

int dist_regex(char *src, char *tar)
{
  regex_t regex;
  regmatch_t pmatch[1];

  // TODO move error handling to own function?
  int errcode = regcomp(&regex, src, REG_EXTENDED);
  if (errcode != 0)
  {
    // try a error message size of 20 first
    char *errmsg = calloc(20, sizeof(char));
    int errsize = regerror(errcode, &regex, errmsg, 20);

    // potential reallocation to fit the whole error message
    if (errsize >= 20)
    {
      free(errmsg);
      char *errmsg = calloc(errsize, sizeof(char));
      regerror(errcode, &regex, errmsg, errsize);
    }

    // TODO log this
    printf("%s", errmsg);
    free(errmsg);
    regfree(&regex);
    return -1;
  }

  // check for a match
  if (regexec(&regex, tar, 1, pmatch, REG_NOTEOL & REG_NOTBOL) == REG_NOMATCH)
  {
    regfree(&regex);
    return -1;
  }

  regfree(&regex);
  return 0;
}
