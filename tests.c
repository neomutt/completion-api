#include <stdio.h>
#include <string.h>
#include "acutest.h"
#include "completion.h"

#define STR_EQ(s1,s2) strcmp(s1, s2) == 0

void test_match(void) {
  MuttCompletionFlags flags = MUTT_COMP_NO_FLAGS;

  // basic match and non-match
  TEST_CHECK(match("Hello", "Hello", flags));
  TEST_CHECK(!match("Hello", "Bye", flags));

  // match substring only
  TEST_CHECK(match("Hel", "Hello", flags));

  // match empty string
  TEST_CHECK(match("", "HELLO", flags));
  TEST_CHECK(match("", "neomuttisawesome", flags));

  flags = MUTT_COMP_IGNORECASE;
  // match case-insensitive
  TEST_CHECK(match("hel", "Helloworld", flags));
  TEST_CHECK(match("HEL", "Helloworld", flags));

  flags = MUTT_COMP_NO_FLAGS;
  // match case-sensitive ONLY
  TEST_CHECK(!match("hel", "Helloworld", flags));
  TEST_CHECK(!match("HEL", "Helloworld", flags));
  TEST_CHECK(match("HEL", "HELloworld", flags));
}

void test_capital_diff(void) {
  TEST_CHECK(capital_diff('a', 'A'));
  TEST_CHECK(capital_diff('w', 'w'));
  // TODO unicode support
  /* TEST_CHECK(capital_diff('ä', 'Ä')); */
  TEST_CHECK(capital_diff('z', 'Z'));

  TEST_CHECK(!capital_diff('c', 'Z'));
  TEST_CHECK(!capital_diff(';', 'Z'));
  TEST_CHECK(!capital_diff('a', '!'));
}

void test_completion(void) {
  char *items[] = {"hello", "hiho", "ergo", "!wethersuperlongstring"};

  struct CompletionItem *compitems = calloc(4, sizeof(struct CompletionItem));

  for (int i = 0; i < 4; i++){
    compitems[i].full_string = items[i];
    compitems[i].itemlength = strlen(items[i]);
  }

  char *typed, *result;

  // single matches
  typed = "hel";
  result = items[0];
  printf("\nMatching typed '%s' : '%s'", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, 4, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "hi";
  result = items[1];
  printf("\nMatching typed '%s' : '%s'", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, 4, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "HI";
  result = items[1];
  printf("\nMatching typed '%s' : '%s'", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, 4, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "!";
  result = items[3];
  printf("\nMatching typed '%s' : '%s'", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, 4, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "HI";
  printf("\nMatching typed '%s' : '%s'", typed, result);
  TEST_CHECK(complete(compitems, 4, typed, strlen(typed), MUTT_COMP_NO_FLAGS) == NULL);

  // several matches -> firstmatch flag
  typed = "h";
  result = items[0];
  printf("\nMatching typed '%s' : '%s'", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, 4, typed, strlen(typed), MUTT_COMP_IGNORECASE & MUTT_COMP_FIRSTMATCH)->full_string, result));
}

TEST_LIST = {
   { "match", test_match },
   {"capital_diff", test_capital_diff},
   {"completion", test_completion},
   { NULL, NULL } };
