#include "acutest.h"
#include "completion.h"
#include "statemach.h"

void test_match_simple()
{
  Completion *comp = compl_new(MUTT_MATCH_EXACT);
  comp->typed_str = "apples";
  char *tar = "applers";

  comp->flags = MUTT_MATCH_EXACT;
  TEST_CHECK(match_dist(tar, comp) == -1);
  comp->flags = MUTT_MATCH_FUZZY;
  TEST_CHECK(match_dist(tar, comp) == 1);

  comp->typed_str = "derived";
  tar = "drvd";

  comp->flags = MUTT_MATCH_EXACT;
  TEST_CHECK(match_dist(tar, comp) == -1);
  comp->flags = MUTT_MATCH_FUZZY;
  TEST_CHECK(match_dist(tar, comp) == 3);
  comp->typed_str = "drvd";
  comp->flags = MUTT_MATCH_EXACT;
  TEST_CHECK(match_dist(tar, comp) == 0);
}

TEST_LIST = {
  { "simple", test_match_simple },
  { NULL, NULL },
};
