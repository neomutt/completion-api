#include "config.h"
#include "acutest.h"
#include "fuzzy.h"
#include "mutt/mbyte.h"
#include <locale.h>

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

void test_min(void)
{
  TEST_CHECK(min(1, 2, 3) == 1);
  TEST_CHECK(min(2, 1, 3) == 1);
  TEST_CHECK(min(2, 3, 1) == 1);

  TEST_CHECK(min(1, 1, 1) == 1);
  TEST_CHECK(min(1, 1, -1) == -1);
  TEST_CHECK(min(2, 1, 1) == 1);
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

  // null pointers checks
  TEST_CHECK(dist_dam_lev(NULL, "123") == 3);
  TEST_CHECK(dist_dam_lev("123", NULL) == 3);
  TEST_CHECK(dist_dam_lev(NULL, NULL) == 0);
  TEST_CHECK(dist_dam_lev("", "") == 0);
  TEST_CHECK(dist_dam_lev("123", "") == 3);
  TEST_CHECK(dist_dam_lev("", "123") == 3);

  // bad mbytes should always fail
  char *mbyte = "ä";
  TEST_CHECK(dist_dam_lev(&mbyte[1], "abc") == -1);
  TEST_CHECK(dist_dam_lev(mbyte, &mbyte[1]) == -1);
  TEST_CHECK(dist_dam_lev(&mbyte[1], &mbyte[1]) == -1);

  // some made-up tests
  TEST_CHECK(dist_dam_lev("chitin", "chtia") == 2);
  TEST_CHECK(dist_dam_lev("hello", "hell") == 1);
  TEST_CHECK(dist_dam_lev("pete", "ptee") == 1);
  TEST_CHECK(dist_dam_lev("peter", "pteer") == 1);
  TEST_CHECK(dist_dam_lev("pete", "pteer") == 2);
  TEST_CHECK(dist_dam_lev("email", "mail") == 1);

  // one insertion, one transposition
  TEST_CHECK(dist_dam_lev("fltcap", "flatcpa") == 2);

  // mbyte transposition
  TEST_CHECK(dist_dam_lev("päfel", "äpfel") == 1);
  TEST_CHECK(dist_dam_lev("xpäfel", "xäpfel") == 1);
  TEST_CHECK(dist_dam_lev("te", "et") == 1);

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
  { "min", test_min },
  { "levenshtein", test_levenshtein },
  { "damerau levenshtein", test_damerau_levenshtein },
  { NULL, NULL },
};
