#include <stdlib.h>
#include <stdint.h>
#include "config.h"
#include "mutt/string2.h"

int min(int a, int b, int c);
// TODO add fuzzy match function (could be reused for fuzzy finding in pager etc)
int lev(char *stra, char *strb);
