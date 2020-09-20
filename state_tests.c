#include "config.h"
#include "acutest.h"
#include "completion.h"
#include "statemach.h"

#define STR_EQ(s1, s2) strcmp(s1, s2) == 0
#define STR_DF(s1, s2) strcmp(s1, s2) != 0

typedef struct Completion Completion;

void state_init(void)
{
}

void state_empty(void)
{
  printf("\n");
  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  char *result = NULL;
  printf("Tabbing with empty list...\n");
  result = comp_complete(comp);

  printf("  ap -> %s\n", result);
  TEST_CHECK(result == NULL);
}

void state_single(void)
{
  printf("\n");
  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  comp_add(comp, "apfel", 6);
  comp_add(comp, "apple", 6);
  comp_add(comp, "apply", 6);
  comp_add(comp, "arange", 7);

  comp_type(comp, "ar", 3);

  char *result = NULL;
  printf("Tabbing single item...\n");
  result = comp_complete(comp);
  printf("  ar -> %s\n", result);

  TEST_CHECK(STR_EQ(result, "arange"));

  printf("Tabbing again to reset...\n");
  result = comp_complete(comp);
  printf("  ar -> %s\n", result);
  TEST_CHECK(STR_EQ(result, "ar"));
}

void state_multi(void)
{
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
  { "statemachine empty list", state_empty},
  { "statemachine single match", state_single },
  { "statemachine multi match", state_multi },
  { NULL, NULL },
};
