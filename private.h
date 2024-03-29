/**
 * @file
 * Autocompletion API helper functions
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

/**
 * @page completion neomutt completion API
 *
 * Neomutt completion API
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <strings.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include "mutt/array.h"
#include "mutt/string2.h"
#include "mutt/mbyte.h"
#include "mutt/buffer.h"
#include "mutt_logging.h"
#include "config.h"
#include "lib.h"

#ifndef MAX_TYPED
#define MAX_TYPED 100
#endif

// TODO replace with mutt_error(...), mutt_warning(...), mutt_message(...), mutt_debug(LEVEL, ...)
#ifndef LOGGING
#define logerr(M, ...) printf("ERR: %s%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define logwar(M, ...) printf("WAR: %s%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define loginf(M, ...) printf("INF: %s%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define logdeb(L, M, ...) printf("DBG%d: %s%d: " M "\n",\
    L, __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef WSTR_EQ
// TODO how can we best handle this...?
// could use wcscoll as well (locale aware)
#define WSTR_EQ(s1, s2) wcscmp(s1, s2) == 0

// these are helper functions to check string health etc.
// TODO there must be equivalent mutt functions to handle user input safely
int         compl_health_check(const Completion *comp);
int         compl_str_check(const struct Buffer *str);
int         compl_str_check(const struct Buffer *str);
int         compl_get_size(Completion *comp);
bool        compl_check_duplicate(const Completion *comp, const struct Buffer *buf);
int         compl_compile_regex(Completion *comp);

// the main matching function
int         match_dist(const struct Buffer *tar, const Completion *comp);
#endif

#ifndef ISLONGMBYTE
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
#endif
