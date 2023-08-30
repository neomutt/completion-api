/**
 * @file
 * Autocompletion API Test: fuzzy matching
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
#include "mutt/mbyte.h"
#include "lib.h"
#include "private.h"

#define BUF(s1) buf_new(s1)

void test_mbs_char_count(void)
{
  // TODO what about locale unset cases?
  TEST_CHECK(mbs_char_count("apple") == 5);
  TEST_CHECK(mbs_char_count("peter") == 5);

  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");

  TEST_CHECK(mbs_char_count("apple") == 5);

  // check some malformed cases
  TEST_CHECK(mbs_char_count("") == 0);
  TEST_CHECK(mbs_char_count(NULL) == 0);
  char *mbyte = "ä";
  TEST_CHECK(mbs_char_count(&mbyte[1]) == -1);

  TEST_CHECK(mbs_char_count("€") == 1);

  TEST_CHECK(mbs_char_count("Äpfel im Baum kosten £") == 22);

  TEST_CHECK(mbs_char_count("chitin") == 6);
}

void test_mb_equal(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");

  TEST_CHECK(!mb_equal(NULL, "a"));
  TEST_CHECK(!mb_equal("", "a"));

  // wrong mbytes should always fail
  char *mbyte = "ä";
  TEST_CHECK(!mb_equal(&mbyte[1], &mbyte[1]));
  TEST_CHECK(!mb_equal(&mbyte[0], &mbyte[1]));

  TEST_CHECK(mb_equal("ä", "ä"));
  TEST_CHECK(!mb_equal("€", "u"));

  TEST_CHECK(mb_equal("chitin", "chtia"));
}

void test_levenshtein(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");

  // null pointers checks
  TEST_CHECK(dist_lev(NULL, "123") == 3);
  TEST_CHECK(dist_lev("123", NULL) == 3);
  TEST_CHECK(dist_lev(NULL, NULL) == 0);
  TEST_CHECK(dist_lev("", "") == 0);
  TEST_CHECK(dist_lev("123", "") == 3);
  TEST_CHECK(dist_lev("", "123") == 3);

  // bad mbytes should always fail
  char *mbyte = "ä";
  TEST_CHECK(dist_lev(&mbyte[1], "abc") == -1);
  TEST_CHECK(dist_lev(mbyte, &mbyte[1]) == -1);
  TEST_CHECK(dist_lev(&mbyte[1], &mbyte[1]) == -1);

  // some made-up test cases
  TEST_CHECK(dist_lev("chitin", "chtia") == 2);
  TEST_CHECK(dist_lev("hello", "hell") == 1);
  TEST_CHECK(dist_lev("pete", "pteer") == 3);
  TEST_CHECK(dist_lev("email", "mail") == 1);
  TEST_CHECK(dist_lev("email", "mail") == 1);

  // multibyte tests, mbyte symbols should be considered a single change
  TEST_CHECK(dist_lev("Äpfel", "äpfel") == 1);
  TEST_CHECK(dist_lev("Äpfel", "pfel") == 1);
  TEST_CHECK(dist_lev("pÄfel", "päfel") == 1);
}

void test_damerau_levenshtein(void)
{
  // we need to set the locale settings, otherwise UTF8 chars won't work as expected
  setlocale(LC_ALL, "en_US.UTF-8");
  Completion *comp = compl_new(COMPL_MODE_FUZZY);

  // null pointers checks
  comp->typed_item->buf = NULL;
  TEST_CHECK(dist_dam_lev("123", comp) == 3);
  TEST_CHECK(dist_dam_lev(NULL, comp) == 0);
  comp->typed_item->buf = BUF("123");
  TEST_CHECK(dist_dam_lev(NULL, comp) == 3);
  TEST_CHECK(dist_dam_lev("", comp) == 3);
  comp->typed_item->buf = BUF("");
  TEST_CHECK(dist_dam_lev("", comp) == 0);
  TEST_CHECK(dist_dam_lev("123", comp) == 3);

  // bad mbytes should always fail
  char *mbyte = "ä";
  comp->typed_item->buf = BUF(&mbyte[1]);
  TEST_CHECK(dist_dam_lev("abc", comp) == -1);
  TEST_CHECK(dist_dam_lev(&mbyte[1], comp) == -1);
  comp->typed_item->buf = BUF(mbyte);
  TEST_CHECK(dist_dam_lev(&mbyte[1], comp) == -1);

  // some made-up tests
  comp->typed_item->buf = BUF("chitin");
  TEST_CHECK(dist_dam_lev("chtia", comp) == 2);
  comp->typed_item->buf = BUF("hello");
  TEST_CHECK(dist_dam_lev("hell", comp) == 1);
  comp->typed_item->buf = BUF("peter");
  TEST_CHECK(dist_dam_lev("pteer", comp) == 1);
  comp->typed_item->buf = BUF("pete");
  TEST_CHECK(dist_dam_lev("ptee", comp) == 1);
  TEST_CHECK(dist_dam_lev("pteer", comp) == 2);
  comp->typed_item->buf = BUF("email");
  TEST_CHECK(dist_dam_lev("mail", comp) == 1);

  // one insertion, one transposition
  comp->typed_item->buf = BUF("fltcap");
  TEST_CHECK(dist_dam_lev("flatcpa", comp) == 2);

  // mbyte transposition
  comp->typed_item->buf = BUF("päfel");
  TEST_CHECK(dist_dam_lev("äpfel", comp) == 1);
  comp->typed_item->buf = BUF("xpäfel");
  TEST_CHECK(dist_dam_lev("xäpfel", comp) == 1);
  comp->typed_item->buf = BUF("te");
  TEST_CHECK(dist_dam_lev("et", comp) == 1);

  // mbyte substitution
  TEST_CHECK(dist_lev("Äpfel", "Apfel") == 1);

  // mbyte deletion
  TEST_CHECK(dist_lev("Äpfel", "pfel") == 1);

  // mbyte insertion
  TEST_CHECK(dist_lev("Äpfel", "ÄÄpfel") == 1);
}

TEST_LIST = {
  { "mbs_char_count", test_mbs_char_count },
  { "mb_equal", test_mb_equal },
  { "levenshtein", test_levenshtein },
  { "damerau levenshtein", test_damerau_levenshtein },
  { NULL, NULL },
};
