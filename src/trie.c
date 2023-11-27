#include <errno.h>
#include <stdbool.h>
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

int trie_add_entry(Trie trie, char *key, void *value) {
  char c = key[0];
  Trie child = NULL;
  for (int i = 0; i < trie->next_child; i++) {
    if (trie->children[i]->c == c) {
      child = trie->children[i];
    }
  }
  if (child == NULL) {
    trie->children = realloc(trie->children, sizeof(Trie) * (trie->next_child + 1));
    if (trie->children == NULL)
      return errno;
    child = trie_init();
    child->c = c;
    trie->children[trie->next_child] = child;
    trie->next_child++;
  }
  if (key[1] == '\0') {
    child->value = value;
  } else {
    trie_add_entry(child, &(key[1]), value);
  }
  return 0;
}

int _trie_query(Trie trie, char *search_string, int *out_match_length,
                void **result, bool stop_on_prefix_match) {

  char c = search_string[0];

  // Check for match on the current level
  if (
      // Found a match even of prefix
      (stop_on_prefix_match && trie->value != NULL)
      // End of search string is reached
      || c == '\0'
      //
  ) {
    (*result) = trie->value;
    if ((*result) == NULL) {
      (*out_match_length) = -1;
      return false;
    }
    return true;
  }

  // Try to go on step down
  Trie child = NULL;
  for (int i = 0; i < trie->next_child; i++) {
    if (trie->children[i]->c == c) {
      child = trie->children[i];
    }
  }
  if (child == NULL) {
    (*result) = NULL;
    (*out_match_length) = -1;
    return false;
  }

  // Go one step down
  (*out_match_length)++;
  return _trie_query(child, &search_string[1], out_match_length, result,
                    stop_on_prefix_match);
}

int trie_query_prefix(Trie trie, char *search_string, int *out_match_length,
                       void **result) {
  (*out_match_length) = 0;
  (*result) = NULL;
  return _trie_query(trie, search_string, out_match_length, result, true);
}

int trie_query(Trie trie, char *search_string, int*out_match_length, void **result) {
  (*out_match_length) = 0;
  (*result) = NULL;
  return _trie_query(trie, search_string, out_match_length, result, false);
}
