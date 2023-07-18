#include <stdlib.h>
#include "config.h"
#include "acutest.h"
#include <locale.h>
#include "mutt/mbyte.h"
#include "lib.h"
#include "private.h"

void test_simple_regex(void)
{
  Completion *comp = compl_new(COMPL_MODE_REGEX);

  // TODO what about locale unset cases?
  comp->typed_item->str = ".+pple";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("apple", comp) == 1);
  TEST_CHECK(match_dist("aapple", comp) == 0);

  comp->typed_item->str = ".*pple";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadapple", comp) == 6);
  TEST_CHECK(match_dist("unapple", comp) == 1);

  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");

  comp->typed_item->str = ".+pple";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("äpple", comp) == 0);
  TEST_CHECK(match_dist("ääaapple", comp) == 4);

  comp->typed_item->str = ".*pple.*$";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("\nabrakadapple\n", comp) == 5);

  // with ^ and $ these should not match
  comp->typed_item->str = "^.*pple$";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadapple\nerror", comp) == -1);
  comp->typed_item->str = "abra^.*pple$";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadapple\nerror", comp) == -1);

  comp->flags = COMPL_MATCH_IGNORECASE;
  comp->typed_item->str = ".*pple";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadAPPLE", comp) == 6);
  TEST_CHECK(match_dist("unAPPLE", comp) == 1);

  comp->typed_item->str = ".*PPLE";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadapple", comp) == 6);
  TEST_CHECK(match_dist("unapple", comp) == 1);

  comp->typed_item->str = ".*PplE";
  compl_compile_regex(comp);
  TEST_CHECK(match_dist("abrakadApPLe", comp) == 6);
  TEST_CHECK(match_dist("unapPLe", comp) == 1);
}

TEST_LIST = {
  { "test_simple_regex", test_simple_regex },
  { NULL, NULL },
};
