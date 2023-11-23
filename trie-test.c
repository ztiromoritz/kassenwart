#include "src/trie.h"
#include <stdio.h>

int main() {
  Trie trie = trie_init();


  trie_add_entry(trie, "hallo", "test");
  printf("\nadd\n");
  trie_add_entry(trie, "hand", "fuss");
  printf("add3\n");
  trie_add_entry(trie, "baum", "blatt");
  printf("add2\n");

  char *tests[5] = {"ha", "hallo", "handlanger", "baum", "bau"};
  for (int i = 0; i < 5; i++) {
	   printf("%s", tests[i]);
    //printf("%s -> %s\n", tests[i],(char *)trie_query(trie, tests[i]));

  }

  free_trie(trie);
}
