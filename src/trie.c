#include <stdio.h>
#include <stdlib.h>

#include "./trie.h"

#define INIT_TRIE                                                              \
  { NULL, 0, NULL }
struct _trie {
  Trie *children;
  char c;
  int next_child;
  void *value;
};

Trie trie_init() {
  Trie trie = malloc(sizeof(struct _trie));
  trie->next_child = 0;
  trie->children = calloc(1, sizeof(Trie));
  trie->c = '\0';
  trie->value = NULL;
  return trie;
}

void free_trie(Trie trie) {
  for (int i = 0; i < trie->next_child; i++) {
    free_trie(trie->children[i]);
  }
  free(trie->children);
  free(trie);
}

void trie_add_entry(Trie trie, char *key, void *value) {
  char c = key[0];
  // printf("x %s\n ", key);
  printf("x %s\n ", key);
  Trie child = NULL;
  for (int i = 0; i < trie->next_child; i++) {
    if (trie->children[i]->c == c) {
      child = trie->children[i];
    }
  }
  if (child == NULL) {
    // error handling how
    realloc(trie->children, sizeof(struct _trie) * (trie->next_child + 1));
    child = trie_init();
    child->c = c;
    trie->children[trie->next_child] = child;
    trie->next_child++;
  }
  printf("y %s \n", key[1]);
  // printf("y %s\n ", key[1]);
  if (key[1] == '\0') {
    child->value = value;
  } else {
    trie_add_entry(child, &(key[1]), value);
  }
  printf("z %s\n ", key);
}

void *trie_query(Trie trie, char *key) {
  printf("hier 1\n");
  char c = key[0];
  printf("hier 1 %c\n", c);
  if (c == '\0') {
    return trie->value;
  }

  Trie child = NULL;
  for (int i = 0; i < trie->next_child; i++) {
    if (trie->children[i]->c == c) {
      child = trie->children[i];
    }
    printf("hier 2\n");
  }

  if (child == NULL)
    return NULL;

  return trie_query(child, &key[1]);
}
