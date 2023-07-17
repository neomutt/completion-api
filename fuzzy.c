#include "fuzzy.h"

/**
 * min - return the minimum value of three integers
 *
 * @param a first value
 * @param b second value
 * @param c third value
 * @retval int the minimum value of (a, b, c)
 */
static int min(const int a, const int b, const int c)
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

bool is_mbs(const char *str)
{
  if (str == NULL)
    return false;

  while (*str != '\0')
  {
    if (MBCHARLEN(str) > 1)
      return true;
    str++;
  }

  return false;
}

/**
 * mbs_char_count - count the number of multibyte characters
 *
 * this does not count the bytes, but the actual characters of the
 * string (in human terms)
 *
 * @param str multibyte string to count
 * @retval int number of string characters
 */
int mbs_char_count(const char *str)
{
  int i = 0, len = 0;

  if (!str)
    return 0;

  while (str[i] && str[i] != '\0')
  {
    // TODO does this make sense?
    if (ISBADMBYTE(&str[i]))
      return -1;
    else if (ISLONGMBYTE(&str[i]))
      i += mblen(&str[i], MB_CUR_MAX);
    else
      i += 1;

    len += 1;
  }

  return len;
}

/**
 * mb_equal - test whether two string characters are equal
 *
 * this takes the multibyte encoding, bad mbytes and empty strings into account
 *
 * @param stra the first string
 * @param strb the second string
 * @retval bool true if both characters match, false for errors/no match
 */
bool mb_equal(const char *stra, const char *strb)
{
  if (!stra || !strb)
    return false;
  else if (ISBADMBYTE(stra) || ISBADMBYTE(strb))
    return false;

  int m = 0;
  // long mbytes need to be considered as a single symbol
  if (ISLONGMBYTE(stra) || ISLONGMBYTE(strb))
  {
    // length of mbyte sequences missmatching? different characters
    if (MBCHARLEN(stra) != MBCHARLEN(strb))
      return false;

    // check each byte of the mbyte sequence matches
    else
    {
      for (m = 0; m < MBCHARLEN(stra); m++)
      {
        if (stra[m] != strb[m])
          return false;
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

/**
 * lev - Calculate the levenshtein distance between two strings
 *
 * This accounts to the number of insertions/deletions/substitutions to
 * get to string b from string a.
 *
 * The levenshtein distance is computed via recursion on the substrings.
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
    return dist_lev(&stra[MBCHARLEN(stra)], &strb[MBCHARLEN(strb)]);
  }
  else
  {
    return 1 + min(dist_lev(&stra[MBCHARLEN(stra)], strb),
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
 * The damerau-levenshtein distance is computed with dynamic programming
 * (matrix computation), and thus quicker than the recursive levenshtein distance.
 *
 * @param tar target string
 * @param comp Completion
 * @retval int damerau-levenshtein distance between strings
 */
int dist_dam_lev(const char *tar, const struct Completion *comp)
{
  const char *src = comp->typed_str;
  int len_src = mbs_char_count(src);
  int len_tar = mbs_char_count(tar);

  // TODO maybe move these string sanity checks to function?
  if (len_src == -1 || len_tar == -1)
  {
    return -1;
  }
  else if (len_src == 0)
  {
    return len_tar;
  }
  else if (len_tar == 0)
  {
    return len_src;
  }

  int d[len_src][len_tar];
  int ca_idx[len_src];
  int cb_idx[len_tar];

  int i = 0;
  int j = 0;
  int cost = 0;

  // initialise the list of symbol indices (supporting mbyte)
  ca_idx[0] = 0;
  for (i = 1; i < len_src; i++)
  {
    ca_idx[i] = ca_idx[i - 1] + MBCHARLEN(&src[ca_idx[i - 1]]);
  }

  cb_idx[0] = 0;
  for (j = 1; j < len_tar; j++)
  {
    cb_idx[j] = cb_idx[j - 1] + MBCHARLEN(&tar[cb_idx[j - 1]]);
  }

  // initialise calculation matrix
  for (i = 0; i < len_src; i++)
  {
    for (j = 0; j < len_tar; j++)
    {
      d[i][j] = 0;
    }
    d[i][0] = i;
  }

  for (j = 0; j < len_tar; j++)
  {
    d[0][j] = j;
  }

  // calculate the character distance
  i = 1;
  while (i < len_src)
  {
    j = 1;
    while (j < len_tar)
    {
      cost = mb_equal(&src[ca_idx[i]], &tar[cb_idx[j]]) ? 0 : 1;

      d[i][j] = min(d[i - 1][j] + 1,         // deletion
                    d[i][j - 1] + 1,         // insertion
                    d[i - 1][j - 1] + cost); // substitution

      // transposition if symbols next to each other are equal
      if (i > 1 && j > 1 && mb_equal(&src[ca_idx[i]], &tar[cb_idx[j - 1]]) &&
          mb_equal(&src[ca_idx[i - 1]], &tar[cb_idx[j]]))
      {
        d[i][j] = (d[i][j] < d[i - 2][j - 2] + 1) ? d[i][j] : d[i - 2][j - 2] + 1;
      }

      j += 1;
    }
    i += 1;
  }

  return d[len_src - 1][len_tar - 1];
}
