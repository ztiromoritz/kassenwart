#include "src/trie.h"
#include <stdio.h>




struct _trie_entry {
  char *pattern;
  int type;
  char *name;
};

typedef struct _trie_entry _trie_entry;

static const _trie_entry esc_entries[] = {
    {"[A", 1, "Arrow Up new"},
    {"[B", 2, "Arrow Down new"},
    {"[C", 3, "Arrow Right new"},
    {"[D", 4, "Arrow Left new"}};

void fill_esc_trie(Trie trie) {
  // static struct _trie_value x = {"sdf", , "Hello"};
  /*
  int array_len = sizeof(esc_entries) / sizeof(_trie_entry);
  for (int i = 0; i < array_len; i++) {
    const _trie_entry *esc_entry = &esc_entries[i];
    //printf("pattern %s \r\n", esc_entry->pattern);
    trie_add_entry(trie, esc_entry->pattern, (void *)esc_entry);
  }

  */
  trie_add_entry(trie, "[A", "test1");
  trie_add_entry(trie, "[B", "test2");
  trie_add_entry(trie, "[C", "test3");
  trie_add_entry(trie, "[D", "test4");

}




int main() {
  Trie trie = trie_init();
  // fill_esc_trie(trie);

/*
  trie_add_entry(trie, "[A", "test1");
  trie_add_entry(trie, "[B", "test2");
  trie_add_entry(trie, "[C", "test3");
  trie_add_entry(trie, "[D", "test4");
*/

  trie_add_entry(trie, "XA", "test1");
  trie_add_entry(trie, "XB", "test2");
  trie_add_entry(trie, "XC", "test3");
  // trie_add_entry(trie, "XD", "test4");
  // trie_add_entry(trie, "XE", "test4");
  //trie_add_entry(trie, "XF", "test4");
  //trie_add_entry(trie, "XG", "test4");
  //trie_add_entry(trie, "XH", "test4");

/*
  trie_add_entry(trie, "Hallo", "test");
  trie_add_entry(trie, "hell", "hell found");
  trie_add_entry(trie, "hello", "hello found");
  trie_add_entry(trie, "hand", "fuss");
  trie_add_entry(trie, "baum", "blatt");
*/

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
