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
  MuttCompletionFlags flags = MUTT_COMPL_NO_FLAGS;

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

  flags = MUTT_COMPL_IGNORECASE;
  // match case-insensitive
  TEST_CHECK(match(L"hel", L"Helloworld", flags));
  TEST_CHECK(match(L"HEL", L"Helloworld", flags));
  TEST_CHECK(match(L"übel", L"Übel", flags));

  flags = MUTT_COMPL_NO_FLAGS;
  // match case-sensitive ONLY
  TEST_CHECK(!match(L"hel", L"Helloworld", flags));
  TEST_CHECK(!match(L"HEL", L"Helloworld", flags));
  TEST_CHECK(match(L"HEL", L"HELloworld", flags));
}

void test_exact(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  struct Completion *comp = compl_new(MUTT_COMPL_NO_FLAGS);
  comp->flags = MUTT_MATCH_EXACT;

  // test some regular ASCII strings
  TEST_CHECK(match_dist("abc", "Abc", comp) == -1);
  TEST_CHECK(match_dist("abc", "abc", comp) == 0);
  comp->flags = MUTT_MATCH_EXACT & MUTT_MATCH_IGNORECASE;
  TEST_CHECK(match_dist("abc", "Abc", comp) == 0);
  TEST_CHECK(match_dist("wxy", "wxy", comp) == 0);

  // test multibyte comparison
  comp->flags = MUTT_MATCH_EXACT;
  TEST_CHECK(match_dist("äpfel", "Äpfel", comp) == -1);
  comp->flags = MUTT_MATCH_EXACT & MUTT_MATCH_IGNORECASE;
  TEST_CHECK(match_dist("äpfel", "Äpfel", comp) == 0);
  TEST_CHECK(match_dist("zabc", "öxrya", comp) == 0);

  // test some other symbols
  TEST_CHECK(match_dist("c", "Z", comp) == -1);
  TEST_CHECK(match_dist(";", "Z", comp) == -1);
  TEST_CHECK(match_dist("a", "!", comp) == -1);
}

TEST_LIST = {
  { "match", test_match },
  { "exact", test_exact },
  { NULL, NULL },
};
