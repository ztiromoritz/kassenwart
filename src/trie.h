/** implementation of a Trie datastructure **/

typedef struct _trie *Trie;

Trie trie_init();

void free_trie(Trie);

/**
 * Params:
 * trie - the datastructure to add an entry to
 * key - a '\n' terminated string
 * value - the value to store under this key
 **/
void trie_add_entry(Trie trie, char * key, void * value);

/**
 *
 **/
void *trie_query(Trie, char *);
