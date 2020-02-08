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

typedef uint8_t MuttCompletionFlags;
#define MUTT_COMP_NO_FLAGS          0  /// < No flags are set
#define MUTT_COMP_IGNORECASE  (1 << 0) /// < Ignore the case of letters
#define MUTT_COMP_FIRSTMATCH  (1 << 1) /// < Return only the first match
/* #define MUTT_COMP_OPTIONAL    (1 << 2) /// < Allow optional field processing */
/* #define MUTT_COMP_STAT_FILE   (1 << 3) /// < Used by attach_format_str */
/* #define MUTT_COMP_ARROWCURSOR (1 << 4) /// < Reserve space for arrow_cursor */
/* #define MUTT_COMP_INDEX       (1 << 5) /// < This is a main index entry */
/* #define MUTT_COMP_NOFILTER    (1 << 6) /// < Do not allow filtering on this pass */
/* #define MUTT_COMP_PLAIN       (1 << 7) /// < Do not prepend DISP_TO, DISP_CC ... */

#define STR_EQ(s1,s2) strcmp(s1, s2) == 0

struct CompletionItem {
  size_t itemlength;
  char *full_string;
};

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

  // TODO maybe use int strncasecmp(const char *s1, const char *s2, size_t n) from <strings.h>?
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
  struct CompletionItem *matchlist = calloc(items_len, sizeof(struct CompletionItem));
  matchlist[0].full_string = "";

  int n = 0;

  // iterate through possible completions
  for (int i = 0; i < items_len; i++) {
    if (match(typed_string, items[i].full_string, flags)) {
      // return first match only
      if (flags & MUTT_COMP_FIRSTMATCH) {
        return &items[i];
      } else {
        matchlist[n] = items[i];
        n++;
        // TODO add match to matchlist
      }
    }
  }

  // found no match
  if (matchlist[0].full_string[0] == '\0') {
    return NULL;
  } else {
    return &matchlist[0];
  }

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
