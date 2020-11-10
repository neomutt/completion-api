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

#include "config.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "completion.h"

bool capital_diff(wchar_t ch1, wchar_t ch2)
{
  // only alphabetic chars can differ in caps
  if (!(iswalpha(ch1) && iswalpha(ch2)))
  {
    return false;
  }

  // both chars capitalized should be the same
  if (towupper(ch1) == towupper(ch2))
  {
    return true;
  }

  return false;
}

bool match(const wchar_t *str1, const wchar_t *str2, MuttCompletionFlags flags)
{
  // longer string can not be matched anyway
  if (wcslen(str1) > wcslen(str2))
  {
    return false;
  }

  // TODO maybe use int strncasecmp(const char *s1, const char *s2, size_t n) from <strings.h>?
  // character-wise comparison
  for (int i = 0; i < wcslen(str1); ++i)
  {
    if ((flags & MUTT_COMP_IGNORECASE) && capital_diff(str1[i], str2[i]))
    {
      continue;
    }

    if (str1[i] != str2[i])
    {
      return false;
    }
  }

  return true;
}
