# Specific todos

## completion.c

  - what is NNTP, and how to include it? (l75)
  - IMAP completion (l80)

# Auto-Completion API

Encapsulate all the completion code to make it easier to test and maintain.
Currently, this code is scattered throughout NeoMutt and often uses global
variables.  It needs to be separated and tidied up.

- [x] Create an Auto-Completion library -- separate the code from the rest of NeoMutt
- [x] Create an API (list of functions) that everyone uses

This library should be only be dependent on libmutt.

The auto-completion code is given:
- a list of possible symbols to match
- what the user has typed so far

It replies by:
- [x] doing nothing (no matches)
- [x] completing the symbols (one match found)
- [ ] returning the list of matching symbols
- [ ] The user can either <kbd>Tab</kbd> through them or select from a menu

## Fuzzy completion/API

- [x] consider swapping of characters on top of deletion/insertion/substitution (damerau-levenshtein)

# Flatcaps braindump

*Method*: create, populate, query, destroy

## query flags

  - [X] ignore case
  - [x] fuzzy
  - [x] regex (~=fuzzy?)
  - [ ] magic (dash==underscore)
  - [ ] partial

## completion priority

  - [x] prioritise completion *over* flags (run flag consistent first, then with less restrictions)
    - I think it is better to not complete if there are no matches with the current flags
    - it would be confusing to the user, when "flatcap@" suddenly turns into "FLATCAP@" even though they enabled the case-insensitive search...
    - in case of fuzzy matching, it will still find the other matches (but they will be found at the bottom of the match list)
    - in case of exact matching, we would not want any other match to invade our search (unexpected behaviour)
    - just by toggling the "showall" flag, even the non-matches will be available, so widening the search is possible

## client settings

  - [x] option to match first, or cycle
  - [ ] match longest

## Requirements:

  - [x] self-contained, except for libmutt
  - [ ] tests in test-library repo branch

## coding

  - [ ] pass list[] { flags, string }
  - [ ] need work buffer (matching length of longest string)
  - [ ] need cursor position
  - [ ] string data passed in is const
  - [ ] highlighting of matches for fuzzy/partial
  - [ ] flag partial: (anywhere in string); anchor: start, none, end
  - [ ] sort variables, etc before auto-completion or properly sort the sources of the strings

## multi-field matching?
  e.g. Index
  pass [] { flags, str1, str2, etc }
  flags control which are matched?  flags ∀ strings?
  C.add_column(flags, [], num)
    have [] of flags for matches
    doesn't add data, just flags behaviour
  C.add_row() probably better
    for data

  need set of matcher callback functions
    pass flags, row
