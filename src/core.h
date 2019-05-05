#include "trie.h"
#include "sds.h"

typedef struct s_record {
    sds key;
    sds content;
} p_record;

int p_trie_insert(trie *trie, p_record *record);
p_record *trie_search(const trie *trie, sds key);
