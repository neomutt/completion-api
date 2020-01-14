/**
 * @file
 * Autocompletion API
 *
 * @authors
 * Copyright (C) 2019 Simon V. Reichel <simonreichel@giese-optik.de>
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

/**
 * @page completion neomutt completion API
 *
 * Neomutt completion API
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "acutest.h"

typedef uint8_t MuttCompletionFlags;     /// < Flags for mutt_expando_format(), e.g. #MUTT_FORMAT_FORCESUBJ
#define MUTT_COMP_NO_FLAGS          0  /// < No flags are set
#define MUTT_COMP_IGNORECASE  (1 << 0) /// < Print the subject even if unchanged
/* #define MUTT_COMP_TREE        (1 << 1) /// < Draw the thread tree */
/* #define MUTT_COMP_OPTIONAL    (1 << 2) /// < Allow optional field processing */
/* #define MUTT_COMP_STAT_FILE   (1 << 3) /// < Used by attach_format_str */
/* #define MUTT_COMP_ARROWCURSOR (1 << 4) /// < Reserve space for arrow_cursor */
/* #define MUTT_COMP_INDEX       (1 << 5) /// < This is a main index entry */
/* #define MUTT_COMP_NOFILTER    (1 << 6) /// < Do not allow filtering on this pass */
/* #define MUTT_COMP_PLAIN       (1 << 7) /// < Do not prepend DISP_TO, DISP_CC ... */

struct CompletionItem {
  size_t itemlength;
  char *full_string;
};

/* void add_data(*Completion comp, data) { */
/* } */

/* void free_completion(struct Completion *completion) { */
/*   // TODO clean up underlying list as well? */
/*   free(completion); */
/* } */

/* char[] *get_completion(struct Completion *comp) { */
/* } */


bool capital_diff(char ch1, char ch2) {
  // only alphabetic chars can differ in caps
  if (!(isalpha(ch1) && isalpha(ch2))) {
    return false;
  }

  // both chars capitalized should be the same
  if (toupper(ch1) == toupper(ch2)) {
    return true;
  }

  return false;
}

bool match(char *str1, char *str2, MuttCompletionFlags flags) {
  // longer string can not be matched anyway
  if (strlen(str1) > strlen(str2)) {
    return false;
  }

  // character-wise comparison
  for (int i = 0; i < strlen(str1); ++i) {
    if ((flags & MUTT_COMP_IGNORECASE) && capital_diff(str1[i], str2[i])) {
      continue;
    }

    if (str1[i] != str2[i]) {
      return false;
    }
  }

  return true;
}

struct CompletionItem *complete(struct CompletionItem *items, size_t items_len, char *typed_string, size_t typed_len, MuttCompletionFlags flags) {
  for (int i = 0; i < items_len; i++) {
    if (match(typed_string, items[i].full_string, flags)) {
      return &items[i];
    }
  }

  return NULL;
}

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

/* int main(int argc, char *argv[]) */
/* { */
/*   /1* printf("World! "); *1/ */
/*   char inp[30]; */

/*   fgets(inp, 30, stdin); */

/*   char hi[] = "Hi"; */
/*   printf("%s %s", hi, inp); */

/*   struct Completion * c1 = create_completion(true); */

/*   printf("%s", *(c1->items[0]->full_string)); */

/*   free_completion(c1); */
/*   return 0; */
/* } */

TEST_LIST = {
   { "matching test", test_match },
   {"capital_diff_testing", test_capital_diff},
   { NULL, NULL } };
