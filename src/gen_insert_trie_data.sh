#!/bin/bash

for i in {1..50}
do
    echo "(void) trie_insert(t, (const char *) \"key$i\", (void *) \"data$i\");"
done
