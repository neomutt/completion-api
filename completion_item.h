#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t MuttCompletionWipeFlags;
#define MUTT_COMP_LIST_NO_FLAGS   0         /// < No flags are set
#define MUTT_COMP_LIST_POSITIVE   (1 << 0)  /// < Wipe list in positive direction (next->next)
#define MUTT_COMP_LIST_NEGATIVE   (1 << 1)  /// < Wipe list in negative direction (prev->prev)
#define MUTT_COMP_LIST_BOTH       (1 << 2)  /// < Wipe list in both directions

struct CompletionItem {
  size_t itemlength;
  char *full_string;
  struct CompletionItem *next;
  struct CompletionItem *prev;
};

#ifndef MUTT_COMPLETION_ITEM_H
#define MUTT_COMPLETION_ITEM_H

struct CompletionItem *find_first(struct CompletionItem *from);
struct CompletionItem *init_list();
struct CompletionItem *copy_item(struct CompletionItem *from);
void replace_item(struct CompletionItem *base, struct CompletionItem *with);
struct CompletionItem *add_item(struct CompletionItem *base, struct CompletionItem *item);
bool is_empty(struct CompletionItem *base);
void clear_list(struct CompletionItem *from, MuttCompletionWipeFlags flags);

#endif
