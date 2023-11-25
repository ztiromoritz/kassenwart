#include "src/trie.h"
#include <stdio.h>

int main() {
  Trie trie = trie_init();

  trie_add_entry(trie, "hallo", "test");
  trie_add_entry(trie, "hell", "hell found");
  trie_add_entry(trie, "hello", "hello found");
  trie_add_entry(trie, "hand", "fuss");
  trie_add_entry(trie, "baum", "blatt");

  char *tests[7] = {"ha",   "hallo", "hello world", "handlanger",
                    "baum", "bau",   "hand"};

  for (int i = 0; i < 7; i++) {
    void *result;
    int match_length;

    printf("Trie query '%s'\n", tests[i]);
    if (trie_query(trie, tests[i], &match_length, &result)) {
      printf("Found: '%s' ,Length: %d\n", (char *) result, match_length);
    } else {
      printf("Not found\n");
    }

    printf("Trie prefix query '%s'\n", tests[i]);
    if (trie_query_prefix(trie, tests[i], &match_length, &result)) {
      printf("Found: '%s' ,Length: %d\n\n", (char *)result, match_length);
    } else {
      printf("Not found\n\n");
    }
  }

  free_trie(trie);
}
