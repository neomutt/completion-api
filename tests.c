#include "config.h"
#include "acutest.h"
#include <stdio.h>
#include "mutt/lib.h"
#include "completion.h"
#include "completion_item.h"

#define STR_EQ(s1,s2) strcmp(s1, s2) == 0
#define STR_DF(s1,s2) strcmp(s1, s2) != 0

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

void test_copy_item(void) {
  struct CompletionItem *a = calloc(1, sizeof(struct CompletionItem));
  struct CompletionItem *b;

  a->full_string = "helloworld";
  a->itemlength = strlen(a->full_string);
  a->next = a;
  a->prev = a;

  b = copy_item(a);

  TEST_CHECK(STR_EQ(a->full_string, b->full_string));
  TEST_CHECK(a->itemlength == b->itemlength);

  // copying should not affect the links of the original
  TEST_CHECK(a->next == a);
  TEST_CHECK(a->prev == a);

  // linked list should not be copied along
  TEST_CHECK(b->next == NULL);
  TEST_CHECK(b->prev == NULL);

  // changing the original should not affect the copy
  a->full_string = "changed this";
  a->itemlength = strlen(a->full_string);
  TEST_CHECK(STR_DF(a->full_string, b->full_string));
  TEST_CHECK(a->itemlength != b->itemlength);

  // TODO free memory
}

void test_replace_item(void) {
  struct CompletionItem *a = init_list();
  struct CompletionItem *b = init_list();

  a->full_string = calloc(1, 6);
  mutt_str_copy(a->full_string, "hello", 6);
  a->itemlength = strlen(a->full_string);
  b->full_string = calloc(1, 8);
  mutt_str_copy(b->full_string, "replace", 8);
  b->itemlength = strlen(b->full_string);

  TEST_CHECK(STR_DF(a->full_string, b->full_string));
  TEST_CHECK(a->itemlength != b->itemlength);

  replace_item(a, b);

  TEST_CHECK(STR_EQ(a->full_string, b->full_string));
  TEST_CHECK(a->itemlength == b->itemlength);

  // TODO free memory
}

void test_add_item(void) {
  struct CompletionItem *a = calloc(1, sizeof(struct CompletionItem));
  a->full_string = "itema";
  a->itemlength = strlen(a->full_string);

  struct CompletionItem *b = calloc(1, sizeof(struct CompletionItem));
  b->full_string = "itemb";
  b->itemlength = strlen(b->full_string);

  add_item(a, b);

  // content should have been copied, not just linked
  TEST_CHECK(a->next != b);
  TEST_CHECK(b->prev != a);

  // but the content should be the same
  TEST_CHECK(STR_EQ(a->next->full_string, b->full_string));
  TEST_CHECK(a->next->itemlength == b->itemlength);

  // start and end of the list are properly bound
  TEST_CHECK(a->prev == NULL);
  TEST_CHECK(b->next == NULL);

  // TODO free memory
}

void test_find_first(void) {
  struct CompletionItem *a = calloc(1, sizeof(struct CompletionItem));
  a->full_string = "itema";
  a->itemlength = strlen(a->full_string);

  struct CompletionItem *b = calloc(1, sizeof(struct CompletionItem));
  b->full_string = "itemb";
  b->itemlength = strlen(b->full_string);

  a->next = b;
  b->prev = a;

  TEST_CHECK(STR_EQ(find_first(b)->full_string, a->full_string));
  TEST_CHECK(find_first(b)->itemlength == a->itemlength);

  TEST_CHECK(STR_EQ(find_first(a)->full_string, a->full_string));
  TEST_CHECK(find_first(a)->itemlength == a->itemlength);
}

void test_completion(void) {
  char *items[] = {"hello", "hiho", "ergo", "!wethersuperlongstring"};

  printf("\nInitialising empty list...\n");
  struct CompletionItem *compitems = init_list();
  struct CompletionItem *newitem = init_list();

  printf("Adding items to completion list...\n");
  for (int i = 0; i < 4; i++){
    newitem->full_string = calloc(1, strlen(items[i])+1);
    mutt_str_copy(newitem->full_string, items[i], strlen(items[i])+1);
    newitem->itemlength = strlen(items[i]);
    add_item(compitems, newitem);
    printf(" - %s\n", newitem->full_string);
  }

  char *typed, *result;

  // single matches
  typed = "hel";
  result = items[0];
  printf("Matching typed '%s' (ignore case): '%s'\n", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "hi";
  result = items[1];
  printf("Matching typed '%s' (ignore case): '%s'\n", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "HI";
  result = items[1];
  printf("Matching typed '%s' (ignore case): '%s'\n", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "!";
  result = items[3];
  printf("Matching typed '%s' (ignore case): '%s'\n", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE)->full_string, result));

  typed = "HI";
  printf("Matching typed '%s' (no flags): -\n", typed);
  TEST_CHECK(complete(compitems, typed, strlen(typed), MUTT_COMP_NO_FLAGS) == NULL);

  // several matches -> firstmatch flag
  typed = "h";
  result = items[0];
  printf("Matching typed '%s' (first match): '%s'\n", typed, result);
  TEST_CHECK(STR_EQ(complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE & MUTT_COMP_FIRSTMATCH)->full_string, result));

  typed = "h";
  char *results[] = { items[0], items[1] };
  struct CompletionItem *comp_list = complete(compitems, typed, strlen(typed), MUTT_COMP_IGNORECASE);
  printf("Matching multi '%s' (ignore case):\n", typed);
  int i = 0;
  while (comp_list != NULL) {
    printf("  - '%s'\n", results[i]);
    TEST_CHECK(STR_EQ(comp_list->full_string, results[i]));

    i+=1;
    comp_list = comp_list->next;
  }

  clear_list(comp_list, MUTT_COMP_LIST_BOTH);
  clear_list(compitems, MUTT_COMP_LIST_BOTH);
  if (newitem) free(newitem);
}

TEST_LIST = {
   { "match", test_match },
   {"capital_diff", test_capital_diff},
   {"copy_item", test_copy_item},
   {"replace_item", test_replace_item},
   {"add_item", test_add_item},
   {"find_first", test_find_first},
   {"completion", test_completion},
   { NULL, NULL } };
