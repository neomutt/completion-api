/**
 * @file
 * Autocompletion API Test: statemachine
 *
 * @authors
 * Copyright (C) 2023 Simon V. Reichel <simonreichel@giese-optik.de>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "acutest.h"
#include "private.h"
#include "lib.h"
#include "mutt/array.h"

#define STR_EQ(s1, s2) buf_str_equal(s1, s2)
#define STR_DF(s1, s2) !buf_str_equal(s1, s2)
#define BUF(s1) buf_new(s1)

void state_init(void)
{
}

void state_init_from_array(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");

  struct CompletionStringList list = ARRAY_HEAD_INITIALIZER;
  /* char *a = mutt_mem_calloc(6, sizeof(char)); */
  /* a = "apfel\0"; */
  ARRAY_ADD(&list, "apfel");                                                  \
  ARRAY_ADD(&list, "apple");
  ARRAY_ADD(&list, "apply");
  ARRAY_ADD(&list, "arange");

  Completion *comp = compl_from_array(&list, COMPL_MODE_EXACT);
  compl_type(comp, BUF("ap"));

  struct Buffer *result = NULL;
  printf("First tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("apfel")));

  printf("Second tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("apple")));

  printf("Third tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("apply")));

  printf("Third tab to reset...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("ap")));

  ARRAY_FREE(&list);
  compl_free(comp);
}

void malformed_input(void)
{
  printf("\n");
  setlocale(LC_ALL, "en_US.UTF-8");

  // calling functions with null completions
  TEST_CHECK(compl_add(NULL, BUF("hi")) == 0);
  TEST_CHECK(compl_type(NULL, BUF("hi")) == 0);
  TEST_CHECK(!compl_complete(NULL));

  Completion *comp = compl_new(COMPL_MODE_EXACT);

  // calling with null pointer string
  TEST_CHECK(compl_type(comp, NULL) == 0);
  TEST_CHECK(compl_add(comp, NULL) == 0);

  // calling with empty string
  TEST_CHECK(compl_type(comp, BUF("")) == 0);
  TEST_CHECK(compl_add(comp, BUF("")) == 0);

  compl_free(comp);
}

void state_empty(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  struct Buffer *result = NULL;
  result = compl_complete(comp);

  TEST_CHECK(STR_EQ(result, BUF("")));

  compl_free(comp);
}

void state_nomatch(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, BUF("apfel"));
  compl_add(comp, BUF("apple"));
  compl_add(comp, BUF("apply"));
  compl_add(comp, BUF("arange"));

  compl_type(comp, BUF("bertha"));

  struct Buffer *result = NULL;
  result = compl_complete(comp);

  TEST_CHECK(STR_EQ(result, BUF("bertha")));

  compl_free(comp);
}

void state_single(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, BUF("apple"));
  compl_add(comp, BUF("apply"));
  compl_add(comp, BUF("arange"));
  compl_add(comp, BUF("Äpfel"));

  compl_type(comp, BUF("ar"));

  struct Buffer *result = NULL;
  result = compl_complete(comp);

  TEST_CHECK(STR_EQ(result, BUF("arange")));

  printf("Tabbing again to reset...\n");
  result = compl_complete(comp);

  TEST_CHECK(STR_EQ(result, BUF("ar")));

  compl_free(comp);
}

void state_single_utf8(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);
  comp->flags = COMPL_MATCH_IGNORECASE;

  compl_add(comp, BUF("apfel"));
  compl_add(comp, BUF("apple"));
  compl_add(comp, BUF("Äpfel")); // this has to be bigger, because our umlaut is not just one char

  compl_type(comp, BUF("äp"));

  struct Buffer *result = NULL;
  printf("Tabbing single item (ignoring case)...\n");
  result = compl_complete(comp);
  printf("  äp -> Äpfel : (%s)\n", result->data);

  TEST_CHECK(STR_EQ(result, BUF("Äpfel")));

  printf("Tabbing again to reset...\n");
  result = compl_complete(comp);
  printf("  äp -> äp: (%s)\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("äp")));

  compl_free(comp);
}

void state_multi(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, BUF("apfel"));
  compl_add(comp, BUF("apple"));
  compl_add(comp, BUF("apply"));
  compl_add(comp, BUF("arange"));

  compl_type(comp, BUF("ap"));

  struct Buffer *result = NULL;
  printf("First tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("apfel")));

  printf("Second tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("apple")));

  printf("Third tab...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("apply")));

  printf("Third tab to reset...\n");
  result = compl_complete(comp);
  printf("  ap -> %s\n", result->data);
  TEST_CHECK(STR_EQ(result, BUF("ap")));

  compl_free(comp);
}

void duplicate_add(void)
{
  printf("\n");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  compl_add(comp, BUF("apfel"));
  compl_add(comp, BUF("apple"));

  TEST_CHECK(compl_get_size(comp) == 3);

  compl_add(comp, BUF("apple"));

  TEST_CHECK(compl_get_size(comp) == 3);

  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  compl_add(comp, BUF("Äpfel"));

  TEST_CHECK(compl_get_size(comp) == 4);

  compl_add(comp, BUF("Äpfel"));
  printf("Another duplicate, this time unicode...\n");
  TEST_CHECK(compl_get_size(comp) == 4);

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
