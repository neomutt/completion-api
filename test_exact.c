/**
 * @file
 * Autocompletion API Test: exact matching
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
#include <locale.h>
#include <stdio.h>
#include "mutt/lib.h"
#include "private.h"

#define STR_EQ(s1, s2) strcmp(s1, s2) == 0
#define STR_DF(s1, s2) strcmp(s1, s2) != 0
#define BUF(s1) buf_new(s1)

void test_match(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  // basic match and non-match
  comp->typed_item->buf = BUF("Hello");
  TEST_CHECK(match_dist(BUF("Hello"), comp) == 0);
  TEST_CHECK(match_dist(BUF("Bye"), comp) == -1);
  comp->typed_item->buf = BUF("Übel");
  TEST_CHECK(match_dist(BUF("Übel"), comp) == 0);
  comp->typed_item->buf = BUF("übel");
  TEST_CHECK(match_dist(BUF("Übel"), comp) == -1);

  // match substring only
  comp->typed_item->buf = BUF("Hel");
  TEST_CHECK(match_dist(BUF("Hello"), comp) == 2);

  // match empty string
  comp->typed_item->buf = BUF("");
  TEST_CHECK(match_dist(BUF("HELLO"), comp) == 5);
  TEST_CHECK(match_dist(BUF("neomuttisawesome"), comp) == 16);

  comp->mode = COMPL_MODE_EXACT;
  comp->flags = COMPL_MATCH_IGNORECASE;
  // match case-insensitive
  comp->typed_item->buf = BUF("hel");
  TEST_CHECK(match_dist(BUF("Helloworld"), comp) == 7);
  comp->typed_item->buf = BUF("HEL");
  TEST_CHECK(match_dist(BUF("Helloworld"), comp) == 7);
  comp->typed_item->buf = BUF("übel");
  TEST_CHECK(match_dist(BUF("Übel"), comp) == 0);

  // match case-sensitive ONLY
  comp->mode = COMPL_MODE_EXACT;
  comp->flags = COMPL_MATCH_NOFLAGS;
  comp->typed_item->buf = BUF("hel");
  TEST_CHECK(match_dist(BUF("Helloworld"), comp) == -1);
  comp->typed_item->buf = BUF("HEL");
  TEST_CHECK(match_dist(BUF("Helloworld"), comp) == -1);
  TEST_CHECK(match_dist(BUF("HELloworld"), comp) == 7);
}

void test_exact(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  Completion *comp = compl_new(COMPL_MODE_EXACT);

  // test some regular ASCII strings
  comp->typed_item->buf = BUF("abc");
  TEST_CHECK(match_dist(BUF("Abc"), comp) == -1);
  TEST_CHECK(match_dist(BUF("abc"), comp) == 0);

  comp->mode = COMPL_MODE_EXACT;
  comp->flags = COMPL_MATCH_IGNORECASE;
  comp->typed_item->buf = BUF("abc");
  TEST_CHECK(match_dist(BUF("Abc"), comp) == 0);
  comp->typed_item->buf = BUF("wxy");
  TEST_CHECK(match_dist(BUF("wxy"), comp) == 0);

  // test multibyte comparison
  comp->mode = COMPL_MODE_EXACT;
  comp->flags = COMPL_MATCH_NOFLAGS;
  comp->typed_item->buf = BUF("äpfel");
  TEST_CHECK(match_dist(BUF("Äpfel"), comp) == -1);
  comp->mode = COMPL_MODE_EXACT;
  comp->flags = COMPL_MATCH_IGNORECASE;
  TEST_CHECK(match_dist(BUF("Äpfel"), comp) == 0);
  comp->typed_item->buf = BUF("zabc");
  TEST_CHECK(match_dist(BUF("öxrya"), comp) == -1);

  // test some other symbols
  comp->mode = COMPL_MODE_EXACT;
  comp->flags = COMPL_MATCH_NOFLAGS;
  comp->typed_item->buf = BUF("c");
  TEST_CHECK(match_dist(BUF("Z"), comp) == -1);
  comp->typed_item->buf = BUF(";");
  TEST_CHECK(match_dist(BUF("Z"), comp) == -1);
  comp->typed_item->buf = BUF("a");
  TEST_CHECK(match_dist(BUF("!"), comp) == -1);
  comp->typed_item->buf = BUF("ß");
  TEST_CHECK(match_dist(BUF("ß😀"), comp) == 1);
  comp->typed_item->buf = BUF("世");
  TEST_CHECK(match_dist(BUF("世界"), comp) == 1);

  comp->mode = COMPL_MODE_EXACT;
  comp->flags = COMPL_MATCH_IGNORECASE;
  comp->typed_item->buf = BUF("c");
  TEST_CHECK(match_dist(BUF("Z"), comp) == -1);
  comp->typed_item->buf = BUF(";");
  TEST_CHECK(match_dist(BUF("Z"), comp) == -1);
  comp->typed_item->buf = BUF("a");
  TEST_CHECK(match_dist(BUF("!"), comp) == -1);
  comp->typed_item->buf = BUF("ß");
  TEST_CHECK(match_dist(BUF("ß😀"), comp) == 1);
  comp->typed_item->buf = BUF("世");
  TEST_CHECK(match_dist(BUF("世界"), comp) == 1);
}

TEST_LIST = {
  { "match", test_match },
  { "exact", test_exact },
  { NULL, NULL },
};
