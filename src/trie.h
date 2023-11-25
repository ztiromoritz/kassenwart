/** implementation of a Trie datastructure **/

typedef struct _trie *Trie;

Trie trie_init();

void free_trie(Trie);

/**
 * Params:
 * trie - the datastructure to add an entry to
 * key - a '\0' terminated string
 * value - the value to store under this key
 * Returns:
 *  0 on success
 *  errno of error during memory allocation
 **/
int trie_add_entry(Trie trie, char *key, void *value);

/**
 *
 * Searches the full given search string.
 *
 *
 * Params:
 *   trie - the datastructure to search in
 *   search_string - a '\0' terminated string
 *   search_string - a '\0' terminated string
 *   out_match_length - the length of the found match or -1
 *   result - a pointer to the found value or NULL
 * Return:
 *   0 on match, 1 otherwise
 **/
int trie_query(Trie trie, char *search_string, int*out_match_length, void **result);

/**
 *
 * Searches the shortest prefix in the search string
 * that leads to a non NULL value.
 *
 * Note: If the trie contains values that are not on a leaf, these values will
 * hide result further down the tree, when searched by this function.
 * For example: 
 *  If "hell" and "hello" both a valid keys in the trie.
 *  Then "hell" will match even for the search_string "hello world"
 *
 *
 * Params:
 *   trie - the datastructure to search in
 *   search_string - a '\0' terminated string
 *   out_match_length - the length of the found match or -1
 *   result - a pointer to the found value or NULL
 * Return:
 *   0 on match, 1 otherwise
 **/
int trie_query_prefix(Trie trie, char *search_string, int *out_match_length, void**result);
