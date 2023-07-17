#include "config.h"
#include "acutest.h"
#include "statemach.h"
#include "completion.h"
#include "mutt/array.h"

#define STR_EQ(s1, s2) strcmp(s1, s2) == 0
#define STR_DF(s1, s2) strcmp(s1, s2) != 0

void state_init(void)
{
}

void state_init_from_array(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");

  struct StringList list = ARRAY_HEAD_INITIALIZER;
  /* char *a = mutt_mem_calloc(6, sizeof(char)); */
  /* a = "apfel\0"; */
  ARRAY_ADD(&list, "apfel");                                                  \
  ARRAY_ADD(&list, "apple");
  ARRAY_ADD(&list, "apply");
  ARRAY_ADD(&list, "arange");

  Completion *comp = compl_from_array(&list, COMPL_MODE_EXACT);
  compl_type(comp, "ap", 3);

  char *result = NULL;
  printf("First tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apfel"));

  printf("Second tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apple"));

  printf("Third tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apply"));

  printf("Third tab to reset...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "ap"));

  ARRAY_FREE(&list);
  compl_free(comp);
}

void malformed_input(void)
{
  printf("\n");
  setlocale(LC_ALL, "en_US.UTF-8");

  // calling functions with null completions
  TEST_CHECK(compl_add(NULL, "hi", 3) == 0);
  TEST_CHECK(compl_type(NULL, "hi", 3) == 0);
  TEST_CHECK(!compl_complete(NULL));

  Completion *comp = compl_new(COMPL_MODE_EXACT);

  // calling with null pointer string
  TEST_CHECK(compl_type(comp, NULL, 5) == 0);
  TEST_CHECK(compl_add(comp, NULL, 5) == 0);

  // calling with empty string
  TEST_CHECK(compl_type(comp, "", 5) == 0);
  TEST_CHECK(compl_add(comp, "", 5) == 0);

  // calling with small buffer size
  TEST_CHECK(compl_type(comp, "abcd", 1) == 0);
  TEST_CHECK(compl_add(comp, "abcd", 1) == 0);

  compl_free(comp);
}

void state_empty(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  char *result = NULL;
  result = compl_complete(comp);

  TEST_CHECK(result == NULL);

  compl_free(comp);
}

void state_nomatch(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, "apfel", 6);
  compl_add(comp, "apple", 6);
  compl_add(comp, "apply", 6);
  compl_add(comp, "arange", 7);

  compl_type(comp, "bertha", 7);

  char *result = NULL;
  result = compl_complete(comp);

  TEST_CHECK(result == NULL);

  compl_free(comp);
}

void state_single(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, "apple", 6);
  compl_add(comp, "apply", 6);
  compl_add(comp, "arange", 7);
  compl_add(comp, "Äpfel", 6);

  compl_type(comp, "ar", 3);

  char *result = NULL;
  result = compl_complete(comp);

  TEST_CHECK(STR_EQ(result, "arange"));

  printf("Tabbing again to reset...\n");
  result = compl_complete(comp);
  printf("  ar -> ar: (%s)\n", result);
  TEST_CHECK(STR_EQ(result, "ar"));

  compl_free(comp);
}

void state_single_utf8(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);
  comp->flags = MUTT_MATCH_IGNORECASE;

  compl_add(comp, "apfel", 6);
  compl_add(comp, "apple", 6);
  compl_add(comp, "Äpfel", 8); // this has to be bigger, because our umlaut is not just one char

  compl_type(comp, "äp", 6);

  char *result = NULL;
  printf("Tabbing single item (ignoring case)...\n");
  result = compl_complete(comp);
  printf("  äp -> Äpfel : (%s)\n", result);

  TEST_CHECK(STR_EQ(result, "Äpfel"));

  printf("Tabbing again to reset...\n");
  result = compl_complete(comp);
  printf("  äp -> äp: (%s)\n", result);
  TEST_CHECK(STR_EQ(result, "äp"));

  compl_free(comp);
}

void state_multi(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, "apfel", 6);
  compl_add(comp, "apple", 6);
  compl_add(comp, "apply", 6);
  compl_add(comp, "arange", 7);

  compl_type(comp, "ap", 3);

  char *result = NULL;
  printf("First tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apfel"));

  printf("Second tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apple"));

  printf("Third tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apply"));

  printf("Third tab to reset...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "ap"));

  compl_free(comp);
}

void duplicate_add(void)
{
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, "apfel", 6);
  compl_add(comp, "apple", 6);

  TEST_CHECK(compl_get_size(comp) == 2);

  compl_add(comp, "apple", 6);

  printf("We added a duplicate...\n");
  TEST_CHECK(compl_get_size(comp) == 2);

  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  compl_add(comp, "Äpfel", 6);

  TEST_CHECK(compl_get_size(comp) == 3);

  compl_add(comp, "Äpfel", 6);
  printf("Another duplicate, this time unicode...\n");
  TEST_CHECK(compl_get_size(comp) == 3);

  compl_free(comp);
}

TEST_LIST = {
  { "statemachine initialisation", state_init },
  { "statemachine initialisation from array", state_init_from_array },
  { "statemachine malformed input", malformed_input },
  { "statemachine empty list", state_empty },
  { "statemachine no match", state_nomatch },
  { "statemachine single match", state_single },
  { "statemachine single match with utf8 result", state_single_utf8 },
  { "statemachine multi match", state_multi },
  { "statemachine add duplicate", duplicate_add },
  { NULL, NULL },
};
