#include "acutest.h"
#include "completion.h"
#include "statemach.h"

void test_match_simple()
{
  char *a = "apples";
  char *tar = "applers";
  Completion *comp = compl_new(MUTT_COMPL_NO_FLAGS);

  comp->flags = MUTT_MATCH_EXACT;
  TEST_CHECK(match_dist(a, tar, comp) == -1);
  comp->flags = MUTT_MATCH_FUZZY;
  TEST_CHECK(match_dist(a, tar, comp) == 1);

  a = "derived";
  tar = "drvd";

  comp->flags = MUTT_MATCH_EXACT;
  TEST_CHECK(match_dist(a, tar, comp) == -1);
  comp->flags = MUTT_MATCH_FUZZY;
  TEST_CHECK(match_dist(a, tar, comp) == 3);
  a = "drvd";
  comp->flags = MUTT_MATCH_EXACT;
  TEST_CHECK(match_dist(a, tar, comp) == 0);
}

TEST_LIST = {
  { "simple", test_match_simple },
  { NULL, NULL },
};
