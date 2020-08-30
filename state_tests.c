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

void state_single(void)
{
  Completion *comp = comp_new(MUTT_COMP_NO_FLAGS);

  comp_add(comp, "apfel", 6);
  comp_add(comp, "apple", 6);
  comp_add(comp, "apply", 6);
  comp_add(comp, "arange", 7);

  comp_type(comp, "ar", 3);

  char *result = NULL;
  printf("Tabbing single item...\n");
  result = comp_complete(comp);

  TEST_CHECK(STR_EQ(result, "arange"));

  // printf("Tabbing again...\n");
  // result = comp_complete(comp);
  // TEST_CHECK(STR_EQ(result, "ar"));
}

TEST_LIST = {
  { "statemachine initialisation", state_init },
  { "statemachine single match", state_single },
  { NULL, NULL },
};
