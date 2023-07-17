#include <stdlib.h>
#include "config.h"
#include "acutest.h"
#include <locale.h>
#include "mutt/mbyte.h"
#include "completion.h"
#include "statemach.h"

void test_simple_regex(void)
{
  Completion *comp = compl_new(MUTT_COMPL_NO_FLAGS);
  comp->flags = MUTT_MATCH_REGEX;

  // TODO what about locale unset cases?
  comp->typed_str = ".+pple";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("apple", comp) == 1);
  TEST_CHECK(match_dist("aapple", comp) == 0);

  comp->typed_str = ".*pple";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadapple", comp) == 6);
  TEST_CHECK(match_dist("unapple", comp) == 1);

  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");

  comp->typed_str = ".+pple";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("äpple", comp) == 0);
  TEST_CHECK(match_dist("ääaapple", comp) == 4);

  comp->typed_str = ".*pple.*$";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("\nabrakadapple\n", comp) == 5);

  // with ^ and $ these should not match
  comp->typed_str = "^.*pple$";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadapple\nerror", comp) == -1);
  comp->typed_str = "abra^.*pple$";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadapple\nerror", comp) == -1);
}

TEST_LIST = {
  { "test_simple_regex", test_simple_regex },
  { NULL, NULL },
};
