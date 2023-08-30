/**
 * @file
 * Autocompletion API Test: matching mode
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

#include "acutest.h"
#include "lib.h"
#include "private.h"

#define BUF(s1) buf_new(s1)

void test_match_simple()
{
  Completion *comp = compl_new(COMPL_MODE_EXACT);
  comp->typed_item->buf = BUF("apples");
  struct Buffer *tar = BUF("applers");

  comp->mode = COMPL_MODE_EXACT;
  TEST_CHECK(match_dist(tar, comp) == -1);
  comp->mode = COMPL_MODE_FUZZY;
  TEST_CHECK(match_dist(tar, comp) == 1);

  comp->typed_item->buf = BUF("derived");
  tar = BUF("drvd");

  comp->mode = COMPL_MODE_EXACT;
  TEST_CHECK(match_dist(tar, comp) == -1);
  comp->mode = COMPL_MODE_FUZZY;
  TEST_CHECK(match_dist(tar, comp) == 3);
  comp->typed_item->buf = BUF("drvd");
  comp->mode = COMPL_MODE_EXACT;
  TEST_CHECK(match_dist(tar, comp) == 0);
}

TEST_LIST = {
  { "simple", test_match_simple },
  { NULL, NULL },
};
