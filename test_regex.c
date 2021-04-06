#include "config.h"
#include "acutest.h"
#include "matching.h"
#include "mutt/mbyte.h"
#include <locale.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))


void test_simple_regex(void)
{
  // TODO what about locale unset cases?
  TEST_CHECK(dist_regex(".+pple", "apple") == 0);
  TEST_CHECK(dist_regex(".+pple", "aapple") == 0);
  TEST_CHECK(dist_regex(".*pple", "abrakadapple") == 0);
  TEST_CHECK(dist_regex(".*pple", "unapple") == 0);

  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");

  TEST_CHECK(dist_regex(".+pple", "äpple") == 0);
  TEST_CHECK(dist_regex(".+pple", "ääaapple") == 0);

  TEST_CHECK(dist_regex(".*pple.*$", "\nabrakadapple\n") == 0);

  // with ^ and $ these should not match
  TEST_CHECK(dist_regex("^.*pple$", "abrakadapple\nerror") == -1);
  TEST_CHECK(dist_regex("abra^.*pple$", "abrakadapple\nerror") == -1);
}

TEST_LIST = {
  { "test_simple_regex", test_simple_regex },
  { NULL, NULL },
};
