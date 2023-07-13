#include "config.h"
#include "acutest.h"
#include <stdio.h>
#include <locale.h>
#include "mutt/lib.h"
#include "statemach.h"

#define STR_EQ(s1, s2) strcmp(s1, s2) == 0
#define STR_DF(s1, s2) strcmp(s1, s2) != 0

void test_match(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  Completion *comp = compl_new(MUTT_COMPL_NO_FLAGS);
  comp->flags = MUTT_MATCH_EXACT;

  // basic match and non-match
  TEST_CHECK(match_dist("Hello", "Hello", comp) == 0);
  TEST_CHECK(match_dist("Hello", "Bye", comp) == -1);
  printf("\n\n");
  TEST_CHECK(match_dist("Übel", "Übel", comp) == 0);
  TEST_CHECK(match_dist("übel", "Übel", comp) == -1);

  // match substring only
  TEST_CHECK(match_dist("Hel", "Hello", comp));

  // match empty string
  TEST_CHECK(match_dist("", "HELLO", comp));
  TEST_CHECK(match_dist("", "neomuttisawesome", comp));

  comp->flags = MUTT_MATCH_IGNORECASE;
  // match case-insensitive
  TEST_CHECK(match_dist("hel", "Helloworld", comp));
  TEST_CHECK(match_dist("HEL", "Helloworld", comp));
  TEST_CHECK(match_dist("übel", "Übel", comp));

  // match case-sensitive ONLY
  TEST_CHECK(!match_dist("hel", "Helloworld", comp));
  TEST_CHECK(!match_dist("HEL", "Helloworld", comp));
  TEST_CHECK(match_dist("HEL", "HELloworld", comp));
}

void test_exact(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  Completion *comp = compl_new(MUTT_COMPL_NO_FLAGS);
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
