#include "src/trie.h"
#include <stdio.h>

int main() {
  Trie trie = trie_init();


  trie_add_entry(trie, "hallo", "test");
  trie_add_entry(trie, "hand", "fuss");
  trie_add_entry(trie, "baum", "blatt");

  char *tests[5] = {"ha", "hallo", "handlanger", "baum", "bau", "hand"};
  for (int i = 0; i < 5; i++) {
     printf("%s -> %s\n", tests[i],(char *)trie_query(trie, tests[i]));

  }

  free_trie(trie);
}
