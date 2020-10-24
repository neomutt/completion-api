#include "config.h"
#include "acutest.h"
#include <stdio.h>
#include "mutt/lib.h"
#include "completion.h"

#define STR_EQ(s1, s2) strcmp(s1, s2) == 0
#define STR_DF(s1, s2) strcmp(s1, s2) != 0

void test_match(void)
{
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

// TODO tests from the first implementation: recycle or delete?
/* void test_completion(void) */
/* { */
/*   char *items[] = { "hello", "hiho", "ergo", "!wethersuperlongstring" }; */

/*   printf("\nInitialising empty list...\n"); */
/*   struct CompletionItem *compitems = init_list(); */
/*   struct CompletionItem *newitem = init_list(); */

/*   printf("Adding items to completion list...\n"); */
/*   for (int i = 0; i < 4; i++) */
/*   { */
/*     newitem->full_string = calloc(1, strlen(items[i]) + 1); */
/*     mutt_str_copy(newitem->full_string, items[i], strlen(items[i]) + 1); */
/*     newitem->itemlength = strlen(items[i]); */
/*     add_item(compitems, newitem); */
/*     printf(" - %s\n", newitem->full_string); */
/*   } */

/*   char *typed, *result; */

/*   // single matches */
/*   typed = "hel"; */
/*   result = items[0]; */
/*   printf("Matching typed '%s' (ignore case): '%s'\n", typed, result); */
/*   TEST_CHECK(STR_EQ( */
/*       complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result)); */

/*   typed = "hi"; */
/*   result = items[1]; */
/*   printf("Matching typed '%s' (ignore case): '%s'\n", typed, result); */
/*   TEST_CHECK(STR_EQ( */
/*       complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result)); */

/*   typed = "HI"; */
/*   result = items[1]; */
/*   printf("Matching typed '%s' (ignore case): '%s'\n", typed, result); */
/*   TEST_CHECK(STR_EQ( */
/*       complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result)); */

/*   typed = "!"; */
/*   result = items[3]; */
/*   printf("Matching typed '%s' (ignore case): '%s'\n", typed, result); */
/*   TEST_CHECK(STR_EQ( */
/*       complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result)); */

/*   typed = "HI"; */
/*   printf("Matching typed '%s' (no flags): -\n", typed); */
/*   TEST_CHECK(complete(compitems, typed, strlen(typed), MUTT_COMP_NO_FLAGS) == NULL); */

/*   // several matches -> firstmatch flag */
/*   typed = "h"; */
/*   result = items[0]; */
/*   printf("Matching typed '%s' (first match): '%s'\n", typed, result); */
/*   TEST_CHECK(STR_EQ(complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE & MUTT_COMP_FIRSTMATCH) */
/*                         ->full_string, */
/*                     result)); */

/*   typed = "h"; */
/*   char *results[] = { items[0], items[1] }; */
/*   struct CompletionItem *comp_list = */
/*       complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE); */
/*   printf("Matching multi '%s' (ignore case):\n", typed); */
/*   int i = 0; */
/*   while (comp_list != NULL) */
/*   { */
/*     printf("  - '%s'\n", results[i]); */
/*     TEST_CHECK(STR_EQ(comp_list->full_string, results[i])); */

/*     i += 1; */
/*     comp_list = comp_list->next; */
/*   } */

/*   clear_list(comp_list, MUTT_COMP_LIST_BOTH); */
/*   clear_list(compitems, MUTT_COMP_LIST_BOTH); */
/*   if (newitem) */
/*     free(newitem); */
/* } */

TEST_LIST = {
  { "match", test_match },
  { "capital_diff", test_capital_diff },
  { NULL, NULL },
};
