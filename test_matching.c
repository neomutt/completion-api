#include "matching.h"
#include "acutest.h"

void test_match_simple()
{
  char *a = "apples";
  char *tar = "applers";

  TEST_CHECK(match_dist(a, tar, MUTT_MATCH_NORMAL) == -1);
  TEST_CHECK(match_dist(a, tar, MUTT_MATCH_FUZZY) == 1);

  a = "derived";
  tar = "drvd";

  TEST_CHECK(match_dist(a, tar, MUTT_MATCH_NORMAL) == -1);
  TEST_CHECK(match_dist(a, tar, MUTT_MATCH_FUZZY) == 3);
  a = "drvd";
  TEST_CHECK(match_dist(a, tar, MUTT_MATCH_NORMAL) == 0);
}

TEST_LIST = {
  { "simple", test_match_simple },
};
