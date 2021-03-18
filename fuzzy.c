#include "fuzzy.h"

int mbs_char_count(const char *str)
{
  int i = 0, len = 0;

  if (!str) {
    return 0;
  }

  while (str[i] && str[i] != '\0') {
    // TODO does this make sense?
    if (ISBADMBYTE(&str[i]))
    {
      return -1 ;
    }

    else if (ISLONGMBYTE(&str[i])){
      i += mblen(&str[i], MB_CUR_MAX);
    }
    else {
      i += 1;
    }
    len += 1;
  }

  return len;
}

bool mb_equal(const char *stra, const char *strb)
{
  if (!stra || !strb)
  {
    return false;
  }
  else if (ISBADMBYTE(stra) || ISBADMBYTE(strb))
  {
    return false;
  }

  int m = 0;
  // long mbytes need to be considered as a single symbol
  if (ISLONGMBYTE(stra) || ISLONGMBYTE(strb)) {
    // length of mbyte sequences missmatching? different characters
    if (MBCHARLEN(stra) != MBCHARLEN(strb))
    {
      return false;
    }
    // check each byte of the mbyte sequence matches
    else {
      for (m = 0; m < MBCHARLEN(stra); m++)
      {
        if (stra[m] != strb[m])
        {
          return false;
        }
      }
    }
  }
  else
  {
    // for single mbytes, a simple comparison is enough
    return *stra == *strb;
  }

  return true;
}

int min(const int a, const int b, const int c)
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
int dist_lev(const char *stra, const char *strb)
{
  int lena = mbs_char_count(stra);
  int lenb = mbs_char_count(strb);

  // quick checks for null size strings
  if (lena == -1 || lenb == -1)
  {
    return -1;
  }
  else if (lena == 0)
  {
    return lenb;
  }
  else if (lenb == 0)
  {
    return lena;
  }

  if (mb_equal(stra, strb))
  {
    /* printf("\n mbytes equal, recursing..."); */
    return dist_lev(&stra[MBCHARLEN(stra)], &strb[MBCHARLEN(strb)]);
  }
  else
  {
    /* printf("\n mbytes unequal, recursing..."); */
    return 1 + min(
        dist_lev(&stra[MBCHARLEN(stra)], strb),
        dist_lev(stra, &strb[MBCHARLEN(strb)]),
        dist_lev(&stra[MBCHARLEN(stra)], &strb[MBCHARLEN(strb)]));
  }
}

/**
 * dam_lev - Calculate the damerau-levenshtein distance between two strings
 *
 * This accounts to the number of insertions/deletions/substitutions/transpositions to
 * get to string b from string a.
 *
 * @param stra string a
 * @param strb string b
 * @retval int damerau-levenshtein distance between strings
 */
int dist_dam_lev(const char *stra, const char *strb)
{
  int lena = mbs_char_count(stra);
  int lenb = mbs_char_count(strb);

  // TODO maybe move these string sanity checks to function?
  if (lena == -1 || lenb == -1)
  {
    return -1;
  }
  else if (lena == 0)
  {
    return lenb;
  }
  else if (lenb == 0)
  {
    return lena;
  }

  int d[lena][lenb];
  int ca_idx[lena];
  int cb_idx[lenb];

  int i = 0;
  int j = 0;
  int cost = 0;

  // initialise the list of symbol indices (supporting mbyte)
  ca_idx[0] = 0;
  for (i = 1; i < lena; i++)
  {
    ca_idx[i] = ca_idx[i-1] + MBCHARLEN(&stra[ca_idx[i-1]]);
  }

  cb_idx[0] = 0;
  for (j = 1; j < lenb; j++)
  {
    cb_idx[j] = cb_idx[j-1] + MBCHARLEN(&strb[cb_idx[j-1]]);
  }

  // initialise calculation matrix
  for (i = 0; i < lena; i++)
  {
    for (j = 0; j < lenb; j++)
    {
      d[i][j] = 0;
    }
    d[i][0] = i;
  }

  for (j = 0; j < lenb; j++)
  {
    d[0][j] = j;
  }

  // calculate the character distance
  i = 1;
  while (i < lena) {
    j = 1;
    while (j < lenb) {
      cost = mb_equal(&stra[ca_idx[i]], &strb[cb_idx[j]]) ? 0 : 1;

      d[i][j] = min(
          d[i-1][j] + 1,       // deletion
          d[i][j-1] + 1,       // insertion
          d[i-1][j-1] + cost); // substitution

      // transposition if symbols next to each other are equal
      if (i > 1 &&
          j > 1 &&
          mb_equal(&stra[ca_idx[i]], &strb[cb_idx[j-1]]) &&
          mb_equal(&stra[ca_idx[i-1]], &strb[cb_idx[j]]))
      {
        d[i][j] = (d[i][j] < d[i-2][j-2] + 1) ? d[i][j] : d[i-2][j-2] + 1;
      }

      j += 1;
    }
    i += 1;
  }

  return d[lena - 1][lenb - 1];
}
