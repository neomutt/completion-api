#include "fuzzy.h"

int min(int a, int b, int c)
{
  if (a >= b || a >= c)
  {
    return (b < c) ? b : c;
  }
  else if (b >= a || b >= c)
  {
    return (a < c) ? a : c;
  }
  else
  {
    return (a < b) ? a : b;
  }
}

/**
 * lev - Calculate the levenshtein distance between two strings
 *
 * This accounts to the number of insertions/deletions/substitutions to
 * get to string b from string a.
 *
 * @param stra string a
 * @param strb string b
 * @retval int levenshtein distance between strings
 */
int lev(char *stra, char *strb)
{
  // TODO input checks

  // quick checks for null size strings
  if (mutt_str_len(stra) == 0)
  {
    return mutt_str_len(strb);
  }
  else if (mutt_str_len(strb) == 0)
  {
    return mutt_str_len(stra);
  }

  if (stra[0] == strb[0])
  {
    return lev(&stra[1], &strb[1]);
  }
  else
  {
    return 1 + min(lev(&stra[1], strb), lev(stra, &strb[1]), lev(&stra[1], &strb[1]));
  }
}
