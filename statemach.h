/**
 * @file
 * Autocompletion API
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
#include "mutt_logging.h"
#include "config.h"
#include "completion.h"
#include "fuzzy.h"

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

Completion *compl_new(enum MuttMatchMode mode);
Completion *compl_from_array(const struct CompletionStringList *list, enum MuttMatchMode mode);
void compl_free(Completion *comp);

int compl_compile_regex(Completion *comp);

// TODO handle strings with dynamic size (keep track of longest string)
int compl_add(Completion *comp, const char *str, const size_t buf_len);
int compl_type(Completion *comp, const char *str, const size_t buf_len);

// this is the main interface function for users to collect/cycle the next matched string
char *compl_complete(Completion *comp);

// these are helper functions to check string health etc.
// TODO there must be equivalent mutt functions to handle user input safely
int compl_health_check(const Completion *comp);
int compl_str_check(const char *str, const size_t buf_len);
int compl_str_check(const char *str, const size_t buf_len);
int compl_get_size(Completion *comp);
bool compl_check_duplicate(const Completion *comp, const char *str, const size_t buf_len);

// the main matching function
int match_dist(const char *tar, const Completion *comp);
#endif
