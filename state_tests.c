#include "config.h"
#include "acutest.h"
#include "statemach.h"

#define STR_EQ(s1, s2) strcmp(s1, s2) == 0
#define STR_DF(s1, s2) strcmp(s1, s2) != 0

typedef struct Completion Completion;

void state_init(void)
{
}

void malformed_input(void)
{
  printf("\n");
  setlocale(LC_ALL, "en_US.UTF-8");

  // calling functions with null completions
  TEST_CHECK(comp_add(NULL, "hi", 3) == 0);
  TEST_CHECK(comp_type(NULL, "hi", 3) == 0);
  TEST_CHECK(!comp_complete(NULL));

  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  // calling with null pointer string
  TEST_CHECK(comp_type(comp, NULL, 5) == 0);
  TEST_CHECK(comp_add(comp, NULL, 5) == 0);

  // calling with empty string
  TEST_CHECK(comp_type(comp, "", 5) == 0);
  TEST_CHECK(comp_add(comp, "", 5) == 0);

  // calling with small buffer size
  TEST_CHECK(comp_type(comp, "abcd", 1) == 0);
  TEST_CHECK(comp_add(comp, "abcd", 1) == 0);
}

void state_empty(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  char *result = NULL;
  result = comp_complete(comp);

  TEST_CHECK(result == NULL);
}

void state_nomatch(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  comp_add(comp, "apfel", 6);
  comp_add(comp, "apple", 6);
  comp_add(comp, "apply", 6);
  comp_add(comp, "arange", 7);

  comp_type(comp, "bertha", 7);

  char *result = NULL;
  result = comp_complete(comp);

  TEST_CHECK(result == NULL);
}

void state_single(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  comp_add(comp, "apple", 6);
  comp_add(comp, "apply", 6);
  comp_add(comp, "arange", 7);
  comp_add(comp, "Äpfel", 6);

  comp_type(comp, "ar", 3);

  char *result = NULL;
  result = comp_complete(comp);

  TEST_CHECK(STR_EQ(result, "arange"));

  printf("Tabbing again to reset...\n");
  result = comp_complete(comp);
  printf("  ar -> ar: (%s)\n", result);
  TEST_CHECK(STR_EQ(result, "ar"));
}

void state_single_utf8(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = comp_new(MUTT_COMP_IGNORECASE);

  comp_add(comp, "apfel", 6);
  comp_add(comp, "apple", 6);
  comp_add(comp, "Äpfel", 8); // this has to be bigger, because our umlaut is not just one char

  comp_type(comp, "äp", 6);

  char *result = NULL;
  printf("Tabbing single item (ignoring case)...\n");
  result = comp_complete(comp);
  printf("  äp -> Äpfel : (%s)\n", result);

  TEST_CHECK(STR_EQ(result, "Äpfel"));

  printf("Tabbing again to reset...\n");
  result = comp_complete(comp);
  printf("  äp -> äp: (%s)\n", result);
  TEST_CHECK(STR_EQ(result, "äp"));
}

void state_multi(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  comp_add(comp, "apfel", 6);
  comp_add(comp, "apple", 6);
  comp_add(comp, "apply", 6);
  comp_add(comp, "arange", 7);

  comp_type(comp, "ap", 3);

  char *result = NULL;
  printf("First tab...\n");
  result = comp_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apfel"));

  printf("Second tab...\n");
  result = comp_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apple"));

  printf("Third tab...\n");
  result = comp_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "apply"));

  printf("Third tab to reset...\n");
  result = comp_complete(comp);
  printf("  ap -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "ap"));
}

TEST_LIST = {
  { "statemachine initialisation", state_init },
  { "statemachine malformed input", malformed_input },
  { "statemachine empty list", state_empty },
  { "statemachine no match", state_nomatch },
  { "statemachine single match", state_single },
  { "statemachine single match with utf8 result", state_single_utf8 },
  { "statemachine multi match", state_multi },
  { NULL, NULL },
};
