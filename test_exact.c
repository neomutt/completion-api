#include "config.h"
#include "acutest.h"
#include <locale.h>
#include <stdio.h>
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
  comp->typed_str = "Hello";
  TEST_CHECK(match_dist("Hello", comp) == 0);
  TEST_CHECK(match_dist("Bye", comp) == -1);
  comp->typed_str = "Übel";
  TEST_CHECK(match_dist("Übel", comp) == 0);
  comp->typed_str = "übel";
  TEST_CHECK(match_dist("Übel", comp) == -1);

  // match substring only
  comp->typed_str = "Hel";
  TEST_CHECK(match_dist("Hello", comp) == 2);

  // match empty string
  comp->typed_str = "";
  TEST_CHECK(match_dist("HELLO", comp) == 5);
  TEST_CHECK(match_dist("neomuttisawesome", comp) == 16);

  comp->flags = MUTT_MATCH_EXACT | MUTT_MATCH_IGNORECASE;
  // match case-insensitive
  comp->typed_str = "hel";
  TEST_CHECK(match_dist("Helloworld", comp) == 7);
  comp->typed_str = "HEL";
  TEST_CHECK(match_dist("Helloworld", comp) == 7);
  comp->typed_str = "übel";
  TEST_CHECK(match_dist("Übel", comp) == 0);

  // match case-sensitive ONLY
  comp->flags = MUTT_MATCH_EXACT;
  comp->typed_str = "hel";
  TEST_CHECK(match_dist("Helloworld", comp) == -1);
  comp->typed_str = "HEL";
  TEST_CHECK(match_dist("Helloworld", comp) == -1);
  TEST_CHECK(match_dist("HELloworld", comp) == 7);
}

void test_exact(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  Completion *comp = compl_new(MUTT_COMPL_NO_FLAGS);
  comp->flags = MUTT_MATCH_EXACT;

  // test some regular ASCII strings
  comp->typed_str = "abc";
  TEST_CHECK(match_dist("Abc", comp) == -1);
  TEST_CHECK(match_dist("abc", comp) == 0);

  comp->flags = MUTT_MATCH_EXACT | MUTT_MATCH_IGNORECASE;
  comp->typed_str = "abc";
  TEST_CHECK(match_dist("Abc", comp) == 0);
  comp->typed_str = "wxy";
  TEST_CHECK(match_dist("wxy", comp) == 0);

  // test multibyte comparison
  comp->flags = MUTT_MATCH_EXACT;
  comp->typed_str = "äpfel";
  TEST_CHECK(match_dist("Äpfel", comp) == -1);
  comp->flags = MUTT_MATCH_EXACT | MUTT_MATCH_IGNORECASE;
  TEST_CHECK(match_dist("Äpfel", comp) == 0);
  comp->typed_str = "zabc";
  TEST_CHECK(match_dist("öxrya", comp) == -1);

  // test some other symbols
  comp->flags = MUTT_MATCH_EXACT;
  comp->typed_str = "c";
  TEST_CHECK(match_dist("Z", comp) == -1);
  comp->typed_str = ";";
  TEST_CHECK(match_dist("Z", comp) == -1);
  comp->typed_str = "a";
  TEST_CHECK(match_dist("!", comp) == -1);
  comp->typed_str = "ß";
  TEST_CHECK(match_dist("ß😀", comp) == 1);
  comp->typed_str = "世";
  TEST_CHECK(match_dist("世界", comp) == 1);

  comp->flags = MUTT_MATCH_EXACT | MUTT_MATCH_IGNORECASE;
  comp->typed_str = "c";
  TEST_CHECK(match_dist("Z", comp) == -1);
  comp->typed_str = ";";
  TEST_CHECK(match_dist("Z", comp) == -1);
  comp->typed_str = "a";
  TEST_CHECK(match_dist("!", comp) == -1);
  comp->typed_str = "ß";
  TEST_CHECK(match_dist("ß😀", comp) == 1);
  comp->typed_str = "世";
  TEST_CHECK(match_dist("世界", comp) == 1);
}

TEST_LIST = {
  { "match", test_match },
  { "exact", test_exact },
  { NULL, NULL },
};
