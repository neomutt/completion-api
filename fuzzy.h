#include <stdlib.h>
#include <stdint.h>
#include "config.h"
#include "mutt/string2.h"
#include "completion.h"

// TODO might want to move these to mutt/mbyte.h or mutt/string2.h
#define ISLONGMBYTE(mbyte) mblen(mbyte, MB_CUR_MAX) > 1
#define MBCHARLEN(mbyte) mblen(mbyte, MB_CUR_MAX)
#define ISBADMBYTE(mbyte) mblen(mbyte, MB_CUR_MAX) == -1

bool is_mbs(const char *str);
int mbs_char_count(const char *str);
bool mb_equal(const char *stra, const char *strb);

// TODO add fuzzy match function (could be reused for fuzzy finding in pager etc)
int dist_lev(const char *stra, const char *strb);
int dist_dam_lev(const char *tar, const Completion *comp);
