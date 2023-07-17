#include "config.h"
#include "acutest.h"
#include <locale.h>
#include "mutt/mbyte.h"
#include "completion.h"
#include "statemach.h"
#include "fuzzy.h"

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
  Completion *comp = compl_new(MUTT_COMPL_NO_FLAGS);

  // null pointers checks
  comp->typed_str = NULL;
  TEST_CHECK(dist_dam_lev("123", comp) == 3);
  TEST_CHECK(dist_dam_lev(NULL, comp) == 0);
  comp->typed_str = "123";
  TEST_CHECK(dist_dam_lev(NULL, comp) == 3);
  TEST_CHECK(dist_dam_lev("", comp) == 3);
  comp->typed_str = "";
  TEST_CHECK(dist_dam_lev("", comp) == 0);
  TEST_CHECK(dist_dam_lev("123", comp) == 3);

  // bad mbytes should always fail
  char *mbyte = "ä";
  comp->typed_str = &mbyte[1];
  TEST_CHECK(dist_dam_lev("abc", comp) == -1);
  TEST_CHECK(dist_dam_lev(&mbyte[1], comp) == -1);
  comp->typed_str = mbyte;
  TEST_CHECK(dist_dam_lev(&mbyte[1], comp) == -1);

  // some made-up tests
  comp->typed_str = "chitin";
  TEST_CHECK(dist_dam_lev("chtia", comp) == 2);
  comp->typed_str = "hello";
  TEST_CHECK(dist_dam_lev("hell", comp) == 1);
  comp->typed_str = "peter";
  TEST_CHECK(dist_dam_lev("pteer", comp) == 1);
  comp->typed_str = "pete";
  TEST_CHECK(dist_dam_lev("ptee", comp) == 1);
  TEST_CHECK(dist_dam_lev("pteer", comp) == 2);
  comp->typed_str = "email";
  TEST_CHECK(dist_dam_lev("mail", comp) == 1);

  // one insertion, one transposition
  comp->typed_str = "fltcap";
  TEST_CHECK(dist_dam_lev("flatcpa", comp) == 2);

  // mbyte transposition
  comp->typed_str = "päfel";
  TEST_CHECK(dist_dam_lev("äpfel", comp) == 1);
  comp->typed_str = "xpäfel";
  TEST_CHECK(dist_dam_lev("xäpfel", comp) == 1);
  comp->typed_str = "te";
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
