#include "config.h"
#include "acutest.h"
#include <stdio.h>
#include "mutt/lib.h"
#include "completion.h"

#define STR_EQ(s1, s2) strcmp(s1, s2) == 0
#define STR_DF(s1, s2) strcmp(s1, s2) != 0

void test_match(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  MuttCompletionFlags flags = MUTT_COMP_NO_FLAGS;

  // basic match and non-match
  TEST_CHECK(match(L"Hello", L"Hello", flags));
  TEST_CHECK(!match(L"Hello", L"Bye", flags));
  printf("\n\n");
  TEST_CHECK(match(L"Übel", L"Übel", flags));
  TEST_CHECK(!match(L"übel", L"Übel", flags));

  // match substring only
  TEST_CHECK(match(L"Hel", L"Hello", flags));

  // match empty string
  TEST_CHECK(match(L"", L"HELLO", flags));
  TEST_CHECK(match(L"", L"neomuttisawesome", flags));

  flags = MUTT_COMP_IGNORECASE;
  // match case-insensitive
  TEST_CHECK(match(L"hel", L"Helloworld", flags));
  TEST_CHECK(match(L"HEL", L"Helloworld", flags));
  TEST_CHECK(match(L"übel", L"Übel", flags));

  flags = MUTT_COMP_NO_FLAGS;
  // match case-sensitive ONLY
  TEST_CHECK(!match(L"hel", L"Helloworld", flags));
  TEST_CHECK(!match(L"HEL", L"Helloworld", flags));
  TEST_CHECK(match(L"HEL", L"HELloworld", flags));
}

void test_capital_diff(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");

  TEST_CHECK(capital_diff('a', 'A'));
  TEST_CHECK(capital_diff('w', 'w'));

  TEST_CHECK(capital_diff(L'ä', L'Ä'));
  TEST_CHECK(capital_diff('z', 'Z'));

  TEST_CHECK(!capital_diff('c', 'Z'));
  TEST_CHECK(!capital_diff(';', 'Z'));
  TEST_CHECK(!capital_diff('a', '!'));
}

TEST_LIST = {
  { "match", test_match },
  { "capital_diff", test_capital_diff },
  { NULL, NULL },
};
