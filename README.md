# Specific todos

## completion.c

  - what is NNTP, and how to include it? (l75)
  - IMAP completion (l80)

# Auto-Completion API

Encapsulate all the completion code to make it easier to test and maintain.
Currently, this code is scattered throughout NeoMutt and often uses global
variables.  It needs to be separated and tidied up.

- [ ] Create an Auto-Completion library -- separate the code from the rest of NeoMutt
- [ ] Create an API (list of functions) that everyone uses

This library should be only be dependent on libmutt.

The auto-completion code is given:
- a list of possible symbols to match
- what the user has typed so far

It replies by:
- [x] doing nothing (no matches)
- [x] completing the symbols (one match found)
- [x] returning the list of matching symbols
- [ ] The user can either <kbd>Tab</kbd> through them or select from a menu

# Flatcaps braindump

*Method*: create, populate, query, destroy

## query flags

  - [X] ignore case
  - [ ] fuzzy
  - [ ] magic (dash==underscore)
  - [ ] regex (~=fuzzy?)
  - [ ] partial

## completion priority

  - complete *over* flag

## client settings

  - option to match first, then cycle
  - match longest

## Requirements:

  - self-contained, except for libmutt
  - tests in test-library repo branch

## coding

  - pass list[] { flags, string }
  - need work buffer (matching length of longest string)
  - need cursor position
  - string data passed in is const
  - highlighting of matches for fuzzy/partial
  - flag partial: (anywhere in string); anchor: start, none, end
  - sort variables, etc before auto-completion or properly sort the sources of the strings

### has a static vars (init.c)

	`#define NUMVARS mutt_array_size(MuttVars)`
	`#define NUMCOMMANDS mutt_array_size(Commands)`

	/* Initial string that starts completion. No telling how much the user has
	* typed so far. Allocate 1024 just to be sure! */
	static char UserTyped[1024] = { 0 };

	static int NumMatched = 0;          /* Number of matches for completion */
	static char Completed[256] = { 0 }; /* completed string (command or variable) */
	static const char **Matches;
	/* this is a lie until mutt_init runs: */
	static int MatchesListsize = MAX(NUMVARS, NUMCOMMANDS) + 10;

	#ifdef USE_NOTMUCH
	/* List of tags found in last call to mutt_nm_query_complete(). */
	static char **nm_tags;
	#endif

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
