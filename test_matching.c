#include "acutest.h"
#include "completion.h"
#include "statemach.h"

void test_match_simple()
{
  Completion *comp = compl_new(COMPL_MODE_EXACT);
  comp->typed_str = "apples";
  char *tar = "applers";

  comp->mode = COMPL_MODE_EXACT;
  TEST_CHECK(match_dist(tar, comp) == -1);
  comp->mode = COMPL_MODE_FUZZY;
  TEST_CHECK(match_dist(tar, comp) == 1);

  comp->typed_str = "derived";
  tar = "drvd";

  comp->mode = COMPL_MODE_EXACT;
  TEST_CHECK(match_dist(tar, comp) == -1);
  comp->mode = COMPL_MODE_FUZZY;
  TEST_CHECK(match_dist(tar, comp) == 3);
  comp->typed_str = "drvd";
  comp->mode = COMPL_MODE_EXACT;
  TEST_CHECK(match_dist(tar, comp) == 0);
}

TEST_LIST = {
  { "simple", test_match_simple },
  { NULL, NULL },
};
