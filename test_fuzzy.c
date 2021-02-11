#include "config.h"
#include "acutest.h"
#include <stdio.h>
#include "mutt/lib.h"
#include "fuzzy.h"


void test_min(void)
{
  TEST_CHECK(min(1, 2, 3) == 1);
  TEST_CHECK(min(2, 1, 3) == 1);
  TEST_CHECK(min(2, 3, 1) == 1);

  TEST_CHECK(min(1, 1, 1) == 1);
  TEST_CHECK(min(1, 1, -1) == -1);
  TEST_CHECK(min(2, 1, 1) == 1);
}

void test_levenshtein(void)
{
  TEST_CHECK(lev("chitin", "chtia") == 2);

  TEST_CHECK(lev("hello", "hell") == 1);

  TEST_CHECK(lev("", "") == 0);
  // TODO add more tests
  // TODO check data input
  // TODO check multibyte strings
}

TEST_LIST = {
  { "min", test_min },
  { "levenshtein", test_levenshtein },
  { NULL, NULL },
};
